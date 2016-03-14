/******************************************************
**
** Copyright (C) 2016 by Jeremy Magland
**
** Some rights reserved.
** See accompanying LICENSE and README files.
**
*******************************************************/

#ifndef MDA_H
#define MDA_H

#ifdef QT_CORE_LIB
#include <QString>
#include <QDebug>
#endif

class MdaPrivate;
class Mda
{
public:
	friend class MdaPrivate;
	Mda(long N1=1,long N2=1,long N3=1,long N4=1,long N5=1,long N6=1);
	Mda(const Mda &other);
	void operator=(const Mda &other);
	virtual ~Mda();
	bool allocate(long N1,long N2,long N3=1,long N4=1,long N5=1,long N6=1);
	#ifdef QT_CORE_LIB
	bool read(const QString &path);
	bool write32(const QString &path) const;
	bool write64(const QString &path) const;
	#endif
	bool read(const char *path);
	bool write32(const char *path) const;
	bool write64(const char *path) const;

	int ndims() const;
	long N1() const;
	long N2() const;
	long N3() const;
	long N4() const;
	long N5() const;
	long N6() const;
	long totalSize() const;

	double get(long i) const;
	double get(long i1,long i2) const;
	double get(long i1,long i2,long i3) const;
	double get(long i1,long i2,long i3,long i4,long i5=0,long i6=0) const;

	void set(double val,long i);
	void set(double val,long i1,long i2);
	void set(double val,long i1,long i2,long i3);
	void set(double val,long i1,long i2,long i3,long i4,long i5=0,long i6=0);

	double value(long i) const;
	double value(long i1,long i2) const;
	double value(long i1,long i2,long i3) const;
	double value(long i1,long i2,long i3,long i4,long i5=0,long i6=0) const;

	void setValue(double val,long i);
	void setValue(double val,long i1,long i2);
	void setValue(double val,long i1,long i2,long i3);
	void setValue(double val,long i1,long i2,long i3,long i4,long i5=0,long i6=0);

	double *dataPtr();
	double *dataPtr(long i);
	double *dataPtr(long i1,long i2);
	double *dataPtr(long i1,long i2,long i3);
	double *dataPtr(long i1,long i2,long i3,long i4,long i5=0,long i6=0);

	void getSubArray(Mda &ret,long i,long size);
	void getSubArray(Mda &ret,long i1,long i2,long size1,long size2);
	void getSubArray(Mda &ret,long i1,long i2,long i3,long size1,long size2,long size3);
private:
	MdaPrivate *d;
};

#endif // MDA_H

