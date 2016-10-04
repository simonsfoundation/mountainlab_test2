/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 9/29/2016
*******************************************************/

#include "prvguitreewidget.h"

#include <QFileInfo>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QMutex>
#include <QProcess>
#include <QSplitter>
#include <QTreeWidget>
#include <taskprogress.h>
#include "taskprogressview.h"
#include "prvguicontrolpanel.h"

class PrvGuiTreeWidgetPrivate {
public:
    PrvGuiTreeWidget* q;
    QList<PrvRecord> m_prvs;
    QStringList m_server_names;

    PrvGuiWorkerThread m_worker_thread;

    void refresh_tree();
    void restart_worker_thread();
    void update_tree_item_data(QTreeWidgetItem* it, QMap<QString, PrvGuiWorkerThreadResult> results);
};

PrvGuiTreeWidget::PrvGuiTreeWidget()
{
    d = new PrvGuiTreeWidgetPrivate;
    d->q = this;

    this->setSelectionMode(QAbstractItemView::ExtendedSelection);

    QObject::connect(&d->m_worker_thread, SIGNAL(results_updated()), this, SLOT(slot_update_tree_data()));

    d->refresh_tree();

    QFont fnt = this->font();
    fnt.setPixelSize(18);
    this->setFont(fnt);
}

PrvGuiTreeWidget::~PrvGuiTreeWidget()
{
    d->m_worker_thread.requestInterruption();
    d->m_worker_thread.wait(2000);
    d->m_worker_thread.terminate();
    delete d;
}

void PrvGuiTreeWidget::setPrvs(const QList<PrvRecord>& prvs)
{
    d->m_prvs = prvs;
}

void PrvGuiTreeWidget::setServerNames(QStringList names)
{
    d->m_server_names = names;
}

void PrvGuiTreeWidget::refresh()
{
    d->refresh_tree();
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

void PrvGuiTreeWidget::slot_update_tree_data()
{
    d->m_worker_thread.results_mutex.lock();
    QMap<QString, PrvGuiWorkerThreadResult> results = d->m_worker_thread.results;
    d->m_worker_thread.results_mutex.unlock();

    QTreeWidget* TT = this;
    for (int i = 0; i < TT->topLevelItemCount(); i++) {
        QTreeWidgetItem* it = TT->topLevelItem(i);
        d->update_tree_item_data(it, results);
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

QTreeWidgetItem* make_tree_item_from_prv(PrvRecord prv, int column_count, const QList<PrvProcessRecord>& additional_processes)
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
        qDebug() << prv.label << prv.processes.count() << outputs.count();
        if (outputs_include_checksum(outputs, prv.checksum)) {
            foreach (PrvRecord inp, inputs) {
                dependencies << inp;
                dep_processor_names << prv.processes[i].processor_name;
                dep_processor_versions << prv.processes[i].processor_version;
            }
        }
    }
    qDebug() << "Dependencies count" << dependencies.count();
    for (int i = 0; i < dependencies.count(); i++) {
        QTreeWidgetItem* it2 = make_tree_item_from_prv(dependencies[i], column_count, prv.processes);
        it2->setText(1, dep_processor_names[i] + ' ' + dep_processor_versions[i]);
        it->addChild(it2);
    }

    return it;
}

void PrvGuiTreeWidgetPrivate::refresh_tree()
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

    restart_worker_thread();
}

void PrvGuiTreeWidgetPrivate::restart_worker_thread()
{
    if (m_worker_thread.isRunning()) {
        m_worker_thread.requestInterruption();
        m_worker_thread.wait(5000);
        m_worker_thread.terminate();
    }
    m_worker_thread.prvs = m_prvs;
    m_worker_thread.server_names = m_server_names;
    m_worker_thread.start();
}

void PrvGuiTreeWidgetPrivate::update_tree_item_data(QTreeWidgetItem* it, QMap<QString, PrvGuiWorkerThreadResult> results)
{
    PrvRecord prv = PrvRecord::fromVariantMap(it->data(0, Qt::UserRole).toMap());

    PrvGuiWorkerThreadResult result0 = results.value(to_prv_code(prv));
    {
        int col = 3;
        it->setText(col, to_string(result0.on_local_disk));
        it->setForeground(col, to_color(result0.on_local_disk));
        it->setForeground(0, to_color(result0.on_local_disk));
        it->setToolTip(col, result0.local_path);
    }
    for (int a = 0; a < m_server_names.count(); a++) {
        int col = 4 + a;
        fuzzybool tmp = result0.on_server.value(m_server_names[a], UNKNOWN);
        it->setText(col, to_string(tmp));
        it->setForeground(col, to_color(tmp));
        it->setToolTip(col, result0.server_urls.value(m_server_names[a]).toString());
    }
    it->setData(0, Qt::UserRole + 1, result0.local_path);
    it->setData(0, Qt::UserRole + 2, result0.server_urls);
    for (int a = 0; a < it->childCount(); a++) {
        update_tree_item_data(it->child(a), results);
    }
}
