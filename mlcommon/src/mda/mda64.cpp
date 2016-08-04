#include "mda64.h"

Mda64::Mda64() : MdaBase(MdaBase::Double)
{

}

Mda64::Mda64(const QString &mda_filename) : MdaBase(MdaBase::Double,mda_filename)
{

}

Mda64::Mda64(long N1, long N2, long N3, long N4, long N5, long N6) : MdaBase(MdaBase::Double,N1,N2,N3,N4,N5,N6)
{

}

Mda64::Mda64(const Mda64 &other) : MdaBase(MdaBase::Double)
{
    copy_from(other);
}

Mda64::~Mda64()
{

}

void Mda64::operator=(const Mda64& other)
{
    copy_from(other);
}

void Mda64::copy_from(const Mda64& other)
{

    this->allocate(other.N1(), other.N2(), other.N3(), other.N4(), other.N5(), other.N6());
    if (totalSize() != other.totalSize()) {
        qWarning() << "Unexpected problem copying mda data... sizes don't match:" << totalSize() << other.totalSize();
        return;
    }
    if (totalSize() > 0) {
        memcpy(dataPtr(), other.constDataPtr(), sizeof(double) * totalSize());
    }
}
