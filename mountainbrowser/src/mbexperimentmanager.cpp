/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/6/2016
*******************************************************/

#include "mbexperimentmanager.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QMap>
#include <QDebug>

class MBExperimentManagerPrivate {
public:
    MBExperimentManager* q;

    QMap<QString, MBExperiment> m_experiments;
};

MBExperimentManager::MBExperimentManager()
{
    d = new MBExperimentManagerPrivate;
    d->q = this;
}

MBExperimentManager::~MBExperimentManager()
{
    delete d;
}

MBExperiment create_experiment_from_json(QJsonObject obj)
{
    MBExperiment ret;
    ret.id = obj["exp_id"].toString();
    ret.json = obj;
    return ret;
}

void MBExperimentManager::loadExperiments(const QString& json_str)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json_str.toLatin1(),&error);
    if (!error.errorString().isEmpty()) {
        qWarning() << "JSON Parse Error:" << error.errorString();
    }
    if (doc.isArray()) {
        QJsonArray A = doc.array();
        for (int i = 0; i < A.count(); i++) {
            QJsonValue val = A.at(i);
            addExperiment(create_experiment_from_json(val.toObject()));
        }
    } else if (doc.isObject()) {
        QJsonObject obj = doc.object();
        if (obj.contains("experiments")) {
            QJsonValue val = obj["experiments"];
            QByteArray str = QJsonDocument(val.toArray()).toJson();
            loadExperiments(str);
        }
    }
}

void MBExperimentManager::addExperiment(const MBExperiment& E)
{
    d->m_experiments[E.id] = E;
}

QStringList MBExperimentManager::allExperimentIds() const
{
    return d->m_experiments.keys();
}

MBExperiment MBExperimentManager::experiment(const QString& id)
{
    return d->m_experiments.value(id);
}
