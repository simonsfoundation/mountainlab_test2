/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef MDA32_H
#define MDA32_H

#include "mda.h"

class Mda32 : public Mda {
public:
    Mda32();
    Mda32(const QString &fname);
    void allocate(long N1,long N2,long N3=1,long N4=1,long N5=1,long N6=1);
    bool read(const QString &fname);
};


#endif // MDA_H
