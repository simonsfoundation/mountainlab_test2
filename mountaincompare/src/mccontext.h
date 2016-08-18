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
    int currentCluster2() const;

    void setFirings2(const DiskReadMda &F);
    void setCurrentCluster2(int k);
signals:
    void currentCluster2Changed();
private:
    MCContextPrivate *d;
};

#endif // MCCONTEXT_H

