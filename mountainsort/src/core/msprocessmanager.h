/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef MSPROCESSMANAGER_H
#define MSPROCESSMANAGER_H

#include "msprocessor.h"

#include <QString>
#include <QMap>
#include <QVariant>

class MSProcessManagerPrivate;
class MSProcessManager {
public:
    friend class MSProcessManagerPrivate;
    MSProcessManager();
    static MSProcessManager* globalInstance();
    virtual ~MSProcessManager();
    void loadDefaultProcessors();

    bool containsProcessor(const QString& processor_name) const;
    bool checkProcess(const QString& processor_name, const QVariantMap& parameters) const;
    bool runProcess(const QString& processor_name, const QVariantMap& parameters);
    bool checkAndRunProcess(const QString& processor_name, const QVariantMap& parameters);

    MSProcessor* processor(const QString& processor_name);

    void loadProcessor(MSProcessor* P);
    QStringList allProcessorNames() const;

    QString usageString() const;
    void printDetails() const;
    void printJsonSpec() const;

private:
    MSProcessManagerPrivate* d;
};

#endif // MSPROCESSMANAGER_H
