/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 10/1/2016
*******************************************************/

#include "prvgui.h"

#include <QColor>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QProcess>
#include <taskprogress.h>
#include "mlcommon.h"

QString to_string(fuzzybool fb)
{
    if (fb == YES)
        return "YES";
    if (fb == NO)
        return "x";
    if (fb == REGENERATABLE)
        return "R";
    return ".";
}

QColor to_color(fuzzybool fb)
{
    if (fb == YES)
        return Qt::darkGreen;
    if (fb == NO)
        return Qt::darkRed;
    if (fb == REGENERATABLE)
        return Qt::darkBlue;
    return Qt::black;
}

QString to_prv_code(PrvRecord prv)
{
    return prv.checksum + ":" + QString("%1").arg(prv.size);
}

bool check_if_regeneratable(const PrvRecord& prv)
{
    QString output_pname;
    int index = find_process_corresponding_to_output(prv.processes, prv.original_path, output_pname);
    if (index < 0)
        return false;
    PrvProcessRecord P = prv.processes[index];
    QStringList ikeys = P.inputs.keys();
    foreach (QString ikey, ikeys) {
        PrvRecord prv0 = P.inputs[ikey];
        prv0.processes.append(prv.processes);
        if (prv0.find_local_file().isEmpty()) {
            if (!check_if_regeneratable(prv0)) {
                return false;
            }
        }
    }
    return true;
}

void PrvGuiWorkerThread::run()
{
    TaskProgress task("Searching");
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
            QString local_path = prv.find_local_file();
            results[prv_code].local_path = local_path;
            {
                QMutexLocker locker(&results_mutex);
                if (!local_path.isEmpty())
                    results[prv_code].on_local_disk = YES;
                else {
                    if (check_if_regeneratable(prv))
                        results[prv_code].on_local_disk = REGENERATABLE;
                    else
                        results[prv_code].on_local_disk = NO;
                }
            }
            emit results_updated();
        }
        foreach (QString server_name, server_names) {
            task.log() << "check if on server" << name << server_name;
            if (QThread::currentThread()->isInterruptionRequested())
                return;
            QString server_url = check_if_on_server(prv, server_name);
            results[prv_code].server_urls[server_name] = server_url;
            {
                QMutexLocker locker(&results_mutex);
                if (!server_url.isEmpty())
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
    return P.readAll().trimmed();
}

QString PrvGuiWorkerThread::check_if_on_server(PrvRecord prv, QString server_name)
{
    QString cmd = "prv";
    QStringList args;
    args << "locate";
    args << "--checksum=" + prv.checksum;
    args << "--checksum1000=" + prv.checksum1000;
    args << QString("--size=%1").arg(prv.size);
    args << "--server=" + server_name;
    QString output = exec_process_and_return_output(cmd, args);
    return output;
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
        this->inputs[ikey] = rec;
    }

    QStringList okeys = outputs.keys();
    foreach (QString okey, okeys) {
        PrvRecord rec(okey, outputs[okey].toObject());
        this->outputs[okey] = rec;
    }
}

QVariantMap PrvProcessRecord::toVariantMap() const
{
    QVariantMap ret;

    QVariantMap inputs0;
    QStringList ikeys = this->inputs.keys();
    foreach (QString ikey, ikeys) {
        inputs0[ikey] = this->inputs[ikey].toVariantMap();
    }

    QVariantMap outputs0;
    QStringList okeys = this->outputs.keys();
    foreach (QString okey, okeys) {
        outputs0[okey] = this->outputs[okey].toVariantMap();
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
        QVariantMap map = X["inputs"].toMap();
        QStringList keys = map.keys();
        foreach (QString key, keys) {
            ret.inputs[key] = PrvRecord::fromVariantMap(map[key].toMap());
        }
    }
    {
        QVariantMap map = X["outputs"].toMap();
        QStringList keys = map.keys();
        foreach (QString key, keys) {
            ret.outputs[key] = PrvRecord::fromVariantMap(map[key].toMap());
        }
    }
    ret.parameters = X["parameters"].toMap();

    return ret;
}

PrvRecord::PrvRecord(QString label_in, QJsonObject obj)
{
    this->original_object = obj;
    this->label = label_in;
    this->original_path = obj["original_path"].toString();
    this->checksum = obj["original_checksum"].toString();
    this->checksum1000 = obj["original_checksum_1000"].toString();
    this->size = obj["original_size"].toVariant().toLongLong();

    QJsonArray X = obj["processes"].toArray();
    foreach (QJsonValue val, X) {
        QJsonObject P = val.toObject();
        this->processes << PrvProcessRecord(P);
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
    QJsonObject obj;
    obj["original_checksum"] = X["checksum"].toString();
    obj["original_checksum_1000"] = X["checksum1000"].toString();
    obj["original_size"] = X["size"].toLongLong();
    obj["original_path"] = X["original_path"].toString();
    PrvRecord ret(X["label"].toString(), obj);

    QList<PrvProcessRecord> processes0;
    QVariantList list = X["processes"].toList();
    foreach (QVariant P, list) {
        processes0 << PrvProcessRecord::fromVariantMap(P.toMap());
    }

    ret.processes = processes0;

    return ret;
}

QString PrvRecord::find_local_file() const
{
    QString cmd = "prv";
    QStringList args;
    args << "locate";
    args << "--checksum=" + this->checksum;
    args << "--checksum1000=" + this->checksum1000;
    args << QString("--size=%1").arg(this->size);
    args << "--local-only";
    QString output = exec_process_and_return_output(cmd, args);
    return output;
}

QString PrvRecord::find_remote_url(QString server_name) const
{
    QJsonObject obj=get_server_object_for_name(server_name);
    QString cmd = "prv";
    QStringList args;
    args << "locate";
    args << "--checksum=" + this->checksum;
    args << "--checksum1000=" + this->checksum1000;
    args << QString("--size=%1").arg(this->size);
    args << "--server=" + server_name;
    QString output = exec_process_and_return_output(cmd, args);
    return output;
}

QString get_server_url_for_name(QString server_name)
{
    QJsonObject server0 = get_server_object_for_name(server_name);
    QString host = server0["host"].toString();
    if (host.isEmpty())
        return "";
    int port = server0["port"].toInt();
    QString url_path = server0["path"].toString();
    QString url0 = host + ":" + QString::number(port) + url_path;
    return url0;
}

QJsonObject get_server_object_for_name(QString server_name)
{
    QJsonArray remote_servers = MLUtil::configValue("prv", "servers").toArray();
    for (int i = 0; i < remote_servers.count(); i++) {
        QJsonObject server0 = remote_servers[i].toObject();
        if (server0["name"].toString() == server_name) {
            return server0;
        }
    }
    return QJsonObject();
}

int find_process_corresponding_to_output(QList<PrvProcessRecord> processes, QString original_path, QString& output_pname)
{
    for (int i = 0; i < processes.count(); i++) {
        PrvProcessRecord P = processes[i];
        QStringList okeys = P.outputs.keys();
        foreach (QString okey, okeys) {
            if (P.outputs[okey].original_path == original_path) {
                output_pname = okey;
                return i;
            }
        }
    }
    return -1;
}
