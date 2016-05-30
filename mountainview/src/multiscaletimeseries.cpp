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
#include <sys/stat.h>

class MultiScaleTimeSeriesPrivate {
public:
    MultiScaleTimeSeries* q;

    DiskReadMda m_data;

    QString get_multiscale_fname();
    bool create_multiscale_file(const QString& mspath);
    void downsample_min(DiskReadMda& source, long source_offset, long source_size, DiskWriteMda& dest, long dest_offset);
    void downsample_max(DiskReadMda& source, long source_offset, long source_size, DiskWriteMda& dest, long dest_offset);
    static long smallest_power_of_3_larger_than(long N);
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
}

bool MultiScaleTimeSeries::getData(Mda& min, Mda& max, long t1, long t2, long ds_factor)
{
    long M = d->m_data.N1();
    long N = MultiScaleTimeSeriesPrivate::smallest_power_of_3_larger_than(d->m_data.N2());

    if (ds_factor == 1) {
        d->m_data.readChunk(min, 0, t1, M, t2 - t1 + 1);
        max = min;
        return true;
    }

    //check if power of 3
    double factor = ds_factor;
    while (factor > 1) {
        factor /= 3;
    }
    if (factor < 1) {
        qWarning() << "Invalid ds_factor: " + ds_factor;
        return false;
    }

    QString multiscale_fname = d->get_multiscale_fname();
    /// TODO also check whether multiscale file has the correct size (and/or dimensions). If not, then recreate it
    if (!QFile(multiscale_fname).exists()) {
        if (!d->create_multiscale_file(multiscale_fname)) {
            qWarning() << "Unable to create multiscale file";
            return false;
        }
    }

    DiskReadMda Y(multiscale_fname);
    long t_offset_min = 0;
    long ds_factor_0 = 3;
    while (ds_factor_0 < ds_factor) {
        t_offset_min += 2 * (ceil(N * 1.0 / ds_factor_0));
        ds_factor_0 *= 3;
    }
    long t_offset_max = t_offset_min + ceil(N * 1.0 / ds_factor);

    Y.readChunk(min, 0, t1 + t_offset_min, M, t2 - t1 + 1);
    Y.readChunk(max, 0, t1 + t_offset_max, M, t2 - t1 + 1);

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
    Mda min0, max0;
    Z.getData(min0, max0, 0, 0, 3);

    QString out_path_min = CacheManager::globalInstance()->makeLocalFile("multiscaletimeseries_unit_test.min.mda");
    QString out_path_max = CacheManager::globalInstance()->makeLocalFile("multiscaletimeseries_unit_test.max.mda");

    min0.write32(out_path_min);
    max0.write32(out_path_max);

    return true;
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
    QString path = m_data.path();
    if (path.isEmpty()) {
        qWarning() << "Unable to get_multiscale_fname.... path is empty.";
        return "";
    }
    QString code = compute_hash(compute_file_code(path));
    return CacheManager::globalInstance()->makeLocalFile(code + ".multiscale.mda", CacheManager::ShortTerm);
}

bool MultiScaleTimeSeriesPrivate::create_multiscale_file(const QString& mspath)
{

    if (QFile::exists(mspath)) {
        if (!QFile::remove(mspath)) {
            qWarning() << "Unable to remove file: " + mspath;
            return false;
        }
    }

    long M = m_data.N1();
    long N = smallest_power_of_3_larger_than(m_data.N2());

    QString tmp_mspath = mspath + ".tmp";

    long NY = 0;
    for (long ds_factor = 3; ds_factor <= N; ds_factor *= 3) {
        NY += 2 * (ceil(N * 1.0 / ds_factor));
    }

    DiskWriteMda Y_write;
    if (!Y_write.open(MDAIO_TYPE_FLOAT32, tmp_mspath, M, NY)) {
        qWarning() << "Unable to open diskwritemda: " + mspath;
        return false;
    }

    DiskReadMda Y_read(tmp_mspath);

    long offset_min = 0;
    long offset_max = offset_min + (ceil(N * 1.0 / 3));
    long prev_offset_min;
    long prev_offset_max;
    long prev_N = N;
    for (long ds_factor = 3; ds_factor <= N; ds_factor *= 3) {
        if (ds_factor == 3) {
            downsample_min(m_data, 9, N, Y_write, offset_min);
            downsample_max(m_data, 0, N, Y_write, offset_max);
        }
        else {
            downsample_min(Y_read, prev_offset_min, prev_N, Y_write, offset_min);
            downsample_max(Y_read, prev_offset_max, prev_N, Y_write, offset_max);
        }
        prev_offset_min = offset_min;
        prev_offset_max = offset_max;
        prev_N = ceil(N * 1.0 / ds_factor);
        offset_min += 2 * (ceil(N * 1.0 / ds_factor));
        offset_max = offset_min + (ceil(N * 1.0 / ds_factor));
    }

    Y_write.close();

    if (!QFile::rename(tmp_mspath, mspath)) {
        qWarning() << "Unable to rename file: " + tmp_mspath + " to " + mspath;
        return false;
    }

    return true;
}

void MultiScaleTimeSeriesPrivate::downsample_min(DiskReadMda& source, long source_offset, long source_size, DiskWriteMda& dest, long dest_offset)
{
    long M = source.N1();
    /// TODO read this in chunks so as not to use RAM
    Mda X;
    source.readChunk(X, 0, source_offset, source.N1(), source_size);
    Mda X2(M, source_size / 3);
    for (int t = 0; t < source_size / 3; t++) {
        for (int m = 0; m < M; m++) {
            double val1 = X.value(m, t * 3);
            double val2 = X.value(m, t * 3 + 1);
            double val3 = X.value(m, t * 3 + 2);
            X2.setValue(qMin(qMin(val1, val2), val3), m, t);
        }
    }
    dest.writeChunk(X2, 0, dest_offset);
}

void MultiScaleTimeSeriesPrivate::downsample_max(DiskReadMda& source, long source_offset, long source_size, DiskWriteMda& dest, long dest_offset)
{
    long M = source.N1();
    /// TODO read this in chunks so as not to use RAM
    Mda X;
    source.readChunk(X, 0, source_offset, source.N1(), source_size);
    Mda X2(M, source_size / 3);
    for (int t = 0; t < source_size / 3; t++) {
        for (int m = 0; m < M; m++) {
            double val1 = X.value(m, t * 3);
            double val2 = X.value(m, t * 3 + 1);
            double val3 = X.value(m, t * 3 + 2);
            X2.setValue(qMax(qMax(val1, val2), val3), m, t);
        }
    }
    dest.writeChunk(X2, 0, dest_offset);
}

long MultiScaleTimeSeriesPrivate::smallest_power_of_3_larger_than(long N)
{
    long ret = 1;
    while (ret < N) {
        ret *= 3;
    }
    return ret;
}
