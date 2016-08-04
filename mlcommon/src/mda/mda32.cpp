#include "mda32.h"

Mda32::Mda32()
{
    this->allocate32(1,1);
}

Mda32::Mda32(const QString &fname)
{
    this->read32(fname);
}

void Mda32::allocate(long N1, long N2, long N3, long N4, long N5, long N6)
{
    this->allocate32(N1,N2,N3,N4,N5,N6);
}

bool Mda32::read(const QString &fname)
{
    return this->read32(fname);
}
