/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/2/2016
*******************************************************/

#ifndef REMOTEREADMDA_H
#define REMOTEREADMDA_H

#include <QString>
#include "mda.h"

class RemoteReadMdaPrivate;
class RemoteReadMda
{
public:
    friend class RemoteReadMdaPrivate;
    RemoteReadMda(const QString &url="");
    RemoteReadMda(const RemoteReadMda &other);
    void operator=(const RemoteReadMda &other);
    virtual ~RemoteReadMda();

    void setUrl(const QString &url);
    QString url() const;

    long N1();
    long N2();
    long N3();
    QDateTime fileLastModified();

    ///Retrieve a chunk of the vectorized data of size 1xN starting at position i
    bool readChunk(Mda &X,long i,long size) const;

private:
    RemoteReadMdaPrivate *d;
};

void unit_test_remote_read_mda_2(const QString& url);
void unit_test_remote_read_mda();

#endif // REMOTEREADMDA_H

