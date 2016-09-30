/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 9/29/2016
*******************************************************/

#include "prvguimainwindow.h"

#include <QFileInfo>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QMutex>
#include <QProcess>
#include <QTreeWidget>
#include <taskprogress.h>

class PrvGuiMainWindowPrivate {
public:
    PrvGuiMainWindow* q;
    QList<PrvRecord> m_prvs;
    QStringList m_server_names;

    QTreeWidget* m_tree;
    PrvGuiWorkerThread m_worker_thread;

    void refresh_tree();
    void restart_worker_thread();
    void update_tree_item_data(QTreeWidgetItem* it, QMap<QString, PrvGuiWorkerThreadResult> results);
};

PrvGuiMainWindow::PrvGuiMainWindow()
{
    d = new PrvGuiMainWindowPrivate;
    d->q = this;

    d->m_tree = new QTreeWidget;

    QVBoxLayout* vlayout = new QVBoxLayout;
    vlayout->addWidget(d->m_tree);
    this->setLayout(vlayout);

    QObject::connect(&d->m_worker_thread, SIGNAL(results_updated()), this, SLOT(slot_update_tree_data()));

    d->refresh_tree();

    QFont fnt = this->font();
    fnt.setPixelSize(18);
    this->setFont(fnt);
}

PrvGuiMainWindow::~PrvGuiMainWindow()
{
    d->m_worker_thread.requestInterruption();
    d->m_worker_thread.wait(2000);
    delete d;
}

void PrvGuiMainWindow::setPrvs(const QList<PrvRecord>& prvs)
{
    d->m_prvs = prvs;
    d->refresh_tree();
}

void PrvGuiMainWindow::setServerNames(QStringList names)
{
    d->m_server_names = names;
    d->refresh_tree();
}

QString to_string(fuzzybool fb)
{
    if (fb == YES)
        return "YES";
    if (fb == NO)
        return "x";
    return ".";
}

QColor to_color(fuzzybool fb)
{
    if (fb == YES)
        return Qt::darkGreen;
    if (fb == NO)
        return Qt::darkRed;
    return ".";
}

QString to_prv_code(PrvRecord prv)
{
    return prv.checksum + ":" + QString("%1").arg(prv.size);
}

void PrvGuiMainWindow::slot_update_tree_data()
{
    d->m_worker_thread.results_mutex.lock();
    QMap<QString, PrvGuiWorkerThreadResult> results = d->m_worker_thread.results;
    d->m_worker_thread.results_mutex.unlock();

    QTreeWidget* TT = d->m_tree;
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

bool outputs_include_checksum(QList<PrvRecord> outputs, QString checksum)
{
    foreach (PrvRecord out, outputs) {
        if (out.checksum == checksum)
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
        QList<PrvRecord> inputs = prv.processes[i].inputs;
        QList<PrvRecord> outputs = prv.processes[i].outputs;
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

void PrvGuiMainWindowPrivate::refresh_tree()
{
    QTreeWidget* TT = m_tree;
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

void PrvGuiMainWindowPrivate::restart_worker_thread()
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

void PrvGuiMainWindowPrivate::update_tree_item_data(QTreeWidgetItem* it, QMap<QString, PrvGuiWorkerThreadResult> results)
{
    PrvRecord prv = PrvRecord::fromVariantMap(it->data(0, Qt::UserRole).toMap());

    PrvGuiWorkerThreadResult result0 = results.value(to_prv_code(prv));
    {
        int col = 3;
        it->setText(col, to_string(result0.on_local_disk));
        it->setForeground(col, to_color(result0.on_local_disk));
        it->setForeground(0, to_color(result0.on_local_disk));
    }
    for (int a = 0; a < m_server_names.count(); a++) {
        int col = 4 + a;
        fuzzybool tmp = result0.on_server.value(m_server_names[a], UNKNOWN);
        it->setText(col, to_string(tmp));
        it->setForeground(col, to_color(tmp));
    }
    for (int a = 0; a < it->childCount(); a++) {
        update_tree_item_data(it->child(a), results);
    }
}

void PrvGuiWorkerThread::run()
{
    TaskProgress task("PRV Upload info...");
    results.clear();
    for (int i = 0; i < prvs.count(); i++) {
        task.setProgress((i + 0.5) / prvs.count());
        if (QThread::currentThread()->isInterruptionRequested())
            return;
        PrvRecord prv = prvs[i];
        QString prv_code = to_prv_code(prv);
        QString name = QFileInfo(prv.original_path).fileName();
        long size = prv.size;
        {
            task.log() << "check if on local disk" << name << size;
            bool is = check_if_on_local_disk(prv);
            {
                QMutexLocker locker(&results_mutex);
                if (is)
                    results[prv_code].on_local_disk = YES;
                else
                    results[prv_code].on_local_disk = NO;
            }
            emit results_updated();
        }
        foreach (QString server_name, server_names) {
            task.log() << "check if on server" << name << server_name;
            if (QThread::currentThread()->isInterruptionRequested())
                return;
            bool is = check_if_on_server(prv, server_name);
            {
                QMutexLocker locker(&results_mutex);
                if (is)
                    results[prv_code].on_server[server_name] = YES;
                else
                    results[prv_code].on_server[server_name] = NO;
            }
            emit results_updated();
        }
    }
}

QString exec_process_and_return_output(QString cmd, QStringList args)
{
    QProcess P;
    P.setReadChannelMode(QProcess::MergedChannels);
    P.start(cmd, args);
    P.waitForStarted();
    P.waitForFinished(-1);
    return P.readAll();
}

bool PrvGuiWorkerThread::check_if_on_local_disk(PrvRecord prv)
{
    QString cmd = "prv";
    QStringList args;
    args << "locate";
    args << "--checksum=" + prv.checksum;
    args << "--checksum1000=" + prv.checksum1000;
    args << QString("--size=%1").arg(prv.size);
    args << "--local-only";
    QString output = exec_process_and_return_output(cmd, args);
    return !output.isEmpty();
}

bool PrvGuiWorkerThread::check_if_on_server(PrvRecord prv, QString server_name)
{
    QString cmd = "prv";
    QStringList args;
    args << "locate";
    args << "--checksum=" + prv.checksum;
    args << "--checksum1000=" + prv.checksum1000;
    args << QString("--size=%1").arg(prv.size);
    args << "--server=" + server_name;
    QString output = exec_process_and_return_output(cmd, args);
    return !output.isEmpty();
}

QList<PrvRecord> find_prvs(QString label, const QJsonValue& X)
{
    QList<PrvRecord> ret;
    if (X.isObject()) {
        QJsonObject obj = X.toObject();
        if ((obj.contains("original_checksum")) && (obj.contains("original_size"))) {
            ret << PrvRecord(label, obj);
            return ret;
        }
        else {
            QStringList keys = obj.keys();
            foreach (QString key, keys) {
                QString label0 = key;
                if (label0 == "data")
                    label0 = label;
                ret.append(find_prvs(label0, obj[key]));
            }
            return ret;
        }
    }
    else if (X.isArray()) {
        QJsonArray array = X.toArray();
        for (int i = 0; i < array.count(); i++) {
            ret.append(find_prvs(label + QString("[%1]").arg(i), array[i]));
        }
        return ret;
    }
    else {
        return ret;
    }
}

PrvProcessRecord::PrvProcessRecord(QJsonObject obj)
{
    QJsonObject inputs = obj["inputs"].toObject();
    QJsonObject outputs = obj["outputs"].toObject();

    this->processor_name = obj["processor_name"].toString();
    this->processor_version = obj["processor_version"].toString();
    this->parameters = obj["parameters"].toObject().toVariantMap();

    QStringList ikeys = inputs.keys();
    foreach (QString ikey, ikeys) {
        PrvRecord rec(ikey, inputs[ikey].toObject());
        this->inputs << rec;
    }

    QStringList okeys = outputs.keys();
    foreach (QString okey, okeys) {
        PrvRecord rec(okey, outputs[okey].toObject());
        this->outputs << rec;
    }
}

QVariantMap PrvProcessRecord::toVariantMap() const
{
    QVariantMap ret;

    QVariantList inputs0;
    foreach (PrvRecord inp, this->inputs) {
        inputs0 << inp.toVariantMap();
    }

    QVariantList outputs0;
    foreach (PrvRecord out, this->outputs) {
        outputs0 << out.toVariantMap();
    }

    ret["processor_name"] = processor_name;
    ret["processor_version"] = processor_version;
    ret["inputs"] = inputs0;
    ret["outputs"] = outputs0;
    ret["parameters"] = this->parameters;

    return ret;
}

PrvProcessRecord PrvProcessRecord::fromVariantMap(QVariantMap X)
{
    PrvProcessRecord ret;

    ret.processor_name = X["processor_name"].toString();
    ret.processor_version = X["processor_version"].toString();
    {
        QVariantList list = X["inputs"].toList();
        foreach (QVariant item, list) {
            ret.inputs << PrvRecord::fromVariantMap(item.toMap());
        }
    }
    {
        QVariantList list = X["outputs"].toList();
        foreach (QVariant item, list) {
            ret.outputs << PrvRecord::fromVariantMap(item.toMap());
        }
    }
    ret.parameters = X["parameters"].toMap();

    return ret;
}

PrvRecord::PrvRecord(QString label_in, QJsonObject obj)
{
    this->label = label_in;
    this->original_path = obj["original_path"].toString();
    this->checksum = obj["original_checksum"].toString();
    this->checksum1000 = obj["original_checksum_1000"].toString();
    this->size = obj["original_size"].toVariant().toLongLong();

    QJsonArray X = obj["processes"].toArray();
    foreach (QJsonValue val, X) {
        QJsonObject P = val.toObject();
        this->processes << PrvProcessRecord(P);
        qDebug() << "::::::::::::::::::::::::::::::::::" << label_in << this->processes.count() << this->processes.value(0).outputs.count();
        qDebug() << P;
    }
}

QVariantMap PrvRecord::toVariantMap() const
{
    QVariantMap ret;

    ret["label"] = this->label;

    ret["checksum"] = this->checksum;
    ret["checksum1000"] = this->checksum1000;
    ret["size"] = (long long)this->size;
    ret["original_path"] = this->original_path;

    QVariantList processes0;
    foreach (PrvProcessRecord P, this->processes) {
        processes0 << P.toVariantMap();
    }
    ret["processes"] = processes0;

    return ret;
}

PrvRecord PrvRecord::fromVariantMap(QVariantMap X)
{
    PrvRecord ret;

    ret.label = X["label"].toString();

    ret.checksum = X["checksum"].toString();
    ret.checksum1000 = X["checksum1000"].toString();
    ret.size = X["size"].toLongLong();
    ret.original_path = X["original_path"].toString();

    QList<PrvProcessRecord> processes0;
    QVariantList list = X["processes"].toList();
    foreach (QVariant P, list) {
        processes0 << PrvProcessRecord::fromVariantMap(P.toMap());
    }

    ret.processes = processes0;

    return ret;
}
