/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/30/2016
*******************************************************/

#ifndef TASKPROGRESS_H
#define TASKPROGRESS_H

#include <QDateTime>
#include <QObject>
#include <QString>

struct TaskProgressLogMessage {
    QString message;
    QDateTime time;
};

struct TaskInfo {
    QString label;
    QString description;
    QList<TaskProgressLogMessage> log_messages;
    double progress;
    QDateTime start_time;
    QDateTime end_time;
};

class TaskProgressPrivate;
class TaskProgress
{
public:
    friend class TaskProgressPrivate;
    TaskProgress(const QString &label="",const QString &description="");
    virtual ~TaskProgress();
    void setLabel(const QString &label);
    void setDescription(const QString &description);
    void log(const QString &log_message);
    void setProgress(double pct);

    TaskInfo getInfo() const;
private:
    TaskProgressPrivate *d;
};

class TaskProgressAgentPrivate;
class TaskProgressAgent : public QObject {
    Q_OBJECT
public:
    friend class TaskProgressAgentPrivate;
    friend class TaskProgress;
    friend class TaskProgressPrivate;
    TaskProgressAgent();
    virtual ~TaskProgressAgent();

    QList<TaskInfo> activeTasks();
    QList<TaskInfo> inactiveTasks();

    static TaskProgressAgent *globalInstance();
signals:
    void tasksChanged();
private slots:
    void slot_emit_tasks_changed();
private:
    TaskProgressAgentPrivate *d;
};

#endif // TASKPROGRESS_H

