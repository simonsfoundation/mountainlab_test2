#include "mdabase.h"
#include "mdaio.h"
#include <cachemanager.h>
#include <stdio.h>
#include "mlcommon.h"
#include "taskprogress.h"

#define MDA_MAX_DIMS 6

#ifdef USE_SSE2
static void* malloc_aligned(const size_t alignValue, const size_t nbytes)
{
    void* result = 0;
#ifdef __linux__
    if (posix_memalign(&result, alignValue, nbytes) != 0)
        result = 0;
#elif defined(__WIN32)
    result = _aligned_malloc(nbytes, alignValue);
#endif
    return result;
}
#endif

void* allocate(const size_t nbytes)
{
#ifdef USE_SSE2
    return malloc_aligned(16, nbytes);
#else
    return malloc(nbytes);
#endif
}

class MdaBasePrivate {
public:
    MdaBase* q;

    MdaBase::MdaBaseDataType m_dtype=MdaBase::Float;
    float* m_data_32=0;
    double* m_data_64=0;
    long m_dims[MDA_MAX_DIMS];
    long m_total_size=0;

    void do_construct();
    void free_data();
    int determine_num_dims(long N1, long N2, long N3, long N4, long N5, long N6);
    bool safe_index(long i);
    bool safe_index(long i1, long i2);
    bool safe_index(long i1, long i2, long i3);
    bool safe_index(long i1, long i2, long i3, long i4, long i5, long i6);

    bool read_from_text_file(const QString& path);
    bool write_to_text_file(const QString& path);
};

MdaBase::MdaBase(MdaBaseDataType dtype, long N1, long N2, long N3, long N4, long N5, long N6)
{
    d = new MdaBasePrivate;
    d->q = this;
    d->m_dtype=dtype;
    d->do_construct();
    this->allocate(N1, N2, N3, N4, N5, N6);
}

MdaBase::MdaBase(MdaBaseDataType dtype, const QString mda_filename)
{
    d = new MdaBasePrivate;
    d->q = this;
    d->m_dtype=dtype;
    this->read(mda_filename);
}

MdaBase::~MdaBase()
{
    d->free_data();
    delete d;
}

bool MdaBase::allocate(MdaBase::MdaBaseDataType dtype, long N1, long N2, long N3, long N4, long N5, long N6)
{
    d->free_data();
    d->m_dtype=dtype;
    return allocate(N1,N2,N3,N4,N5,N6);
}

bool MdaBase::allocate(long N1, long N2, long N3, long N4, long N5, long N6)
{
    d->free_data();

    d->m_dims[0] = N1;
    d->m_dims[1] = N2;
    d->m_dims[2] = N3;
    d->m_dims[3] = N4;
    d->m_dims[4] = N5;
    d->m_dims[5] = N6;
    d->m_total_size = N1 * N2 * N3 * N4 * N5 * N6;

    if (d->m_total_size) {
        if (d->m_dtype==Float) {
            d->m_data_32 = (float*)::allocate(sizeof(float) * d->m_total_size);
            if (!d->m_data_32) {
                qCritical() << QString("Unable to allocate MdaBase of size %1x%2x%3x%4x%5x%6 (total=%7)").arg(N1).arg(N2).arg(N3).arg(N4).arg(N5).arg(N6).arg(d->m_total_size);
                exit(-1);
            }
            TaskManager::TaskProgressMonitor::globalInstance()->incrementQuantity("bytes_allocated", d->m_total_size*sizeof(float));
            for (long i = 0; i < d->m_total_size; i++)
                d->m_data_32[i] = 0;
        }
        else if (d->m_dtype==Double) {
            d->m_data_64 = (double*)::allocate(sizeof(double) * d->m_total_size);
            if (!d->m_data_64) {
                qCritical() << QString("Unable to allocate MdaBase of size %1x%2x%3x%4x%5x%6 (total=%7)").arg(N1).arg(N2).arg(N3).arg(N4).arg(N5).arg(N6).arg(d->m_total_size);
                exit(-1);
            }
            TaskManager::TaskProgressMonitor::globalInstance()->incrementQuantity("bytes_allocated", d->m_total_size*sizeof(float));
            for (long i = 0; i < d->m_total_size; i++)
                d->m_data_64[i] = 0;
        }
    }

    return true;
}

bool MdaBase::read(const QString& path)
{
    if ((QString(path).endsWith(".txt")) || (QString(path).endsWith(".csv"))) {
        return d->read_from_text_file(path);
    }
    FILE* input_file = fopen(path.toLatin1().data(), "rb");
    if (!input_file) {
        printf("Warning: Unable to open mda file for reading: %s\n", path.toLatin1().data());
        return false;
    }
    MDAIO_HEADER H;
    if (!mda_read_header(&H, input_file)) {
        qWarning() << "Problem reading mda file: " + QString(path);
        fclose(input_file);
        return false;
    }
    this->allocate(H.dims[0], H.dims[1], H.dims[2], H.dims[3], H.dims[4], H.dims[5]);
    if (d->m_dtype==Float) {
        if (d->m_data_32)
            mda_read_float32(d->m_data_32, &H, d->m_total_size, input_file);
    }
    else if (d->m_dtype==Double) {
        if (d->m_data_64)
            mda_read_float64(d->m_data_64, &H, d->m_total_size, input_file);
    }
    TaskManager::TaskProgressMonitor::globalInstance()->incrementQuantity("bytes_read", d->m_total_size * H.num_bytes_per_entry);
    fclose(input_file);
    return true;
}

bool MdaBase::write8(const QString& path) const
{
    if ((QString(path).endsWith(".txt")) || (QString(path).endsWith(".csv"))) {
        return d->write_to_text_file(path);
    }
    FILE* output_file = fopen(path.toLatin1().data(), "wb");
    if (!output_file) {
        printf("Warning: Unable to open mda file for writing: %s\n", path.toLatin1().data());
        return false;
    }
    MDAIO_HEADER H;
    H.data_type = MDAIO_TYPE_BYTE;
    H.num_bytes_per_entry = 1;
    for (int i = 0; i < MDAIO_MAX_DIMS; i++)
        H.dims[i] = 1;
    for (int i = 0; i < MDA_MAX_DIMS; i++)
        H.dims[i] = d->m_dims[i];
    H.num_dims = d->determine_num_dims(N1(), N2(), N3(), N4(), N5(), N6());
    mda_write_header(&H, output_file);
    if (d->m_dtype==Float) {
        if (d->m_data_32)
            mda_write_float32(d->m_data_32, &H, d->m_total_size, output_file);
    }
    else if (d->m_dtype==Double) {
        if (d->m_data_64)
            mda_write_float64(d->m_data_64, &H, d->m_total_size, output_file);
    }
    TaskManager::TaskProgressMonitor::globalInstance()->incrementQuantity("bytes_written", d->m_total_size * H.num_bytes_per_entry);
    fclose(output_file);
    return true;
}

bool MdaBase::write32(const QString& path) const
{
    if ((QString(path).endsWith(".txt")) || (QString(path).endsWith(".csv"))) {
        return d->write_to_text_file(path);
    }
    FILE* output_file = fopen(path.toLatin1().data(), "wb");
    if (!output_file) {
        printf("Warning: Unable to open mda file for writing: %s\n", path.toLatin1().data());
        return false;
    }
    MDAIO_HEADER H;
    H.data_type = MDAIO_TYPE_FLOAT32;
    H.num_bytes_per_entry = 4;
    for (int i = 0; i < MDAIO_MAX_DIMS; i++)
        H.dims[i] = 1;
    for (int i = 0; i < MDA_MAX_DIMS; i++)
        H.dims[i] = d->m_dims[i];
    H.num_dims = d->determine_num_dims(N1(), N2(), N3(), N4(), N5(), N6());
    mda_write_header(&H, output_file);
    if (d->m_dtype==Float) {
        if (d->m_data_32)
            mda_write_float32(d->m_data_32, &H, d->m_total_size, output_file);
    }
    else if (d->m_dtype==Double) {
        if (d->m_data_64)
            mda_write_float64(d->m_data_64, &H, d->m_total_size, output_file);
    }
    TaskManager::TaskProgressMonitor::globalInstance()->incrementQuantity("bytes_written", d->m_total_size * H.num_bytes_per_entry);
    fclose(output_file);
    return true;
}

bool MdaBase::write64(const QString& path) const
{
    if ((QString(path).endsWith(".txt")) || (QString(path).endsWith(".csv"))) {
        return d->write_to_text_file(path);
    }
    FILE* output_file = fopen(path.toLatin1().data(), "wb");
    if (!output_file) {
        printf("Warning: Unable to open mda file for writing: %s\n", path.toLatin1().data());
        return false;
    }
    MDAIO_HEADER H;
    H.data_type = MDAIO_TYPE_FLOAT64;
    H.num_bytes_per_entry = 4;
    for (int i = 0; i < MDAIO_MAX_DIMS; i++)
        H.dims[i] = 1;
    for (int i = 0; i < MDA_MAX_DIMS; i++)
        H.dims[i] = d->m_dims[i];
    H.num_dims = d->determine_num_dims(N1(), N2(), N3(), N4(), N5(), N6());
    mda_write_header(&H, output_file);
    if (d->m_dtype==Float) {
        if (d->m_data_32)
            mda_write_float32(d->m_data_32, &H, d->m_total_size, output_file);
    }
    else if (d->m_dtype==Double) {
        if (d->m_data_64)
            mda_write_float64(d->m_data_64, &H, d->m_total_size, output_file);
    }
    TaskManager::TaskProgressMonitor::globalInstance()->incrementQuantity("bytes_written", d->m_total_size * H.num_bytes_per_entry);
    fclose(output_file);
    return true;
}

QByteArray MdaBase::toByteArray8() const
{
    QString path = CacheManager::globalInstance()->makeLocalFile("", CacheManager::ShortTerm);
    write8(path);
    return MLUtil::readByteArray(path);
}

QByteArray MdaBase::toByteArray32() const
{
    QString path = CacheManager::globalInstance()->makeLocalFile("", CacheManager::ShortTerm);
    write32(path);
    return MLUtil::readByteArray(path);
}

QByteArray MdaBase::toByteArray64() const
{
    QString path = CacheManager::globalInstance()->makeLocalFile("", CacheManager::ShortTerm);
    write64(path);
    return MLUtil::readByteArray(path);
}

bool MdaBase::fromByteArray(const QByteArray& X)
{
    QString path = CacheManager::globalInstance()->makeLocalFile("", CacheManager::ShortTerm);
    MLUtil::writeByteArray(path, X);
    return this->read(path);
}

MdaBase::MdaBaseDataType MdaBase::dataType() const
{
    return d->m_dtype;
}

int MdaBase::ndims() const
{
    return d->determine_num_dims(N1(), N2(), N3(), N4(), N5(), N6());
}

long MdaBase::N1() const
{
    return d->m_dims[0];
}

long MdaBase::N2() const
{
    return d->m_dims[1];
}

long MdaBase::N3() const
{
    return d->m_dims[2];
}

long MdaBase::N4() const
{
    return d->m_dims[3];
}

long MdaBase::N5() const
{
    return d->m_dims[4];
}

long MdaBase::N6() const
{
    return d->m_dims[5];
}

long MdaBase::totalSize() const
{
    return d->m_total_size;
}

long MdaBase::size(int dimension_index) const
{
    if (dimension_index < 0)
        return 0;
    if (dimension_index >= MDA_MAX_DIMS)
        return 1;
    return d->m_dims[dimension_index];
}

float MdaBase::get32(long i) const
{
    return d->m_data_32[i];
}

float MdaBase::get32(long i1, long i2) const
{
    return d->m_data_32[i1 + d->m_dims[0] * i2];
}

float MdaBase::get32(long i1, long i2, long i3) const
{
    return d->m_data_32[i1 + d->m_dims[0] * i2 + d->m_dims[0] * d->m_dims[1] * i3];
}

float MdaBase::get32(long i1, long i2, long i3, long i4, long i5, long i6) const
{
    return d->m_data_32[i1 + d->m_dims[0] * i2 + d->m_dims[0] * d->m_dims[1] * i3 + d->m_dims[0] * d->m_dims[1] * d->m_dims[2] * i4 + d->m_dims[0] * d->m_dims[1] * d->m_dims[2] * d->m_dims[3] * i5 + d->m_dims[0] * d->m_dims[1] * d->m_dims[2] * d->m_dims[3] * d->m_dims[4] * i6];
}

double MdaBase::get64(long i) const
{
    return d->m_data_64[i];
}

double MdaBase::get64(long i1, long i2) const
{
    return d->m_data_64[i1 + d->m_dims[0] * i2];
}

double MdaBase::get64(long i1, long i2, long i3) const
{
    return d->m_data_64[i1 + d->m_dims[0] * i2 + d->m_dims[0] * d->m_dims[1] * i3];
}

double MdaBase::get64(long i1, long i2, long i3, long i4, long i5, long i6) const
{
    return d->m_data_64[i1 + d->m_dims[0] * i2 + d->m_dims[0] * d->m_dims[1] * i3 + d->m_dims[0] * d->m_dims[1] * d->m_dims[2] * i4 + d->m_dims[0] * d->m_dims[1] * d->m_dims[2] * d->m_dims[3] * i5 + d->m_dims[0] * d->m_dims[1] * d->m_dims[2] * d->m_dims[3] * d->m_dims[4] * i6];
}

double MdaBase::value(long i) const
{
    if (!d->safe_index(i))
        return 0;
    if (d->m_dtype==Float)
        return get32(i);
    else if (d->m_dtype==Double)
        return get64(i);
    else return 0;
}

double MdaBase::value(long i1, long i2) const
{
    if (!d->safe_index(i1, i2))
        return 0;
    if (d->m_dtype==Float)
        return get32(i1,i2);
    else if (d->m_dtype==Double)
        return get64(i1,i2);
    else return 0;
}

double MdaBase::value(long i1, long i2, long i3) const
{
    if (!d->safe_index(i1, i2, i3))
        return 0;
    if (d->m_dtype==Float)
        return get32(i1,i2,i3);
    else if (d->m_dtype==Double)
        return get64(i1,i2,i3);
    else return 0;
}

double MdaBase::value(long i1, long i2, long i3, long i4, long i5, long i6) const
{
    if (!d->safe_index(i1, i2, i3, i4, i5, i6))
        return 0;
    if (d->m_dtype==Float)
        return get32(i1,i2,i3,i4,i5,i6);
    else if (d->m_dtype==Double)
        return get64(i1,i2,i3,i4,i5,i6);
    else return 0;
}

void MdaBase::setValue(double val, long i)
{
    if (!d->safe_index(i))
        return;
    if (d->m_dtype==Float)
        set32(val, i);
    else if (d->m_dtype==Double)
        set64(val,i);
}

void MdaBase::setValue(double val, long i1, long i2)
{
    if (!d->safe_index(i1, i2))
        return;
    if (d->m_dtype==Float)
        set32(val, i1,i2);
    else if (d->m_dtype==Double)
        set64(val,i1,i2);
}

void MdaBase::setValue(double val, long i1, long i2, long i3)
{
    if (!d->safe_index(i1, i2, i3))
        return;
    if (d->m_dtype==Float)
        set32(val, i1,i2,i3);
    else if (d->m_dtype==Double)
        set64(val,i1,i2,i3);
}

void MdaBase::setValue(double val, long i1, long i2, long i3, long i4, long i5, long i6)
{
    if (!d->safe_index(i1, i2, i3, i4, i5, i6))
        return;
    if (d->m_dtype==Float)
        set32(val, i1,i2,i3,i4,i5,i6);
    else if (d->m_dtype==Double)
        set64(val,i1,i2,i3,i4,i5,i6);
}

float* MdaBase::dataPtr32()
{
    if (d->m_dtype!=Float) {
        qCritical() << "Invalid access to m_data_32" << __FILE__ << __LINE__;
        exit(-1);
    }
    return d->m_data_32;
}

const float* MdaBase::constDataPtr32() const
{
    if (d->m_dtype!=Float) {
        qCritical() << "Invalid access to m_data_32" << __FILE__ << __LINE__;
        exit(-1);
    }
    return d->m_data_32;
}

float* MdaBase::dataPtr32(long i)
{
    if (d->m_dtype!=Float) {
        qCritical() << "Invalid access to m_data_32" << __FILE__ << __LINE__;
        exit(-1);
    }
    return &d->m_data_32[i];
}

float* MdaBase::dataPtr32(long i1, long i2)
{
    if (d->m_dtype!=Float) {
        qCritical() << "Invalid access to m_data_32" << __FILE__ << __LINE__;
        exit(-1);
    }
    return &d->m_data_32[i1 + N1() * i2];
}

float* MdaBase::dataPtr32(long i1, long i2, long i3)
{
    if (d->m_dtype!=Float) {
        qCritical() << "Invalid access to m_data_32" << __FILE__ << __LINE__;
        exit(-1);
    }
    return &d->m_data_32[i1 + N1() * i2 + N1() * N2() * i3];
}

float* MdaBase::dataPtr32(long i1, long i2, long i3, long i4, long i5, long i6)
{
    if (d->m_dtype!=Float) {
        qCritical() << "Invalid access to m_data_32" << __FILE__ << __LINE__;
        exit(-1);
    }
    return &d->m_data_32[i1 + N1() * i2 + N1() * N2() * i3 + N1() * N2() * N3() * i4 + N1() * N2() * N3() * N4() * i5 + N1() * N2() * N3() * N4() * N5() * i6];
}

double* MdaBase::dataPtr64()
{
    if (d->m_dtype!=Double) {
        qCritical() << "Invalid access to m_data_64" << __FILE__ << __LINE__;
        exit(-1);
    }
    return d->m_data_64;
}

const double* MdaBase::constDataPtr64() const
{
    if (d->m_dtype!=Double) {
        qCritical() << "Invalid access to m_data_64" << __FILE__ << __LINE__;
        exit(-1);
    }
    return d->m_data_64;
}

double* MdaBase::dataPtr64(long i)
{
    if (d->m_dtype!=Double) {
        qCritical() << "Invalid access to m_data_64" << __FILE__ << __LINE__;
        exit(-1);
    }
    return &d->m_data_64[i];
}

double* MdaBase::dataPtr64(long i1, long i2)
{
    if (d->m_dtype!=Double) {
        qCritical() << "Invalid access to m_data_64" << __FILE__ << __LINE__;
        exit(-1);
    }
    return &d->m_data_64[i1 + N1() * i2];
}

double* MdaBase::dataPtr64(long i1, long i2, long i3)
{
    if (d->m_dtype!=Double) {
        qCritical() << "Invalid access to m_data_64" << __FILE__ << __LINE__;
        exit(-1);
    }
    return &d->m_data_64[i1 + N1() * i2 + N1() * N2() * i3];
}

double* MdaBase::dataPtr64(long i1, long i2, long i3, long i4, long i5, long i6)
{
    if (d->m_dtype!=Double) {
        qCritical() << "Invalid access to m_data_64" << __FILE__ << __LINE__;
        exit(-1);
    }
    return &d->m_data_64[i1 + N1() * i2 + N1() * N2() * i3 + N1() * N2() * N3() * i4 + N1() * N2() * N3() * N4() * i5 + N1() * N2() * N3() * N4() * N5() * i6];
}

void MdaBase::getChunk(MdaBase& ret, long i, long size)
{
    // A lot of bugs fixed on 5/31/16
    long a_begin = i;
    long x_begin = 0;
    long a_end = i + size - 1;
    long x_end = size - 1;

    if (a_begin < 0) {
        x_begin += 0 - a_begin;
        a_begin += 0 - a_begin;
    }
    if (a_end >= d->m_total_size) {
        x_end += d->m_total_size - 1 - a_end;
        a_end += d->m_total_size - 1 - a_end;
    }

    ret.allocate(d->m_dtype, 1, size);

    if (d->m_dtype==Float) {
        float* ptr1 = this->dataPtr32();
        float* ptr2 = ret.dataPtr32();

        long ii = 0;
        for (long a = a_begin; a <= a_end; a++) {
            ptr2[x_begin + ii] = ptr1[a_begin + ii];
            ii++; //it was a bug that this was left out, fixed on 5/31/16 by jfm
        }
    }
    else {
        double* ptr1 = this->dataPtr64();
        double* ptr2 = ret.dataPtr64();

        long ii = 0;
        for (long a = a_begin; a <= a_end; a++) {
            ptr2[x_begin + ii] = ptr1[a_begin + ii];
            ii++; //it was a bug that this was left out, fixed on 5/31/16 by jfm
        }
    }
}

void MdaBase::getChunk(MdaBase& ret, long i1, long i2, long size1, long size2)
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

    if (d->m_dtype==Float) {
        float* ptr1 = this->dataPtr32();
        float* ptr2 = ret.dataPtr32();

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
    else if (d->m_dtype==Double) {
        double* ptr1 = this->dataPtr64();
        double* ptr2 = ret.dataPtr64();

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
}

void MdaBase::getChunk(MdaBase& ret, long i1, long i2, long i3, long size1, long size2, long size3)
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

    if (d->m_dtype==Float) {
        float* ptr1 = this->dataPtr32();
        float* ptr2 = ret.dataPtr32();

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
    else if (d->m_dtype==Double) {
        double* ptr1 = this->dataPtr64();
        double* ptr2 = ret.dataPtr64();

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
}

void MdaBase::setChunk(MdaBase& X, long i)
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
    if (a_end >= d->m_total_size) {
        a_end += d->m_total_size - 1 - a_end;
        x_end += d->m_total_size - 1 - a_end;
    }

    if (d->m_dtype==Float) {
        float* ptr1 = this->dataPtr32();
        float* ptr2 = X.dataPtr32();

        long ii = 0;
        for (long a = a_begin; a <= a_end; a++) {
            ptr1[a_begin + ii] = ptr2[x_begin + ii];
        }
    }
    else if (d->m_dtype==Double) {
        double* ptr1 = this->dataPtr64();
        double* ptr2 = X.dataPtr64();

        long ii = 0;
        for (long a = a_begin; a <= a_end; a++) {
            ptr1[a_begin + ii] = ptr2[x_begin + ii];
        }
    }
}

void MdaBase::setChunk(MdaBase& X, long i1, long i2)
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

    if (d->m_dtype==Float) {
        float* ptr1 = this->dataPtr32();
        float* ptr2 = X.dataPtr32();

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
    else if (d->m_dtype==Double) {
        double* ptr1 = this->dataPtr64();
        double* ptr2 = X.dataPtr64();

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
}

void MdaBase::setChunk(MdaBase& X, long i1, long i2, long i3)
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

    if (d->m_dtype==Float) {
        float* ptr1 = this->dataPtr32();
        float* ptr2 = X.dataPtr32();

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
    else if (d->m_dtype==Double) {
        double* ptr1 = this->dataPtr64();
        double* ptr2 = X.dataPtr64();

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
}

double MdaBase::minimum() const
{
    long NN = this->totalSize();
    if (d->m_dtype==Float) {
        const float* ptr = this->constDataPtr32();
        if ((!NN) || (!ptr)) {
            return 0;
        }
        double ret = ptr[0];
        for (long i = 0; i < NN; i++) {
            if (ptr[i] < ret)
                ret = ptr[i];
        }
        return ret;
    }
    else if (d->m_dtype==Double) {
        const double* ptr = this->constDataPtr64();
        if ((!NN) || (!ptr)) {
            return 0;
        }
        double ret = ptr[0];
        for (long i = 0; i < NN; i++) {
            if (ptr[i] < ret)
                ret = ptr[i];
        }
        return ret;
    }
    else return 0;
}

double MdaBase::maximum() const
{
    long NN = this->totalSize();
    if (d->m_dtype==Float) {
        const float* ptr = this->constDataPtr32();
        if ((!NN) || (!ptr)) {
            return 0;
        }
        double ret = ptr[0];
        for (long i = 0; i < NN; i++) {
            if (ptr[i] > ret)
                ret = ptr[i];
        }
        return ret;
    }
    else if (d->m_dtype==Double) {
        const double* ptr = this->constDataPtr64();
        if ((!NN) || (!ptr)) {
            return 0;
        }
        double ret = ptr[0];
        for (long i = 0; i < NN; i++) {
            if (ptr[i] > ret)
                ret = ptr[i];
        }
        return ret;
    }
    else return 0;
}

bool MdaBase::reshape(int N1b, int N2b, int N3b, int N4b, int N5b, int N6b)
{
    if (N1b * N2b * N3b * N4b * N5b * N6b != this->totalSize()) {
        qWarning() << "Unable to reshape MdaBase, wrong total size";
        qWarning() << N1b << N2b << N3b << N4b << N5b << N6b;
        qWarning() << N1() << N2() << N3() << N4() << N5() << N6();
        return false;
    }
    d->m_dims[0] = N1b;
    d->m_dims[1] = N2b;
    d->m_dims[2] = N3b;
    d->m_dims[3] = N4b;
    d->m_dims[4] = N5b;
    d->m_dims[5] = N6b;
    return true;
}

void MdaBase::set32(float val, long i)
{
    d->m_data_32[i] = val;
}

void MdaBase::set32(float val, long i1, long i2)
{
    d->m_data_32[i1 + d->m_dims[0] * i2] = val;
}

void MdaBase::set32(float val, long i1, long i2, long i3)
{
    d->m_data_32[i1 + d->m_dims[0] * i2 + d->m_dims[0] * d->m_dims[1] * i3] = val;
}

void MdaBase::set32(float val, long i1, long i2, long i3, long i4, long i5, long i6)
{
    d->m_data_32[i1 + d->m_dims[0] * i2 + d->m_dims[0] * d->m_dims[1] * i3 + d->m_dims[0] * d->m_dims[1] * d->m_dims[2] * i4 + d->m_dims[0] * d->m_dims[1] * d->m_dims[2] * d->m_dims[3] * i5 + d->m_dims[0] * d->m_dims[1] * d->m_dims[2] * d->m_dims[3] * d->m_dims[4] * i6]
        = val;
}

void MdaBase::set64(double val, long i)
{
    d->m_data_64[i] = val;
}

void MdaBase::set64(double val, long i1, long i2)
{
    d->m_data_64[i1 + d->m_dims[0] * i2] = val;
}

void MdaBase::set64(double val, long i1, long i2, long i3)
{
    d->m_data_64[i1 + d->m_dims[0] * i2 + d->m_dims[0] * d->m_dims[1] * i3] = val;
}

void MdaBase::set64(double val, long i1, long i2, long i3, long i4, long i5, long i6)
{
    d->m_data_64[i1 + d->m_dims[0] * i2 + d->m_dims[0] * d->m_dims[1] * i3 + d->m_dims[0] * d->m_dims[1] * d->m_dims[2] * i4 + d->m_dims[0] * d->m_dims[1] * d->m_dims[2] * d->m_dims[3] * i5 + d->m_dims[0] * d->m_dims[1] * d->m_dims[2] * d->m_dims[3] * d->m_dims[4] * i6]
        = val;
}

void MdaBasePrivate::do_construct()
{
    for (int i = 0; i < MDA_MAX_DIMS; i++) {
        m_dims[i] = 0;
    }
}

void MdaBasePrivate::free_data()
{
    if (m_data_32) {
        free(m_data_32);
        m_data_32=0;
        TaskManager::TaskProgressMonitor::globalInstance()->incrementQuantity("bytes_freed", m_total_size*4);
    }
    if (m_data_64) {
        free(m_data_64);
        m_data_64=0;
        TaskManager::TaskProgressMonitor::globalInstance()->incrementQuantity("bytes_freed", m_total_size*8);
    }
}

int MdaBasePrivate::determine_num_dims(long N1, long N2, long N3, long N4, long N5, long N6)
{
#ifdef QT_CORE_LIB
    Q_UNUSED(N1)
    Q_UNUSED(N2)
#endif
    if (N6 > 1)
        return 6;
    if (N5 > 1)
        return 5;
    if (N4 > 1)
        return 4;
    if (N3 > 1)
        return 3;
    return 2;
}

bool MdaBasePrivate::safe_index(long i)
{
    return ((0 <= i) && (i < m_total_size));
}

bool MdaBasePrivate::safe_index(long i1, long i2)
{
    return ((0 <= i1) && (i1 < m_dims[0]) && (0 <= i2) && (i2 < m_dims[1]));
}

bool MdaBasePrivate::safe_index(long i1, long i2, long i3)
{
    return ((0 <= i1) && (i1 < m_dims[0]) && (0 <= i2) && (i2 < m_dims[1]) && (0 <= i3) && (i3 < m_dims[2]));
}

bool MdaBasePrivate::safe_index(long i1, long i2, long i3, long i4, long i5, long i6)
{
    return (
        (0 <= i1) && (i1 < m_dims[0]) && (0 <= i2) && (i2 < m_dims[1]) && (0 <= i3) && (i3 < m_dims[2]) && (0 <= i4) && (i4 < m_dims[3]) && (0 <= i5) && (i5 < m_dims[4]) && (0 <= i6) && (i6 < m_dims[5]));
}

bool MdaBasePrivate::read_from_text_file(const QString& path)
{
    QString txt = TextFile::read(path);
    QStringList lines = txt.split("\n", QString::SkipEmptyParts);
    QStringList lines2;
    for (int i = 0; i < lines.count(); i++) {
        QString line = lines[i].trimmed();
        if (!line.isEmpty()) {
            if (i == 0) {
                //check whether this is a header line, if so, don't include it
                line = line.split(",", QString::SkipEmptyParts).join(" ");
                QList<QString> vals = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
                bool ok;
                vals.value(0).toDouble(&ok);
                if (ok) {
                    lines2 << line;
                }
            }
            else {
                lines2 << line;
            }
        }
    }
    for (int i = 0; i < lines2.count(); i++) {
        QString line = lines2[i].trimmed();
        line = line.split(",", QString::SkipEmptyParts).join(" ");
        QList<QString> vals = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
        if (i == 0) {
            q->allocate(vals.count(), lines2.count());
        }
        for (int j = 0; j < vals.count(); j++) {
            q->setValue(vals[j].toDouble(), j, i);
        }
    }
    return true;
}

bool MdaBasePrivate::write_to_text_file(const QString& path)
{
    char sep = ' ';
    if (path.endsWith(".csv"))
        sep = ',';
    long max_num_entries = 1e6;
    if (q->N1() * q->N2() == max_num_entries) {
        qWarning() << "mda is too large to write text file";
        return false;
    }
    QList<QString> lines;
    for (long i = 0; i < q->N2(); i++) {
        QStringList vals;
        for (long j = 0; j < q->N1(); j++) {
            vals << QString("%1").arg(q->value(j, i));
        }
        QString line = vals.join(sep);
        lines << line;
    }
    return TextFile::write(path, lines.join("\n"));
}

