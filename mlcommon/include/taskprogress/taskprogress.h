/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland, Witold Wysota
** Created: 4/30/2016
*******************************************************/

#ifndef TASKPROGRESS_H
#define TASKPROGRESS_H

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QDebug>
#include <QAbstractItemModel>
#include <QReadWriteLock>

struct TaskProgressLogMessage {
    QString message;
    QDateTime time;
    TaskProgressLogMessage()
    {
    }
    TaskProgressLogMessage(const QString& msg)
        : message(msg)
        , time(QDateTime::currentDateTime())
    {
    }
    TaskProgressLogMessage(const QDateTime& dt, const QString& msg)
        : message(msg)
        , time(dt)
    {
    }
};

struct TaskInfo {
    QSet<QString> tags;
    QString label;
    QString description;
    QList<TaskProgressLogMessage> log_messages;
    QString error;
    double progress;
    QDateTime start_time;
    QDateTime end_time;
};

Q_DECLARE_METATYPE(TaskInfo)

#if 0

class TaskProgressPrivate;
class TaskProgress : public QObject {
    Q_OBJECT
public:
    friend class TaskProgressPrivate;
    TaskProgress(const QString& label = "", const QString& description = "");
    virtual ~TaskProgress();
    void setLabel(const QString& label);
    void setDescription(const QString& description);
    void log(const QString& log_message);
    void error(const QString& error_message);
    void setProgress(double pct);

    TaskInfo getInfo() const;
signals:
    void changed();
    void completed(TaskInfo info);

private:
    TaskProgressPrivate* d;
};

class TaskProgressAgentPrivate;
class TaskProgressAgent : public QObject {
    Q_OBJECT
public:
    friend class TaskProgressAgentPrivate;
    TaskProgressAgent();
    virtual ~TaskProgressAgent();
    void addTask(TaskProgress* T);
    void removeTask(TaskProgress* T);
    void incrementQuantity(QString name, double val);
    double getQuantity(QString name);

    QList<TaskInfo> activeTasks();
    QList<TaskInfo> completedTasks();

    static TaskProgressAgent* globalInstance();
signals:
    void tasksChanged();
    void quantitiesChanged();
private slots:
    void slot_schedule_emit_tasks_changed();
    void slot_emit_tasks_changed();
    void slot_task_completed(TaskInfo info);

private:
    TaskProgressAgentPrivate* d;
};
#else

namespace TaskManager {
class TaskProgressAgent;
}

class TaskProgress : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString label READ label WRITE setLabel)
    Q_PROPERTY(QString description READ description WRITE setDescription)
public:
    enum StandardCategory {
        None = 0,
        Download = (1 << 0),
        Calculate = (1 << 1),
        Process = (1 << 2)
    };
    Q_DECLARE_FLAGS(StandardCategories, StandardCategory)

    TaskProgress();
    TaskProgress(const QString& label);
    TaskProgress(StandardCategories tags, const QString& label);
    ~TaskProgress();
    QString label() const;
    QString description() const;
    QSet<QString> tags() const;
    void setLabel(const QString& label);
    void setDescription(const QString& description);
    void addTag(StandardCategory);
    void addTag(const QString& tag);
    void removeTag(StandardCategory);
    void removeTag(const QString& tag);
    bool hasTag(StandardCategory) const;
    bool hasTag(const QString& tag) const;
    double progress() const;
public
slots:
    void log(const QString& log_message);
    void error(const QString& error_message);
    void setProgress(double pct);

protected:
    QStringList catsToString(StandardCategories) const;
    QString catToString(StandardCategory) const;
    TaskManager::TaskProgressAgent* agent() const
    {
        return m_agent;
    }

private:
    TaskManager::TaskProgressAgent* m_agent;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TaskProgress::StandardCategories);

#endif

namespace TaskManager {

class TaskProgressAgent : public QObject {
    Q_OBJECT
public:
    TaskProgressAgent(QObject* parent = 0);
    TaskProgressAgent(const QString& tag, QObject* parent = 0);
    void addTag(const QString&);
    void addTags(const QStringList& l);
#ifdef Q_COMPILER_INITIALIZER_LISTS
    void addTags(const std::initializer_list<QString>& l);
#endif
    void removeTag(const QString& t);
    bool hasTag(const QString& t) const;
    QSet<QString> tags() const;
    void setLabel(const QString&);
    void setDescription(const QString&);
    void setProgress(double);
    void log(const QString& log_message);
    void error(const QString& error_message);
    void finish();

    const QString& label() const;
    const QString& description() const;
    double progress() const;
    void lockForRead()
    {
        m_lock.lockForRead();
    }
    void lockForWrite()
    {
        m_lock.lockForWrite();
    }
    void unlock()
    {
        m_lock.unlock();
    }
    TaskInfo taskInfo() const;

    class ReadLocker {
    public:
        ~ReadLocker();
        ReadLocker(ReadLocker&& other);

    private:
        ReadLocker(const TaskProgressAgent* agent);
        friend class TaskProgressAgent;
        TaskProgressAgent* m_agent;
    };

    ReadLocker readLocker() const
    {
        return ReadLocker(this);
    }

signals:
    void changed();
    void changed(TaskProgressAgent*);
    void logAdded();
    void logAdded(TaskProgressAgent*);

private:
    mutable QReadWriteLock m_lock;
    TaskInfo m_info;
};

class TaskProgressMonitor : public QObject {
    Q_OBJECT
public:
    static void addTask(TaskProgressAgent* agent);
    static void removeTask(TaskProgressAgent* agent);

    virtual int count() const = 0;
    virtual TaskProgressAgent* at(int index) const = 0;
    virtual int indexOf(TaskProgressAgent*) const = 0;
    static TaskProgressMonitor* globalInstance();

    virtual void incrementQuantity(QString name, double val) = 0;
    virtual double getQuantity(QString name) const = 0;
signals:
    void added(TaskProgressAgent*);
    void changed(TaskProgressAgent*);
    void logAdded(TaskProgressAgent*);
    void moved(TaskProgressAgent*, int from, int to);
    void quantitiesChanged();
};

class TaskProgressModel : public QAbstractItemModel {
    Q_OBJECT
public:
    enum {
        ProgressRole = Qt::UserRole,
        StartTimeRole,
        EndTimeRole,
        TagsRole,
        LogRole,
        IndentedLogRole
    };
    enum {
        InvalidId = 0xDEADBEEF
    };
    TaskProgressModel(QObject* parent = 0);
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant taskData(const QModelIndex& index, int role = Qt::DisplayRole) const;
    QVariant logData(const QModelIndex& index, int role = Qt::DisplayRole) const;

protected:
    QString assembleLog(const TaskInfo& task, const QString& prefix = QString()) const;
    QString singleLog(const TaskProgressLogMessage& msg, const QString& prefix = QString()) const;
private
slots:
    void _q_added(TaskProgressAgent*);
    void _q_changed(TaskProgressAgent*);
    void _q_logAdded(TaskProgressAgent*);

private:
    TaskProgressMonitor* m_monitor;
};

class TaskProgressMonitorFilter : public QObject {
    Q_OBJECT
public:
    TaskProgressMonitorFilter(QObject* parent = 0);
    TaskProgressMonitorFilter(const QString& tag, QObject* parent = 0);
    TaskProgressMonitorFilter(const QStringList& tags, QObject* parent = 0);
    void addTag(const QString& tag);
    int count() const;
    TaskProgressAgent* at(int index) const;
    int indexOf(TaskProgressAgent* a) const;
signals:
    void added(TaskProgressAgent*);
    void changed(TaskProgressAgent*);
    void reset();
private
slots:
    void _q_added(TaskProgressAgent* a);
    void _q_changed(TaskProgressAgent* a);

protected:
    void initialize();
    bool matches(TaskProgressAgent* a) const;

private:
    QSet<QString> m_tags;
    QList<TaskProgressAgent*> m_agents;
};
}
#endif // TASKPROGRESS_H
