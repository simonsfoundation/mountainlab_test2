#ifndef MVFILE_H
#define MVFILE_H

#include <QJsonObject>
#include <QObject>
#include <QString>

class MVFilePrivate;
class MVFile : public QObject {
    Q_OBJECT
public:
    friend class MVFilePrivate;
    MVFile();
    MVFile(const MVFile& other);
    virtual ~MVFile();
    void operator=(const MVFile& other);

    bool read(const QString& path);
    bool write(const QString& path);
    bool load(const QString& url);
    QString path() const;

    bool setJson(const QString& json);
    QString json() const;

    double sampleRate() const;
    QString mlproxyUrl() const;
    QString basePath() const;
    QString firingsPathResolved() const;
    QStringList timeseriesNames() const;
    QString timeseriesPathResolved(const QString& name) const;
    QJsonObject viewOptions() const;
    QJsonObject eventFilter() const;
    QJsonObject annotations() const;
    QString currentTimeseriesName() const;

    void setFiringsPath(QString path);
    void setSampleRate(double rate);
    void setMlproxyUrl(QString url);
    void addTimeseriesPath(const QString& name, const QString& path);
    void setViewOptions(QJsonObject obj);
    void setEventFilter(QJsonObject obj);
    void setAnnotations(QJsonObject obj);
    void setCurrentTimeseriesName(QString name);

    QString mvfile_version() const;

signals:
    void resultsChanged();

private:
    MVFilePrivate* d;
};

#endif // MVFILE_H
