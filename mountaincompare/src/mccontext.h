#ifndef MCCONTEXT_H
#define MCCONTEXT_H

#include "mvcontext.h"

class MCContextPrivate;
class MCContext : public MVContext
{
    Q_OBJECT
public:
    friend class MCContextPrivate;
    MCContext();
    virtual ~MCContext();
    DiskReadMda firings2();

    void setFirings2(const DiskReadMda &F);
private:
    MCContextPrivate *d;
};

#endif // MCCONTEXT_H

