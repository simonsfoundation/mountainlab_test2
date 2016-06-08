/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/2/2016
*******************************************************/

#include "remotereadmda.h"
#include "diskreadmda.h"
#include <QStringList>
#include <QDir>
#include <QDateTime>
#include <taskprogress.h>
#include "cachemanager.h"
#include "msmisc.h"
#include "mlutils.h"

#define REMOTE_READ_MDA_CHUNK_SIZE 5e5

struct RemoteReadMdaInfo {
    long N1, N2, N3;
    QString checksum;
    QDateTime file_last_modified;
};

class RemoteReadMdaPrivate {
public:
    RemoteReadMda* q;

    QString m_path;
    RemoteReadMdaInfo m_info;
    bool m_info_downloaded;
    QString m_remote_datatype;
    long m_download_chunk_size;
    bool m_download_failed; //don't make excessive calls. Once we failed, that's it.

    void download_info_if_needed();
    QString download_chunk_at_index(long ii);
};

RemoteReadMda::RemoteReadMda(const QString& path)
{
    d = new RemoteReadMdaPrivate;
    d->q = this;
    d->m_remote_datatype = "float64";
    d->m_download_chunk_size = REMOTE_READ_MDA_CHUNK_SIZE;
    d->m_download_failed = false;
    this->setPath(path);
}

RemoteReadMda::RemoteReadMda(const RemoteReadMda& other)
{
    d = new RemoteReadMdaPrivate;
    d->q = this;
    this->setPath(other.d->m_path);
    d->m_info = other.d->m_info;
    d->m_info_downloaded = other.d->m_info_downloaded;
    d->m_remote_datatype = other.d->m_remote_datatype;
    d->m_download_chunk_size = other.d->m_download_chunk_size;
}

void RemoteReadMda::operator=(const RemoteReadMda& other)
{
    this->setPath(other.d->m_path);
    d->m_info = other.d->m_info;
    d->m_info_downloaded = other.d->m_info_downloaded;
    d->m_remote_datatype = other.d->m_remote_datatype;
    d->m_download_chunk_size = other.d->m_download_chunk_size;
}

RemoteReadMda::~RemoteReadMda()
{
    delete d;
}

void RemoteReadMda::setRemoteDataType(QString dtype)
{
    d->m_remote_datatype = dtype;
}

void RemoteReadMda::setDownloadChunkSize(long size)
{
    d->m_download_chunk_size = size;
}

long RemoteReadMda::downloadChunkSize()
{
    return d->m_download_chunk_size;
}

void RemoteReadMda::setPath(const QString& path)
{
    d->m_info.N1 = d->m_info.N2 = d->m_info.N3 = 0;
    d->m_info_downloaded = false;
    d->m_path = path;
}

QString RemoteReadMda::path() const
{
    return d->m_path;
}

long RemoteReadMda::N1() const
{
    d->download_info_if_needed();
    return d->m_info.N1;
}

long RemoteReadMda::N2() const
{
    d->download_info_if_needed();
    return d->m_info.N2;
}

long RemoteReadMda::N3() const
{
    d->download_info_if_needed();
    return d->m_info.N3;
}

QDateTime RemoteReadMda::fileLastModified() const
{
    d->download_info_if_needed();
    return d->m_info.file_last_modified;
}

bool RemoteReadMda::readChunk(Mda& X, long i, long size) const
{
    if (d->m_download_failed) {
        //don't make excessive calls... once we fail, that's it.
        return false;
    }
    //double size_mb = size * datatype_size * 1.0 / 1e6;
    //TaskProgress task(TaskProgress::Download, "Downloading array chunk");
    //if (size_mb > 0.5) {
    //    task.setLabel(QString("Downloading array chunk: %1 MB").arg(size_mb));
    //}
    //read a chunk of the remote array considered as a 1D array

    TaskProgress task(TaskProgress::Download, QString("Downloading %4 (%1x%2x%3)").arg(N1()).arg(N2()).arg(N3()).arg(d->m_remote_datatype));
    task.log(this->path());

    X.allocate(size, 1); //allocate the output array
    double* Xptr = X.dataPtr(); //pointer to the output data
    long ii1 = i; //start index of the remote array
    long ii2 = i + size - 1; //end index of the remote array
    long jj1 = ii1 / d->m_download_chunk_size; //start chunk index of the remote array
    long jj2 = ii2 / d->m_download_chunk_size; //end chunk index of the remote array
    if (jj1 == jj2) { //in this case there is only one chunk we need to worry about
        task.setProgress(0.2);
        QString fname = d->download_chunk_at_index(jj1); //download the single chunk
        if (fname.isEmpty()) {
            //task.error("fname is empty");
            if (!thread_interrupt_requested()) {
                TaskProgress errtask("Download chunk at index");
                errtask.log(QString("m_remote_data_type = %1, download chunk size = %2").arg(d->m_remote_datatype).arg(d->m_download_chunk_size));
                errtask.log(d->m_path);
                errtask.error(QString("Failed to download chunk at index %1").arg(jj1));
                d->m_download_failed = true;
            }
            return false;
        }
        DiskReadMda A(fname);
        A.readChunk(X, ii1 - jj1 * d->m_download_chunk_size, size); //starting reading at the offset of ii1 relative to the start index of the chunk
        return true;
    } else {
        for (long jj = jj1; jj <= jj2; jj++) { //otherwise we need to step through the chunks
            task.setProgress((jj - jj1 + 0.5) / (jj2 - jj1 + 1));
            if (thread_interrupt_requested()) {
                //X = Mda(); //maybe it's better to return the right size.
                //task.error("Halted");
                return false;
            }
            QString fname = d->download_chunk_at_index(jj); //download the chunk at index jj
            if (fname.isEmpty()) {
                //task.error("fname is empty *");
                return false;
            }
            DiskReadMda A(fname);
            if (jj == jj1) { //case 1/3, this is the first chunk
                Mda tmp;
                long size0 = (jj1 + 1) * d->m_download_chunk_size - ii1; //the size is going to be the difference between ii1 and the start index of the next chunk
                A.readChunk(tmp, ii1 - jj1 * d->m_download_chunk_size, size0); //again we start reading at the offset of ii1 relative to the start index of the chunk
                double* tmp_ptr = tmp.dataPtr(); //copy the data directly from tmp_ptr to Xptr
                long b = 0;
                for (long a = 0; a < size0; a++) {
                    Xptr[b] = tmp_ptr[a];
                    b++;
                }
            } else if (jj == jj2) { //case 2/3, this is the last chunk
                Mda tmp;
                long size0 = ii2 + 1 - jj2 * d->m_download_chunk_size; //the size is going to be the difference between the start index of the last chunk and ii2+1
                A.readChunk(tmp, 0, size0); //we start reading at position zero
                double* tmp_ptr = tmp.dataPtr();
                //copy the data to the last part of X
                long b = size - size0;
                for (long a = 0; a < size0; a++) {
                    Xptr[b] = tmp_ptr[a];
                    b++;
                }
            }

            /*
             ///this was the old code, which was wrong!!!!!!!!! -- fixed on 5/16/2016
            else if (jj == jj2) {
                Mda tmp;
                long size0 = ii2 + 1 - jj2 * d->m_download_chunk_size;
                A.readChunk(tmp, d->m_download_chunk_size - size0, size0);
                double* tmp_ptr = tmp.dataPtr();
                long b = jj2 * d->m_download_chunk_size - ii1;
                for (long a = 0; a < size0; a++) {
                    Xptr[b] = tmp_ptr[a];
                    b++;
                }
            }
            */
            else { //case 3/3, this is a middle chunk
                Mda tmp;
                A.readChunk(tmp, 0, d->m_download_chunk_size); //read the entire chunk, because we'll use it all
                double* tmp_ptr = tmp.dataPtr();
                long b = jj * d->m_download_chunk_size - ii1; //we start writing at the offset between the start index of the chunk and the start index
                for (long a = 0; a < d->m_download_chunk_size; a++) {
                    Xptr[b] = tmp_ptr[a];
                    b++;
                }
            }
        }
        return true;
    }
}

#ifdef USE_REMOTE_MDA
void RemoteReadMdaPrivate::download_info_if_needed()
{
    if (m_info_downloaded)
        return;
    //if (in_gui_thread()) {
    //    qCritical() << "Cannot call download_info_if_needed from within the GUI thread!";
    //    exit(-1);
    //}
    m_info_downloaded = true;
    //QString url=file_url_for_remote_path(m_path);
    QString url = m_path;
    QString url2 = url + "?a=info&output=text";
    QString txt = http_get_text(url2);
    QStringList lines = txt.split("\n");
    QStringList sizes = lines.value(0).split(",");
    m_info.N1 = sizes.value(0).toLong();
    m_info.N2 = sizes.value(1).toLong();
    m_info.N3 = sizes.value(2).toLong();
    m_info.checksum = lines.value(1);
    m_info.file_last_modified = QDateTime::fromMSecsSinceEpoch(lines.value(2).toLong());
}
#endif

void unquantize8(Mda& X, double minval, double maxval);
QString RemoteReadMdaPrivate::download_chunk_at_index(long ii)
{
    download_info_if_needed();
    long Ntot = m_info.N1 * m_info.N2 * m_info.N3;
    long size = m_download_chunk_size;
    if (ii * m_download_chunk_size + size > Ntot) {
        size = Ntot - ii * m_download_chunk_size;
    }
    if (size <= 0)
        return "";
    if (m_info.checksum.isEmpty())
        return "";
    QString file_name = m_info.checksum + "-" + QString("%1-%2").arg(m_download_chunk_size).arg(ii);
    QString fname = CacheManager::globalInstance()->makeLocalFile(file_name, CacheManager::ShortTerm);
    if (QFile::exists(fname))
        return fname;
    QString url = m_path;
    QString url0 = url + QString("?a=readChunk&output=text&index=%1&size=%2&datatype=%3").arg((long)(ii * m_download_chunk_size)).arg(size).arg(m_remote_datatype);
    QString binary_url = http_get_text(url0).trimmed();
    if (binary_url.isEmpty())
        return "";

    //the following is ugly
    int ind = m_path.indexOf("/mdaserver");
    if (ind > 0) {
        binary_url = m_path.mid(0, ind) + "/mdaserver/" + binary_url;
    }

    QString mda_fname = http_get_binary_file(binary_url);
    if (mda_fname.isEmpty())
        return "";
    DiskReadMda tmp(mda_fname);
    if (tmp.totalSize() != size) {
        qWarning() << "Unexpected total size problem: " << tmp.totalSize() << size;
        QFile::remove(mda_fname);
        return "";
    }
    if (m_remote_datatype == "float32_q8") {
        QString dynamic_range_fname = http_get_binary_file(binary_url + ".q8");
        if (dynamic_range_fname.isEmpty()) {
            qWarning() << "problem downloading .q8 file: " + binary_url + ".q8";
            return "";
        }
        Mda dynamic_range(dynamic_range_fname);
        if (dynamic_range.totalSize() != 2) {
            qWarning() << QString("Problem in .q8 file. Unexpected size %1: ").arg(dynamic_range.totalSize()) + binary_url + ".q8";
            return "";
        }
        Mda chunk(mda_fname);
        unquantize8(chunk, dynamic_range.value(0), dynamic_range.value(1));
        if (!chunk.write32(fname)) {
            qWarning() << "Unable to write file: " + fname;
            return "";
        }
    } else {
        QFile::rename(mda_fname, fname);
    }
    return fname;
}

void unit_test_remote_read_mda()
{
    QString url = "http://localhost:8000/firings.mda";
    RemoteReadMda X(url);
    Mda chunk;
    X.readChunk(chunk, 0, 100);
    for (int j = 0; j < 10; j++) {
        qDebug() << j << chunk.value(j); // unit_test
    }
}

#include "diskreadmda.h"
void unit_test_remote_read_mda_2(const QString& path)
{
    // run this test by calling
    // > mountainview unit_test remotereadmda2

    DiskReadMda X(path);
    for (int j = 0; j < 20; j++) {
        printf("%d: ", j);
        for (int i = 0; i < X.N1(); i++) {
            printf("%g, ", X.value(i, j));
        }
        printf("\n");
    }
}

void unquantize8(Mda& X, double minval, double maxval)
{
    long N = X.totalSize();
    double* Xptr = X.dataPtr();
    for (long i = 0; i < N; i++) {
        Xptr[i] = minval + (Xptr[i] / 255) * (maxval - minval);
    }
}
