#ifndef MOFILE_H
#define MOFILE_H

#include <QJsonObject>
#include <QObject>
#include <QString>

class MOFilePrivate;
class MOFile : public QObject {
    Q_OBJECT
public:
    friend class MOFilePrivate;
    MOFile();
    virtual ~MOFile();

    bool read(const QString& path);
    bool write(const QString& path);
    bool load(const QString& url);

    bool setJson(const QString& json);
    QString json() const;

    QString mofile_version() const;

    QStringList resultNames();
    QJsonObject result(QString name);
    void removeResult(QString name);
    void addResult(QJsonObject result);

signals:
    void resultsChanged();

private:
    MOFilePrivate* d;
};

#endif // MOFILE_H
