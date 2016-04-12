#include "extractclips2.h"
#include <QStringList>
#include <math.h>
#include <QDebug>
#include "mdaio.h"
#include <QSet>

typedef QSet<int> IntSet;

int extractclips2(
		FILE *infile1,FILE *infile2,
		FILE *infile_TL1,FILE *infile_TL2,
		FILE *outfile1,FILE *outfile2,
		FILE *outfile_TL,FILE *outfile_TM,
		const QMap<QString,QVariant> &params)
{
	int clipsize=params["clipsize"].toInt();
	QStringList labels_str=params["labels"].toStringList();
	QList<int> labels;
	for (int ii=0; ii<labels_str.count(); ii++) {
		int label=labels_str[ii].toInt();
		if (label<0) {
			qCritical() << "invalid labels" << params["labels"].toStringList();
			return 0;
		}
		labels << label;
	}
	if (labels.count()!=2) {
		qCritical() << "invalid labels" << params["labels"].toStringList();
		return 0;
	}
	int label1=labels.value(0);
	int label2=labels.value(1);

	//load the time labels
	QSet<int> label1_times,label2_times;

	{
		struct MDAIO_HEADER HH_TL1;
		if (!mda_read_header(&HH_TL1,infile_TL1)) return 0;
		if (HH_TL1.dims[0]!=2) {
			printf("Unexpected dimensions for TL1\n");
			return 0;
		}
		int32_t N2=HH_TL1.dims[1];
		if (N2<=0) return 0;
		qint32 *buffer_TL1=(qint32 *)malloc(sizeof(qint32)*N2*2);
		mda_read_int32(buffer_TL1,&HH_TL1,N2*2,infile_TL1);
		for (long i=0; i<N2; i++) {
			int t0=buffer_TL1[i*2];
			int l0=buffer_TL1[i*2+1];
			if (l0==label1) label1_times.insert(t0);
		}
		free(buffer_TL1);
	}
	{
		struct MDAIO_HEADER HH_TL2;
		if (!mda_read_header(&HH_TL2,infile_TL2)) return 0;
		if (HH_TL2.dims[0]!=2) {
			printf("Unexpected dimensions for TL1\n");
			return 0;
		}
		int32_t N2=HH_TL2.dims[1];
		if (N2<=0) return 0;
		qint32 *buffer_TL2=(qint32 *)malloc(sizeof(qint32)*N2*2);
		mda_read_int32(buffer_TL2,&HH_TL2,N2*2,infile_TL2);
		for (long i=0; i<N2; i++) {
			int t0=buffer_TL2[i*2];
			int l0=buffer_TL2[i*2+1];
			if (l0==label2) label2_times.insert(t0);
		}
		free(buffer_TL2);
	}

	printf("...\n");

	//find the critical times
	int offset1=-clipsize/2;
	int offset2=offset1+clipsize-1;
	QSet<int> critical_times;
	foreach (int t,label1_times) {
		bool okay=false;
		for (int dt=-2; dt<=2; dt++) {
			if (label2_times.contains(t+dt)) okay=true;
		}
		if (okay) critical_times.insert(t);
	}

	printf("Found %d critical times\n",critical_times.count());

	//find the clip intervals
	QList<int> clip_t1,clip_t2;
	QList<int> critical_times_list=critical_times.toList();
	qSort(critical_times_list);
	for (int iii=0; iii<critical_times_list.count(); iii++) {
		int t=critical_times_list[iii];
		int t1=t+offset1;
		int t2=t+offset2;
		clip_t1 << t1;
		clip_t2 << t2;
	}

	printf("Number of clips: %d\n",clip_t1.count());

	//get the input file header
	MDAIO_HEADER HH_infile1;
	if (!mda_read_header(&HH_infile1,infile1)) return 0;
	int32_t M=HH_infile1.dims[0];
	int32_t N=HH_infile1.dims[1];
	if (M<=0) return 0;
	if (N<=0) return 0;

	MDAIO_HEADER HH_infile2;
	if (!mda_read_header(&HH_infile2,infile2)) return 0;

	//write the output

	bool output_3d=false; //change this for future

	if (!output_3d) {
		//find out how many timepoints we need for output
		//int padding=clipsize/2;
		int N3=0;
		int max_clipsize=0;
		for (int iii=0; iii<clip_t1.count(); iii++) {
			int t1=qMax(0,clip_t1[iii]);
			int t2=qMin(N-1,clip_t2[iii]);
			//if (iii>0) N3+=padding;
			N3+=t2-t1+1;
			max_clipsize=qMax(max_clipsize,t2-t1+1);
		}

		//write the output
		MDAIO_HEADER HH_outfile1;
		mda_copy_header(&HH_outfile1,&HH_infile1);
		HH_outfile1.dims[1]=N3;
		mda_write_header(&HH_outfile1,outfile1);

		MDAIO_HEADER HH_outfile2;
		mda_copy_header(&HH_outfile2,&HH_infile2);
		HH_outfile2.dims[1]=N3;
		mda_write_header(&HH_outfile2,outfile2);

		MDAIO_HEADER HH_outfile_TM;
		mda_copy_header(&HH_outfile_TM,&HH_infile1);
		HH_outfile_TM.dims[0]=1;
		HH_outfile_TM.dims[1]=N3;
		HH_outfile_TM.num_dims=2;
		HH_outfile_TM.data_type=MDAIO_TYPE_INT32;
		mda_write_header(&HH_outfile_TM,outfile_TM);

		float *buffer_in=(float *)malloc(sizeof(float)*max_clipsize*M);
		qint32 *buffer0=(qint32 *)malloc(sizeof(qint32)*max_clipsize);
		//float *buffer_padding=(float *)malloc(sizeof(float)*padding*M);
		//for (int kkk=0; kkk<padding*M; kkk++) buffer_padding[kkk]=0;
		//qint32 *buffer_padding_TM=(qint32 *)malloc(sizeof(qint32)*padding);
		//for (int kkk=0; kkk<padding; kkk++) buffer_padding_TM[kkk]=-1;
		int n3=0;
		for (int iii=0; iii<clip_t1.count(); iii++) {
			int t1=qMax(0,clip_t1[iii]);
			int t2=qMin(N-1,clip_t2[iii]);
			//if (iii>0) {
				//mda_write_float32(buffer_padding,&HH_outfile1,M*padding,outfile1);
				//mda_write_float32(buffer_padding,&HH_outfile2,M*padding,outfile2);
				//mda_write_int32(buffer_padding_TM,&HH_outfile_TM,padding,outfile_TM);
				//n3+=padding;
			//}
			fseek(infile1,HH_infile1.header_size+HH_infile1.num_bytes_per_entry*M*t1,SEEK_SET);
			mda_read_float32(buffer_in,&HH_infile1,M*(t2-t1+1),infile1);
			mda_write_float32(buffer_in,&HH_outfile1,M*(t2-t1+1),outfile1);
			fseek(infile2,HH_infile2.header_size+HH_infile2.num_bytes_per_entry*M*t1,SEEK_SET);
			mda_read_float32(buffer_in,&HH_infile2,M*(t2-t1+1),infile2);
			mda_write_float32(buffer_in,&HH_outfile2,M*(t2-t1+1),outfile2);
			for (int tt=t1; tt<=t2; tt++) {
				buffer0[tt-t1]=tt;
			}
			mda_write_int32(buffer0,&HH_outfile_TM,(t2-t1+1),outfile_TM);
			n3+=t2-t1+1;
		}
		free(buffer_in);
		free(buffer0);
		//free(buffer_padding);
		//free(buffer_padding_TM);
	}
	else {
		MDAIO_HEADER HH_outfile1;
		mda_copy_header(&HH_outfile1,&HH_infile1);
		HH_outfile1.dims[0]=HH_infile1.dims[0];
		HH_outfile1.dims[1]=clipsize;
		HH_outfile1.dims[2]=clip_t1.count();
		HH_outfile1.num_dims=3;
		mda_write_header(&HH_outfile1,outfile1);

		MDAIO_HEADER HH_outfile2;
		mda_copy_header(&HH_outfile2,&HH_infile2);
		HH_outfile2.dims[0]=HH_infile2.dims[0];
		HH_outfile2.dims[1]=clipsize;
		HH_outfile2.dims[2]=clip_t1.count();
		HH_outfile2.num_dims=3;
		mda_write_header(&HH_outfile2,outfile2);

		MDAIO_HEADER HH_outfile_TM;
		mda_copy_header(&HH_outfile_TM,&HH_infile1);
		HH_outfile_TM.dims[0]=1;
		HH_outfile_TM.dims[1]=clipsize;
		HH_outfile_TM.dims[2]=clip_t1.count();
		HH_outfile_TM.num_dims=3;
		HH_outfile_TM.data_type=MDAIO_TYPE_INT32;
		mda_write_header(&HH_outfile_TM,outfile_TM);

		float *buffer_in=(float *)malloc(sizeof(float)*clipsize*M);
		qint32 *buffer0=(qint32 *)malloc(sizeof(qint32)*clipsize);
		int n3=0;
		for (int iii=0; iii<clip_t1.count(); iii++) {
			if (iii%100==0) printf("Writing clip %d of %d\n",iii,clip_t1.count());
			int t1=qMax(0,clip_t1[iii]);
			int t2=qMin(N-1,clip_t2[iii]);
			fseek(infile1,HH_infile1.header_size+HH_infile1.num_bytes_per_entry*M*t1,SEEK_SET);
			mda_read_float32(buffer_in,&HH_infile1,M*(t2-t1+1),infile1);
			mda_write_float32(buffer_in,&HH_outfile1,M*(t2-t1+1),outfile1);
			fseek(infile2,HH_infile2.header_size+HH_infile2.num_bytes_per_entry*M*t1,SEEK_SET);
			mda_read_float32(buffer_in,&HH_infile2,M*(t2-t1+1),infile2);
			mda_write_float32(buffer_in,&HH_outfile2,M*(t2-t1+1),outfile2);
			for (int tt=t1; tt<=t2; tt++) {
				buffer0[tt-t1]=tt;
			}
			mda_write_int32(buffer0,&HH_outfile_TM,(t2-t1+1),outfile_TM);
			n3+=t2-t1+1;
		}
		free(buffer_in);
		free(buffer0);
	}

	return 1;
}
