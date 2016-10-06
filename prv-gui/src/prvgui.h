/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 10/1/2016
*******************************************************/
#ifndef PRVGUI_H
#define PRVGUI_H

#include <QJsonObject>
#include <QMutex>
#include <QString>
#include <QThread>
#include <QVariantMap>

QJsonObject get_server_object_for_name(QString server_name);
QString get_server_url_for_name(QString server_name);

struct PrvProcessRecord;
struct PrvRecord {
    PrvRecord() {}
    PrvRecord(QString label_in, QJsonObject obj);
    PrvRecord(const PrvRecord& other)
    {
        copy_from(other);
    }

    QJsonObject original_object;

    QString label; //not really part of prv, but very useful for display

    QString checksum;
    long size = 0;
    QString checksum1000;
    QString original_path;

    QList<PrvProcessRecord> processes;

    QVariantMap toVariantMap() const;
    static PrvRecord fromVariantMap(QVariantMap X);
    QString find_local_file() const;
    QString find_remote_url(QString server_name) const;

private:
    void copy_from(const PrvRecord& other)
    {
        original_object = other.original_object;
        label = other.label;
        checksum = other.checksum;
        size = other.size;
        checksum1000 = other.checksum1000;
        original_path = other.original_path;
        processes = other.processes;
    }
};

struct PrvProcessRecord {
    PrvProcessRecord() {}
    PrvProcessRecord(QJsonObject obj);
    PrvProcessRecord(const PrvProcessRecord& other)
    {
        copy_from(other);
    }

    QString processor_name;
    QString processor_version;
    QMap<QString, PrvRecord> inputs;
    QMap<QString, PrvRecord> outputs;
    QVariantMap parameters;

    QVariantMap toVariantMap() const;
    static PrvProcessRecord fromVariantMap(QVariantMap X);

private:
    void copy_from(const PrvProcessRecord& other)
    {
        processor_name = other.processor_name;
        processor_version = other.processor_version;
        inputs = other.inputs;
        outputs = other.outputs;
        parameters = other.parameters;
    }
};

enum fuzzybool {
    YES,
    NO,
    REGENERATABLE,
    UNKNOWN
};

QString to_string(fuzzybool fb);
QColor to_color(fuzzybool fb);
QString to_prv_code(PrvRecord prv);

struct PrvGuiWorkerThreadResult {
    fuzzybool on_local_disk = UNKNOWN;
    QMap<QString, fuzzybool> on_server;
    QString local_path;
    QMap<QString, QVariant> server_urls;
};

class PrvGuiWorkerThread : public QThread {
    Q_OBJECT
public:
    //input
    QList<PrvRecord> prvs;
    QStringList server_names;

    //output
    QMutex results_mutex;
    QMap<QString, PrvGuiWorkerThreadResult> results; //by prv code

    void run();

private:
    QString check_if_on_server(PrvRecord prv, QString server_name);

signals:
    void results_updated();
};

QList<PrvRecord> find_prvs(QString label, const QJsonValue& X);
QString exec_process_and_return_output(QString cmd, QStringList args);
int find_process_corresponding_to_output(QList<PrvProcessRecord> processes, QString original_path, QString& output_pname);

#endif // PRVGUI_H
