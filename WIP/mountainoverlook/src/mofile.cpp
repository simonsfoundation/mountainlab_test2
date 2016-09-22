#include "mofile.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include "mlnetwork.h"

#include "mlcommon.h"

class MOFilePrivate {
public:
    MOFile* q;

    QJsonObject m_obj;
};

MOFile::MOFile()
{
    d = new MOFilePrivate;
    d->q = this;
    d->m_obj["mofile_version"] = "0.1";
}

MOFile::~MOFile()
{
    delete d;
}

bool MOFile::read(const QString& path)
{
    QString json = TextFile::read(path);
    if (json.isEmpty())
        return false;
    bool ret = this->setJson(json);
    emit resultsChanged();
    return ret;
}

bool MOFile::write(const QString& path)
{
    return TextFile::write(path, this->json());
}

bool MOFile::load(const QString& url)
{
    QString json = MLNetwork::httpGetText(url);
    bool ret = setJson(json);
    emit resultsChanged();
    return ret;
}

bool MOFile::setJson(const QString& json)
{
    QJsonParseError err;
    d->m_obj = QJsonDocument::fromJson(json.toLatin1(), &err).object();
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "Problem parsing json in OFile::setJson";
        return false;
    }
    return true;
}

QString MOFile::json() const
{
    return QJsonDocument(d->m_obj).toJson();
}

QString MOFile::mofile_version() const
{
    return d->m_obj["mofile_version"].toString();
}

QStringList MOFile::resultNames()
{
    QStringList ret;
    QJsonArray X = d->m_obj["results"].toArray();
    for (int i = 0; i < X.count(); i++) {
        ret << X.at(i).toObject()["name"].toString();
    }
    qSort(ret);
    return ret;
}

QJsonObject MOFile::result(QString name)
{
    QJsonArray X = d->m_obj["results"].toArray();
    for (int i = 0; i < X.count(); i++) {
        QString name0 = X.at(i).toObject()["name"].toString();
        if (name0 == name) {
            return X.at(i).toObject();
        }
    }
    return QJsonObject();
}

void MOFile::removeResult(QString name)
{
    QJsonArray X = d->m_obj["results"].toArray();
    for (int i = 0; i < X.count(); i++) {
        QString name0 = X.at(i).toObject()["name"].toString();
        if (name0 == name) {
            X.removeAt(i);
            break;
        }
    }
    d->m_obj["results"] = X;
    emit resultsChanged();
}

void MOFile::addResult(QJsonObject result)
{
    QString name = result["name"].toString();
    QJsonArray X = d->m_obj["results"].toArray();
    bool already_added = false;
    for (int i = 0; i < X.count(); i++) {
        QString name0 = X.at(i).toObject()["name"].toString();
        if (name0 == name) {
            X[i] = result;
            already_added = true;
            break;
        }
    }
    if (!already_added) {
        X.append(result);
    }
    d->m_obj["results"] = X;
    emit resultsChanged();
}
