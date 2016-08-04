/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/2/2016
*******************************************************/

#ifndef REMOTEREADMDA_H
#define REMOTEREADMDA_H

#include <QString>
#include "mda.h"
#include "mda32.h"

class RemoteReadMdaPrivate;
/**
 * @brief Do not use this class directly -- it is used by DiskReadMda
 */
class RemoteReadMda {
public:
    friend class RemoteReadMdaPrivate;
    RemoteReadMda(const QString& path = "");
    RemoteReadMda(const RemoteReadMda& other);
    void operator=(const RemoteReadMda& other);
    virtual ~RemoteReadMda();

    void setRemoteDataType(QString dtype);
    void setDownloadChunkSize(long size);
    long downloadChunkSize();

    void setPath(const QString& path);
    QString makePath() const; //not capturing the reshaping

    bool reshape(long N1b, long N2b, long N3b);

    long N1() const;
    long N2() const;
    long N3() const;
    QDateTime fileLastModified() const;

    ///Retrieve a chunk of the vectorized data of size 1xN starting at position i
    bool readChunk(Mda64& X, long i, long size) const;
    bool readChunk(Mda32& X, long i, long size) const;

private:
    RemoteReadMdaPrivate* d;
};

#endif // REMOTEREADMDA_H
