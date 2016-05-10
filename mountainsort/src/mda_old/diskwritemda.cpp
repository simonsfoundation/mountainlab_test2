#include "diskwritemda.h"
#include "mdaio.h"

#include <QFile>
#include <QString>
#include "mda.h"
#include <QDebug>

class DiskWriteMdaPrivate
{
public:
	DiskWriteMda *q;
	QString m_path;
	MDAIO_HEADER m_header;
	FILE *m_file;

	int determine_ndims(long N1, long N2, long N3, long N4, long N5, long N6);
};

DiskWriteMda::DiskWriteMda() {
	d=new DiskWriteMdaPrivate;
	d->q=this;
	d->m_file=0;
}

DiskWriteMda::DiskWriteMda(int data_type,const QString &path, long N1, long N2, long N3, long N4, long N5, long N6)
{
	d=new DiskWriteMdaPrivate;
	d->q=this;
	d->m_file=0;
	this->open(data_type,path,N1,N2,N3,N4,N5,N6);
}

DiskWriteMda::~DiskWriteMda() {
	close();
	delete d;
}

bool DiskWriteMda::open(int data_type, const QString &path, long N1, long N2, long N3, long N4, long N5, long N6)
{
	if (d->m_file) return false; //can't open twice!

	if (QFile::exists(path)) {
		if (!QFile::remove(path)) {
			qWarning() << "Unable to remove file in diskwritemda::open" << path;
			return false;
		}
	}

	d->m_path=path;

	d->m_header.data_type=data_type;
	for (int i=0; i<MDAIO_MAX_DIMS; i++) d->m_header.dims[i]=1;
	d->m_header.dims[0]=N1;
	d->m_header.dims[1]=N2;
	d->m_header.dims[2]=N3;
	d->m_header.dims[3]=N4;
	d->m_header.dims[4]=N5;
	d->m_header.dims[5]=N6;
	d->m_header.num_dims=d->determine_ndims(N1,N2,N3,N4,N5,N6);

	d->m_file=fopen((path+".tmp").toLatin1().data(),"wb");

	if (!d->m_file) return false;

	long NN=N1*N2*N3*N4*N5*N6;
    //long buf_size=1e6;

	//write the header
	mda_write_header(&d->m_header,d->m_file);

    /*
	//fill it all with zeros!
	float *zeros=(float *)malloc(sizeof(float)*buf_size);
	for (int i=0; i<buf_size; i++) zeros[i]=0;
	long i=0;
	while (i<NN) {
		long num_to_write=NN-i;
		if (num_to_write>buf_size) num_to_write=buf_size;
		mda_write_float32(zeros,&d->m_header,num_to_write,d->m_file);
		i+=buf_size;
	}
	free(zeros);
    */
    fseek(d->m_file,d->m_header.header_size+d->m_header.num_bytes_per_entry*NN-1,SEEK_SET);
    unsigned char zero=0;
    fwrite(&zero,1,1,d->m_file);

	return true;
}

void DiskWriteMda::close()
{
	if (d->m_file) {
		fclose(d->m_file);
		if (!QFile::rename(d->m_path+".tmp",d->m_path)) {
			qWarning() << "Unable to rename file in diskwritemda::open" << d->m_path+".tmp" << d->m_path;
		}
		d->m_file=0;
	}
}

long DiskWriteMda::N1()
{
	if (!d->m_file) return 0;
	return d->m_header.dims[0];
}

long DiskWriteMda::N2()
{
	if (!d->m_file) return 0;
	return d->m_header.dims[1];
}

long DiskWriteMda::N3()
{
	if (!d->m_file) return 0;
	return d->m_header.dims[2];
}

long DiskWriteMda::N4()
{
	if (!d->m_file) return 0;
	return d->m_header.dims[3];
}

long DiskWriteMda::N5()
{
	if (!d->m_file) return 0;
	return d->m_header.dims[4];
}

long DiskWriteMda::N6()
{
	if (!d->m_file) return 0;
	return d->m_header.dims[5];
}

long DiskWriteMda::totalSize()
{
	return N1()*N2()*N3()*N4()*N5()*N6();
}

void DiskWriteMda::writeChunk(Mda &X, long i)
{
	if (!d->m_file) return;
	fseek(d->m_file,d->m_header.header_size+d->m_header.num_bytes_per_entry*i,SEEK_SET);
	long size=X.totalSize();
	if (i+size>this->totalSize()) size=this->totalSize()-i;
	if (size>0) {
		mda_write_float64(X.dataPtr(),&d->m_header,size,d->m_file);
	}
}

void DiskWriteMda::writeChunk(Mda &X, long i1, long i2)
{
    if ((X.N1()==N1())&&(i1==0)) {
        writeChunk(X,i1+this->N1()*i2);
	}
	else {
        qWarning() << "This case not yet supported in 2d writeSubArray" << X.N1() << X.N2() << N1() << N2() << i1 << i2;
	}
}

void DiskWriteMda::writeChunk(Mda &X, long i1, long i2, long i3)
{
    if ((X.N1()==N1())&&(X.N2()==N2())&&(i1==0)&&(i2==0)) {
        writeChunk(X,i1+this->N1()*i2+this->N1()*this->N2()*i3);
	}
	else {
        qWarning() << "This case not yet supported in 3d writeSubArray" << X.N1() << X.N2() << X.N3() << N1() << N2() << N3() << i1 << i2 << i3;
	}
}

int DiskWriteMdaPrivate::determine_ndims(long N1, long N2, long N3, long N4, long N5, long N6)
{
	#ifdef QT_CORE_LIB
	Q_UNUSED(N1)
	Q_UNUSED(N2)
	#endif
	if (N6>1) return 6;
	if (N5>1) return 5;
	if (N4>1) return 4;
	if (N3>1) return 3;
	return 2;
}
