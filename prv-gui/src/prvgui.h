/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 10/1/2016
*******************************************************/
#ifndef PRVGUI_H
#define PRVGUI_H

struct PrvProcessRecord;
struct PrvRecord {
    PrvRecord() {}
    PrvRecord(QString label_in, QJsonObject obj);

    QString label; //not really part of prv, but very useful for display

    QString checksum;
    long size = 0;
    QString checksum1000;
    QString original_path;

    QList<PrvProcessRecord> processes;

    QVariantMap toVariantMap() const;
    static PrvRecord fromVariantMap(QVariantMap X);
};

struct PrvProcessRecord {
    PrvProcessRecord() {}
    PrvProcessRecord(QJsonObject obj);

    QString processor_name;
    QString processor_version;
    QList<PrvRecord> inputs;
    QList<PrvRecord> outputs;
    QVariantMap parameters;

    QVariantMap toVariantMap() const;
    static PrvProcessRecord fromVariantMap(QVariantMap X);
};

enum fuzzybool {
    YES,
    NO,
    UNKNOWN
};

QString to_string(fuzzybool fb);
QColor to_color(fuzzybool fb);
QString to_prv_code(PrvRecord prv);

struct PrvGuiWorkerThreadResult {
    fuzzybool on_local_disk = UNKNOWN;
    QMap<QString, fuzzybool> on_server;
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
    bool check_if_on_local_disk(PrvRecord prv);
    bool check_if_on_server(PrvRecord prv, QString server_name);

signals:
    void results_updated();
};

QList<PrvRecord> find_prvs(QString label, const QJsonValue& X);

#endif // PRVGUI_H
