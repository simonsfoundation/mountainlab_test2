/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/4/2016
*******************************************************/

#ifndef MOUNTAINSORTTHREAD_H
#define MOUNTAINSORTTHREAD_H

#include "computationthread.h"

class MountainsortThreadPrivate;
class MountainsortThread : public ComputationThread {
public:
    friend class MountainsortThreadPrivate;
    MountainsortThread();
    virtual ~MountainsortThread();

    void setProcessorName(const QString& pname);
    void setInputParameters(const QMap<QString, QVariant>& parameters);
    void setMscmdServerUrl(const QString &url);
    QString makeOutputFilePath(const QString& pname);
    void compute();

private:
    MountainsortThreadPrivate* d;
};

#endif // MOUNTAINSORTTHREAD_H
