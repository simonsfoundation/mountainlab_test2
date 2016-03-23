#include "diskarraymodel_new.h"

#include <QDateTime>
#include <QFileInfo>
#include <diskwritemda.h>
#include "diskreadmda.h"

class DiskArrayModel_NewPrivate {
public:
    DiskArrayModel_New* q;
    QString m_path;
    DiskReadMda m_X;
    Mda m_memory_array;
    Mda m_multiscale_memory_array;
    bool m_use_memory_array;
    long m_dim3;
    void create_multiscale_array_if_needed();
    QString get_multiscale_file_name();
    QString get_file_timestamp(const QString& path);
};

DiskArrayModel_New::DiskArrayModel_New()
{
    d = new DiskArrayModel_NewPrivate;
    d->q = this;
    d->m_use_memory_array = false;
    d->m_dim3 = 1;
}

DiskArrayModel_New::~DiskArrayModel_New()
{
    delete d;
}

void DiskArrayModel_New::setPath(const QString& path)
{
    d->m_path = path;
    d->m_X.setPath(path);
    d->m_dim3 = d->m_X.N3();
    //TODO: reshape here!
    d->m_use_memory_array = false;
}

void DiskArrayModel_New::setFromMda(const Mda& X)
{
    d->m_memory_array = X;
    d->m_dim3 = X.N3();
    d->m_memory_array.reshape(X.N1(), X.N2() * X.N3());
    d->m_use_memory_array = true;
    d->m_multiscale_memory_array.allocate(1, 1);
}

long get_multiscale_position(long scale, long N)
{
    long ret = 0;
    for (long s = 2; s < scale; s *= 2) {
        ret += (N / s) * 2;
    }
    return ret;
}

long round_up_to_power_of_two(long N)
{
    long ret = 1;
    while (ret < N)
        ret *= 2;
    return ret;
}

Mda DiskArrayModel_New::loadData(long scale, long t1, long t2)
{
    d->create_multiscale_array_if_needed();
    long N = round_up_to_power_of_two(N2());
    long pos = get_multiscale_position(scale, N);

    if (d->m_use_memory_array) {
        Mda chunk;
        if (scale == 1) {
            d->m_memory_array.getChunk(chunk, 0, pos + t1, N1(), (t2 - t1 + 1));
        } else {
            d->m_multiscale_memory_array.getChunk(chunk, 0, pos + t1 * 2, N1(), (t2 - t1 + 1) * 2);
        }
        return chunk;
    } else {

        Mda chunk;
        if (scale == 1) {
            d->m_X.readChunk(chunk, 0, pos + t1, N1(), (t2 - t1 + 1));
        } else {
            DiskReadMda A(d->get_multiscale_file_name());
            A.readChunk(chunk, 0, pos + t1 * 2, N1(), (t2 - t1 + 1) * 2);
        }
        return chunk;
    }
}

long DiskArrayModel_New::N1()
{
    if (d->m_use_memory_array)
        return d->m_memory_array.N1();
    return d->m_X.N1();
}

long DiskArrayModel_New::N2()
{
    if (d->m_use_memory_array)
        return d->m_memory_array.N2();
    return d->m_X.N2();
}

long DiskArrayModel_New::dim3()
{
    return d->m_dim3;
}

void DiskArrayModel_New::createFileHierarchyIfNeeded()
{
    d->create_multiscale_array_if_needed();
}

Mda downsample_1(Mda chunk)
{
    long M = chunk.N1();
    Mda ret(M, chunk.N2());
    long aa = 0;
    long bb = 0;
    for (long i = 0; i + 1 < chunk.N2(); i += 2) {
        for (long m = 0; m < M; m++) {
            double val1 = chunk.get(aa);
            double val2 = chunk.get(aa + M);
            ret.set(qMin(val1, val2), bb);
            ret.set(qMax(val1, val2), bb + M);
            aa++;
            bb++;
        }
        aa += M;
        bb += M;
    }
    return ret;
}
Mda downsample_2(Mda chunk)
{
    long M = chunk.N1();
    Mda ret(M, chunk.N2() / 2);
    long aa = 0;
    long bb = 0;
    for (long i = 0; i + 3 < chunk.N2(); i += 4) {
        for (long m = 0; m < M; m++) {
            double val1 = chunk.get(aa);
            double val2 = chunk.get(aa + M);
            double val3 = chunk.get(aa + 2 * M);
            double val4 = chunk.get(aa + 3 * M);
            ret.set(qMin(val1, val3), bb);
            ret.set(qMax(val2, val4), bb + M);
            aa++;
            bb++;
        }
        aa += M * 3;
        bb += M;
    }
    return ret;
}

void DiskArrayModel_NewPrivate::create_multiscale_array_if_needed()
{
    long N = round_up_to_power_of_two(q->N2());

    if (m_use_memory_array) {
        if ((m_multiscale_memory_array.N1() == m_memory_array.N1()) && (m_multiscale_memory_array.N2() == N * 2)) {
            return;
        }
        m_multiscale_memory_array.allocate(m_memory_array.N1(), N * 2);
        Mda chunk;
        m_memory_array.getChunk(chunk, 0, 0, m_memory_array.N1(), N);
        Mda chunk2;
        chunk2 = downsample_1(chunk);
        for (long scale = 2; scale <= N / 2; scale *= 2) {
            long pos = get_multiscale_position(scale, N);
            m_multiscale_memory_array.setChunk(chunk2, 0, pos);
            chunk2 = downsample_2(chunk2);
        }
        return;
    }

    QString fname = get_multiscale_file_name();
    long multiscale_file_size = N * 2;
    if (QFile::exists(fname)) {
        DiskReadMda X2(fname);
        if ((X2.N1() == m_X.N1()) && (X2.N2() == multiscale_file_size)) {
            return;
        }
    }

    DiskWriteMda Y;
    Y.open(MDAIO_TYPE_FLOAT32, fname, m_X.N1(), multiscale_file_size);
    Mda chunk;
    m_X.readChunk(chunk, 0, 0, m_X.N1(), N);
    Mda chunk2;
    chunk2 = downsample_1(chunk);
    for (long scale = 2; scale <= N / 2; scale *= 2) {
        long pos = get_multiscale_position(scale, N);
        Y.writeChunk(chunk2, 0, pos);
        chunk2 = downsample_2(chunk2);
    }
    Y.close();
}

QString DiskArrayModel_NewPrivate::get_multiscale_file_name()
{
    return QFileInfo(m_path).path() + "/spikespy." + get_file_timestamp(m_path) + "." + QFileInfo(m_path).fileName();
}

QString DiskArrayModel_NewPrivate::get_file_timestamp(const QString& path)
{
    QDateTime dt = QFileInfo(path).lastModified();
    return QString("%1").arg(dt.toMSecsSinceEpoch());
}
