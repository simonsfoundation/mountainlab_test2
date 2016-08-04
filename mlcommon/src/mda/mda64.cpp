#include "mda64.h"

Mda64::Mda64()
{

}

Mda64::Mda64(const QString &fname)
{
    this->read64(fname);
}

Mda64::Mda64(long N1, long N2, long N3, long N4, long N5, long N6)
{
    this->allocate64(N1,N2,N3,N4,N5,N6);
}

bool Mda64::read(const QString &fname)
{
    return this->read64(fname);
}
