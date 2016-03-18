#ifndef mda_H
#define mda_H

#define MDA_MAX_DIMS 50
//#define MDA_MAX_SIZE 512 * 512 * 512 * 10
#define MDA_MAX_SIZE 1e12
#define MDA_TYPE_COMPLEX -1
#define MDA_TYPE_BYTE -2
#define MDA_TYPE_FLOAT32 -3
#define MDA_TYPE_SHORT -4
#define MDA_TYPE_INT32 -5
#define MDA_TYPE_UINT16 -6
#define MDA_TYPE_FLOAT64 -7

#ifdef QT_CORE_LIB
#include <QString>
#endif

//changed ints to longs on 3/10/16


class MdaPrivate;
class Mda {
public:
	friend class MdaPrivate;
	Mda();
	Mda(const Mda &X);
	virtual ~Mda();
	
	void operator=(const Mda &X);
    void allocate(long N1,long N2,long N3=1,long N4=1,long N5=1,long N6=1);
    void allocate(long num_dims,int *size);
    void allocate(long num_dims,long *size);
    long size(long dim) const;
    long N1() const;
    long N2() const;
    long N3() const;
    long N4() const;
    long N5() const;
    long N6() const;
    long dimCount() const;
    long totalSize() const;
    void reshape(long N1,long N2,long N3=1,long N4=1,long N5=1,long N6=1);
    double value1(long i) const;
    double value(long i1,long i2) const;
    double value(long i1,long i2,long i3,long i4=0) const;
    double value(long num_dims,int *ind) const;
    double value(long num_dims,long *ind) const;
    void setValue1(double val,long i);
    void setValue(double val,long i1,long i2,long i3=0,long i4=0);
    void setValue(double val,long num_dims,int *ind);
    void setValue(double val,long num_dims,long *ind);
	void setValues(double *vals);
	void setValues(int *vals);
	void setValues(short *vals);
	void setValues(unsigned char *vals);
    Mda getDataXY(long num_inds,int *inds) const;
    Mda getDataXZ(long num_inds,int *inds) const;
    Mda getDataYZ(long num_inds,int *inds) const;
	Mda transpose() const;
    bool read(const char *path);
    bool write(const char *path);
    bool write32(const char *path);
    bool write64(const char *path);
    #ifdef QT_CORE_LIB
    bool read(const QString &path);
    bool write(const QString &path);
    bool write32(const QString &path);
    bool write64(const QString &path);
    #endif

    double *dataPtr();
	
private:
	MdaPrivate *d;
};

#endif


	
	

	
	
