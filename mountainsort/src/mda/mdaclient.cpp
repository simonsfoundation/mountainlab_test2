/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/30/2016
*******************************************************/

#include "mdaclient.h"
#include <QDateTime>
#include <QDir>
#include <QMap>
#include <QString>
#include "textfile.h"
#include <QCoreApplication>

class MdaClientPrivate {
public:
	MdaClient *q;

	QString m_url;
	QMap<QString,MdaClientLoader *> m_chunk_loaders;
	MdaClientLoader *m_header_loader;
	void clear();
	QString chunk_code(const ChunkParams &CP);
};

static QString s_local_cache_path;
void setMdaClientLocalCachePath(const QString &path)
{
	if (!QDir(path).exists()) {
		QDir(QFileInfo(path).path()).mkdir(QFileInfo(path).fileName());
	}
	s_local_cache_path=path;
}


MdaClient::MdaClient(const QString &url)
{
	d=new MdaClientPrivate;
	d->q=this;

	d->m_header_loader=0;
	d->m_url=url;
}

MdaClient::~MdaClient()
{
	d->clear();
	delete d;
}

void MdaClient::setUrl(const QString &url)
{
	d->m_url=url;
	d->clear();
	this->loadHeader(0);
}

MdaClientStatus MdaClient::loadHeader(int timeout)
{
	if (d->m_header_loader) {
		d->m_header_loader->waitUntilFinished(timeout);
		return d->m_header_loader->status();
	}
	else {
		d->m_header_loader=new MdaClientLoader;
		d->m_header_loader->setUrl(d->m_url);
		d->m_header_loader->startLoadingHeader();
		d->m_header_loader->waitUntilFinished(timeout);
		return d->m_header_loader->status();
	}
}

long MdaClient::N1()
{
	if (!d->m_header_loader) return 0;
	return d->m_header_loader->N1();
}

long MdaClient::N2()
{
	if (!d->m_header_loader) return 0;
	return d->m_header_loader->N2();
}

long MdaClient::N3()
{
	if (!d->m_header_loader) return 0;
	return d->m_header_loader->N3();
}

MdaClientStatus MdaClient::loadChunk(int timeout, const ChunkParams &params)
{
	QString code=d->chunk_code(params);
	if (d->m_chunk_loaders.contains(code)) {
		d->m_chunk_loaders[code]->waitUntilFinished(timeout);
		return d->m_chunk_loaders[code]->status();
	}
	else {
		MdaClientLoader *X=new MdaClientLoader;
		X->setUrl(d->m_url);
		X->startLoadingChunk(params);
		d->m_chunk_loaders[code]=X;
		X->waitUntilFinished(timeout);
		return X->status();
	}
}

Mda MdaClient::getChunk(const ChunkParams &params)
{
	QString code=d->chunk_code(params);
	if (!d->m_chunk_loaders.contains(code)) {
		return Mda();
	}
	return d->m_chunk_loaders[code]->chunk();
}


void MdaClientPrivate::clear()
{
	if (m_header_loader) {
		delete m_header_loader;
		m_header_loader=0;
	}
	qDeleteAll(m_chunk_loaders);
	m_chunk_loaders.clear();
}

QString MdaClientPrivate::chunk_code(const ChunkParams &CP)
{
	return QString("%1;%2-%3-%4;%5-%6-%7").arg(CP.dtype).arg(CP.i1).arg(CP.i2).arg(CP.i3).arg(CP.s1).arg(CP.s2).arg(CP.s3);
}


class MdaClientLoaderPrivate {
public:
	MdaClientLoader *q;
	QString m_url;

	bool m_loading_header;
	LoadHeaderThread m_load_header_thread;

	bool m_loading_chunk;
	LoadChunkThread m_load_chunk_thread;
};

MdaClientLoader::MdaClientLoader()
{
	d=new MdaClientLoaderPrivate;
	d->q=this;

	d->m_loading_header=false;
	d->m_loading_chunk=false;

	d->m_load_chunk_thread.local_cache_path=s_local_cache_path;
}

MdaClientLoader::~MdaClientLoader()
{
	delete d;
}

MdaClientStatus MdaClientLoader::status()
{
	if (d->m_loading_chunk) return d->m_load_chunk_thread.status();
	if (d->m_loading_header) return d->m_load_header_thread.status();
	return NotStarted;
}


void MdaClientLoader::setUrl(const QString &url)
{
	d->m_url=url;
}

void MdaClientLoader::startLoadingHeader()
{
	d->m_load_header_thread.url=d->m_url;
	d->m_loading_header=true;
	d->m_load_header_thread.start();
}

long MdaClientLoader::N1()
{
	if (d->m_load_header_thread.isRunning()) return 0;
	return d->m_load_header_thread.N1;
}

long MdaClientLoader::N2()
{
	if (d->m_load_header_thread.isRunning()) return 0;
	return d->m_load_header_thread.N2;
}

long MdaClientLoader::N3()
{
	if (d->m_load_header_thread.isRunning()) return 0;
	return d->m_load_header_thread.N3;
}

void MdaClientLoader::startLoadingChunk(const ChunkParams &CP)
{
	d->m_load_chunk_thread.url=d->m_url;
	d->m_load_chunk_thread.chunk_params=CP;
	d->m_loading_chunk=true;
	d->m_load_chunk_thread.start();
}

Mda MdaClientLoader::chunk()
{
	if (d->m_load_chunk_thread.isRunning()) return Mda();
	return d->m_load_chunk_thread.chunk;
}

bool MdaClientLoader::waitUntilFinished(int timeout)
{
	QTime timer; timer.start();
	while (((timer.elapsed()<timeout)||(timeout<0))&&((this->status()==NotStarted)||(this->status()==Loading)));
	return ((this->status()==Finished)||(this->status()==Error));
}

QString get_temp_fname() {
	long rand_num=qrand()+QDateTime::currentDateTime().toMSecsSinceEpoch();
	return QString("%1/MdaClient_%2.tmp").arg(QDir::tempPath()).arg(rand_num);
}

QString http_get_text(QString url) {
	QString tmp_fname=get_temp_fname();
	QString cmd=QString("curl \"%1\" > %2").arg(url).arg(tmp_fname);
	int exit_code=system(cmd.toLatin1().data());
	if (exit_code!=0) {
		qWarning() << "Problem with system call: "+cmd;
		QFile::remove(tmp_fname);
		return "";
	}
	QString ret=read_text_file(tmp_fname);
	QFile::remove(tmp_fname);
	return ret;
}

QString http_get_binary_mda_file(QString url) {
	QString tmp_fname=get_temp_fname()+".mda";
	QString cmd=QString("curl \"%1\" > %2").arg(url).arg(tmp_fname);
	int exit_code=system(cmd.toLatin1().data());
	if (exit_code!=0) {
		qWarning() << "Problem with system call: "+cmd;
		QFile::remove(tmp_fname);
		return "";
	}
	return tmp_fname;

}

bool check_correct_size(Mda &X,const ChunkParams &P) {
	if (X.N1()!=P.s1) return false;
	if (X.N2()!=P.s2) return false;
	if (X.N3()!=P.s3) return false;
	return true;
}

void LoadChunkThread::run()
{
	QString str=this->url+"?a=readChunk&";
	str+=QString("index=%1,%2,%3&").arg(this->chunk_params.i1).arg(this->chunk_params.i2).arg(this->chunk_params.i3);
	str+=QString("size=%1,%2,%3&").arg(this->chunk_params.s1).arg(this->chunk_params.s2).arg(this->chunk_params.s3);
	str+=QString("datatype=%1&").arg(this->chunk_params.dtype);
	QString result=http_get_text(str).trimmed();
	if (!result.startsWith("http")) {
		this->error=QString("ERROR in %1: %2").arg(str).arg(result);
		return;
	}
	QString result_url=result;
	QString mda_fname;
	QString local_fname=find_in_local_cache(result_url);
	if (!local_fname.isEmpty()) {
		mda_fname=local_fname;
	}
	else {
		mda_fname=http_get_binary_mda_file(result_url);
	}
	if (mda_fname.isEmpty()) {
		this->error="Error getting binary file: "+result_url;
	}
	if (!this->chunk.read(mda_fname)) {
		if (mda_fname==local_fname) {
			this->error=QString("ERROR reading locally cached mda file.");
			QFile::remove(mda_fname);
		}
		else {
			this->error=QString("ERROR reading downloaded mda file.");
			QFile::remove(mda_fname);
		}
		return;
	}
	if (!check_correct_size(this->chunk,this->chunk_params)) {
		if (mda_fname==local_fname) {
			this->error=QString("ERROR Problem with locally cached mda file dimensions.");
		}
		else {
			this->error=QString("ERROR Problem with downloaded mda file dimensions.");
		}
		QFile::remove(mda_fname);
		return;
	}
	//here is where we should put it in the local cache.
	put_in_local_cache_or_remove(mda_fname,result_url);
}





void LoadHeaderThread::run()
{
	QString str=this->url+"?a=size";
	QString result=http_get_text(str);
	QStringList list=result.split(",");
	if (list.count()>=3) {
		this->N1=list.value(0).toLong();
		this->N2=list.value(1).toLong();
		this->N3=list.value(2).toLong();
		this->error="";
	}
	else {
		this->error=QString("ERROR in %1: %2").arg(str).arg(result);
	}
}

MdaClientStatus LoadHeaderThread::status()
{
	if (this->isRunning()) return Loading;
	if (this->isFinished()) {
		if (error.isEmpty()) return Finished;
		else return Error;
	}
	return NotStarted;
}
MdaClientStatus LoadChunkThread::status()
{
	if (this->isRunning()) return Loading;
	if (this->isFinished()) {
		if (error.isEmpty()) return Finished;
		else return Error;
	}
	return NotStarted;
}

QString LoadChunkThread::find_in_local_cache(const QString &url)
{
	if (this->local_cache_path.isEmpty()) return "";
	QString fname=url;
	int ind=fname.lastIndexOf("/");
	if (ind>=0) fname=fname.mid(ind+1);
	fname=this->local_cache_path+"/"+fname;
	if (QFile::exists(fname)) {
		return fname;
	}
	else return "";
}

void LoadChunkThread::put_in_local_cache_or_remove(const QString &fname_in, const QString &url)
{
	if (this->local_cache_path.isEmpty()) {
		QFile::remove(fname_in);
		return;
	}
	QString fname=url;
	int ind=fname.lastIndexOf("/");
	if (ind>=0) fname=fname.mid(ind+1);
	fname=this->local_cache_path+"/"+fname;
	if (fname==fname_in) return;
	if (QFile::exists(fname)) {
		QFile::remove(fname);
	}
	if (!QFile::rename(fname_in,fname)) {
		QFile::remove(fname_in);
	}
}


void mdaclient_unit_test()
{
    QString url="http://magland.org:8000/firings.mda";
	MdaClient X(url);

	setMdaClientLocalCachePath("/tmp/mdaclient_local_cache");

	MdaClientStatus status;

	QTime timer; timer.start();
	status=X.loadHeader(-1);
	qDebug()  << "Elapsed:" << timer.elapsed();
	if (status!=Finished) {
		printf("Problem loading header.\n");
		return;
	}
	qDebug()  << X.N1() << X.N2() << X.N3();


	ChunkParams CP("float32",0,0,5,30);
	timer.start();
	status=X.loadChunk(-1,CP);
	qDebug()  << "Elapsed:" << timer.elapsed();
	if (status!=Finished) {
		printf("Problem loading chunk.\n");
		return;
	}
	Mda A=X.getChunk(CP);
	qDebug()  << A.N1() << A.N2();
	qDebug()  << A.value(1,0) << A.value(1,1) << A.value(1,2);
	qDebug()  << A.value(2,0) << A.value(2,1) << A.value(2,2);
}


ChunkParams::ChunkParams(QString dtype_in, long i1_in, long i2_in, long s1_in, long s2_in)
{
	i1=i1_in;
	i2=i2_in;
	i3=0;
	s1=s1_in;
	s2=s2_in;
	s3=1;
	dtype=dtype_in;
}

ChunkParams::ChunkParams(QString dtype_in, long i1_in, long i2_in,long i3_in, long s1_in, long s2_in, long s3_in)
{
	i1=i1_in;
	i2=i2_in;
	i3=i3_in;
	s1=s1_in;
	s2=s2_in;
	s3=s3_in;
	dtype=dtype_in;
}

ChunkParams::ChunkParams(const ChunkParams &other)
{
	i1=other.i1;
	i2=other.i2;
	i3=other.i3;
	s1=other.s1;
	s2=other.s2;
	s3=other.s3;
	dtype=other.dtype;
}

void ChunkParams::operator=(const ChunkParams &other)
{
	i1=other.i1;
	i2=other.i2;
	i3=other.i3;
	s1=other.s1;
	s2=other.s2;
	s3=other.s3;
	dtype=other.dtype;
}

