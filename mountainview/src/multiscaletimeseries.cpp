/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/27/2016
*******************************************************/

#include "multiscaletimeseries.h"
#include "cachemanager.h"
#include "msmisc.h"

#include <QFile>
#include <diskwritemda.h>

#define MSTS_TOTAL_CHUNK_SIZE 32 * 100

struct msts_chunk {
    //store timestep here??
    DiskReadMda data_min;
    DiskReadMda data_max;
};

class MultiScaleTimeSeriesPrivate {
public:
    MultiScaleTimeSeries* q;

    DiskReadMda m_data;
    long m_chunk_size;

    msts_chunk load_chunk(long ds_factor, long chunk_num);
    QString get_multiscale_fname();
    bool create_multiscale_file(const QString &mspath);
};

MultiScaleTimeSeries::MultiScaleTimeSeries()
{
    d = new MultiScaleTimeSeriesPrivate;
    d->q = this;
}

MultiScaleTimeSeries::~MultiScaleTimeSeries()
{
    delete d;
}

void MultiScaleTimeSeries::setData(const DiskReadMda& X)
{
    d->m_data = X;
    long M = X.N1();
    d->m_chunk_size = 100;
    while (d->m_chunk_size * M < MSTS_TOTAL_CHUNK_SIZE) {
        d->m_chunk_size *= 2;
    }
}

bool MultiScaleTimeSeries::getData(Mda& min, Mda& max, long t1, long t2, long ds_factor)
{
    long M=d->m_data.N1();
    long N=d->m_data.N2();

    if (ds_factor==1) {
        d->m_data.readChunk(min,0,t1,M,t2-t1+1);
        max=min;
        return true;
    }

    //check if power of 3
    double factor=ds_factor;
    while (factor>1) {
        factor/=3;
    }
    if (factor<1) {
        qWarning() << "Invalid ds_factor: "+ds_factor;
        return false;
    }

    QString multiscale_fname=d->get_multiscale_fname();
    if (!QFile(multiscale_fname).exists()) {
        if (!d->create_multiscale_file(multiscale_fname)) {
            qWarning() << "Unable to create multiscale file";
            return false;
        }
    }

    DiskReadMda Y(multiscale_fname);
    long t_offset_min=0;
    long ds_factor_0=3;
    while (ds_factor_0<ds_factor) {
        t_offset+=2*(ceil(N*1.0/ds_factor_0));
        ds_factor_0*=3;
    }
    long t_offset_max=t_offset_min+ceil(N*1.0/ds_factor);

    Y.readChunk(min,0,t1+t_offset_min,M,t2-t1+1);
    Y.readChunk(max,0,t1+t_offset_max,M,t2-t1+1);

    return true;
}

bool MultiScaleTimeSeries::unit_test(long M, long N)
{
    Mda X(M, N);
    for (long n = 0; n < N; n++) {
        for (long m = 0; m < M; m++) {
            X.setValue(sin(m + m * n), m, n);
        }
    }
    DiskReadMda Y(X);

    MultiScaleTimeSeries Z;
    Z.setData(Y);
    Mda min0,max0;
    Z.getData(min0,max0,0,0,3);

    QString out_path_min=CacheManager::globalInstance()->makeLocalFile("multiscaletimeseries_unit_test.min.mda");
    QString out_path_max=CacheManager::globalInstance()->makeLocalFile("multiscaletimeseries_unit_test.max.mda");

    min0.write32(out_path_min);
    max0.write32(out_path_max);

    return true;
}

msts_chunk MultiScaleTimeSeriesPrivate::load_chunk(long ds_factor, long chunk_num)
{
    if (ds_factor < 1)
        return msts_chunk(); //should not happen

    long M = m_data.N1();

    QString file_code = "fixme";
    QString code = QString("%1-ds%2-ch%3-cs%4").arg(file_code).arg(ds_factor).arg(chunk_num).arg(m_chunk_size);

    QString path_min = CacheManager::globalInstance()->makeLocalFile(code + ".min.msts.mda");
    QString path_max = CacheManager::globalInstance()->makeLocalFile(code + ".max.msts.mda");

    if ((!QFile::exists(path_min)) || (!QFile::exists(path_max))) {
        Mda min0(M, m_chunk_size);
        Mda max0(M, m_chunk_size);

        if (ds_factor == 1) {
            m_data.readChunk(min0, m_chunk_size * chunk_num, 0, m_chunk_size, M);
            m_data.readChunk(max0, m_chunk_size * chunk_num, 0, m_chunk_size, M);
        }
        else {
            msts_chunk chunk1 = load_chunk(ds_factor / 2, chunk_num * 2);
            msts_chunk chunk2 = load_chunk(ds_factor / 2, chunk_num * 2 + 1);

            for (long i = 0; i < m_chunk_size / 2; i++) {
                for (int m = 0; m < M; m++) {
                    min0.setValue(qMin(chunk1.data_min.value(m, i * 2), chunk1.data_min.value(m, i * 2 + 1)), m, i);
                    max0.setValue(qMax(chunk1.data_max.value(m, i * 2), chunk2.data_max.value(m, i * 2 + 1)), m, i);
                }
            }
            for (long i = m_chunk_size / 2; i < m_chunk_size; i++) {
                long j = i - m_chunk_size / 2;
                for (int m = 0; m < M; m++) {
                    min0.setValue(qMin(chunk2.data_min.value(m, j * 2), chunk2.data_min.value(m, j / 2 + 1)), m, i);
                    max0.setValue(qMax(chunk2.data_max.value(m, j * 2), chunk2.data_max.value(m, j / 2 + 1)), m, i);
                }
            }
        }

        min0.write32(path_min);
        max0.write32(path_max);
    }

    msts_chunk X;
    X.data_min.setPath(path_min);
    X.data_max.setPath(path_max);

    return X;
}

QString compute_file_code(const QString& path)
{
    //the code comprises the device,inode,size, and modification time (in seconds)
    //note that it is not dependent on the file name
    struct stat SS;
    stat(path.toLatin1().data(), &SS);
    QString id_string = QString("%1:%2:%3:%4").arg(SS.st_dev).arg(SS.st_ino).arg(SS.st_size).arg(SS.st_mtim.tv_sec);
    return id_string;
}


QString MultiScaleTimeSeriesPrivate::get_multiscale_fname()
{
    QString path=m_data.path();
    if (path.isEmpty()) {
        qWarning() << "Unable to get_multiscale_fname.... path is empty.";
        return "";
    }
    QString code=compute_hash(compute_file_code(path));
    return CacheManager::globalInstance()->makeLocalFile(code+".multiscale.mda",ShortTerm);
}

bool MultiScaleTimeSeriesPrivate::create_multiscale_file(const QString &mspath)
{
    DiskWriteMda Y;
    /// TODO finish
}
