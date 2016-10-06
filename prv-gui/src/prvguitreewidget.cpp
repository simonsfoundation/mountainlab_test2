/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 9/29/2016
*******************************************************/

#include "locatemanager.h"
#include "prvguitreewidget.h"

#include <QApplication>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QMutex>
#include <QProcess>
#include <QSplitter>
#include <QTimer>
#include <QTreeWidget>
#include <taskprogress.h>
#include "taskprogressview.h"
#include "prvguicontrolpanel.h"

class PrvGuiTreeWidgetPrivate {
public:
    PrvGuiTreeWidget* q;
    QList<PrvRecord> m_prvs;
    QStringList m_server_names;
    bool m_dirty = false;

    //PrvGuiWorkerThread m_worker_thread;
    LocateManager m_locate_manager;

    void create_tree();
    //void restart_worker_thread();
    void update_tree_item_data(QTreeWidgetItem* it);
    void replace_prv_in_processes(QList<PrvProcessRecord>& processes, QString original_path, const PrvRecord& prv_new);
    void replace_prv_in_process(PrvProcessRecord& P, QString original_path, const PrvRecord& prv_new);
    QTreeWidgetItem* make_tree_item_from_prv(PrvRecord prv, int column_count, const QList<PrvProcessRecord>& additional_processes);
    void start_all_searches();
};

PrvGuiTreeWidget::PrvGuiTreeWidget()
{
    d = new PrvGuiTreeWidgetPrivate;
    d->q = this;

    this->setSelectionMode(QAbstractItemView::ExtendedSelection);

    //QObject::connect(&d->m_locate_manager,SIGNAL(searchStatesUpdated()),this,SLOT(slot_update_tree_data()));

    // Somehow we need to do it this way rather than listening to the searchStatesUpdated signal.
    // Otherwise, it crashes in a way that is very difficult for me to debug (failed after >2 hours)
    // Witold: why????
    QTimer* timer = new QTimer;
    timer->setInterval(1000);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(slot_update_tree_data()));
    timer->start();

    d->create_tree();

    QFont fnt = this->font();
    fnt.setPixelSize(18);
    this->setFont(fnt);
}

PrvGuiTreeWidget::~PrvGuiTreeWidget()
{
    /*
    d->m_worker_thread.requestInterruption();
    d->m_worker_thread.wait(2000);
    if (d->m_worker_thread.isRunning())
        d->m_worker_thread.terminate();
        */
    delete d;
}

void PrvGuiTreeWidget::setPrvs(const QList<PrvRecord>& prvs)
{
    d->m_prvs = prvs;
    this->setDirty(true);
}

QList<PrvRecord> PrvGuiTreeWidget::prvs() const
{
    return d->m_prvs;
}

void PrvGuiTreeWidget::setServerNames(QStringList names)
{
    d->m_server_names = names;
}

void PrvGuiTreeWidgetPrivate::replace_prv_in_process(PrvProcessRecord& P, QString original_path, const PrvRecord& prv_new)
{
    {
        QStringList keys = P.inputs.keys();
        foreach (QString key, keys) {
            if (P.inputs[key].original_path == original_path) {
                if (P.inputs[key].checksum != prv_new.checksum) {
                    P.inputs[key] = prv_new;
                    q->setDirty(true);
                }
            }
            else {
                replace_prv_in_processes(P.inputs[key].processes, original_path, prv_new);
            }
        }
    }
    {
        QStringList keys = P.outputs.keys();
        foreach (QString key, keys) {
            if (P.outputs[key].original_path == original_path) {
                if (P.outputs[key].checksum != prv_new.checksum) {
                    P.outputs[key] = prv_new;
                    q->setDirty(true);
                }
            }
            else {
                replace_prv_in_processes(P.outputs[key].processes, original_path, prv_new);
            }
        }
    }
}

void PrvGuiTreeWidgetPrivate::replace_prv_in_processes(QList<PrvProcessRecord>& processes, QString original_path, const PrvRecord& prv_new)
{
    for (int i = 0; i < processes.count(); i++) {
        replace_prv_in_process(processes[i], original_path, prv_new);
    }
}

void PrvGuiTreeWidget::replacePrv(QString original_path, const PrvRecord& prv_new)
{
    for (int i = 0; i < d->m_prvs.count(); i++) {
        if (d->m_prvs[i].original_path == original_path) {
            if (d->m_prvs[i].checksum != prv_new.checksum) {
                d->m_prvs[i] = prv_new;
                setDirty(true);
            }
        }
        else {
            d->replace_prv_in_processes(d->m_prvs[i].processes, original_path, prv_new);
        }
    }
}

void PrvGuiTreeWidget::refresh()
{
    d->create_tree();
}

void PrvGuiTreeWidget::startAllSearches()
{
    d->start_all_searches();
}

bool PrvGuiTreeWidget::isDirty() const
{
    return d->m_dirty;
}

void PrvGuiTreeWidget::setDirty(bool val)
{
    if (d->m_dirty == val)
        return;
    d->m_dirty = val;
    emit dirtyChanged();
}

QList<PrvRecord> PrvGuiTreeWidget::selectedPrvs()
{
    QList<PrvRecord> ret;
    QList<QTreeWidgetItem*> items = this->selectedItems();
    foreach (QTreeWidgetItem* it, items) {
        PrvRecord prv = PrvRecord::fromVariantMap(it->data(0, Qt::UserRole).toMap());
        ret << prv;
    }
    return ret;
}

QStringList PrvGuiTreeWidget::serverNames() const
{
    return d->m_server_names;
}

QVariantMap PrvGuiTreeWidget::currentItemDetails() const
{
    QVariantMap ret;
    QTreeWidgetItem* it = this->currentItem();
    if (!it)
        return ret;
    PrvRecord prv = PrvRecord::fromVariantMap(it->data(0, Qt::UserRole).toMap());
    ret["prv"] = prv.toVariantMap();
    ret["local_path"] = it->data(0, Qt::UserRole + 1);
    ret["server_urls"] = it->data(0, Qt::UserRole + 2);
    return ret;
}

void PrvGuiTreeWidget::searchAgain(QString checksum, long size, QString server)
{
    d->m_locate_manager.startSearchForPrv(checksum, size, server);
    slot_update_tree_data();
}

void PrvGuiTreeWidget::slot_update_tree_data()
{
    //d->m_worker_thread.results_mutex.lock();
    //QMap<QString, PrvGuiWorkerThreadResult> results = d->m_worker_thread.results;
    //d->m_worker_thread.results_mutex.unlock();

    QTreeWidget* TT = this;
    for (int i = 0; i < TT->topLevelItemCount(); i++) {
        QTreeWidgetItem* it = TT->topLevelItem(i);
        d->update_tree_item_data(it);
    }
}

QString format_file_size(long file_size)
{
    if (file_size < 1e3) {
        return QString("%1 bytes").arg(file_size);
    }
    else if (file_size < 1e6) {
        return QString("%1K").arg(((long)(file_size * 1.0 / 1e3 * 10)) / 10.0);
    }
    else if (file_size < 1e9) {
        return QString("%1M").arg(((long)(file_size * 1.0 / 1e6 * 10)) / 10.0);
    }
    else {
        return QString("%1G").arg(((long)(file_size * 1.0 / 1e9 * 10)) / 10.0);
    }
}

QString shorten(QString str, int num1, int num2)
{
    if (str.count() <= num1 + num2 + 3 + 3)
        return str;
    return str.mid(0, num1) + " ... " + str.mid(str.count() - num2);
}

bool outputs_include_checksum(QMap<QString, PrvRecord> outputs, QString checksum)
{
    QStringList keys = outputs.keys();
    foreach (QString key, keys) {
        if (outputs[key].checksum == checksum)
            return true;
    }
    return false;
}

QTreeWidgetItem* PrvGuiTreeWidgetPrivate::make_tree_item_from_prv(PrvRecord prv, int column_count, const QList<PrvProcessRecord>& additional_processes)
{
    prv.processes.append(additional_processes);

    QTreeWidgetItem* it = new QTreeWidgetItem();
    it->setText(0, shorten(prv.label, 6, 30));
    it->setData(0, Qt::UserRole, prv.toVariantMap());
    it->setText(2, format_file_size(prv.size));
    it->setText(column_count - 1, prv.checksum);
    for (int col = 2; col < column_count - 1; col++) {
        it->setTextAlignment(col, Qt::AlignCenter);
    }

    QList<PrvRecord> dependencies;
    QStringList dep_processor_names;
    QStringList dep_processor_versions;

    for (int i = 0; i < prv.processes.count(); i++) {
        QMap<QString, PrvRecord> inputs = prv.processes[i].inputs;
        QMap<QString, PrvRecord> outputs = prv.processes[i].outputs;
        if (outputs_include_checksum(outputs, prv.checksum)) {
            foreach (PrvRecord inp, inputs) {
                dependencies << inp;
                dep_processor_names << prv.processes[i].processor_name;
                dep_processor_versions << prv.processes[i].processor_version;
            }
        }
    }
    for (int i = 0; i < dependencies.count(); i++) {
        QTreeWidgetItem* it2 = make_tree_item_from_prv(dependencies[i], column_count, prv.processes);
        it2->setText(1, dep_processor_names[i] + ' ' + dep_processor_versions[i]);
        it->addChild(it2);
    }

    return it;
}

void PrvGuiTreeWidgetPrivate::start_all_searches()
{
    QList<PrvRecord> all_prvs;
    for (int i = 0; i < m_prvs.count(); i++) {
        PrvRecord prv = m_prvs[i];
        all_prvs << prv;
        for (int i=0; i<prv.processes.count(); i++) {
            QStringList ikeys=prv.processes[i].inputs.keys();
            foreach (QString ikey,ikeys) {
                all_prvs << prv.processes[i].inputs[ikey];
            }
        }
    }
    for (int i = 0; i < all_prvs.count(); i++) {
        PrvRecord prv = all_prvs[i];
        m_locate_manager.startSearchForPrv(prv.checksum, prv.size, "");
        foreach (QString server, m_server_names) {
            m_locate_manager.startSearchForPrv(prv.checksum, prv.size, server);
        }
    }
}

void PrvGuiTreeWidgetPrivate::create_tree()
{
    QTreeWidget* TT = q;
    TT->clear();

    QStringList headers;
    headers << "Name"
            << "Processor"
            << "Size"
            << "Local disk";
    headers.append(m_server_names);
    headers << "Sha-1";
    TT->setHeaderLabels(headers);

    for (int i = 0; i < m_prvs.count(); i++) {
        PrvRecord prv = m_prvs[i];
        QTreeWidgetItem* it = make_tree_item_from_prv(prv, TT->columnCount(), QList<PrvProcessRecord>());
        TT->addTopLevelItem(it);
    }

    for (int i = 0; i < TT->columnCount(); i++) {
        TT->resizeColumnToContents(i);
    }
    int minwid = 200;
    TT->setColumnWidth(0, qMax(TT->columnWidth(0), minwid));
    TT->setColumnWidth(1, qMax(TT->columnWidth(1), minwid));

    minwid = 100;
    TT->setColumnWidth(2, qMax(TT->columnWidth(2), minwid));

    //q->slot_update_tree_data();

    //restart_worker_thread();
}

/*
void PrvGuiTreeWidgetPrivate::restart_worker_thread()
{
    if (m_worker_thread.isRunning()) {
        m_worker_thread.requestInterruption();
        m_worker_thread.wait(5000);
        if (m_worker_thread.isRunning())
            m_worker_thread.terminate();
    }
    m_worker_thread.prvs = m_prvs;
    m_worker_thread.server_names = m_server_names;
    m_worker_thread.start();
}
*/

void PrvGuiTreeWidgetPrivate::update_tree_item_data(QTreeWidgetItem* it)
{
    PrvRecord prv = PrvRecord::fromVariantMap(it->data(0, Qt::UserRole).toMap());

    //PrvGuiWorkerThreadResult result0 = results.value(to_prv_code(prv));

    QString local_path;
    QVariantMap server_urls;
    {
        fuzzybool state = m_locate_manager.getSearchState(prv, "");
        QString result_path = m_locate_manager.getResultPathOrUrl(prv, "");
        local_path = result_path;
        int col = 3;
        it->setText(col, to_string(state));
        it->setForeground(col, to_color(state));
        it->setForeground(0, to_color(state));
        it->setToolTip(col, result_path);
    }
    for (int a = 0; a < m_server_names.count(); a++) {
        fuzzybool state = m_locate_manager.getSearchState(prv, m_server_names[a]);
        QString result_path = m_locate_manager.getResultPathOrUrl(prv, m_server_names[a]);
        server_urls[m_server_names[a]] = result_path;
        int col = 4 + a;
        it->setText(col, to_string(state));
        it->setForeground(col, to_color(state));
        it->setToolTip(col, result_path);
    }
    it->setData(0, Qt::UserRole + 1, local_path);
    it->setData(0, Qt::UserRole + 2, server_urls);

    for (int a = 0; a < it->childCount(); a++) {
        update_tree_item_data(it->child(a));
    }
}
