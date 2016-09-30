/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 9/29/2016
*******************************************************/
#ifndef PRVGUIMAINWINDOW_H
#define PRVGUIMAINWINDOW_H

#include <QJsonObject>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QThread>
#include <QWidget>
#include <QVariant>

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

class PrvGuiMainWindowPrivate;
class PrvGuiMainWindow : public QWidget {
    Q_OBJECT
public:
    friend class PrvGuiMainWindowPrivate;
    PrvGuiMainWindow();
    virtual ~PrvGuiMainWindow();
    void setPrvs(const QList<PrvRecord>& prvs);
    void setServerNames(QStringList names);
private slots:
    void slot_update_tree_data();

private:
    PrvGuiMainWindowPrivate* d;
};

enum fuzzybool {
    YES,
    NO,
    UNKNOWN
};

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

#endif // PRVGUIMAINWINDOW_H
