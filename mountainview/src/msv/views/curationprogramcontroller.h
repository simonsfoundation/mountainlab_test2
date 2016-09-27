#ifndef CURATIONPROGRAMCONTROLLER_H
#define CURATIONPROGRAMCONTROLLER_H

#include <QObject>
#include <QJSValue>
#include <mvcontext.h>

class CurationProgramControllerPrivate;
class CurationProgramController : public QObject
{
    Q_OBJECT
public:
    friend class CurationProgramControllerPrivate;
    CurationProgramController(MVContext *mvcontext);
    virtual ~CurationProgramController();

    QString log() const;

    Q_INVOKABLE QString clusterNumbers();
    Q_INVOKABLE double metric(int cluster_number, QString metric_name);
    Q_INVOKABLE void addTag(int cluster_number,QString tag_name);
    Q_INVOKABLE void removeTag(int cluster_number,QString tag_name);

    Q_INVOKABLE void log(const QString& message);

private:
    CurationProgramControllerPrivate *d;
};

#endif // CURATIONPROGRAMCONTROLLER_H
