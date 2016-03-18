#include "mda.h"

#include <stdlib.h>
#include <stdio.h>
#define UNUSED(expr) do { (void)(expr); } while (0);
#ifdef QT_CORE_LIB
#include <QDebug>
#include "usagetracking.h"
#else
#define jmalloc malloc
#define jfree free
#define jfread fread
#define jfopen fopen
#define jfclose fclose
#endif

class MdaPrivate {
public:
	Mda *q;

    long *m_size;
	double *m_data_real;
	
	void construct() {
        m_size=(long *)jmalloc(sizeof(long)*MDA_MAX_DIMS);
        for (long i=0; i<MDA_MAX_DIMS; i++) m_size[i]=1;
		
        m_data_real=(double *)jmalloc(sizeof(double)*1);
		m_data_real[0]=0;
	}
	
	bool do_read(FILE *inf);
    int read_int(FILE *inf);
	float read_float(FILE *inf);
    double read_double(FILE *inf);
	short read_short(FILE *inf);	
	unsigned short read_unsigned_short(FILE *inf);
	unsigned char read_unsigned_char(FILE *inf);
    bool do_write(FILE *outf,long data_type);
	void write_int(FILE *outf,int val);
	void write_float(FILE *outf,float val);
    void write_double(FILE *outf,double val);
	void write_short(FILE *outf,short val);	
	void write_unsigned_short(FILE *outf,unsigned short val);
	void write_unsigned_char(FILE *outf,unsigned char val);
};

Mda::Mda() 
{
	d=new MdaPrivate;
	d->q=this;
	
	d->construct();
}

Mda::Mda(const Mda &X) {
	d=new MdaPrivate;
	d->q=this;
	
	d->construct();
	
    //setDataType(X.dataType());
	allocate(X.dimCount(),X.d->m_size);
	setValues(X.d->m_data_real);
}

Mda::~Mda()
{
	jfree(d->m_size);
	jfree(d->m_data_real);
	delete d;
}

void Mda::operator=(const Mda &X) {
    //setDataType(X.dataType());
	allocate(X.dimCount(),X.d->m_size);
	setValues(X.d->m_data_real);
}

void Mda::allocate(long N1,long N2,long N3,long N4,long N5,long N6) {
    long tmp[6];
	tmp[0]=N1; tmp[1]=N2; tmp[2]=N3; tmp[3]=N4; tmp[4]=N5; tmp[5]=N6;
	allocate(6,tmp);
}
void Mda::allocate(long num_dims,int *size) {
    for (long i=0; i<MDA_MAX_DIMS; i++) d->m_size[i]=1;
	
    long NN=1;
    for (long i = 0; i < num_dims; i++) {
		if (size[i] <= 0) {
			size[i] = 1;
		}
		d->m_size[i] = size[i];
		NN *= size[i];
	}
	if (NN >MDA_MAX_SIZE) {
        printf ("Unable to allocate mda. Size is too large: %ld.\n", NN);
		allocate(1, 1);
		return;
	}
	if (NN > 0) {
		jfree(d->m_data_real);
		d->m_data_real = (double *)jmalloc(sizeof(double)*NN);
        for (long i=0; i<NN; i++) d->m_data_real[i]=0;
	}
}
void Mda::allocate(long num_dims,long *size) {
    for (long i=0; i<MDA_MAX_DIMS; i++) d->m_size[i]=1;

    long NN=1;
    for (long i = 0; i < num_dims; i++) {
        if (size[i] <= 0) {
            size[i] = 1;
        }
        d->m_size[i] = size[i];
        NN *= size[i];
    }
    if (NN >MDA_MAX_SIZE) {
        printf ("Unable to allocate mda. Size is too large: %ld.\n", NN);
        allocate(1, 1);
        return;
    }
    if (NN > 0) {
        jfree(d->m_data_real);
        d->m_data_real = (double *)jmalloc(sizeof(double)*NN);
        for (long i=0; i<NN; i++) d->m_data_real[i]=0;
    }
}
long Mda::size(long dim) const {
	if (dim >= MDA_MAX_DIMS) {
		return 1;
	}
	if (dim < 0) {
		return 0;
	}
	return d->m_size[dim];
}
long Mda::N1() const {
	return size(0);
}
long Mda::N2() const {
	return size(1);
}
long Mda::N3() const {
	return size(2);
}
long Mda::N4() const {
	return size(3);
}
long Mda::N5() const {
	return size(4);
}
long Mda::N6() const {
	return size(5);
}
long Mda::dimCount() const {
    long ret = 2;
    for (long i = 2; i < MDA_MAX_DIMS; i++) {
		if (d->m_size[i] > 1) {
			ret = i + 1;
		}
	}
	return ret;
}
long Mda::totalSize() const {
    long ret = 1;
    for (long j = 0; j < MDA_MAX_DIMS; j++) {
		ret *= d->m_size[j];
	}
	return ret;
}

void Mda::reshape(long N1, long N2, long N3, long N4, long N5, long N6)
{
	if (N1*N2*N3*N4*N5*N6!=totalSize()) {
        printf("WARNING: Unable to reshape. Inconsistent size. %ld,%ld,%ld,%ld,%ld,%ld %ld",N1,N2,N3,N4,N5,N6,totalSize());
		return;
	}
	d->m_size[0]=N1;
	d->m_size[1]=N2;
	d->m_size[2]=N3;
	d->m_size[3]=N4;
	d->m_size[4]=N5;
	d->m_size[5]=N6;
    for (long j=6; j<MDA_MAX_DIMS; j++) d->m_size[j]=1;
}
double Mda::value1(long i) const {
	return d->m_data_real[i];
}

double Mda::value(long i1, long i2) const
{
	if ((i1<0)||(i1>=size(0))) return 0;
	if ((i2<0)||(i2>=size(1))) return 0;
	return d->m_data_real[i1+size(0)*i2];
}
double Mda::value(long i1,long i2,long i3,long i4) const {
	if ((i1<0)||(i1>=size(0))) return 0;
	if ((i2<0)||(i2>=size(1))) return 0;
	if ((i3<0)||(i3>=size(2))) return 0;
	if ((i4<0)||(i4>=size(3))) return 0;
	return d->m_data_real[i1+size(0)*i2+size(0)*size(1)*i3+size(0)*size(1)*size(2)*i4];
}
double Mda::value(long num_dims,int *ind) const {
    long tmp=0;
    long factor=1;
    for (long i=0; i<num_dims; i++) {
		if ((ind[i]<0)||(ind[i]>=size(i))) return 0;
		tmp+=ind[i]*factor;
		factor*=size(i);
	}
	return d->m_data_real[tmp];
}
double Mda::value(long num_dims,long *ind) const {
    long tmp=0;
    long factor=1;
    for (long i=0; i<num_dims; i++) {
        if ((ind[i]<0)||(ind[i]>=size(i))) return 0;
        tmp+=ind[i]*factor;
        factor*=size(i);
    }
    return d->m_data_real[tmp];
}
void Mda::setValue1(double val,long i) {
	d->m_data_real[i]=val;
}
void Mda::setValue(double val,long i1,long i2,long i3,long i4) {
	if ((i1<0)||(i1>=size(0))) return;
	if ((i2<0)||(i2>=size(1))) return;
	if ((i3<0)||(i3>=size(2))) return;
	if ((i4<0)||(i4>=size(3))) return;
	d->m_data_real[i1+size(0)*i2+size(0)*size(1)*i3+size(0)*size(1)*size(2)*i4]=val;
}
void Mda::setValue(double val,long num_dims,int *ind) {
    long tmp=0;
    long factor=1;
    for (long i=0; i<num_dims; i++) {
		if ((ind[i]<0)||(ind[i]>=size(i))) return;
		tmp+=ind[i]*factor;
		factor*=size(i);
	}
	d->m_data_real[tmp]=val;
}
void Mda::setValue(double val,long num_dims,long *ind) {
    long tmp=0;
    long factor=1;
    for (long i=0; i<num_dims; i++) {
        if ((ind[i]<0)||(ind[i]>=size(i))) return;
        tmp+=ind[i]*factor;
        factor*=size(i);
    }
    d->m_data_real[tmp]=val;
}
void Mda::setValues(double *vals) {
    long ts=totalSize();
    for (long i=0; i<ts; i++) {
		d->m_data_real[i]=vals[i];
	}
}
void Mda::setValues(int *vals) {
    long ts=totalSize();
    for (long i=0; i<ts; i++) {
        d->m_data_real[i]=(double)vals[i];
	}
}
void Mda::setValues(short *vals) {
    long ts=totalSize();
    for (long i=0; i<ts; i++) {
        d->m_data_real[i]=(double)vals[i];
	}
}
void Mda::setValues(unsigned char *vals) {
    long ts=totalSize();
    for (long i=0; i<ts; i++) {
        d->m_data_real[i]=(double)vals[i];
	}
}

Mda Mda::getDataXY(long num_inds,int *inds) const {
	Mda ret;
	ret.allocate(N1(), N2());
    long *inds0=(long *)jmalloc(sizeof(long)*(num_inds+2));
    for (long j = 0; j < num_inds; j++) {
		inds0[j + 2] = inds[j];
	}
    long n1=size(0);
    long n2=size(1);
    for (long y = 0; y < n2; y++) {
		inds0[1] = y;
        for (long x = 0; x < n1; x++) {
			inds0[0] = x;
			ret.setValue(value((num_inds+2),inds0), x, y);
		}
	}
	jfree(inds0);
	return ret;
}
Mda Mda::getDataXZ(long num_inds,int *inds) const {
	Mda ret;
	ret.allocate(N1(), N3());
    long *inds0=(long *)jmalloc(sizeof(long)*(num_inds+2));
	inds0[1]=inds[0];
    for (long j = 1; j < num_inds; j++) {
		inds0[j + 2] = inds[j];
	}
    long n1=size(0);
    long n2=size(2);
    for (long z = 0; z < n2; z++) {
		inds0[2] = z;
        for (long x = 0; x < n1; x++) {
			inds0[0] = x;
			ret.setValue(value((num_inds+2),inds0), x, z);
		}
	}
	jfree(inds0);
	return ret;
}
Mda Mda::getDataYZ(long num_inds,int *inds) const {
	Mda ret;
	ret.allocate(N2(), N3());
    long *inds0=(long *)jmalloc(sizeof(long)*(num_inds+2));
	inds0[0]=inds[0];
    for (long j = 1; j < num_inds; j++) {
		inds0[j + 2] = inds[j];
	}
    long n1=size(1);
    long n2=size(2);
    for (long z = 0; z < n2; z++) {
		inds0[2] = z;
        for (long y = 0; y < n1; y++) {
			inds0[1] = y;
			ret.setValue(value((num_inds+2),inds0), y, z);
		}
	}
	jfree(inds0);
	return ret;
}
Mda Mda::transpose() const {
	Mda ret;
    long *size=(long *)jmalloc(sizeof(long)*MDA_MAX_DIMS);
    for (long i=0; i<MDA_MAX_DIMS; i++) size[i]=d->m_size[i];
    long s1=size[0];
    long s2=size[1];
	size[0] = s2;
	size[1] = s1;
	ret.allocate(MDA_MAX_DIMS,size);
    long tot_size = totalSize();
    long num_planes = tot_size / (size[0] * size[1]);
    for (long i = 0; i < num_planes; i++) {
        long offset = i * size[0] * size[1];
        for (long y = 0; y < d->m_size[1]; y++) {
            for (long x = 0; x < d->m_size[0]; x++) {
				ret.setValue1(value1(offset + x + y * d->m_size[0]), offset + y + x * d->m_size[1]);
			}
		}
	}
	jfree(size);
	return ret;
}
bool Mda::read(const char *path) {
	allocate(1, 1);

	FILE *inf=jfopen(path,"rb");
	if (!inf) return false;
	
	bool ret=d->do_read(inf);
	
	jfclose(inf);
	
	return ret;
	
}
bool MdaPrivate::do_read(FILE *inf) {
    long hold_num_dims;
    long hold_dims[MDA_MAX_DIMS];
    for (long i=0; i<MDA_MAX_DIMS; i++) hold_dims[i]=1;
	
	hold_num_dims = read_int(inf);
	
    long data_type;
	if (hold_num_dims < 0) {
		data_type = hold_num_dims;
        long num_bytes = read_int(inf);
		UNUSED(num_bytes)
		hold_num_dims = read_int(inf);
	} else {
		data_type = MDA_TYPE_COMPLEX;
	}
	if (hold_num_dims > MDA_MAX_DIMS) {
        printf ("number of dimensions exceeds maximum: %ld\n", hold_num_dims);
		return false;
	}
	if (hold_num_dims <= 0) {
        printf ("unexpected number of dimensions: %ld\n", hold_num_dims);
		return false;
	}
    for (long j = 0; j < hold_num_dims; j++) {
        long holdval = read_int(inf);
		hold_dims[j] = holdval;
	}
	{
        //q->setDataType(data_type);
		q->allocate(hold_num_dims,hold_dims);
        long N = q->totalSize();
		if (data_type == MDA_TYPE_COMPLEX) {
            for (long ii = 0; ii < N; ii++) {
				float re0 = read_float(inf);
				float im0 = read_float(inf);
				UNUSED(im0);
				m_data_real[ii] = re0;
			}
        } else if (data_type == MDA_TYPE_FLOAT32) {
            for (long ii = 0; ii < N; ii++) {
				float re0 = read_float(inf);
				m_data_real[ii] = re0;
			}
		} else if (data_type == MDA_TYPE_SHORT) {
            for (long ii = 0; ii < N; ii++) {
				float re0 = read_short(inf);
				m_data_real[ii] = re0;
			}
		} else if (data_type == MDA_TYPE_UINT16) {
            for (long ii = 0; ii < N; ii++) {
                long re0 = read_unsigned_short(inf);
				m_data_real[ii] = re0;
			}
		} else if (data_type == MDA_TYPE_INT32) {
            for (long ii = 0; ii < N; ii++) {
                long re0 = read_int(inf);
				m_data_real[ii] = re0;
			}
		} else if (data_type == MDA_TYPE_BYTE) {
            for (long ii = 0; ii < N; ii++) {
				unsigned char re0 = read_unsigned_char(inf);
				m_data_real[ii] = re0;
			}
        } else if (data_type == MDA_TYPE_FLOAT64) {
            for (long ii = 0; ii < N; ii++) {
                double re0 = read_double(inf);
                m_data_real[ii] = re0;
            }
		} else {
            printf ("Unrecognized data type %ld\n", data_type);
			return false;
		}
	}
	
	return true;
}
int MdaPrivate::read_int(FILE *inf) {
	int ret=0;
    long b0=jfread(&ret,4,1,inf);
	UNUSED(b0)
	return ret;
}
float MdaPrivate::read_float(FILE *inf) {
	float ret=0;
    long b0=jfread(&ret,4,1,inf);
	UNUSED(b0)
	return ret;
}
double MdaPrivate::read_double(FILE *inf) {
    double ret=0;
    long b0=jfread(&ret,8,1,inf);
    UNUSED(b0)
    return ret;
}
short MdaPrivate::read_short(FILE *inf) {
	short ret=0;
    long b0=jfread(&ret,2,1,inf);
	UNUSED(b0)
	return ret;
}
unsigned short MdaPrivate::read_unsigned_short(FILE *inf) {
	unsigned short ret=0;
    long b0=jfread(&ret,2,1,inf);
	UNUSED(b0)
	return ret;
}
unsigned char MdaPrivate::read_unsigned_char(FILE *inf) {
	unsigned char ret=0;
    long b0=jfread(&ret,1,1,inf);
	UNUSED(b0)
	return ret;
}

bool Mda::write(const char *path) {

    return write32(path);
}

bool Mda::write32(const char *path)
{
    FILE *outf=jfopen(path,"wb");
    if (!outf) {
        printf ("Unable to write mda file: %s\n",path);
        return false;
    }

    bool ret=d->do_write(outf,MDA_TYPE_FLOAT32);

    jfclose(outf);

    return ret;
}

bool Mda::write64(const char *path)
{
    FILE *outf=jfopen(path,"wb");
    if (!outf) {
        printf ("Unable to write mda file: %s\n",path);
        return false;
    }

    bool ret=d->do_write(outf,MDA_TYPE_FLOAT64);

    jfclose(outf);

    return ret;
}

#ifdef QT_CORE_LIB
bool Mda::read(const QString &path)
{
    return read(path.toLatin1().data());
}

bool Mda::write(const QString &path)
{
    return write(path.toLatin1().data());
}

bool Mda::write32(const QString &path)
{
    return write32(path.toLatin1().data());
}

bool Mda::write64(const QString &path)
{
    return write64(path.toLatin1().data());
}
#endif

double *Mda::dataPtr()
{
    return d->m_data_real;
}

bool MdaPrivate::do_write(FILE *outf,long data_type) {

    write_int(outf, data_type);
    long num_bytes = 4;
    if (data_type == MDA_TYPE_COMPLEX) {
		num_bytes = 8;
    } else if (data_type == MDA_TYPE_BYTE) {
		num_bytes = 1;
    } else if (data_type == MDA_TYPE_SHORT) {
		num_bytes = 2;
    } else if (data_type == MDA_TYPE_UINT16) {
		num_bytes = 2;
    } else if (data_type == MDA_TYPE_FLOAT64 ) {
        num_bytes = 8;
    }

	write_int(outf, num_bytes);
    long num_dims = 2;
    for (long i = 2; i < MDA_MAX_DIMS; i++) {
		if (m_size[i] > 1) {
			num_dims = i + 1;
		}
	}
	write_int(outf, num_dims);
    for (long ii = 0; ii < num_dims; ii++) {
		write_int(outf, m_size[ii]);
	}
    long N = q->totalSize();
    if (data_type == MDA_TYPE_COMPLEX) {
        for (long i = 0; i < N; i++) {
			float re0 = (float) m_data_real[i];
			write_float(outf, re0);
			write_float(outf, 0);
		}
    } else if (data_type == MDA_TYPE_FLOAT32) {
        for (long i = 0; i < N; i++) {
			float re0 = (float) m_data_real[i];
			write_float(outf, re0);
		}
    } else if (data_type == MDA_TYPE_BYTE) {
        for (long i = 0; i < N; i++) {
			unsigned char re0 = (unsigned char) m_data_real[i];
			write_unsigned_char(outf,re0);
		}
    } else if (data_type == MDA_TYPE_SHORT) {
        for (long i = 0; i < N; i++) {
			short re0 = (short) m_data_real[i];
			write_short(outf, (short) re0);
		}
    } else if (data_type == MDA_TYPE_UINT16) {
        for (long i = 0; i < N; i++) {
			unsigned short re0 = (unsigned short) m_data_real[i];
			write_unsigned_short(outf, (unsigned short) re0);
		}
    } else if (data_type == MDA_TYPE_INT32) {
        for (long i = 0; i < N; i++) {
			int re0 = (int) m_data_real[i];
			write_int(outf, re0);
		}
	}
    else if (data_type == MDA_TYPE_FLOAT64) {
        for (long i = 0; i < N; i++) {
            double re0 = m_data_real[i];
            write_double(outf, re0);
        }
    }
	else {
        printf ("Problem in do_write... unexpected data type: %ld\n",data_type);
		return false;
	}
	return true;
}
void MdaPrivate::write_int(FILE *outf,int val) {
	fwrite(&val,4,1,outf);
}
void MdaPrivate::write_float(FILE *outf,float val) {
	fwrite(&val,4,1,outf);
}
void MdaPrivate::write_double(FILE *outf,double val) {
    fwrite(&val,8,1,outf);
}
void MdaPrivate::write_short(FILE *outf,short val) {
	fwrite(&val,2,1,outf);
}
void MdaPrivate::write_unsigned_short(FILE *outf,unsigned short val) {
	fwrite(&val,2,1,outf);
}
void MdaPrivate::write_unsigned_char(FILE *outf,unsigned char val) {
	fwrite(&val,1,1,outf);
}
