#include "mda32.h"
#include "mda_p.h"
#include "mdaio.h"
#include <cachemanager.h>
#include <stdio.h>
#include "mlcommon.h"
#include "taskprogress.h"

#define MDA_MAX_DIMS 6

class MdaDataFloat : public MdaData<float> {};

Mda32::Mda32(long N1, long N2, long N3, long N4, long N5, long N6)
{
    d = new MdaDataFloat;
    this->allocate(N1, N2, N3, N4, N5, N6);
}

Mda32::Mda32(const QString mda_filename)
{
    d = new MdaDataFloat;
    this->read(mda_filename);
}

Mda32::Mda32(const Mda32& other)
{
    d = other.d;
}

void Mda32::operator=(const Mda32& other)
{
    d = other.d;
}

Mda32::~Mda32()
{
}

bool Mda32::allocate(long N1, long N2, long N3, long N4, long N5, long N6)
{

    return d->allocate((float)0, N1, N2, N3, N4, N5, N6);
}

bool Mda32::read(const QString& path)
{
    return read(path.toLatin1().data());
}

bool Mda32::write8(const QString& path) const
{
    return write8(path.toLatin1().data());
}

bool Mda32::write32(const QString& path) const
{
    return write32(path.toLatin1().data());
}

bool Mda32::write64(const QString& path) const
{
    return write64(path.toLatin1().data());
}

bool Mda32::read(const char* path)
{
    if ((QString(path).endsWith(".txt")) || (QString(path).endsWith(".csv"))) {
        return d->read_from_text_file(path);
    }
    FILE* input_file = fopen(path, "rb");
    if (!input_file) {
        printf("Warning: Unable to open mda file for reading: %s\n", path);
        return false;
    }
    MDAIO_HEADER H;
    if (!mda_read_header(&H, input_file)) {
        qWarning() << "Problem reading mda file: " + QString(path);
        fclose(input_file);
        return false;
    }
    this->allocate(H.dims[0], H.dims[1], H.dims[2], H.dims[3], H.dims[4], H.dims[5]);
    mda_read_float32(d->data(), &H, d->totalSize(), input_file);
    TaskManager::TaskProgressMonitor::globalInstance()->incrementQuantity("bytes_read", d->totalSize() * H.num_bytes_per_entry);
    fclose(input_file);
    return true;
}

bool Mda32::write8(const char* path) const
{
    if ((QString(path).endsWith(".txt")) || (QString(path).endsWith(".csv"))) {
        return d->write_to_text_file(path);
    }
    FILE* output_file = fopen(path, "wb");
    if (!output_file) {
        printf("Warning: Unable to open mda file for writing: %s\n", path);
        return false;
    }
    MDAIO_HEADER H;
    H.data_type = MDAIO_TYPE_BYTE;
    H.num_bytes_per_entry = 1;
    for (int i = 0; i < MDAIO_MAX_DIMS; i++)
        H.dims[i] = 1;
    for (int i = 0; i < MDA_MAX_DIMS; i++)
        H.dims[i] = d->dims(i);
    H.num_dims = d->determine_num_dims(N1(), N2(), N3(), N4(), N5(), N6());
    mda_write_header(&H, output_file);
    mda_write_float32(d->constData(), &H, d->totalSize(), output_file);
    TaskManager::TaskProgressMonitor::globalInstance()->incrementQuantity("bytes_written", d->totalSize() * H.num_bytes_per_entry);
    fclose(output_file);
    return true;
}

bool Mda32::write32(const char* path) const
{
    if ((QString(path).endsWith(".txt")) || (QString(path).endsWith(".csv"))) {
        return d->write_to_text_file(path);
    }
    FILE* output_file = fopen(path, "wb");
    if (!output_file) {
        printf("Warning: Unable to open mda file for writing: %s\n", path);
        return false;
    }
    MDAIO_HEADER H;
    H.data_type = MDAIO_TYPE_FLOAT32;
    H.num_bytes_per_entry = 4;
    for (int i = 0; i < MDAIO_MAX_DIMS; i++)
        H.dims[i] = 1;
    for (int i = 0; i < MDA_MAX_DIMS; i++)
        H.dims[i] = d->dims(i);
    H.num_dims = d->determine_num_dims(N1(), N2(), N3(), N4(), N5(), N6());
    mda_write_header(&H, output_file);
    mda_write_float32(d->constData(), &H, d->totalSize(), output_file);
    TaskManager::TaskProgressMonitor::globalInstance()->incrementQuantity("bytes_written", d->totalSize() * H.num_bytes_per_entry);
    fclose(output_file);
    return true;
}

bool Mda32::write64(const char* path) const
{
    if ((QString(path).endsWith(".txt")) || (QString(path).endsWith(".csv"))) {
        return d->write_to_text_file(path);
    }
    FILE* output_file = fopen(path, "wb");
    if (!output_file) {
        printf("Warning: Unable to open mda file for writing: %s\n", path);
        return false;
    }
    MDAIO_HEADER H;
    H.data_type = MDAIO_TYPE_FLOAT64;
    H.num_bytes_per_entry = 4;
    for (int i = 0; i < MDAIO_MAX_DIMS; i++)
        H.dims[i] = 1;
    for (int i = 0; i < MDA_MAX_DIMS; i++)
        H.dims[i] = d->dims(i);
    H.num_dims = d->determine_num_dims(N1(), N2(), N3(), N4(), N5(), N6());
    mda_write_header(&H, output_file);
    mda_write_float32(d->constData(), &H, d->totalSize(), output_file);
    TaskManager::TaskProgressMonitor::globalInstance()->incrementQuantity("bytes_written", d->totalSize() * H.num_bytes_per_entry);
    fclose(output_file);
    return true;
}

QByteArray Mda32::toByteArray8() const
{
    QString path = CacheManager::globalInstance()->makeLocalFile("", CacheManager::ShortTerm);
    write8(path);
    return MLUtil::readByteArray(path);
}

QByteArray Mda32::toByteArray32() const
{
    QString path = CacheManager::globalInstance()->makeLocalFile("", CacheManager::ShortTerm);
    write32(path);
    return MLUtil::readByteArray(path);
}

QByteArray Mda32::toByteArray64() const
{
    QString path = CacheManager::globalInstance()->makeLocalFile("", CacheManager::ShortTerm);
    write64(path);
    return MLUtil::readByteArray(path);
}

bool Mda32::fromByteArray(const QByteArray& X)
{
    QString path = CacheManager::globalInstance()->makeLocalFile("", CacheManager::ShortTerm);
    MLUtil::writeByteArray(path, X);
    return this->read(path);
}

int Mda32::ndims() const
{
    return d->determine_num_dims(N1(), N2(), N3(), N4(), N5(), N6());
}

long Mda32::N1() const
{
    return d->dims(0);
}

long Mda32::N2() const
{
    return d->dims(1);
}

long Mda32::N3() const
{
    return d->dims(2);
}

long Mda32::N4() const
{
    return d->dims(3);
}

long Mda32::N5() const
{
    return d->dims(4);
}

long Mda32::N6() const
{
    return d->dims(5);
}

long Mda32::totalSize() const
{
    return d->totalSize();
}

long Mda32::size(int dimension_index) const
{
    if (dimension_index < 0)
        return 0;
    if (dimension_index >= MDA_MAX_DIMS)
        return 1;
    return d->dims(dimension_index);
}

dtype32 Mda32::get(long i) const
{
    return d->at(i);
}

dtype32 Mda32::get(long i1, long i2) const
{
    return d->at(i1 + d->dims(0) * i2);
}

dtype32 Mda32::get(long i1, long i2, long i3) const
{
    return d->at(i1 + d->dims(0) * i2 + d->dims(0) * d->dims(1) * i3);
}

dtype32 Mda32::get(long i1, long i2, long i3, long i4, long i5, long i6) const
{
    const long d01 = d->dims(0)*d->dims(1);
    const long d02 = d01 * d->dims(2);
    const long d03 = d02 * d->dims(3);
    const long d04 = d03 * d->dims(4);

    return d->at(i1 + d->dims(0) * i2 + d01 * i3 + d02 * i4 + d03 * i5 + d04 * i6);
}

dtype32 Mda32::value(long i) const
{
    if (!d->safe_index(i))
        return 0;
    return get(i);
}

dtype32 Mda32::value(long i1, long i2) const
{
    if (!d->safe_index(i1, i2))
        return 0;
    return get(i1, i2);
}

dtype32 Mda32::value(long i1, long i2, long i3) const
{
    if (!d->safe_index(i1, i2, i3))
        return 0;
    return get(i1, i2, i3);
}

dtype32 Mda32::value(long i1, long i2, long i3, long i4, long i5, long i6) const
{
    if (!d->safe_index(i1, i2, i3, i4, i5, i6))
        return 0;
    return get(i1, i2, i3, i4, i5, i6);
}

void Mda32::setValue(dtype32 val, long i)
{
    if (!d->safe_index(i))
        return;
    set(val, i);
}

void Mda32::setValue(dtype32 val, long i1, long i2)
{
    if (!d->safe_index(i1, i2))
        return;
    set(val, i1, i2);
}

void Mda32::setValue(dtype32 val, long i1, long i2, long i3)
{
    if (!d->safe_index(i1, i2, i3))
        return;
    set(val, i1, i2, i3);
}

void Mda32::setValue(dtype32 val, long i1, long i2, long i3, long i4, long i5, long i6)
{
    if (!d->safe_index(i1, i2, i3, i4, i5, i6))
        return;
    set(val, i1, i2, i3, i4, i5, i6);
}

dtype32* Mda32::dataPtr()
{
    return d->data();
}

const dtype32* Mda32::constDataPtr() const
{
    return d->constData();
}

dtype32* Mda32::dataPtr(long i)
{
    return d->data()+i;
}

dtype32* Mda32::dataPtr(long i1, long i2)
{
    return d->data()+(i1 + N1() * i2);
}

dtype32* Mda32::dataPtr(long i1, long i2, long i3)
{
    return d->data()+(i1 + N1() * i2 + N1() * N2() * i3);
}

dtype32* Mda32::dataPtr(long i1, long i2, long i3, long i4, long i5, long i6)
{
    const long N12 = N1()*N2();
    const long N13 = N12*N3();
    const long N14 = N13*N4();
    const long N15 = N14*N5();

    return d->data()+(i1 + N1() * i2 + N12 * i3 + N13 * i4 + N14 * i5 + N15 * i6);
}

void Mda32::getChunk(Mda32& ret, long i, long size)
{
    // A lot of bugs fixed on 5/31/16
    long a_begin = i;
    long x_begin = 0;
    long a_end = i + size - 1;
//    long x_end = size - 1;  // unused?

    if (a_begin < 0) {
        x_begin += 0 - a_begin;
        a_begin += 0 - a_begin;
    }
    if (a_end >= (long)d->totalSize()) {
//        x_end += (long)d->totalSize() - 1 - a_end; // unused?
        a_end += (long)d->totalSize() - 1 - a_end;
    }

    ret.allocate(1, size);

    const float* ptr1 = this->constDataPtr();
    float* ptr2 = ret.dataPtr();

    std::copy(ptr1+a_begin, ptr1+a_end+1, ptr2+x_begin);
}

void Mda32::getChunk(Mda32& ret, long i1, long i2, long size1, long size2)
{
    // A lot of bugs fixed on 5/31/16
    long a1_begin = i1;
    long x1_begin = 0;
    long a1_end = i1 + size1 - 1;
    long x1_end = size1 - 1;
    if (a1_begin < 0) {
        x1_begin += 0 - a1_begin;
        a1_begin += 0 - a1_begin;
    }
    if (a1_end >= N1()) {
        x1_end += N1() - 1 - a1_end;
        a1_end += N1() - 1 - a1_end;
    }

    long a2_begin = i2;
    long x2_begin = 0;
    long a2_end = i2 + size2 - 1;
    long x2_end = size2 - 1;
    if (a2_begin < 0) {
        x2_begin += 0 - a2_begin;
        a2_begin += 0 - a2_begin;
    }
    if (a2_end >= N2()) {
        x2_end += N2() - 1 - a2_end;
        a2_end += N2() - 1 - a2_end;
    }

    ret.allocate(size1, size2);

    const float* ptr1 = this->constDataPtr();
    float* ptr2 = ret.dataPtr();

    for (long ind2 = 0; ind2 <= a2_end - a2_begin; ind2++) {
        long ii_out = (ind2 + x2_begin) * size1 + x1_begin; //bug fixed on 5/31/16 by jfm
        long ii_in = (ind2 + a2_begin) * N1() + a1_begin; //bug fixed on 5/31/16 by jfm
        for (long ind1 = 0; ind1 <= a1_end - a1_begin; ind1++) {
            ptr2[ii_out] = ptr1[ii_in];
            ii_in++;
            ii_out++;
        }
    }
}

void Mda32::getChunk(Mda32& ret, long i1, long i2, long i3, long size1, long size2, long size3)
{
    // A lot of bugs fixed on 5/31/16
    long a1_begin = i1;
    long x1_begin = 0;
    long a1_end = i1 + size1 - 1;
    long x1_end = size1 - 1;
    if (a1_begin < 0) {
        x1_begin += 0 - a1_begin;
        a1_begin += 0 - a1_begin;
    }
    if (a1_end >= N1()) {
        x1_end += N1() - 1 - a1_end;
        a1_end += N1() - 1 - a1_end;
    }

    long a2_begin = i2;
    long x2_begin = 0;
    long a2_end = i2 + size2 - 1;
    long x2_end = size2 - 1;
    if (a2_begin < 0) {
        x2_begin += 0 - a2_begin;
        a2_begin += 0 - a2_begin;
    }
    if (a2_end >= N2()) {
        x2_end += N2() - 1 - a2_end;
        a2_end += N2() - 1 - a2_end;
    }

    long a3_begin = i3;
    long x3_begin = 0;
    long a3_end = i3 + size3 - 1;
    long x3_end = size3 - 1;
    if (a3_begin < 0) {
        x3_begin += 0 - a3_begin;
        a3_begin += 0 - a3_begin;
    }
    if (a3_end >= N3()) {
        x3_end += N3() - 1 - a3_end;
        a3_end += N3() - 1 - a3_end;
    }

    ret.allocate(size1, size2, size3);

    const float* ptr1 = this->constDataPtr();
    float* ptr2 = ret.dataPtr();

    for (long ind3 = 0; ind3 <= a3_end - a3_begin; ind3++) {
        for (long ind2 = 0; ind2 <= a2_end - a2_begin; ind2++) {
            long ii_out = x1_begin + (ind2 + x2_begin) * size1 + (ind3 + x3_begin) * size1 * size2; //bug fixed on 5/31/16 by jfm
            long ii_in = a1_begin + (ind2 + a2_begin) * N1() + (ind3 + a3_begin) * N1() * N2(); //bug fixed on 5/31/16 by jfm
            for (long ind1 = 0; ind1 <= a1_end - a1_begin; ind1++) {
                ptr2[ii_out] = ptr1[ii_in];
                ii_in++;
                ii_out++;
            }
        }
    }
}

void Mda32::setChunk(Mda32& X, long i)
{
    long size = X.totalSize();

    long a_begin = i;
    long x_begin = 0;
    long a_end = i + size - 1;
    long x_end = size - 1;

    if (a_begin < 0) {
        a_begin += 0 - a_begin;
        x_begin += 0 - a_begin;
    }
    if (a_end >= (long)d->totalSize()) {
        a_end += (long)d->totalSize() - 1 - a_end;
        x_end += (long)d->totalSize() - 1 - a_end;
    }

    float* ptr1 = this->dataPtr();
    float* ptr2 = X.dataPtr();

    long ii = 0;
    for (long a = a_begin; a <= a_end; a++) {
        ptr1[a_begin + ii] = ptr2[x_begin + ii];
        ii++;
    }
}

void Mda32::setChunk(Mda32& X, long i1, long i2)
{
    long size1 = X.N1();
    long size2 = X.N2();

    long a1_begin = i1;
    long x1_begin = 0;
    long a1_end = i1 + size1 - 1;
    long x1_end = size1 - 1;
    if (a1_begin < 0) {
        a1_begin += 0 - a1_begin;
        x1_begin += 0 - a1_begin;
    }
    if (a1_end >= N1()) {
        a1_end += N1() - 1 - a1_end;
        x1_end += N1() - 1 - a1_end;
    }

    long a2_begin = i2;
    long x2_begin = 0;
    long a2_end = i2 + size2 - 1;
    long x2_end = size2 - 1;
    if (a2_begin < 0) {
        a2_begin += 0 - a2_begin;
        x2_begin += 0 - a2_begin;
    }
    if (a2_end >= N2()) {
        a2_end += N2() - 1 - a2_end;
        x2_end += N2() - 1 - a2_end;
    }

    dtype32* ptr1 = this->dataPtr();
    dtype32* ptr2 = X.dataPtr();

    for (long ind2 = 0; ind2 <= a2_end - a2_begin; ind2++) {
        long ii_out = (ind2 + x2_begin) * size1;
        long ii_in = (ind2 + a2_begin) * N1();
        for (long ind1 = 0; ind1 <= a1_end - a1_begin; ind1++) {
            ptr1[ii_in] = ptr2[ii_out];
            ii_in++;
            ii_out++;
        }
    }
}

void Mda32::setChunk(Mda32& X, long i1, long i2, long i3)
{
    long size1 = X.N1();
    long size2 = X.N2();
    long size3 = X.N3();

    long a1_begin = i1;
    long x1_begin = 0;
    long a1_end = i1 + size1 - 1;
    long x1_end = size1 - 1;
    if (a1_begin < 0) {
        a1_begin += 0 - a1_begin;
        x1_begin += 0 - a1_begin;
    }
    if (a1_end >= N1()) {
        a1_end += N1() - 1 - a1_end;
        x1_end += N1() - 1 - a1_end;
    }

    long a2_begin = i2;
    long x2_begin = 0;
    long a2_end = i2 + size2 - 1;
    long x2_end = size2 - 1;
    if (a2_begin < 0) {
        a2_begin += 0 - a2_begin;
        x2_begin += 0 - a2_begin;
    }
    if (a2_end >= N2()) {
        a2_end += N2() - 1 - a2_end;
        x2_end += N2() - 1 - a2_end;
    }

    long a3_begin = i3;
    long x3_begin = 0;
    long a3_end = i3 + size3 - 1;
    long x3_end = size3 - 1;
    if (a3_begin < 0) {
        a2_begin += 0 - a3_begin;
        x3_begin += 0 - a3_begin;
    }
    if (a3_end >= N3()) {
        a3_end += N3() - 1 - a3_end;
        x3_end += N3() - 1 - a3_end;
    }

    dtype32* ptr1 = this->dataPtr();
    dtype32* ptr2 = X.dataPtr();

    for (long ind3 = 0; ind3 <= a3_end - a3_begin; ind3++) {
        for (long ind2 = 0; ind2 <= a2_end - a2_begin; ind2++) {
            long ii_out = (ind2 + x2_begin) * size1 + (ind3 + x3_begin) * size1 * size2;
            long ii_in = (ind2 + a2_begin) * N1() + (ind3 + a3_begin) * N1() * N2();
            for (long ind1 = 0; ind1 <= a1_end - a1_begin; ind1++) {
                ptr1[ii_in] = ptr2[ii_out];
                ii_in++;
                ii_out++;
            }
        }
    }
}

dtype32 Mda32::minimum() const
{
    long NN = this->totalSize();
    const dtype32* ptr = this->constDataPtr();
    if ((!NN) || (!ptr)) {
        return 0;
    }
    return *std::min_element(ptr, ptr+NN);
}

dtype32 Mda32::maximum() const
{
    long NN = this->totalSize();
    const dtype32* ptr = this->constDataPtr();
    if ((!NN) || (!ptr)) {
        return 0;
    }
    return *std::max_element(ptr, ptr+NN);
}

bool Mda32::reshape(int N1b, int N2b, int N3b, int N4b, int N5b, int N6b)
{
    if (N1b * N2b * N3b * N4b * N5b * N6b != this->totalSize()) {
        qWarning() << "Unable to reshape Mda32, wrong total size";
        qWarning() << N1b << N2b << N3b << N4b << N5b << N6b;
        qWarning() << N1() << N2() << N3() << N4() << N5() << N6();
        return false;
    }
    d->setDims(N1b, N2b, N3b, N4b, N5b, N6b);
    return true;
}

void Mda32::set(dtype32 val, long i)
{
    d->set(val, i);
}

void Mda32::set(dtype32 val, long i1, long i2)
{
    d->set(val, i1 + d->dims(0) * i2);
}

void Mda32::set(dtype32 val, long i1, long i2, long i3)
{
    d->set(val, i1 + d->dims(0) * i2 + d->dims(0) * d->dims(1) * i3);
}

void Mda32::set(dtype32 val, long i1, long i2, long i3, long i4, long i5, long i6)
{
    const long d01 = d->dims(0)*d->dims(1);
    const long d02 = d01 * d->dims(2);
    const long d03 = d02*d->dims(3);
    const long d04 = d03*d->dims(4);

    d->set(val, i1 + d->dims(0) * i2 + d01 * i3 + d02 * i4 + d03 * i5 + d04 * i6);
}
