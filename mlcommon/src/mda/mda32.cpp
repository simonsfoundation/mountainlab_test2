#include "mda32.h"

Mda32::Mda32()
    : MdaBase(MdaBase::Float)
{
}

Mda32::Mda32(const QString& mda_filename)
    : MdaBase(MdaBase::Float, mda_filename)
{
}

Mda32::Mda32(long N1, long N2, long N3, long N4, long N5, long N6)
    : MdaBase(MdaBase::Float, N1, N2, N3, N4, N5, N6)
{
}

Mda32::Mda32(const Mda32& other)
    : MdaBase(MdaBase::Float)
{
    copy_from(other);
}

Mda32::~Mda32()
{
}

void Mda32::operator=(const Mda32& other)
{
    copy_from(other);
}

void Mda32::copy_from(const Mda32& other)
{

    this->allocate(other.N1(), other.N2(), other.N3(), other.N4(), other.N5(), other.N6());
    if (totalSize() != other.totalSize()) {
        qWarning() << "Unexpected problem copying mda data... sizes don't match:" << totalSize() << other.totalSize();
        return;
    }
    if (totalSize() > 0) {
        memcpy(dataPtr(), other.constDataPtr(), sizeof(float) * totalSize());
    }
}
