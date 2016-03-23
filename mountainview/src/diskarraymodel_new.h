#ifndef DISKARRAYMODEL_NEW_H
#define DISKARRAYMODEL_NEW_H

#include <QString>
#include "mda.h"

class DiskArrayModel_NewPrivate;
class DiskArrayModel_New {
public:
    friend class DiskArrayModel_NewPrivate;
    DiskArrayModel_New();
    virtual ~DiskArrayModel_New();
    void setPath(const QString& path);
    void setFromMda(const Mda& X);
    Mda loadData(long scale, long t1, long t2);
    long N1();
    long N2();
    long dim3();
    void createFileHierarchyIfNeeded();

private:
    DiskArrayModel_NewPrivate* d;
};

#endif // DISKARRAYMODEL_NEW_H
