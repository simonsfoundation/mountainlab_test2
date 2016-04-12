#include "extractclips.h"
#include <QStringList>
#include <math.h>
#include <QDebug>
#include "mdaio.h"
#include <QSet>

typedef QSet<int> IntSet;

int extractclips(FILE *infile,FILE *infile_TL,FILE *outfile,FILE *outfile_TL,FILE *outfile_TM,const QMap<QString,QVariant> &params) {
	int clipsize=params["clipsize"].toInt();
	bool fixed_clipsize=params.contains("fixed-clipsize");
	if (fixed_clipsize) printf("Using fixed clip size.\n");
	QStringList labels_str;

	bool combine_with_or=false;
	if (params["labels"].toString().contains("|")) {
			combine_with_or=true;
			labels_str=params["labels"].toString().split("|");
	}
	else {
		labels_str=params["labels"].toString().split(",");
	}

	QList<int> labels;
	bool all_labels=false;
	for (int ii=0; ii<labels_str.count(); ii++) {
		if (labels_str[ii]=="all") all_labels=true;
		else {
			int label=labels_str[ii].toInt();
			if (label<=0) {
				qCritical() << "invalid labels" << params["labels"].toStringList();
				return 0;
			}
			labels << label;
		}
	}
	int label1=labels.value(0);

	if (combine_with_or) all_labels=true;

	QMap<int,int> target_label_counts;
	for (int jj=0; jj<labels.count(); jj++) {
		int l0=labels[jj];
		if (!target_label_counts.contains(l0)) target_label_counts[l0]=0;
		target_label_counts[l0]++;
	}

	//load the time labels
	QMap<int,IntSet> time_labels; //for each label, a set of times
	IntSet all_event_times;
	struct MDAIO_HEADER HH_TL;
	if (!mda_read_header(&HH_TL,infile_TL)) return 0;
	if (HH_TL.dims[0]!=2) {
		printf("Unexpected dimensions for TL\n");
		return 0;
	}
	int32_t N2=HH_TL.dims[1];
	if (N2<=0) return 0;
	qint32 *buffer_TL=(qint32 *)malloc(sizeof(qint32)*N2*2);
	mda_read_int32(buffer_TL,&HH_TL,N2*2,infile_TL);
	for (long i=0; i<N2; i++) {
		int t0=buffer_TL[i*2];
		int l0=buffer_TL[i*2+1];
		if (!time_labels.contains(l0)) time_labels[l0]=QSet<int>();
		time_labels[l0].insert(t0);
		if (all_labels) {
			if (!combine_with_or) {
				all_event_times.insert(t0);
			}
			else {
				if (target_label_counts[l0]) all_event_times.insert(t0);
			}
		}
	}
	QList<int> time_labels_keys=time_labels.keys();
	free(buffer_TL);

	printf("...\n");
	if (all_labels) {
		printf("Finding all labels\n");
	}
	else if (combine_with_or) {
		printf("Combining with or\n");
		foreach (int ll,time_labels_keys) {
			if (target_label_counts[ll]) {
				printf("Label %d: %d times\n",ll,time_labels[ll].count());
			}
		}
	}
	else {
		foreach (int ll,time_labels_keys) {
			printf("Label %d: %d times, target counts = %d\n",ll,time_labels[ll].count(),target_label_counts[ll]);
		}
		printf("Label1 = %d\n",label1);
	}

	//find the critical times
	int offset1=-clipsize/2;
	int offset2=offset1+clipsize-1;
	QSet<int> critical_times;
	QSet<int> TL1;
	if (all_labels) TL1=all_event_times;
	else TL1=time_labels[label1];
	foreach (int t,TL1) {
		QMap<int,int> counts;
		bool okay=true;
		if (!all_labels) {
			foreach (int ll,time_labels_keys) counts[ll]=0;
			int t1=t+offset1;
			int t2=t+offset2;
			for (int tt=t1; tt<=t2; tt++) {
				foreach (int ll,time_labels_keys) if (time_labels[ll].contains(tt)) counts[ll]++;
			}

			foreach (int ll,time_labels_keys) {
				if (target_label_counts.value(ll,0)>counts.value(ll,0)) okay=false;
			}
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
		if (critical_times.contains(t)) {
			int t1=t+offset1;
			int t2=t+offset2;

			if (!fixed_clipsize) {
				//expand the interval
				bool done=false;
				while (!done) {
					done=true;
					int t1b=t1,t2b=t2;
					for (int tt=t1b; tt<=t2b; tt++) {
						if ((t!=tt)&&(critical_times.contains(tt))) {
							critical_times.remove(tt);
							t1=qMin(t1,tt+offset1);
							t2=qMax(t2,tt+offset2);
							done=false;
						}
					}
				}
			}

			clip_t1 << t1;
			clip_t2 << t2;
		}
	}

	printf("Number of clips: %d\n",clip_t1.count());

	//get the input file header
	MDAIO_HEADER HH_infile;
	if (!mda_read_header(&HH_infile,infile)) return 0;
	int32_t M=HH_infile.dims[0];
	int32_t N=HH_infile.dims[1];
	if (M<=0) return 0;
	if (N<=0) return 0;

	if (!fixed_clipsize) {
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
		MDAIO_HEADER HH_outfile;
		mda_copy_header(&HH_outfile,&HH_infile);
		HH_outfile.dims[1]=N3;
		mda_write_header(&HH_outfile,outfile);
		MDAIO_HEADER HH_outfile_TM;
		mda_copy_header(&HH_outfile_TM,&HH_infile);
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
				//mda_write_float32(buffer_padding,&HH_outfile,M*padding,outfile);
				//mda_write_int32(buffer_padding_TM,&HH_outfile_TM,padding,outfile_TM);
				//n3+=padding;
			//}
			fseek(infile,HH_infile.header_size+HH_infile.num_bytes_per_entry*M*t1,SEEK_SET);
			mda_read_float32(buffer_in,&HH_infile,M*(t2-t1+1),infile);
			mda_write_float32(buffer_in,&HH_outfile,M*(t2-t1+1),outfile);
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
		//write the output
		MDAIO_HEADER HH_outfile;
		mda_copy_header(&HH_outfile,&HH_infile);
		HH_outfile.dims[0]=HH_infile.dims[0];
		HH_outfile.dims[1]=clipsize;
		HH_outfile.dims[2]=clip_t1.count();
		HH_outfile.num_dims=3;
		mda_write_header(&HH_outfile,outfile);
		MDAIO_HEADER HH_outfile_TM;
		mda_copy_header(&HH_outfile_TM,&HH_infile);
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
			fseek(infile,HH_infile.header_size+HH_infile.num_bytes_per_entry*M*t1,SEEK_SET);
			mda_read_float32(buffer_in,&HH_infile,M*(t2-t1+1),infile);
			mda_write_float32(buffer_in,&HH_outfile,M*(t2-t1+1),outfile);
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
