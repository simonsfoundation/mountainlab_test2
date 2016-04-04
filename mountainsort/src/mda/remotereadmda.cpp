/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/2/2016
*******************************************************/

#include "remotereadmda.h"
#include "msmisc.h"
#include "diskreadmda.h"
#include <QStringList>
#include <QDir>
#include <QDateTime>

#define REMOTE_READ_MDA_CHUNK_SIZE 5e6

struct RemoteReadMdaInfo {
    long N1,N2,N3;
    QString checksum;
    QDateTime file_last_modified;
};

class RemoteReadMdaPrivate
{
public:
    RemoteReadMda *q;

    QString m_url;
    RemoteReadMdaInfo m_info;
    bool m_info_downloaded;

    void download_info_if_needed();
    QString download_chunk_at_index(long ii);

};

RemoteReadMda::RemoteReadMda(const QString &url) {
    d=new RemoteReadMdaPrivate;
    d->q=this;
    this->setUrl(url);
}

RemoteReadMda::RemoteReadMda(const RemoteReadMda &other)
{
    d=new RemoteReadMdaPrivate;
    d->q=this;
    this->setUrl(other.d->m_url);
}

void RemoteReadMda::operator=(const RemoteReadMda &other)
{
    this->setUrl(other.d->m_url);
}

RemoteReadMda::~RemoteReadMda() {
    delete d;
}

void RemoteReadMda::setUrl(const QString &url)
{
    d->m_info.N1=d->m_info.N2=d->m_info.N3=0;
    d->m_info_downloaded=false;
    d->m_url=url;
}

long RemoteReadMda::N1()
{
    d->download_info_if_needed();
    return d->m_info.N1;
}

long RemoteReadMda::N2()
{
    d->download_info_if_needed();
    return d->m_info.N2;
}

long RemoteReadMda::N3()
{
    d->download_info_if_needed();
    return d->m_info.N3;
}

QDateTime RemoteReadMda::fileLastModified()
{
    d->download_info_if_needed();
    return d->m_info.file_last_modified;
}

bool RemoteReadMda::readChunk(Mda &X, long i, long size) const
{
    X.allocate(size,1);
    double *Xptr=X.dataPtr();
    long ii1=i;
    long ii2=i+size-1;
    long jj1=ii1/REMOTE_READ_MDA_CHUNK_SIZE;
    long jj2=ii2/REMOTE_READ_MDA_CHUNK_SIZE;
    if (jj1==jj2) {
        QString fname=d->download_chunk_at_index(jj1);
        if (fname.isEmpty()) return false;
        DiskReadMda A(fname);
        A.readChunk(X,ii1-jj1*REMOTE_READ_MDA_CHUNK_SIZE,size);
        return true;
    }
    else {
        for (long jj=jj1; jj<=jj2; jj++) {
            QString fname=d->download_chunk_at_index(jj);
            if (fname.isEmpty()) return false;
            DiskReadMda A(fname);
            if (jj==jj1) {
                Mda tmp;
                long size0=(jj1+1)*REMOTE_READ_MDA_CHUNK_SIZE-ii1;
                A.readChunk(tmp,ii1-jj1*REMOTE_READ_MDA_CHUNK_SIZE,size0);
                double *tmp_ptr=tmp.dataPtr();
                long b=0;
                for (long a=0; a<size0; a++) {
                    Xptr[b]=tmp_ptr[a];
                    b++;
                }
            }
            else if (jj==jj2) {
                Mda tmp;
                long size0=ii2+1-jj2*REMOTE_READ_MDA_CHUNK_SIZE;
                A.readChunk(tmp,REMOTE_READ_MDA_CHUNK_SIZE-size0,size0);
                double *tmp_ptr=tmp.dataPtr();
                long b=jj2*REMOTE_READ_MDA_CHUNK_SIZE-ii1;
                for (long a=0; a<size0; a++) {
                    Xptr[b]=tmp_ptr[a];
                    b++;
                }
            }
            else {
                Mda tmp;
                A.readChunk(tmp,0,REMOTE_READ_MDA_CHUNK_SIZE);
                double *tmp_ptr=tmp.dataPtr();
                long b=jj*REMOTE_READ_MDA_CHUNK_SIZE-ii1;
                for (long a=0; a<REMOTE_READ_MDA_CHUNK_SIZE; a++) {
                    Xptr[b]=tmp_ptr[a];
                    b++;
                }
            }
        }
        return true;
    }
}

void RemoteReadMdaPrivate::download_info_if_needed()
{
    if (m_info_downloaded) return;
    m_info_downloaded=true;
    QString url2=m_url+"?a=info";
    QString txt=http_get_text(url2);
    QStringList lines=txt.split("\n");
    QStringList sizes=lines.value(0).split(",");
    m_info.N1=sizes.value(0).toLong();
    m_info.N2=sizes.value(1).toLong();
    m_info.N3=sizes.value(2).toLong();
    m_info.checksum=lines.value(1);
    m_info.file_last_modified=QDateTime::fromMSecsSinceEpoch(lines.value(2).toLong());
}

QString RemoteReadMdaPrivate::download_chunk_at_index(long ii)
{
    long Ntot=m_info.N1*m_info.N2*m_info.N3;
    long size=REMOTE_READ_MDA_CHUNK_SIZE;
    if (ii*REMOTE_READ_MDA_CHUNK_SIZE+size>Ntot) {
        size=Ntot-ii*REMOTE_READ_MDA_CHUNK_SIZE;
    }
    if (size<=0) return "";
    if (m_info.checksum.isEmpty()) return "";
    QString cache_path=QDir::tempPath()+"/RemoteReadMda";
    if (!QDir(cache_path).exists()) QDir(QDir::tempPath()).mkdir("RemoteReadMda");
    QString fname=cache_path+"/"+m_info.checksum+"-"+QString("%1-%2").arg(REMOTE_READ_MDA_CHUNK_SIZE).arg(ii);
    if (QFile::exists(fname)) return fname;
    QString url0=m_url+QString("?a=readChunk&index=%1&size=%2&datatype=float64").arg((long)(ii*REMOTE_READ_MDA_CHUNK_SIZE)).arg(size);
    QString binary_url=http_get_text(url0).trimmed();
    if (binary_url.isEmpty()) return "";
    QString mda_fname=http_get_binary_mda_file(binary_url);
    if (mda_fname.isEmpty()) return "";
    DiskReadMda tmp(mda_fname);
    if (tmp.totalSize()!=size) {
        qWarning() << "Unexpected total size problem: " << tmp.totalSize() << size;
        QFile::remove(mda_fname);
        return "";
    }
    QFile::rename(mda_fname,fname);
    return fname;
}

void unit_test_remote_read_mda()
{
    QString url="http://localhost:8000/firings.mda";
    RemoteReadMda X(url);
    qDebug()  << X.N1() << X.N2() << X.N3();
    Mda chunk;
    X.readChunk(chunk,0,100);
    for (int j=0;  j<10; j++) {
        qDebug()  << j << chunk.value(j);
    }
}

#include "diskreadmda.h"
void unit_test_remote_read_mda_2(const QString& url)
{
    // run this test by calling
    // > mountainview unit_test remotereadmda2

    QUrl url0(url);
    DiskReadMda X(url0);
    for (int j=0; j<20; j++) {
        printf("%d: ",j);
        for (int i=0; i<X.N1(); i++) {
            printf("%g, ",X.value(i,j));
        }
        printf("\n");
    }
}
