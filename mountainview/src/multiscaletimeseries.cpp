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
#include <taskprogress.h>
#include <sys/stat.h>
#include <QMutex>
#include <math.h>
#include "mountainprocessrunner.h"

class MultiScaleTimeSeriesPrivate {
public:
    MultiScaleTimeSeries* q;

    DiskReadMda m_data;
    DiskReadMda m_multiscale_data;
    QString m_ml_proxy_url;

    QString get_multiscale_fname();
    bool create_multiscale_file(const QString& mspath);

    static bool downsample_min(const DiskReadMda& X, QString out_fname, long N);
    static bool downsample_max(const DiskReadMda& X, QString out_fname, long N);
    static bool write_concatenation(QStringList input_fnames, QString output_fname);
    static bool is_power_of_3(long N);

    QMutex m_mutex;
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
    QMutexLocker locker(&d->m_mutex);
    d->m_data = X;
}

void MultiScaleTimeSeries::setMLProxyUrl(const QString& url)
{
    d->m_ml_proxy_url = url;
}

long MultiScaleTimeSeries::N1()
{
    QMutexLocker locker(&d->m_mutex);
    return d->m_data.N1();
}

long MultiScaleTimeSeries::N2()
{
    QMutexLocker locker(&d->m_mutex);
    return d->m_data.N2();
}

bool MultiScaleTimeSeries::getData(Mda& min, Mda& max, long t1, long t2, long ds_factor)
{
    QMutexLocker locker(&d->m_mutex);
    long M = d->m_data.N1();
    long N = MultiScaleTimeSeries::smallest_power_of_3_larger_than(d->m_data.N2());

    if (ds_factor == 1) {
        d->m_data.readChunk(min, 0, t1, M, t2 - t1 + 1);
        max = min;
        return true;
    }

    if (!d->is_power_of_3(ds_factor)) {
        qWarning() << "Invalid ds_factor: " + ds_factor;
        return false;
    }

    if (d->m_multiscale_data.path().isEmpty()) {
        d->m_multiscale_data.setPath(d->m_data.makePath() + ".multiscale");
        d->m_multiscale_data.setRemoteDataType("float32"); //to save download time!
        /*
        QString multiscale_fname = d->get_multiscale_fname();
        if (multiscale_fname.isEmpty()) {
            qWarning() << "Unable to create multiscale file";
            return false;
        }
        d->m_multiscale_data.setPath(multiscale_fname);
        d->m_multiscale_data.setRemoteDataType("float32"); //to save download time!
        */
    }

    long t_offset_min = 0;
    long ds_factor_0 = 3;
    while (ds_factor_0 < ds_factor) {
        t_offset_min += 2 * (N / ds_factor_0);
        ds_factor_0 *= 3;
    }
    long t_offset_max = t_offset_min + N / ds_factor;

    /// TODO what if t1 and t2 are out of bounds? I think we want to put zeros in ... otherwise contaminated by other downsampling factors

    d->m_multiscale_data.readChunk(min, 0, t1 + t_offset_min, M, t2 - t1 + 1);
    d->m_multiscale_data.readChunk(max, 0, t1 + t_offset_max, M, t2 - t1 + 1);

    return true;
}

bool MultiScaleTimeSeries::unit_test(long M, long N)
{
    QString fname1 = "tmp1.mda";
    QString fname_min = "tmp1_min.mda";
    QString fname_max = "tmp1_max.mda";

    //write an array
    Mda X(M, N);
    for (long n = 0; n < N; n++) {
        for (long m = 0; m < M; m++) {
            X.setValue(sin(m + m * n), m, n);
        }
    }
    X.write32(fname1);

    //load it as a diskreadmda
    DiskReadMda Y(fname1);

    //open it as a multiscale time series
    MultiScaleTimeSeries Z;
    Z.setData(Y);

    //get some data
    Mda min0, max0;
    Z.getData(min0, max0, 0, N - 1, 9);

    min0.write32(fname_min);
    max0.write32(fname_max);

    return true;
}

long MultiScaleTimeSeries::smallest_power_of_3_larger_than(long N)
{
    long ret = 1;
    while (ret < N) {
        ret *= 3;
    }
    return ret;
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
    QString path = m_data.makePath();
    if (path.isEmpty()) {
        qWarning() << "Unable to get_multiscale_fname.... path is empty.";
        return "";
    }

    if (path.startsWith("http:")) {
        MountainProcessRunner MPR;
        MPR.setProcessorName("create_multiscale_timeseries");
        QVariantMap params;
        params["timeseries"] = path;
        MPR.setInputParameters(params);
        MPR.setMLProxyUrl(m_ml_proxy_url);
        QString path_out = MPR.makeOutputFilePath("timeseries_out");
        MPR.runProcess(0);
        return path_out;
    } else {
        QString code = compute_hash(compute_file_code(path));
        QString ret = CacheManager::globalInstance()->makeLocalFile(code + ".multiscale.mda", CacheManager::ShortTerm);
        if (!QFile::exists(ret)) {
            if (!create_multiscale_file(ret))
                return "";
        }
        if (QFile::exists(ret))
            return ret;
        else
            return "";
    }
}

bool MultiScaleTimeSeriesPrivate::create_multiscale_file(const QString& mspath)
{
    TaskProgress task("Create multiscale file");
    task.log(mspath);
    if (QFile::exists(mspath)) {
        if (!QFile::remove(mspath)) {
            task.error("Unable to remove file: " + mspath);
            qWarning() << "Unable to remove file: " + mspath;
            return false;
        }
    }

    long N = MultiScaleTimeSeries::smallest_power_of_3_larger_than(m_data.N2());

    QStringList file_names;
    QString prev_min_fname;
    QString prev_max_fname;
    double progress_numer = 0;
    double progress_denom = N * 3 / 2; //3+9+27+...+N; //approximately
    for (long ds_factor = 3; ds_factor <= N; ds_factor *= 3) {
        task.setProgress(progress_numer / progress_denom);
        task.log(QString("ds_factor = %1").arg(ds_factor));
        QString min_fname = CacheManager::globalInstance()->makeLocalFile("", CacheManager::ShortTerm);
        QString max_fname = CacheManager::globalInstance()->makeLocalFile("", CacheManager::ShortTerm);
        if (ds_factor == 3) {
            if (!downsample_min(m_data, min_fname, N)) {
                task.error("Problem in downsample_min");
                return false;
            }
            if (!downsample_max(m_data, max_fname, N)) {
                task.error("Problem in downsample_max");
                return false;
            }
        } else {
            if (!downsample_min(DiskReadMda(prev_min_fname), min_fname, (N * 3) / ds_factor)) {
                task.error("Problem in downsample_min");
                return false;
            }
            if (!downsample_max(DiskReadMda(prev_max_fname), max_fname, (N * 3) / ds_factor)) {
                task.error("Problem in downsample_max");
                return false;
            }
        }
        file_names << min_fname;
        file_names << max_fname;
        prev_min_fname = min_fname;
        prev_max_fname = max_fname;
        progress_numer += N / ds_factor;
    }

    if (!write_concatenation(file_names, mspath)) {
        task.error("Problem in write_concatenation");
        return false;
    }
    task.log("Removing temporary files");
    foreach(QString fname, file_names)
    {
        QFile::remove(fname);
    }

    return true;
}

bool MultiScaleTimeSeriesPrivate::downsample_min(const DiskReadMda& X, QString out_fname, long N)
{
    /// TODO do this in chunks to avoid using RAM
    Mda input;
    if (!X.readChunk(input, 0, 0, X.N1(), N)) {
        qWarning() << "Problem reading chunk in downsample_min";
        return false;
    }
    Mda output(X.N1(), N / 3);
    for (long i = 0; i < N / 3; i++) {
        for (long m = 0; m < X.N1(); m++) {
            double val1 = input.value(m, i * 3);
            double val2 = input.value(m, i * 3 + 1);
            double val3 = input.value(m, i * 3 + 2);
            double val = qMin(qMin(val1, val2), val3);
            output.setValue(val, m, i);
        }
    }
    if (!output.write32(out_fname)) {
        qWarning() << "Problem write output file in downsample_min: " + out_fname;
        return false;
    }
    return true;
}

bool MultiScaleTimeSeriesPrivate::downsample_max(const DiskReadMda& X, QString out_fname, long N)
{
    /// TO DO do this in chunks to avoid using RAM
    Mda input;
    if (!X.readChunk(input, 0, 0, X.N1(), N)) {
        qWarning() << "Problem reading chunk in downsample_max";
        return false;
    }
    Mda output(X.N1(), N / 3);
    for (long i = 0; i < N / 3; i++) {
        for (long m = 0; m < X.N1(); m++) {
            double val1 = input.value(m, i * 3);
            double val2 = input.value(m, i * 3 + 1);
            double val3 = input.value(m, i * 3 + 2);
            double val = qMax(qMax(val1, val2), val3);
            output.setValue(val, m, i);
        }
    }
    if (!output.write32(out_fname)) {
        qWarning() << "Problem write output file in downsample_max: " + out_fname;
        return false;
    }
    return true;
}

bool MultiScaleTimeSeriesPrivate::write_concatenation(QStringList input_fnames, QString output_fname)
{
    long M = 1, N = 0;
    foreach(QString fname, input_fnames)
    {
        DiskReadMda X(fname);
        M = X.N1();
        N += X.N2();
    }
    DiskWriteMda Y;
    if (!Y.open(MDAIO_TYPE_FLOAT32, output_fname, M, N)) {
        qWarning() << "Unable to open output file: " + output_fname;
        return false;
    }
    long offset = 0;
    foreach(QString fname, input_fnames)
    {
        DiskReadMda X(fname);
        /// TODO do this in chunks so we don't use RAM
        Mda tmp;
        X.readChunk(tmp, 0, 0, M, X.N2());
        Y.writeChunk(tmp, 0, offset);
        offset += X.N2();
    }
    Y.close();

    return true;
}

bool MultiScaleTimeSeriesPrivate::is_power_of_3(long N)
{
    double val = N;
    while (val > 1) {
        val /= 3;
    }
    return (val == 1);
}
