/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/30/2016
*******************************************************/

#include "taskprogress.h"

#include <QTimer>

class TaskProgressPrivate {
public:
    TaskProgress *q;

    TaskInfo m_info;
};

class TaskProgressAgentPrivate {
public:
    TaskProgressAgent *q;
    QList<TaskInfo> m_inactive_tasks;
    QList<TaskProgress *> m_active_tasks;
    bool m_emit_tasks_changed_scheduled;

    void report_change();
    void schedule_emit_tasks_changed();
};

TaskProgress::TaskProgress(const QString &label,const QString &description)
{
    d=new TaskProgressPrivate;
    d->q=this;

    d->m_info.label=label;
    d->m_info.description=description;
    d->m_info.progress=0;
    d->m_info.start_time=QDateTime::currentDateTime();

    TaskProgressAgent::globalInstance()->d->m_active_tasks << this;
    TaskProgressAgent::globalInstance()->d->report_change();
}

TaskProgress::~TaskProgress()
{
    this->setProgress(1);
    TaskProgressAgent::globalInstance()->d->m_inactive_tasks << this->getInfo();
    TaskProgressAgent::globalInstance()->d->m_active_tasks.removeAll(this);
    TaskProgressAgent::globalInstance()->d->report_change();
    delete d;
}


void TaskProgress::setLabel(const QString &label)
{
    if (d->m_info.label==label) return;
    d->m_info.label=label;
    TaskProgressAgent::globalInstance()->d->report_change();
}


void TaskProgress::setDescription(const QString &description)
{
    if (d->m_info.description==description) return;
    d->m_info.description=description;
    TaskProgressAgent::globalInstance()->d->report_change();
}


void TaskProgress::log(const QString &log_message)
{
    TaskProgressLogMessage MSG;
    MSG.message=log_message;
    MSG.time=QDateTime::currentDateTime();
    d->m_info.log_messages << MSG;
    TaskProgressAgent::globalInstance()->d->report_change();
}


void TaskProgress::setProgress(double pct)
{
    if (d->m_info.progress==pct) return;
    d->m_info.progress=pct;
    if (pct==1) {
        d->m_info.end_time=QDateTime::currentDateTime();
    }
    TaskProgressAgent::globalInstance()->d->report_change();
}

TaskInfo TaskProgress::getInfo() const
{
    return d->m_info;
}

TaskProgressAgent::TaskProgressAgent()
{
    d=new TaskProgressAgentPrivate;
    d->q=this;
    d->m_emit_tasks_changed_scheduled=false;
}

TaskProgressAgent::~TaskProgressAgent()
{
    delete d;
}

QList<TaskInfo> TaskProgressAgent::activeTasks()
{
    QList<TaskInfo> ret;
    foreach (TaskProgress *X,d->m_active_tasks) {
        ret << X->getInfo();
    }
    return ret;
}

QList<TaskInfo> TaskProgressAgent::inactiveTasks()
{
    return d->m_inactive_tasks;
}

Q_GLOBAL_STATIC(TaskProgressAgent,theInstance)
TaskProgressAgent *TaskProgressAgent::globalInstance()
{
    return theInstance;
}

void TaskProgressAgent::slot_emit_tasks_changed()
{
    d->m_emit_tasks_changed_scheduled=false;
    emit tasksChanged();
}


void TaskProgressAgentPrivate::report_change()
{
    schedule_emit_tasks_changed();
}

void TaskProgressAgentPrivate::schedule_emit_tasks_changed()
{
    if (m_emit_tasks_changed_scheduled) return;
    int msec=1000;
    m_emit_tasks_changed_scheduled=true;
    QTimer::singleShot(msec,q,SLOT(slot_emit_tasks_changed()));

}
