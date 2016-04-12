#ifndef array2d_H
#define array2d_H

#include <QString>
#include <QList>
#include <QDebug>
#include <math.h>

class Array2DPrivate;

class Array2D {
public:
	friend class Array2DPrivate;
	Array2D(int W=0,int H=0);
	Array2D(const Array2D &X);
	virtual ~Array2D();
	void clear();
	void allocate(int w,int h=1);
	int N1() const;
	int N2() const;
	int width() const;
	int height() const;
	float getValue(int i,int j=0) const;
	float value(int i,int j=0) const;
	float value1(long i) const;
	void setValue1(float val,long i) const;
	void setValue(float val,int i,int j=0);
	void incrementValue(float val,int i,int j=0);
	void setAll(float val);
	void getData(float *ret) const;
	void setData(float *D);
	void operator=(const Array2D &X);
	float max() const;
	float min() const;
	QList<float> dataY(int x) const;
	QList<float> dataX(int y) const;
	float *ptrX(int y) const;
	void setDataX(const QList<float> &X,int y);
	void setDataY(const QList<float> &X,int x);
	void scaleBy(float val);
	void add(const Array2D &X);
	Array2D transpose() const;
	
	float *data1() const;
private:
	Array2DPrivate *d;
};

class Array1D : public Array2D {
public:
	Array1D() : Array2D(0,0) {}
	Array1D(int N) : Array2D(N,1) {}
};

void write_mda(const QString &fname,const QList<float> &data);
void read_mda(const QString &fname,QList<float> &data);
//QList<float> read_float_list(const QString &fname);
void add(QList<float> &X,const QList<float> &Y);

void copy_array(float *dst,float *src,long N);
double ip(float *X1,float *X2,long N);
void normalize(float *X,long N);
void normalize(QList<float> &X);

Array2D matrix_multiply(const Array2D &M1,const Array2D &M2);

#endif

