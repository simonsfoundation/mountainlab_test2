#include "diskreadmda.h"
#include <stdio.h>
#include "mdaio.h"
#include <math.h>
#include <QFile>
#include <QCryptographicHash>
#include <QDir>
#include "cachemanager.h"
#include "taskprogress.h"
#include "diskreadmda_common_impl.h"
#ifdef USE_REMOTE_READ_MDA
#include "remotereadmda.h"
#endif

#define MAX_PATH_LEN 10000

/// TODO (LOW) make tmp directory with different name on server, so we can really test if it is doing the computation in the right place

class DiskReadMdaPrivate {
public:
    DiskReadMda* q;

    DiskReadMda_Common_Impl* dc;

    void copy_from(const DiskReadMda& other);
};

DiskReadMda::DiskReadMda(const QString& path)
{
    d = new DiskReadMdaPrivate;
    d->dc = new DiskReadMda_Common_Impl;
    d->q = this;
    dc = d->dc;
    dc->construct_and_clear();
    if (!path.isEmpty()) {
        this->setPath(path);
    }
}

DiskReadMda::DiskReadMda(const DiskReadMda& other)
{
    d = new DiskReadMdaPrivate;
    d->dc = new DiskReadMda_Common_Impl;
    d->q = this;
    dc = d->dc;
    dc->construct_and_clear();
    d->copy_from(other);
}

DiskReadMda::DiskReadMda(const Mda& X)
{
    d = new DiskReadMdaPrivate;
    d->dc = new DiskReadMda_Common_Impl;
    d->q = this;
    dc = d->dc;
    dc->construct_and_clear();
    dc->m_use_memory_mda_64 = true;
    dc->m_memory_mda_64 = X;
}

DiskReadMda::~DiskReadMda()
{
    if (dc->m_file) {
        fclose(dc->m_file);
    }
    delete dc;
    delete d;
}

void DiskReadMda::operator=(const DiskReadMda& other)
{
    d->copy_from(other);
}

void DiskReadMda::setPath(const QString& file_path)
{
    if (dc->m_file) {
        fclose(dc->m_file);
        dc->m_file = 0;
    }
    dc->construct_and_clear();

    if (file_path.startsWith("http://")) {
#ifdef USE_REMOTE_READ_MDA
        dc->m_use_remote_mda = true;
        dc->m_remote_mda.setPath(file_path);
#endif
    }
    else if ((file_path.endsWith(".txt")) || (file_path.endsWith(".csv"))) {
        Mda X(file_path);
        (*this) = X;
        return;
    }
    else {
        dc->m_path = file_path;
    }
}

void DiskReadMda::setRemoteDataType(const QString& dtype)
{
    dc->set_remote_data_type(dtype);
}

void DiskReadMda::setDownloadChunkSize(long size)
{
    dc->set_download_chunk_size(size);
}

long DiskReadMda::downloadChunkSize()
{
    return dc->download_chunk_size();
}

QString DiskReadMda::makePath() const
{
    return dc->make_path();
}

long DiskReadMda::N1() const
{
    return dc->dim(1);
}

long DiskReadMda::N2() const
{
    return dc->dim(2);
}

long DiskReadMda::N3() const
{
    return dc->dim(3);
}

long DiskReadMda::N4() const
{
    return dc->dim(4);
}

long DiskReadMda::N5() const
{
    return dc->dim(5);
}

long DiskReadMda::N6() const
{
    return dc->dim(6);
}

long DiskReadMda::totalSize() const
{
    return dc->total_size();
}

bool DiskReadMda::reshape(long N1b, long N2b, long N3b, long N4b, long N5b, long N6b)
{
    return dc->reshape(N1b, N2b, N3b, N4b, N5b, N6b);
}

DiskReadMda DiskReadMda::reshaped(long N1b, long N2b, long N3b, long N4b, long N5b, long N6b)
{
    long size_b = N1b * N2b * N3b * N4b * N5b * N6b;
    if (size_b != this->totalSize()) {
        qWarning() << "Cannot reshape because sizes do not match" << this->totalSize() << N1b << N2b << N3b << N4b << N5b << N6b;
        return (*this);
    }
    DiskReadMda ret = (*this);
    ret.reshape(N1b, N2b, N3b, N4b, N5b, N6b);
    return ret;
}

bool DiskReadMda::readChunk(Mda& X, long i, long size) const
{
    return dc->read_chunk_64(X, i, size);
}

bool DiskReadMda::readChunk(Mda& X, long i1, long i2, long size1, long size2) const
{
    return dc->read_chunk_64(X, i1, i2, size1, size2);
}

bool DiskReadMda::readChunk(Mda& X, long i1, long i2, long i3, long size1, long size2, long size3) const
{
    return dc->read_chunk_64(X, i1, i2, i3, size1, size2, size3);
}

double DiskReadMda::value(long i) const
{
    return dc->value64(i);
}

double DiskReadMda::value(long i1, long i2) const
{
    if (dc->m_use_memory_mda_64)
        return dc->m_memory_mda_64.value(i1, i2);
    if ((i1 < 0) || (i1 >= N1()))
        return 0;
    if ((i2 < 0) || (i2 >= N2()))
        return 0;
    return value(i1 + N1() * i2);
}

double DiskReadMda::value(long i1, long i2, long i3) const
{
    if (dc->m_use_memory_mda_64)
        return dc->m_memory_mda_64.value(i1, i2, i3);
    if ((i1 < 0) || (i1 >= N1()))
        return 0;
    if ((i2 < 0) || (i2 >= N2()))
        return 0;
    if ((i3 < 0) || (i3 >= N3()))
        return 0;
    return value(i1 + N1() * i2 + N1() * N2() * i3);
}

void DiskReadMdaPrivate::copy_from(const DiskReadMda& other)
{
    /// TODO (LOW) think about copying over additional information such as internal chunks

    dc->copy_from(other.dc);
}
