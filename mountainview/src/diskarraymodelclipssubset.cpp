#include "diskarraymodelclipssubset.h"
#include <QDebug>

class DiskArrayModelClipsSubsetPrivate {
public:
	DiskArrayModelClipsSubset *q;
    long m_t1,m_t2;
};

DiskArrayModelClipsSubset::DiskArrayModelClipsSubset() : DiskArrayModel()
{
	d=new DiskArrayModelClipsSubsetPrivate;
	d->q=this;
	d->m_t1=0;
	d->m_t2=0;
}

DiskArrayModelClipsSubset::~DiskArrayModelClipsSubset()
{
	delete d;
}

Mda DiskArrayModelClipsSubset::loadData(long scale, long t1, long t2)
{
    long t1_b=(t1*scale+d->m_t1)/scale;
    long t2_b=(t2*scale+d->m_t1)/scale;
	return DiskArrayModel::loadData(scale,t1_b,t2_b);
}

long DiskArrayModelClipsSubset::size(long dim)
{
	if (dim==0) return DiskArrayModel::size(dim);
	else if (dim==1) return d->m_t2-d->m_t1;
	else return 1;
}

long DiskArrayModelClipsSubset::dim3()
{
    long dim3a=DiskArrayModel::dim3();
    long size1a=DiskArrayModel::size(1);
    long size1=size(1);

	if (size1==0) return dim3a;
	if (size1a==0) return dim3a;
    return (long)(dim3a*(size1*1.0/size1a));
}

void DiskArrayModelClipsSubset::setRange(long t1, long t2)
{
	d->m_t1=t1;
	d->m_t2=t2;
}
