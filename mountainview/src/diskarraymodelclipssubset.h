#ifndef DISKARRAYMODELCLIPSSUBSET_H
#define DISKARRAYMODELCLIPSSUBSET_H

#include "diskarraymodel.h"

class DiskArrayModelClipsSubsetPrivate;
class DiskArrayModelClipsSubset : public DiskArrayModel
{
	Q_OBJECT
public:
	friend class DiskArrayModelClipsSubsetPrivate;
	DiskArrayModelClipsSubset();
	virtual ~DiskArrayModelClipsSubset();

    virtual Mda loadData(long scale,long t1,long t2);
    virtual long size(long dim);
    virtual long dim3();

    void setRange(long t1,long t2);
signals:

private slots:

private:
	DiskArrayModelClipsSubsetPrivate *d;
};

#endif // DISKARRAYMODELCLIPSSUBSET_H
