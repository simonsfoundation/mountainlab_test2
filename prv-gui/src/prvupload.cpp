/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 10/1/2016
*******************************************************/

#include "prvupload.h"

#include <QJsonDocument>
#include <QProcess>
#include <taskprogress.h>
#include "cachemanager.h"
#include "mlcommon.h"

namespace PrvUpload {
}

void PrvUpload::initiateUploadToServer(QString server_name, PrvRecord prv)
{
    QString tmp_fname = CacheManager::globalInstance()->makeLocalFile(MLUtil::makeRandomId(10) + ".PrvManagerdlg.prv");
    qDebug() << "----------------------------------------------------------------";
    qDebug() << prv.original_object << prv.checksum << prv.original_path;
    TextFile::write(tmp_fname, QJsonDocument(prv.original_object).toJson());
    QString cmd = "prv";
    QStringList args;
    args << "ensure-remote" << tmp_fname << "--server=" + server_name;

    execute_command_in_separate_thread(cmd, args);
}

class ExecCmdThread : public QThread {
public:
    QString cmd;
    QStringList args;

    void run()
    {
        TaskProgress task("Running: " + cmd + " " + args.join(" "));
        task.log() << "Running: " + cmd + " " + args.join(" ");
        QProcess P;
        P.setReadChannelMode(QProcess::MergedChannels);
        P.start(cmd, args);
        P.waitForStarted();
        P.waitForFinished(-1);
        if (P.exitCode() != 0) {
            task.error() << "Error running: " + cmd + " " + args.join(" ");
        }
        task.log() << "Exit code: " + P.exitCode();
        task.log() << P.readAll();
    }
};

void execute_command_in_separate_thread(QString cmd, QStringList args, QObject* on_finished_receiver, const char* sig_or_slot)
{
    ExecCmdThread* thread = new ExecCmdThread;

    if (on_finished_receiver) {
        QObject::connect(thread, SIGNAL(finished()), on_finished_receiver, sig_or_slot);
    }

    /// Witold, is the following line okay?
    QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    thread->cmd = cmd;
    thread->args = args;
    thread->start();
}
