/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland, Witold Wysota
** Created: 4/30/2016
*******************************************************/

#include "taskprogress.h"

#include <QTimer>
#include <QCoreApplication>
#include <QDebug>
#include <QMutex>
#include <QThread>
#ifdef QT_WIDGETS_LIB
#include <QColor>
#endif

#if 0

class TaskProgressPrivate {
public:
    TaskProgress* q;
    TaskInfo m_info;
};

class TaskProgressAgentPrivate {
public:
    TaskProgressAgent* q;
    QList<TaskInfo> m_completed_tasks;
    QList<TaskProgress*> m_active_tasks;
    bool m_emit_tasks_changed_scheduled;
    QDateTime m_last_emit_tasks_changed;
    QMutex m_addtask_removetask_mutex;
    QMap<QString, double> m_quantities;
    QMutex m_mutex;
};

TaskProgress::TaskProgress(const QString& label, const QString& description)
{
    d = new TaskProgressPrivate;
    d->q = this;

    d->m_info.label = label;
    d->m_info.description = description;
    d->m_info.progress = 0;
    d->m_info.start_time = QDateTime::currentDateTime();

    TaskProgressAgent::globalInstance()->addTask(this);
}

TaskProgress::~TaskProgress()
{
    this->log("Destructor");
    if (d->m_info.progress != 1) {
        d->m_info.end_time = QDateTime::currentDateTime();
    }
    TaskProgressAgent::globalInstance()->removeTask(this);
    delete d;
}

void TaskProgress::setLabel(const QString& label)
{
    {
        if (d->m_info.label == label)
            return;
        d->m_info.label = label;
        emit changed();
    }
    this->log(label);
}

void TaskProgress::setDescription(const QString& description)
{
    {
        if (d->m_info.description == description)
            return;
        d->m_info.description = description;
        emit changed();
    }
    this->log(description);
}

void TaskProgress::log(const QString& log_message)
{
    TaskProgressLogMessage MSG;
    MSG.message = log_message;
    MSG.time = QDateTime::currentDateTime();
    d->m_info.log_messages << MSG;
    emit changed();
}

void TaskProgress::error(const QString& error_message)
{
    qWarning() << "TaskProgress Error:: " + error_message;
    this->log("ERROR: " + error_message);
    {
        d->m_info.error = error_message;
    }
}

void TaskProgress::setProgress(double pct)
{
    if (d->m_info.progress == pct)
        return;
    d->m_info.progress = pct;
    emit changed();
    if (pct == 1) {
        d->m_info.end_time = QDateTime::currentDateTime();
        emit completed(d->m_info);
    }
}

TaskInfo TaskProgress::getInfo() const
{
    return d->m_info;
}

TaskProgressAgent::TaskProgressAgent()
{
    d = new TaskProgressAgentPrivate;
    d->q = this;
    d->m_emit_tasks_changed_scheduled = false;
}

TaskProgressAgent::~TaskProgressAgent()
{
    delete d;
}

QList<TaskInfo> TaskProgressAgent::activeTasks()
{
    QMutexLocker locker(&d->m_mutex);
    QList<TaskInfo> ret;
    QList<TaskProgress*> to_remove;
    foreach(TaskProgress * X, d->m_active_tasks)
    {
        TaskInfo info = X->getInfo();
        if (!info.label.isEmpty()) {
            if (info.progress < 1) {
                ret << X->getInfo();
            }
        }
    }

    return ret;
}

QList<TaskInfo> TaskProgressAgent::completedTasks()
{
    QMutexLocker locker(&d->m_mutex);
    for (int i = 0; i < d->m_completed_tasks.count(); i++) {
        d->m_completed_tasks[i].progress = 1; // kind of a hack to make sure the progress is 1 for all completed tasks
    }
    QList<TaskInfo> ret;
    for (int i = 0; i < d->m_completed_tasks.count(); i++) {
        if (!d->m_completed_tasks[i].label.isEmpty())
            ret << d->m_completed_tasks[i];
    }
    return ret;
}

/// Witold will this way work?
Q_GLOBAL_STATIC(QMutex, global_instance_mutex)
Q_GLOBAL_STATIC(TaskProgressAgent, theInstance)
TaskProgressAgent* TaskProgressAgent::globalInstance()
{
    QMutexLocker locker(global_instance_mutex);
    return theInstance;
}

void TaskProgressAgent::slot_schedule_emit_tasks_changed()
{
    if (d->m_emit_tasks_changed_scheduled)
        return;
    if (d->m_last_emit_tasks_changed.secsTo(QDateTime::currentDateTime()) >= 1) {
        slot_emit_tasks_changed();
    } else {
        d->m_emit_tasks_changed_scheduled = true;
        QTimer::singleShot(300, this, SLOT(slot_emit_tasks_changed()));
    }
}

void TaskProgressAgent::slot_emit_tasks_changed()
{
    {
        d->m_last_emit_tasks_changed = QDateTime::currentDateTime();
        d->m_emit_tasks_changed_scheduled = false;
    }
    emit tasksChanged();
}

void TaskProgressAgent::slot_task_completed(TaskInfo info)
{
    QMutexLocker locker(&d->m_mutex);
    d->m_completed_tasks.prepend(info);
}

void TaskProgressAgent::addTask(TaskProgress* T)
{
    {
        QMutexLocker locker(&d->m_mutex);
        QString label = T->getInfo().label;
        d->m_active_tasks.prepend(T);
        connect(T, SIGNAL(changed()), this, SLOT(slot_schedule_emit_tasks_changed()), Qt::QueuedConnection);
        connect(T, SIGNAL(completed(TaskInfo)), this, SLOT(slot_task_completed(TaskInfo)), Qt::QueuedConnection);
    }
    emit tasksChanged();
}

void TaskProgressAgent::removeTask(TaskProgress* T)
{
    {
        QMutexLocker locker(&d->m_mutex);
        QString label = T->getInfo().label;
        TaskInfo info = T->getInfo();
        if (info.progress != 1) {
            d->m_completed_tasks.prepend(T->getInfo());
        }
        d->m_active_tasks.removeAll(T);
    }
    emit tasksChanged();
}

void TaskProgressAgent::incrementQuantity(QString name, double val)
{
    {
        QMutexLocker locker(&d->m_mutex);
        d->m_quantities[name] = d->m_quantities[name] + val;
    }
    emit quantitiesChanged();
}

double TaskProgressAgent::getQuantity(QString name)
{
    QMutexLocker locker(&d->m_mutex);
    return d->m_quantities.value(name);
}

#else

TaskProgress::TaskProgress()
    : QObject()
{
    m_agent = new TaskManager::TaskProgressAgent(this);
    TaskManager::TaskProgressMonitor::addTask(m_agent);
}

TaskProgress::TaskProgress(const QString& label)
    : QObject()
{
    m_agent = new TaskManager::TaskProgressAgent(this);
    m_agent->setLabel(label);
    TaskManager::TaskProgressMonitor::addTask(m_agent);
}

TaskProgress::TaskProgress(StandardCategories tags, const QString& label)
    : QObject()
{
    m_agent = new TaskManager::TaskProgressAgent(this);
    m_agent->setLabel(label);
    m_agent->addTags(catsToString(tags));
    TaskManager::TaskProgressMonitor::addTask(m_agent);
}

TaskProgress::~TaskProgress()
{
    m_agent->finish();
    // the following reparents the agent to the monitor
    TaskManager::TaskProgressMonitor::removeTask(m_agent);
}

QString TaskProgress::label() const
{
    return agent()->label();
}

QString TaskProgress::description() const
{
    return agent()->description();
}

QSet<QString> TaskProgress::tags() const
{
    return agent()->tags();
}

void TaskProgress::setLabel(const QString& label)
{
    agent()->setLabel(label);
}

void TaskProgress::setDescription(const QString& description)
{
    agent()->setDescription(description);
}

void TaskProgress::addTag(TaskProgress::StandardCategory cat)
{
    QString catName = catToString(cat);
    if (catName.isEmpty())
        return;
    agent()->addTag(catName);
}

void TaskProgress::addTag(const QString& tag)
{
    agent()->addTag(tag);
}

void TaskProgress::removeTag(TaskProgress::StandardCategory cat)
{
    QString catName = catToString(cat);
    if (catName.isEmpty())
        return;
    agent()->removeTag(catName);
}

void TaskProgress::removeTag(const QString& tag)
{
    agent()->removeTag(tag);
}

bool TaskProgress::hasTag(TaskProgress::StandardCategory cat) const
{
    QString catName = catToString(cat);
    if (catName.isEmpty())
        return false;
    return agent()->hasTag(catName);
}

bool TaskProgress::hasTag(const QString& tag) const
{
    return agent()->hasTag(tag);
}

double TaskProgress::progress() const
{
    return agent()->progress();
}

void TaskProgress::log(const QString& log_message)
{
    agent()->log(log_message);
}

void TaskProgress::error(const QString& error_message)
{
    agent()->error(error_message);
}

void TaskProgress::setProgress(double pct)
{
    agent()->setProgress(pct);
}

QStringList TaskProgress::catsToString(StandardCategories cats) const
{
    QStringList result;
    if (cats.testFlag(Download))
        result << "download";
    if (cats.testFlag(Calculate))
        result << "calculate";
    if (cats.testFlag(Process))
        result << "process";
    return result;
}

QString TaskProgress::catToString(TaskProgress::StandardCategory cat) const
{
    switch (cat) {
    case Download:
        return "download";
    case Calculate:
        return "calculate";
    case Process:
        return "process";
    case None:
    default:
        return QString();
    }
}

#endif

namespace TaskManager {

/*!
 * \section4 Lock Ordering
 *
 * To avoid deadlocks the order of grabbing locks has to be maintained.
 * The rule is as follows:
 * \list
 *   \li Grab a lock on private instance of TaskProgressMonitor
 *   \li Now you can grab a lock on individual items
 * \endlist
 *
 * You can release the locks in arbitrary order.
 *
 * After you release lock on TaskProgressMonitor, you cannot lock it back again
 * without prior releasing all locks on individual items.
 *
 * In case you already have a pointer to an individual item, you can lock it directly
 * without grabbing the lock on the master object.
 *
 */

class TaskProgressMonitorPrivate : public TaskProgressMonitor {
public:
    TaskProgressMonitorPrivate()
        : lock(QMutex::Recursive)
    {
    }
    ~TaskProgressMonitorPrivate()
    {
        // tasks we own will get destroyed
        // tasks we don't own will get ignored
    }
    void add(TaskProgressAgent* agent)
    {
        // lock is already taken
        connect(agent,
            static_cast<void (TaskProgressAgent::*)(TaskProgressAgent*)>(&TaskProgressAgent::changed),
            [this](TaskProgressAgent* agent) {
            update(agent);
            });
        connect(agent,
            static_cast<void (TaskProgressAgent::*)(TaskProgressAgent*)>(&TaskProgressAgent::logAdded),
            [this](TaskProgressAgent* agent) {
            addLog(agent);
            });
        m_data.prepend(agent);
        emit added(agent);
    }

    void update(TaskProgressAgent* agent)
    {
        emit changed(agent);
    }
    void addLog(TaskProgressAgent* agent)
    {
        emit logAdded(agent);
    }

    int count() const override
    {
        QMutexLocker locker(&lock);
        return m_data.count();
    }
    TaskProgressAgent* at(int index) const override
    {
        QMutexLocker locker(&lock);
        return m_data.at(index);
    }

    int indexOf(TaskProgressAgent* agent) const override
    {
        QMutexLocker locker(&lock);
        return m_data.indexOf(agent);
    }
    void move(TaskProgressAgent* agent, int to)
    {
        QMutexLocker locker(&lock);
        move(indexOf(agent), to);
    }

    void move(int from, int to)
    {
        QMutexLocker locker(&lock);
        TaskProgressAgent* agent = at(from);
        m_data.move(from, to);
        emit moved(agent, from, to);
    }

    void incrementQuantity(QString name, double val) override
    {
        QMutexLocker locker(&lock);
        m_quantities.insert(name, m_quantities.value(name, 0) + val);
        emit quantitiesChanged();
    }
    double getQuantity(QString name) const override
    {
        QMutexLocker locker(&lock);
        return m_quantities.value(name, 0);
    }

    mutable QMutex lock;

    static TaskProgressMonitorPrivate* privateInstance();

private:
    QList<TaskProgressAgent*> m_data;
    QMap<QString, double> m_quantities;

    // TaskProgressMonitor interface
public:
};

Q_GLOBAL_STATIC(TaskProgressMonitorPrivate, _q_tpm_instance)

TaskProgressMonitor* TaskProgressMonitor::globalInstance()
{
    return _q_tpm_instance;
}

void TaskProgressMonitor::addTask(TaskProgressAgent* agent)
{
    TaskProgressMonitorPrivate* instance = TaskProgressMonitorPrivate::privateInstance();
    QMutexLocker locker(&instance->lock);
    instance->add(agent);
}

void TaskProgressMonitor::removeTask(TaskProgressAgent* agent)
{
    // this doesn't actually remove the agent but rather marks it as complete
    // this means the TaskProgress object has released its ownership and we can take it
    TaskProgressMonitorPrivate* instance = TaskProgressMonitorPrivate::privateInstance();
    QMutexLocker locker(&instance->lock);
    // This is going to work only if removeTask() is called from the thread owning the agent
    Q_ASSERT(QThread::currentThread() == agent->thread());
    TaskProgressAgent::ReadLocker readLocker = agent->readLocker();
    agent->setParent(0);
    agent->moveToThread(instance->thread());
    agent->setParent(instance);

    /// TODO: now the manager can reposition the agent on its list
}

TaskProgressMonitorPrivate* TaskProgressMonitorPrivate::privateInstance()
{
    return _q_tpm_instance;
}

TaskProgressAgent::TaskProgressAgent(QObject* parent)
    : QObject(parent)
{
    m_info.start_time = QDateTime::currentDateTime();
    m_info.progress = 0;
}

TaskProgressAgent::TaskProgressAgent(const QString& tag, QObject* parent)
    : QObject(parent)
{
    m_info.start_time = QDateTime::currentDateTime();
    m_info.progress = 0;
    m_info.tags.insert(tag);
}

void TaskProgressAgent::addTag(const QString& t)
{
    lockForWrite();
    m_info.tags << t;
    unlock();
    emit changed();
    emit changed(this);
}

void TaskProgressAgent::addTags(const std::initializer_list<QString>& l)
{
    lockForWrite();
    foreach (const QString& t, l)
        m_info.tags << t;
    unlock();
    emit changed();
    emit changed(this);
}

void TaskProgressAgent::removeTag(const QString& t)
{
    lockForWrite();
    m_info.tags.remove(t);
    unlock();
    emit changed();
    emit changed(this);
}

#ifdef Q_COMPILER_INITIALIZER_LISTS
void TaskProgressAgent::addTags(const QStringList& l)
{
    lockForWrite();
    foreach (const QString& t, l)
        m_info.tags << t;
    unlock();
    emit changed();
    emit changed(this);
}
#endif

bool TaskProgressAgent::hasTag(const QString& t) const
{
    QReadLocker l(&m_lock);
    return m_info.tags.contains(t);
}

QSet<QString> TaskProgressAgent::tags() const
{
    QReadLocker l(&m_lock);
    return m_info.tags;
}

void TaskProgressAgent::setLabel(const QString& l)
{
    lockForWrite();
    m_info.label = l;
    unlock();
    emit changed();
    emit changed(this);
}

void TaskProgressAgent::setDescription(const QString& d)
{
    lockForWrite();
    m_info.description = d;
    unlock();
    emit changed();
    emit changed(this);
}

void TaskProgressAgent::setProgress(double p)
{
    lockForWrite();
    m_info.progress = p;
    if (m_info.progress >= 1.0) {
        m_info.end_time = QDateTime::currentDateTime();
    }
    unlock();
    emit changed();
    emit changed(this);
}

void TaskProgressAgent::log(const QString& log_message)
{
    lockForWrite();
    m_info.log_messages.append(log_message);
    unlock();
    emit logAdded();
    emit logAdded(this);
}

void TaskProgressAgent::error(const QString& error_message)
{
    lockForWrite();
    m_info.log_messages.append(tr("Error: %1").arg(error_message));
    m_info.error = error_message;
    unlock();
    emit changed();
    emit changed(this);
    emit logAdded();
    emit logAdded(this);
}

void TaskProgressAgent::finish()
{
    lockForWrite();
    if (m_info.progress < 1.0) {
        m_info.end_time = QDateTime::currentDateTime();
        m_info.progress = 1;
    }
    unlock();
}

const QString& TaskProgressAgent::label() const
{
    QReadLocker l(&m_lock);
    return m_info.label;
}

const QString& TaskProgressAgent::description() const
{
    QReadLocker l(&m_lock);
    return m_info.description;
}

double TaskProgressAgent::progress() const
{
    QReadLocker l(&m_lock);
    return m_info.progress;
}

TaskInfo TaskProgressAgent::taskInfo() const
{
    QReadLocker l(&m_lock);
    return m_info;
}

TaskProgressMonitorFilter::TaskProgressMonitorFilter(QObject* parent)
    : QObject(parent)
{
    TaskProgressMonitor* monitor = TaskProgressMonitor::globalInstance();
    connect(monitor, SIGNAL(added(TaskProgressAgent*)), SLOT(_q_added(TaskProgressAgent*)));
    connect(monitor, SIGNAL(changed(TaskProgressAgent*)), SLOT(_q_changed(TaskProgressAgent*)));
}

TaskProgressMonitorFilter::TaskProgressMonitorFilter(const QString& tag, QObject* parent)
    : QObject(parent)
{
    m_tags.insert(tag);
    TaskProgressMonitor* monitor = TaskProgressMonitor::globalInstance();
    connect(monitor, SIGNAL(added(TaskProgressAgent*)), SLOT(_q_added(TaskProgressAgent*)));
    connect(monitor, SIGNAL(changed(TaskProgressAgent*)), SLOT(_q_changed(TaskProgressAgent*)));
    initialize();
}

TaskProgressMonitorFilter::TaskProgressMonitorFilter(const QStringList& tags, QObject* parent)
    : QObject(parent)
{
    m_tags = tags.toSet();
    TaskProgressMonitor* monitor = TaskProgressMonitor::globalInstance();
    connect(monitor, SIGNAL(added(TaskProgressAgent*)), SLOT(_q_added(TaskProgressAgent*)));
    connect(monitor, SIGNAL(changed(TaskProgressAgent*)), SLOT(_q_changed(TaskProgressAgent*)));
    initialize();
}

void TaskProgressMonitorFilter::addTag(const QString& tag)
{
    m_tags.insert(tag);
    m_agents.clear();
    initialize();
}

int TaskProgressMonitorFilter::count() const
{
    return m_agents.size();
}

TaskProgressAgent* TaskProgressMonitorFilter::at(int index) const
{
    return m_agents.at(index);
}

int TaskProgressMonitorFilter::indexOf(TaskProgressAgent* a) const
{
    return m_agents.indexOf(a);
}

void TaskProgressMonitorFilter::_q_added(TaskProgressAgent* a)
{
    if (matches(a)) {
        m_agents.append(a);
        emit added(a);
    }
}

void TaskProgressMonitorFilter::_q_changed(TaskProgressAgent* a)
{
    bool didMatch = m_agents.contains(a);
    bool nowMatch = matches(a);
    // four cases:
    // 1. doesn't and didn't matches(a)
    if (!didMatch && !nowMatch)
        return;
    // 2. did matches(a) and still does
    if (didMatch && nowMatch) {
        emit changed(a);
        return;
    }
    // 3. used to matches(a)
    if (didMatch && !nowMatch) {
        m_agents.removeOne(a);
        emit changed(a);
        return;
    }
    // 4. didn't use to matches(a)
    if (!didMatch && nowMatch) {
        m_agents.append(a);
        emit changed(a);
    }
}

void TaskProgressMonitorFilter::initialize()
{
    TaskProgressMonitorPrivate* monitor = TaskProgressMonitorPrivate::privateInstance();
    QMutexLocker locker(&monitor->lock);
    for (int i = 0; i < monitor->count(); ++i) {
        TaskProgressAgent* a = monitor->at(i);
        TaskProgressAgent::ReadLocker itemLocker = a->readLocker();
        if (!matches(a))
            continue;
        m_agents.append(a);
    }
    locker.unlock();
    emit reset();
}

bool TaskProgressMonitorFilter::matches(TaskProgressAgent* a) const
{

#if QT_VERSION >= 0x050600
    return m_tags.intersects(a->tags());
#else
    return !QSet<QString>(m_tags).intersect(a->tags()).isEmpty();
#endif
}

TaskProgressModel::TaskProgressModel(QObject* parent)
    : QAbstractItemModel(parent)
{
    m_monitor = TaskProgressMonitor::globalInstance();
    connect(m_monitor, SIGNAL(added(TaskProgressAgent*)),
        this, SLOT(_q_added(TaskProgressAgent*)));
    connect(m_monitor, SIGNAL(changed(TaskProgressAgent*)),
        this, SLOT(_q_changed(TaskProgressAgent*)));
    connect(m_monitor, SIGNAL(logAdded(TaskProgressAgent*)),
        this, SLOT(_q_logAdded(TaskProgressAgent*)));
}

QModelIndex TaskProgressModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid() && parent.internalId() != InvalidId)
        return QModelIndex();
    if (parent.isValid()) {
        return createIndex(row, column, parent.row());
    }
    return createIndex(row, column, InvalidId);
}

QModelIndex TaskProgressModel::parent(const QModelIndex& child) const
{
    if (child.internalId() == InvalidId)
        return QModelIndex();
    return createIndex(child.internalId(), 0, InvalidId);
}

int TaskProgressModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() && parent.internalId() != InvalidId)
        return 0;
    if (parent.isValid()) {
        if (parent.row() < 0)
            return 0;
        TaskProgressAgent* agent = m_monitor->at(parent.row());
        TaskInfo task = agent->taskInfo();
        return task.log_messages.size();
    }
    return m_monitor->count();
}

int TaskProgressModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 2;
    return 2;
}

QVariant TaskProgressModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (index.parent().isValid()) {
        return logData(index, role);
    }
    return taskData(index, role);
}

QVariant TaskProgressModel::taskData(const QModelIndex& index, int role) const
{
    if (index.column() != 0)
        return QVariant();
    TaskProgressAgent* agent = m_monitor->at(index.row());
    TaskInfo task = agent->taskInfo();
    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        // modified by jfm -- 5/17/2016
        if (!task.error.isEmpty()) {
            return task.label + ": " + task.error;
        }
        else {
            return task.label;
        }
    case Qt::ToolTipRole:
        // modified by jfm -- 5/17/2016
        if (!task.description.isEmpty())
            return task.description;
        else
            return taskData(index, Qt::DisplayRole);
#ifdef QT_WIDGETS_LIB
    case Qt::ForegroundRole: {
        // modified by jfm -- 5/17/2016
        if (!task.error.isEmpty()) {
            return QColor(Qt::red);
        }
        else {
            if (task.progress < 1)
                return QColor(Qt::blue);
        }
        return QVariant();
    }
#endif
    case ProgressRole:
        return task.progress;
    case StartTimeRole:
        return task.start_time;
    case EndTimeRole:
        return task.end_time;
    case TagsRole:
        return QStringList(task.tags.toList());
    case LogRole:
        return assembleLog(task);
    case IndentedLogRole:
        return assembleLog(task, "\t");
    }
    return QVariant();
}

QVariant TaskProgressModel::logData(const QModelIndex& index, int role) const
{
    TaskProgressAgent* agent = m_monitor->at(index.internalId());
    TaskInfo task = agent->taskInfo();
    const auto& logMessages = task.log_messages;
    auto logMessage = logMessages.at(logMessages.count() - 1 - index.row()); // newest first
    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        if (index.column() == 0)
            return logMessage.time;
        return logMessage.message;
    case Qt::UserRole:
        return logMessage.time;
    case LogRole:
        return singleLog(logMessage);
    case IndentedLogRole:
        return singleLog(logMessage, "\t");
    default:
        return QVariant();
    }
}

QString TaskProgressModel::assembleLog(const TaskInfo& task, const QString& prefix) const
{
    QStringList entries;
    foreach (const TaskProgressLogMessage& msg, task.log_messages) {
        entries << singleLog(msg, prefix);
    }
    return entries.join("\n");
}

QString TaskProgressModel::singleLog(const TaskProgressLogMessage& msg, const QString& prefix) const
{
    //return QString("%1%2: %3").arg(prefix).arg(msg.time.toString(Qt::ISODate)).arg(msg.message);
    return QString("%1%2: %3").arg(prefix).arg(msg.time.toString("yyyy-MM-dd hh:mm:ss.zzz")).arg(msg.message);
}

void TaskProgressModel::_q_added(TaskProgressAgent* a)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    endInsertRows();
}

void TaskProgressModel::_q_changed(TaskProgressAgent* a)
{
    int idx = m_monitor->indexOf(a);
    if (idx >= 0) {
        QModelIndex modelIdxFirst = createIndex(idx, 0, InvalidId);
        QModelIndex modelIdxLast = createIndex(idx, columnCount(), InvalidId);
        emit dataChanged(modelIdxFirst, modelIdxLast);
    }
}

void TaskProgressModel::_q_logAdded(TaskProgressAgent* a)
{
    int idx = m_monitor->indexOf(a);
    if (idx < 0)
        return;
    QModelIndex parentIndex = createIndex(idx, 0, InvalidId);
    TaskInfo taskInfo = a->taskInfo();
    beginInsertRows(parentIndex, taskInfo.log_messages.size(), taskInfo.log_messages.size());
    endInsertRows();
}

TaskProgressAgent::ReadLocker::~ReadLocker()
{
    if (m_agent)
        m_agent->unlock();
}

TaskProgressAgent::ReadLocker::ReadLocker(TaskProgressAgent::ReadLocker&& other)
{
    m_agent = other.m_agent;
    other.m_agent = 0;
}

TaskProgressAgent::ReadLocker::ReadLocker(const TaskProgressAgent* agent)
    : m_agent(const_cast<TaskProgressAgent*>(agent))
{
    m_agent->lockForRead();
}
}
