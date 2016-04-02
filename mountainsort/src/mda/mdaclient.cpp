/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/30/2016
*******************************************************/

#include "mdaclient.h"
#include <QDateTime>
#include <QDir>
#include <QMap>
#include <QMutex>
#include <QSettings>
#include <QString>
#include <QTemporaryFile>
#include <QUrl>
#include "textfile.h"
#include <QCoreApplication>

#ifdef USE_NETWORK
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QtNetwork/QNetworkAccessManager>
#endif

class MdaClientPrivate {
public:
    MdaClient* q;

    QString m_url;
    QMap<QString, MdaClientLoader*> m_chunk_loaders;
    MdaClientLoader* m_header_loader;
    void clear();
    QString chunk_code(const ChunkParams& CP);
};

MdaClient::MdaClient(const QString& url)
{
    d = new MdaClientPrivate;
    d->q = this;

    d->m_header_loader = 0;
    d->m_url = url;
}

MdaClient::~MdaClient()
{
    d->clear();
    delete d;
}

void MdaClient::setLocalCachePath(const QString &path)
{
    if (!QDir(path).exists()) {
        QDir(QFileInfo(path).path()).mkdir(QFileInfo(path).fileName());
    }
    QSettings settings("SCDA","MountainLab");
    settings.setValue("mdaclient_local_cache_path",path);
}

QString MdaClient::getLocalCachePath()
{
    QSettings settings("SCDA","MountainLab");
    return settings.value("mdaclient_local_cache_path").toString();
}

void MdaClient::setUrl(const QString& url)
{
    d->m_url = url;
    d->clear();
	//this->loadHeader(0); //this causes crash, not sure why
}

QString MdaClient::url()
{
	return d->m_url;
}

MdaClientStatus MdaClient::loadHeader(int timeout)
{
    if (d->m_header_loader) {
        if ((d->m_header_loader->status()==Finished)||(d->m_header_loader->status()==Error)) {
            return d->m_header_loader->status();
        }
        d->m_header_loader->waitUntilFinished(timeout);
        return d->m_header_loader->status();
    } else {
        d->m_header_loader = new MdaClientLoader;
        d->m_header_loader->setUrl(d->m_url);
		d->m_header_loader->startLoadingHeader();
		d->m_header_loader->waitUntilFinished(timeout);
        return d->m_header_loader->status();
    }
}

long MdaClient::N1()
{
    loadHeader(-1);
    if (!d->m_header_loader)
        return 0;
    return d->m_header_loader->N1();
}

long MdaClient::N2()
{
    loadHeader(-1);
    if (!d->m_header_loader)
        return 0;
    return d->m_header_loader->N2();
}

long MdaClient::N3()
{
    loadHeader(-1);
    if (!d->m_header_loader)
        return 0;
    return d->m_header_loader->N3();
}

long MdaClient::totalSize()
{
    loadHeader(-1);
    if (!d->m_header_loader)
        return 0;
    return d->m_header_loader->N1()*d->m_header_loader->N2()*d->m_header_loader->N3();
}

MdaClientStatus MdaClient::loadChunk(int timeout, const ChunkParams& params)
{
    QString code = d->chunk_code(params);
    if (d->m_chunk_loaders.contains(code)) {
        if ((d->m_chunk_loaders[code]->status()==Finished)||(d->m_chunk_loaders[code]->status()==Error)) {
            return d->m_chunk_loaders[code]->status();
        }
        d->m_chunk_loaders[code]->waitUntilFinished(timeout);
        return d->m_chunk_loaders[code]->status();
    } else {
        MdaClientLoader* X = new MdaClientLoader;
        X->setUrl(d->m_url);
        X->startLoadingChunk(params);
        d->m_chunk_loaders[code] = X;
        X->waitUntilFinished(timeout);
        return X->status();
    }
}

Mda MdaClient::getChunk(const ChunkParams& params)
{
    loadChunk(-1,params);
    QString code = d->chunk_code(params);
    if (!d->m_chunk_loaders.contains(code)) {
        return Mda();
    }
    return d->m_chunk_loaders[code]->chunk();
}

void MdaClientPrivate::clear()
{
    if (m_header_loader) {
        delete m_header_loader;
        m_header_loader = 0;
    }
    qDeleteAll(m_chunk_loaders);
    m_chunk_loaders.clear();
}

QString MdaClientPrivate::chunk_code(const ChunkParams& CP)
{
    return QString("%1;%2-%3-%4;%5-%6-%7").arg(CP.dtype).arg(CP.i1).arg(CP.i2).arg(CP.i3).arg(CP.s1).arg(CP.s2).arg(CP.s3);
}

class MdaClientLoaderPrivate {
public:
    MdaClientLoader* q;
    QString m_url;

    bool m_loading_header;
    LoadHeaderThread m_load_header_thread;

    bool m_loading_chunk;
    LoadChunkThread m_load_chunk_thread;
};

MdaClientLoader::MdaClientLoader()
{
    d = new MdaClientLoaderPrivate;
    d->q = this;

    d->m_loading_header = false;
    d->m_loading_chunk = false;
}

MdaClientLoader::~MdaClientLoader()
{
    delete d;
}

MdaClientStatus MdaClientLoader::status()
{
    if (d->m_loading_chunk)
        return d->m_load_chunk_thread.status();
    if (d->m_loading_header)
        return d->m_load_header_thread.status();
    return NotStarted;
}

void MdaClientLoader::setUrl(const QString& url)
{
    d->m_url = url;
}

void MdaClientLoader::startLoadingHeader()
{
	if (d->m_load_header_thread.isRunning()) {
		qWarning("Not supposed to happen. Aborting.");
		exit(-1);
	}
	d->m_load_header_thread.setUrl(d->m_url);
    d->m_loading_header = true;
    d->m_load_header_thread.start();
}

long MdaClientLoader::N1()
{
    if (d->m_load_header_thread.isRunning())
        return 0;
	return d->m_load_header_thread.N1();
}

long MdaClientLoader::N2()
{
    if (d->m_load_header_thread.isRunning())
        return 0;
	return d->m_load_header_thread.N2();
}

long MdaClientLoader::N3()
{
    if (d->m_load_header_thread.isRunning())
        return 0;
	return d->m_load_header_thread.N3();
}

void MdaClientLoader::startLoadingChunk(const ChunkParams& CP)
{
    d->m_load_chunk_thread.setUrl(d->m_url);
    d->m_load_chunk_thread.setChunkParams(CP);
    d->m_loading_chunk = true;
    d->m_load_chunk_thread.start();
}

Mda MdaClientLoader::chunk()
{
    if (d->m_load_chunk_thread.isRunning())
        return Mda();
    return d->m_load_chunk_thread.chunk();
}

bool MdaClientLoader::waitUntilFinished(int timeout)
{
    QTime timer;
    timer.start();
    while (((timer.elapsed() < timeout) || (timeout < 0)) && ((this->status() == NotStarted) || (this->status() == Loading)))
        ;
    return ((this->status() == Finished) || (this->status() == Error));
}

QString get_temp_fname()
{
    long rand_num = qrand() + QDateTime::currentDateTime().toMSecsSinceEpoch();
    return QString("%1/MdaClient_%2.tmp").arg(QDir::tempPath()).arg(rand_num);
}

QString http_get_text(QString url)
{
    QString tmp_fname = get_temp_fname();
	QString cmd = QString("curl \"%1\" > %2").arg(url).arg(tmp_fname);
    qDebug()  << cmd;
    int exit_code = system(cmd.toLatin1().data());
    if (exit_code != 0) {
        qWarning() << "Problem with system call: " + cmd;
        QFile::remove(tmp_fname);
        return "";
    }
    QString ret = read_text_file(tmp_fname);
    QFile::remove(tmp_fname);
	qDebug()  << "RESPONSE: " << ret;
    return ret;
}

QString http_get_binary_mda_file(QString url)
{
    QString tmp_fname = get_temp_fname() + ".mda";
	QString cmd = QString("curl \"%1\" > %2").arg(url).arg(tmp_fname);
    qDebug()  << cmd;
    int exit_code = system(cmd.toLatin1().data());
    if (exit_code != 0) {
        qWarning() << "Problem with system call: " + cmd;
        QFile::remove(tmp_fname);
        return "";
    }
    return tmp_fname;
}

/*

//Witold, I could not get this to work. Kept getting segmentation fault on result->readAll()
//I changed your code around to get it to compile.

QIODevice* http_get(const QString& url)
{
    QNetworkAccessManager manager; // better make it a singleton
    QNetworkReply* reply = manager.get(QNetworkRequest(QUrl(url)));
    QEventLoop loop;
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    if (reply->error() == QNetworkReply::NoError)
        return reply;
    delete reply;
    return 0;
}

QString http_get_text(const QString& url)
{
    QIODevice* result = http_get(url);
    if (!result)
        return QString();
    result->readAll()
    QString ret=result->readAll();
    delete result;
    return ret;
}


QString http_get_binary_mda_file(const QString& url)
{
    QIODevice* result = http_get(url);
    if (!result)
        return QString();
    QFile file(get_temp_fname());
    file.open(QIODevice::WriteOnly);
    file.write(result->readAll()); // this is EVIL™
    delete result;
    return file.fileName();
}
*/

bool check_correct_size(Mda& X, const ChunkParams& P_in)
{
    ChunkParams P=P_in;
    if (P.s2==0) P.s2=1;
    if (P.s3==0) P.s3=1;
    if (X.N1() != P.s1)
        return false;
    if (X.N2() != P.s2)
        return false;
    if (X.N3() != P.s3)
        return false;
    return true;
}

void LoadChunkThread::run()
{
    QString str = this->url() + "?a=readChunk&";
    ChunkParams CP=this->chunkParams();
    str += QString("index=%1,%2,%3&").arg(CP.i1).arg(CP.i2).arg(CP.i3);
    str += QString("size=%1,%2,%3&").arg(CP.s1).arg(CP.s2).arg(CP.s3);
    str += QString("datatype=%1&").arg(CP.dtype);
    QString result = http_get_text(str).trimmed();
    if (!result.startsWith("http")) {
        this->setError(QString("ERROR in %1: %2").arg(str).arg(result));
        return;
    }
    QString result_url = result;
    QString mda_fname;
    QString local_fname = find_in_local_cache(result_url);
    if (!local_fname.isEmpty()) {
        mda_fname = local_fname;
    } else {
        mda_fname = http_get_binary_mda_file(result_url);
    }
    if (mda_fname.isEmpty()) {
        this->setError("Error getting binary file: " + result_url);
    }
    Mda chunk0;
    if (!chunk0.read(mda_fname)) {
        if (mda_fname == local_fname) {
            this->setError(QString("ERROR reading locally cached mda file."));
            QFile::remove(mda_fname);
        } else {
            this->setError(QString("ERROR reading downloaded mda file."));
            QFile::remove(mda_fname);
        }
        return;
    }
    this->setChunk(chunk0);
    if (!check_correct_size(chunk0, this->chunkParams())) {
        if (mda_fname == local_fname) {
            this->setError(QString("ERROR Problem with locally cached mda file dimensions %1x%2x%3 <> %4x%5x%6.").arg(chunk0.N1()).arg(chunk0.N2()).arg(chunk0.N3()).arg(this->chunkParams().s1).arg(this->chunkParams().s2).arg(this->chunkParams().s3));
        } else {
            this->setError(QString("ERROR Problem with downloaded mda file dimensions %1x%2x%3 <> %4x%5x%6.").arg(chunk0.N1()).arg(chunk0.N2()).arg(chunk0.N3()).arg(this->chunkParams().s1).arg(this->chunkParams().s2).arg(this->chunkParams().s3));
        }
        qWarning() << this->error();
        QFile::remove(mda_fname);
        return;
    }
    //here is where we should put it in the local cache.
    if (mda_fname!=local_fname) {
        put_in_local_cache_or_remove(mda_fname, result_url);
    }
}

void LoadHeaderThread::run()
{
	QString str = this->url() + "?a=size";
    QString result = http_get_text(str);
    QStringList list = result.split(",");
    if (list.count() >= 3) {
		this->setN1(list.value(0).toLong());
		this->setN2(list.value(1).toLong());
		this->setN3(list.value(2).toLong());
		this->setError("");
    } else {
		this->setError(QString("ERROR in %1: %2").arg(str).arg(result));
    }
}

MdaClientStatus LoadHeaderThread::status()
{
    if (this->isRunning())
        return Loading;
    if (this->isFinished()) {
		if (error().isEmpty())
            return Finished;
        else
            return Error;
    }
	return NotStarted;
}

void LoadHeaderThread::setUrl(QString url)
{
	QMutexLocker locker(&m_mutex);
	m_url=url;
}

long LoadHeaderThread::N1()
{
	QMutexLocker locker(&m_mutex);
	return m_N1;
}
long LoadHeaderThread::N2()
{
	QMutexLocker locker(&m_mutex);
	return m_N2;
}
long LoadHeaderThread::N3()
{
	QMutexLocker locker(&m_mutex);
	return m_N3;
}

QString LoadHeaderThread::error()
{
	QMutexLocker locker(&m_mutex);
	return m_error;
}

QString LoadHeaderThread::url()
{
	QMutexLocker locker(&m_mutex);
	return m_url;
}

void LoadHeaderThread::setN1(long val)
{
	QMutexLocker locker(&m_mutex);
	m_N1=val;
}

void LoadHeaderThread::setN2(long val)
{
	QMutexLocker locker(&m_mutex);
	m_N2=val;
}

void LoadHeaderThread::setN3(long val)
{
	QMutexLocker locker(&m_mutex);
	m_N3=val;
}

void LoadHeaderThread::setError(QString err)
{
	QMutexLocker locker(&m_mutex);
	m_error=err;
}
MdaClientStatus LoadChunkThread::status()
{
    if (this->isRunning())
        return Loading;
    if (this->isFinished()) {
        if (error().isEmpty())
            return Finished;
        else
            return Error;
    }
    return NotStarted;
}

QString LoadChunkThread::error()
{
    QMutexLocker locker(&m_mutex);
    return m_error;
}

void LoadChunkThread::setError(QString err)
{
    QMutexLocker locker(&m_mutex);
    m_error=err;
}

QString LoadChunkThread::url()
{
    QMutexLocker locker(&m_mutex);
    return m_url;
}

void LoadChunkThread::setUrl(QString url)
{
    QMutexLocker locker(&m_mutex);
    m_url=url;
}

ChunkParams LoadChunkThread::chunkParams()
{
    QMutexLocker locker(&m_mutex);
    return m_chunk_params;
}

void LoadChunkThread::setChunkParams(const ChunkParams &p)
{
    QMutexLocker locker(&m_mutex);
    m_chunk_params=p;
}

QString LoadChunkThread::localCachePath()
{
    QMutexLocker locker(&m_mutex);
    return MdaClient::getLocalCachePath();
}

Mda LoadChunkThread::chunk()
{
    QMutexLocker locker(&m_mutex);
    return m_chunk;
}

void LoadChunkThread::setChunk(const Mda &chunk)
{
    QMutexLocker locker(&m_mutex);
    m_chunk=chunk;
}

QString LoadChunkThread::find_in_local_cache(const QString& url)
{
    QString local_cache_path=this->localCachePath();
    if (local_cache_path.isEmpty())
        return "";
    QString fname = url;
    int ind = fname.lastIndexOf("/");
    if (ind >= 0)
        fname = fname.mid(ind + 1);
    fname = local_cache_path + "/" + fname;
    if (QFile::exists(fname)) {
        return fname;
    } else
        return "";
}

void LoadChunkThread::put_in_local_cache_or_remove(const QString& fname_in, const QString& url)
{
    QString local_cache_path=this->localCachePath();
    if (local_cache_path.isEmpty()) {
        QFile::remove(fname_in);
        return;
    }
    QString fname = url;
    int ind = fname.lastIndexOf("/");
    if (ind >= 0)
        fname = fname.mid(ind + 1);
    fname = local_cache_path + "/" + fname;
    if (fname == fname_in)
        return;
    if (QFile::exists(fname)) {
        QFile::remove(fname);
    }
    if (!QFile::rename(fname_in, fname)) {
        QFile::remove(fname_in);
    }
}

void mdaclient_unit_test(const QString& url)
{
    // run this test by calling
    // > mountainview unit_test mdaclient
    MdaClient X(url);

    MdaClientStatus status;

    QTime timer;
    timer.start();
    status = X.loadHeader(-1);
    if (status != Finished) {
        printf("Problem loading header.\n");
        return;
    }

    ChunkParams CP("float32", 0, 0, 5, 30);
    timer.start();
    status = X.loadChunk(-1, CP);
    if (status != Finished) {
        printf("Problem loading chunk.\n");
        return;
    }
    Mda A = X.getChunk(CP);
}

#include "diskreadmda.h"
void mdaclient_unit_test_2(const QString& url)
{
    // run this test by calling
    // > mountainview unit_test mdaclient2

    QUrl url0(url);
    DiskReadMda X(url0);
    qDebug()  << X.N1() << X.N2() << X.N3();
    qDebug()  << X.value(1, 0) << X.value(1, 1) << X.value(1, 2);
    qDebug()  << X.value(2, 0) << X.value(2, 1) << X.value(2, 2);
}

ChunkParams::ChunkParams(QString dtype_in, long i1_in, long i2_in, long s1_in, long s2_in)
{
    i1 = i1_in;
    i2 = i2_in;
    i3 = 0;
    s1 = s1_in;
    s2 = s2_in;
    s3 = 1;
    dtype = dtype_in;
}

ChunkParams::ChunkParams(QString dtype_in, long i1_in, long i2_in, long i3_in, long s1_in, long s2_in, long s3_in)
{
    i1 = i1_in;
    i2 = i2_in;
    i3 = i3_in;
    s1 = s1_in;
    s2 = s2_in;
    s3 = s3_in;
    dtype = dtype_in;
}

ChunkParams::ChunkParams(const ChunkParams& other)
{
    i1 = other.i1;
    i2 = other.i2;
    i3 = other.i3;
    s1 = other.s1;
    s2 = other.s2;
    s3 = other.s3;
    dtype = other.dtype;
}

void ChunkParams::operator=(const ChunkParams& other)
{
    i1 = other.i1;
    i2 = other.i2;
    i3 = other.i3;
    s1 = other.s1;
    s2 = other.s2;
    s3 = other.s3;
    dtype = other.dtype;
}
