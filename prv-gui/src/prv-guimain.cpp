/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 9/29/2016
*******************************************************/

#include <QApplication>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "mlcommon.h"
#include "prvguimainwindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("prv-gui");
    QApplication::setApplicationVersion("0.1");

    QCommandLineParser parser;
    parser.setApplicationDescription("prv-gui");

    parser.process(app);
    QStringList args = parser.positionalArguments();
    QString fname = args.value(0);
    if (fname.isEmpty()) {
        printf("No file argument specified.\n");
        return -1;
    }

    QJsonParseError err;
    QJsonObject obj = QJsonDocument::fromJson(TextFile::read(fname).toUtf8(), &err).object();
    if (err.error != QJsonParseError::NoError) {
        printf("Error parsing JSON text.\n");
        return -1;
    }

    QJsonArray servers0 = MLUtil::configValue("prv", "servers").toArray();
    QStringList server_names;
    foreach (QJsonValue server, servers0) {
        server_names << server.toObject()["name"].toString();
    }

    QList<PrvRecord> prvs = find_prvs(QFileInfo(fname).fileName(), obj);
    PrvGuiMainWindow W;
    W.setPrvs(prvs);
    W.setServerNames(server_names);
    W.showMaximized();
    W.refresh();

    return app.exec();
}
