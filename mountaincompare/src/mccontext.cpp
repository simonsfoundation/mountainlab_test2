#include "mccontext.h"

class MCContextPrivate {
public:
    MCContext *q;
    DiskReadMda m_firings2;
    int m_current_cluster2=-1;
};

MCContext::MCContext()
{
    d=new MCContextPrivate;
    d->q=this;
}

MCContext::~MCContext()
{
    delete d;
}

DiskReadMda MCContext::firings2()
{
    return d->m_firings2;
}

int MCContext::currentCluster2() const
{
    return d->m_current_cluster2;
}

void MCContext::setFirings2(const DiskReadMda &F)
{
    d->m_firings2=F;
}

void MCContext::setCurrentCluster2(int k)
{
    if (d->m_current_cluster2==k) return;
    d->m_current_cluster2=k;
    emit currentCluster2Changed();
}
