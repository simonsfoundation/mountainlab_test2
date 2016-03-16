/******************************************************
**
** Copyright (C) 2016 by Jeremy Magland
**
** This file is part of the MountainSort C++ project
**
** Some rights reserved.
** See accompanying LICENSE and README files.
**
*******************************************************/

#ifndef DISKREADMDA_H
#define DISKREADMDA_H

#include "mda.h"

class DiskReadMdaPrivate;
class DiskReadMda
{
public:
	friend class DiskReadMdaPrivate;
	DiskReadMda(const QString &path="");
	DiskReadMda(const DiskReadMda &other);
	DiskReadMda(const Mda &X);
	virtual ~DiskReadMda();
	void operator=(const DiskReadMda &other);
	#ifdef QT_CORE_LIB
	void setPath(const QString &file_path);
	#endif
	void setPath(const char *file_path);
	long N1() const;
	long N2() const;
	long N3() const;
	long N4() const;
	long N5() const;
	long N6() const;
	long totalSize() const;
	bool readChunk(Mda &X,long i,long size) const;
	bool readChunk(Mda &X,long i1,long i2,long size1,long size2) const;
	bool readChunk(Mda &X,long i1,long i2,long i3,long size1,long size2,long size3) const;
	double value(long i) const;
	double value(long i1,long i2) const;
	double value(long i1,long i2,long i3) const;
private:
	DiskReadMdaPrivate *d;
};

void diskreadmda_unit_test();

#endif // DISKREADMDA_H

