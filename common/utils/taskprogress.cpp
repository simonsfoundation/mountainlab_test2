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
        qRegisterMetaType<TaskInfo>();
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
        QMutexLocker locker(&lock);
        // if progress reaches 1.0, we should "complete" the task
        // by moving it in the list

        if (agent->progress() >= 1.0)
            complete(agent);
        emit changed(agent);
    }
    void addLog(TaskProgressAgent* agent)
    {
        emit logAdded(agent);
    }

    int count() const override
    {
        QMutexLocker locker(&lock);
        return m_completedData.count()+m_data.count();
    }
    TaskProgressAgent* at(int index) const override
    {
        QMutexLocker locker(&lock);
        const int adc = m_data.count();
        if (index < adc)
            return m_data.at(index);
        if (m_completedData.count() <= index-adc) return 0;
        return m_completedData.at(index-adc);
    }

    int indexOf(TaskProgressAgent* agent) const override
    {
        QMutexLocker locker(&lock);
        int idx = m_data.indexOf(agent);
        if (idx >=0) return idx;
        const int adc = m_data.count();
        return m_completedData.indexOf(agent)+adc;
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
        m_data.move(from, to); /// FIXME
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
    void complete(TaskProgressAgent *agent) {
        QMutexLocker locker(&lock);
        const int idx = m_data.indexOf(agent);
        if (idx < 0) return; // doesn't exist or already complete
        const int adc = m_data.count();
        m_completedData.prepend(agent);
        m_data.removeAt(idx);
        if (idx == adc-1) return; // didn't really move
        emit moved(agent, idx, adc);
    }
    void complete(int idx) {
        QMutexLocker locker(&lock);
        const int adc = m_data.count();
        if (idx >= adc) return; // already complete
        TaskProgressAgent *agent = m_data.at(idx);
        m_completedData.prepend(agent);
        m_data.removeAt(idx);
        if (idx == adc-1) return; // didn't really move
        emit moved(agent, idx, adc);
    }

    mutable QMutex lock;

    static TaskProgressMonitorPrivate* privateInstance();

private:
    QList<TaskProgressAgent*> m_completedData;
    QList<TaskProgressAgent*> m_data;
    QMap<QString, double> m_quantities;

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
    instance->complete(agent);
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
    connect(m_monitor, SIGNAL(moved(TaskProgressAgent*,int,int)),
            this, SLOT(_q_moved(TaskProgressAgent*, int, int)));
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
        return 0; // children of a log entry
    if (parent.isValid()) {
        if (parent.row() < 0)
            return 0;

        TaskProgressAgent* agent = m_data.at(parent.row());
        agent->lockForRead();
//        if (!agent) return 0;
        TaskInfo task = agent->taskInfo();
        int s = task.log_messages.size();
        agent->unlock();
        return s;
    }
    return m_data.count();
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
    TaskProgressAgent* agent = m_data.at(index.row());
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
    case StatusRole:
        if (task.end_time.isValid() && task.progress >= 1)
            return Finished;
        if (task.end_time.isValid())
            return Canceled;
        return Active;
    }
    return QVariant();
}

QVariant TaskProgressModel::logData(const QModelIndex& index, int role) const
{
    TaskProgressAgent* agent = m_data.at(index.internalId());
    TaskInfo task = agent->taskInfo();
    const auto& logMessages = task.log_messages;
    auto logMessage = logMessages.at(logMessages.count() - 1 - index.row()); // newest first
    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
//        return logMessage.time.toString(Qt::SystemLocaleShortDate) + "\t" + logMessage.message;
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

bool TaskProgressModel::isActive(const QModelIndex &task) const
{
    if (!isTask(task)) return false;
    return (task.data(StatusRole).toInt() == Active);
}

bool TaskProgressModel::isCompletedWithin(const QModelIndex &task, int time) const
{
    if (!isTask(task)) return false;
    Status s = (Status)task.data(StatusRole).toInt();
    if (s == Active) return true;
    const QDateTime dt = task.data(EndTimeRole).toDateTime();
    if (dt.addSecs(time) >= QDateTime::currentDateTime()) return true;
    return false;
}

bool TaskProgressModel::isTask(const QModelIndex &idx) const
{
    return !idx.parent().isValid();
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
//    TaskProgressMonitorPrivate *p = TaskProgressMonitorPrivate::privateInstance();
//    QMutexLocker l(&p->lock);

//    int idx = m_monitor->indexOf(a);
//    qDebug() << Q_FUNC_INFO << idx;
    beginInsertRows(QModelIndex(), 0, 0);
    m_data.prepend(a);
//    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    endInsertRows();
}

void TaskProgressModel::_q_changed(TaskProgressAgent* a)
{
//    TaskProgressMonitorPrivate *p = TaskProgressMonitorPrivate::privateInstance();
//    QMutexLocker l(&p->lock);
    int idx = m_data.indexOf(a);
    if (idx >= 0) {
        QModelIndex modelIdxFirst = createIndex(idx, 0, InvalidId);
        QModelIndex modelIdxLast = createIndex(idx, columnCount(), InvalidId);
        emit dataChanged(modelIdxFirst, modelIdxLast);
    }
}

void TaskProgressModel::_q_logAdded(TaskProgressAgent* a)
{
    a->lockForRead();
//    TaskProgressMonitorPrivate *p = TaskProgressMonitorPrivate::privateInstance();
//    QMutexLocker l(&p->lock);
    int idx = m_data.indexOf(a);
    if (idx < 0)
        return;
    QModelIndex parentIndex = createIndex(idx, 0, InvalidId);
    TaskInfo taskInfo = a->taskInfo();
//    beginInsertRows(parentIndex, taskInfo.log_messages.size()-1, taskInfo.log_messages.size()-1);
    beginInsertRows(parentIndex, 0, 0);
    endInsertRows();
    a->unlock();
}

void TaskProgressModel::_q_moved(TaskProgressAgent *a, int from, int to)
{
    beginMoveRows(QModelIndex(), from, from, QModelIndex(), to);
    m_data.move(from, to-1);
    endMoveRows();
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
