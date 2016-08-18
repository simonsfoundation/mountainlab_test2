#include "mccontext.h"

class MCContextPrivate {
public:
    MCContext *q;
    DiskReadMda m_firings2;
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

void MCContext::setFirings2(const DiskReadMda &F)
{
    d->m_firings2=F;
}
