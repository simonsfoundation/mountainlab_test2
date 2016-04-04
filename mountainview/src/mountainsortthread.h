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
    void setParameters(const QMap<QString, QVariant>& parameters);
    void setRemoteName(const QString& name);
    void compute();

private:
    MountainsortThreadPrivate* d;
};

QString create_temporary_output_file_name(const QString& remote_name, const QString& processor_name, const QMap<QString, QVariant>& params, const QString& parameter_name);

#endif // MOUNTAINSORTTHREAD_H
