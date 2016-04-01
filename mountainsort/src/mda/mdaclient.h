/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/30/2016
*******************************************************/


#ifndef MDACLIENT_H
#define MDACLIENT_H

#include "mda.h"

#include <QThread>

enum MdaClientStatus {
	NotStarted,Loading,Finished,Error
};

void mdaclient_unit_test(const QString &url);
void mdaclient_unit_test_2(const QString &url);

class ChunkParams {
public:
	ChunkParams(QString dtype="float32",long i1=0,long i2=0,long s1=1,long s2=1);
	ChunkParams(QString dtype,long i1,long i2,long i3,long s1,long s2,long s3);
	ChunkParams(const ChunkParams &other);
	void operator=(const ChunkParams &other);

	QString dtype;
	long i1,i2,i3;
	long s1,s2,s3;
};

void setMdaClientLocalCachePath(const QString &path);

class MdaClientPrivate;
class MdaClient
{
public:
	friend class MdaClientPrivate;
	MdaClient(const QString &url="");
	virtual ~MdaClient();
	void setUrl(const QString &url);

	MdaClientStatus loadHeader(int timeout);
	long N1();
	long N2();
	long N3();
    long totalSize();

	MdaClientStatus loadChunk(int timeout,const ChunkParams &params);
	Mda getChunk(const ChunkParams &params);

private:
	MdaClientPrivate *d;
};

class MdaClientLoaderPrivate;
class MdaClientLoader : public QObject
{
	Q_OBJECT
public:
	friend class MdaClientLoaderPrivate;
	MdaClientLoader();
	virtual ~MdaClientLoader();
	MdaClientStatus status();
	void setUrl(const QString &url);

	void startLoadingHeader();
	long N1();
	long N2();
	long N3();

	void startLoadingChunk(const ChunkParams &CP);
	Mda chunk();

	bool waitUntilFinished(int timeout);
private:
	MdaClientLoaderPrivate *d;
};

class LoadHeaderThread : public QThread {
	Q_OBJECT
public:
	void run();
	MdaClientStatus status();

	//input
	QString url;

	//output
	int N1,N2,N3;
	QString error;
};

class LoadChunkThread : public QThread {
	Q_OBJECT
public:
	void run();
	MdaClientStatus status();

	//input
	QString url;
	ChunkParams chunk_params;
	QString local_cache_path;

	//output
	Mda chunk;
	QString error;
private:
	QString find_in_local_cache(const QString &url);
	void put_in_local_cache_or_remove(const QString &fname,const QString &url);
};

#endif // MDACLIENT_H

