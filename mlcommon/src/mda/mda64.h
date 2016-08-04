#ifndef MDA64_H
#define MDA64_H

#include "mda.h"

class Mda64 : public Mda {
public:
    Mda64();
    Mda64(const QString &fname);
    Mda64(long N1,long N2,long N3=1,long N4=1,long N5=1,long N6=1);\
    bool read(const QString &fname);
};

#endif // MDA64_H

