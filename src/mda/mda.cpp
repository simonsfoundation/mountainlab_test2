#include "mda.h"
#include "mdaio.h"
#include <stdio.h>

#define MDA_MAX_DIMS 6

class MdaPrivate
{
public:
	Mda *q;
	double *m_data;
	long m_dims[MDA_MAX_DIMS];
	long m_total_size;

	void do_construct();
	void copy_from(const Mda &other);
	int determine_num_dims(long N1,long N2,long N3,long N4,long N5,long N6);
	bool safe_index(long i);
	bool safe_index(long i1,long i2);
	bool safe_index(long i1,long i2,long i3);
	bool safe_index(long i1,long i2,long i3,long i4,long i5,long i6);
};

Mda::Mda() {
	d=new MdaPrivate;
	d->q=this;
	d->do_construct();
}

Mda::Mda(const Mda &other)
{
	d=new MdaPrivate;
	d->q=this;
	d->do_construct();
	d->copy_from(other);
}

Mda::~Mda() {
	if (d->m_data) free(d->m_data);
	delete d;
}

bool Mda::allocate(long N1, long N2, long N3, long N4, long N5, long N6)
{
	d->m_dims[0]=N1;
	d->m_dims[1]=N2;
	d->m_dims[2]=N3;
	d->m_dims[3]=N4;
	d->m_dims[4]=N5;
	d->m_dims[5]=N6;
	d->m_total_size=N1*N2*N3*N4*N5*N6;

	if (d->m_data) free(d->m_data); d->m_data=0;
	if (d->m_total_size>0) {
		d->m_data=(double *)malloc(sizeof(double)*d->m_total_size);
		for (long i=0; i<d->m_total_size; i++) d->m_data[i]=0;
	}

	return true;

}

bool Mda::read(const QString &path)
{
	return read(path.toLatin1().data());
}

bool Mda::write32(const QString &path) const
{
	return write32(path.toLatin1().data());
}

bool Mda::write64(const QString &path) const
{
	return write64(path.toLatin1().data());
}

bool Mda::read(const char *path)
{
	FILE *input_file=fopen(path,"rb");
	if (!input_file) {
		printf("Warning: Unable to open mda file for reading: %s\n",path);
		return false;
	}
	MDAIO_HEADER H;
	mda_read_header(&H,input_file);
	this->allocate(H.dims[0],H.dims[1],H.dims[2],H.dims[3],H.dims[4],H.dims[5]);
	mda_read_float64(d->m_data,&H,d->m_total_size,input_file);
	fclose(input_file);
	return true;
}

bool Mda::write32(const char *path) const
{
	FILE *output_file=fopen(path,"wb");
	if (!output_file) {
		printf("Warning: Unable to open mda file for writing: %s\n",path);
		return false;
	}
	MDAIO_HEADER H;
	H.data_type=MDAIO_TYPE_FLOAT32;
	H.num_bytes_per_entry=4;
	for (int i=0; i<MDAIO_MAX_DIMS; i++) H.dims[i]=1;
	for (int i=0; i<MDA_MAX_DIMS; i++) H.dims[i]=d->m_dims[i];
	H.num_dims=d->determine_num_dims(N1(),N2(),N3(),N4(),N5(),N6());
	mda_write_header(&H,output_file);
	mda_write_float64(d->m_data,&H,d->m_total_size,output_file);
	fclose(output_file);
	return true;
}

bool Mda::write64(const char *path) const
{
	FILE *output_file=fopen(path,"wb");
	if (!output_file) {
		printf("Warning: Unable to open mda file for writing: %s\n",path);
		return false;
	}
	MDAIO_HEADER H;
	H.data_type=MDAIO_TYPE_FLOAT64;
	H.num_bytes_per_entry=4;
	for (int i=0; i<MDAIO_MAX_DIMS; i++) H.dims[i]=1;
	for (int i=0; i<MDA_MAX_DIMS; i++) H.dims[i]=d->m_dims[i];
	H.num_dims=d->determine_num_dims(N1(),N2(),N3(),N4(),N5(),N6());
	mda_write_header(&H,output_file);
	mda_write_float64(d->m_data,&H,d->m_total_size,output_file);
	fclose(output_file);
	return true;
}

int Mda::ndims() const
{
	return d->determine_num_dims(N1(),N2(),N3(),N4(),N5(),N6());
}

long Mda::N1() const
{
	return d->m_dims[0];
}

long Mda::N2() const
{
	return d->m_data[1];
}

long Mda::N3() const
{
	return d->m_data[2];
}

long Mda::N4() const
{
	return d->m_data[3];
}

long Mda::N5() const
{
	return d->m_data[4];
}

long Mda::N6() const
{
	return d->m_data[5];
}

double Mda::get(long i) const
{
	return d->m_data[i];
}

double Mda::get(long i1, long i2) const
{
	return d->m_data[i1+d->m_dims[0]*i2];
}

double Mda::get(long i1, long i2, long i3) const
{
	return d->m_data[i1+d->m_dims[0]*i2+d->m_dims[0]*d->m_dims[1]*i3];
}

double Mda::get(long i1, long i2, long i3, long i4, long i5, long i6) const
{
	return d->m_data[
			i1+
			d->m_dims[0]*i2+
			d->m_dims[0]*d->m_dims[1]*i3+
			d->m_dims[0]*d->m_dims[1]*d->m_dims[2]*i4+
			d->m_dims[0]*d->m_dims[1]*d->m_dims[2]*d->m_dims[3]*i5+
			d->m_dims[0]*d->m_dims[1]*d->m_dims[2]*d->m_dims[3]*d->m_dims[4]*i6
			];
}

double Mda::value(long i) const
{
	if (!d->safe_index(i)) return 0;
	return get(i);
}

double Mda::value(long i1, long i2) const
{
	if (!d->safe_index(i1,i2)) return 0;
	return get(i1,i2);
}

double Mda::value(long i1, long i2, long i3) const
{
	if (!d->safe_index(i1,i2,i3)) return 0;
	return get(i1,i2,i3);
}

double Mda::value(long i1, long i2, long i3, long i4, long i5, long i6) const
{
	if (!d->safe_index(i1,i2,i3,i4,i5,i6)) return 0;
	return get(i1,i2,i3,i4,i5,i6);
}

void Mda::setValue(double val,long i)
{
	if (!d->safe_index(i)) return;
	set(val,i);
}

void Mda::setValue(double val,long i1, long i2)
{
	if (!d->safe_index(i1,i2)) return;
	set(val,i1,i2);
}

void Mda::setValue(double val,long i1, long i2, long i3)
{
	if (!d->safe_index(i1,i2,i3)) return;
	set(val,i1,i2,i3);
}

void Mda::setValue(double val,long i1, long i2, long i3, long i4, long i5, long i6)
{
	if (!d->safe_index(i1,i2,i3,i4,i5,i6)) return;
	set(val,i1,i2,i3,i4,i5,i6);
}

double *Mda::dataPtr()
{
	return d->m_data;
}

double *Mda::dataPtr(long i)
{
	return &d->m_data[i];
}

double *Mda::dataPtr(long i1, long i2)
{
	return &d->m_data[i1+N1()*i2];
}

double *Mda::dataPtr(long i1, long i2, long i3)
{
	return &d->m_data[i1+N1()*i2+N1()*N2()*i3];
}

double *Mda::dataPtr(long i1, long i2, long i3, long i4, long i5, long i6)
{
	return &d->m_data[
			i1+
			N1()*i2+
			N1()*N2()*i3+
			N1()*N2()*N3()*i4+
			N1()*N2()*N3()*N4()*i5+
			N1()*N2()*N3()*N4()*N5()*i6
			];
}

void Mda::set(double val, long i)
{
	d->m_data[i]=val;
}

void Mda::set(double val, long i1, long i2)
{
	d->m_data[i1+d->m_dims[0]*i2]=val;
}

void Mda::set(double val, long i1, long i2, long i3)
{
	d->m_data[i1+d->m_dims[0]*i2+d->m_dims[0]*d->m_dims[1]*i3]=val;
}

void Mda::set(double val, long i1, long i2, long i3, long i4, long i5, long i6)
{
	d->m_data[
			i1+
			d->m_dims[0]*i2+
			d->m_dims[0]*d->m_dims[1]*i3+
			d->m_dims[0]*d->m_dims[1]*d->m_dims[2]*i4+
			d->m_dims[0]*d->m_dims[1]*d->m_dims[2]*d->m_dims[3]*i5+
			d->m_dims[0]*d->m_dims[1]*d->m_dims[2]*d->m_dims[3]*d->m_dims[4]*i6
			]
	=val;
}

void MdaPrivate::do_construct() {
	m_data=(double *)malloc(sizeof(double)*1);
	for (int i=0; i<MDA_MAX_DIMS; i++) {
		m_dims[i]=1;
	}
	m_total_size=1;
}

void MdaPrivate::copy_from(const Mda &other)
{
	if (m_data) free(m_data); m_data=0;
	m_total_size=other.d->m_total_size;
	for (int i=0; i<MDA_MAX_DIMS; i++) m_dims[i]=other.d->m_dims[i];
	if (m_total_size>0) {
		m_data=(double *)malloc(sizeof(double)*m_total_size);
		memcpy(m_data,other.d->m_data,sizeof(double)*m_total_size);
	}
}

int MdaPrivate::determine_num_dims(long N1, long N2, long N3, long N4, long N5, long N6)
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

bool MdaPrivate::safe_index(long i)
{
	return ((0<=i)&&(i<m_total_size));
}

bool MdaPrivate::safe_index(long i1, long i2)
{
	return ((0<=i1)&&(i1<m_dims[0])&&(0<=i2)&&(i2<m_dims[1]));
}

bool MdaPrivate::safe_index(long i1, long i2, long i3)
{
	return ((0<=i1)&&(i1<m_dims[0])&&(0<=i2)&&(i2<m_dims[1])&&(0<=i3)&&(i3<m_dims[2]));
}

bool MdaPrivate::safe_index(long i1, long i2, long i3, long i4, long i5, long i6)
{
	return (
		(0<=i1)&&(i1<m_dims[0])&&
		(0<=i2)&&(i2<m_dims[1])&&
		(0<=i3)&&(i3<m_dims[2])&&
		(0<=i4)&&(i4<m_dims[3])&&
		(0<=i5)&&(i5<m_dims[4])&&
		(0<=i6)&&(i6<m_dims[5])
	);
}
