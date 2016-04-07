/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/7/2016
*******************************************************/

#include "textfile.h"
#include "mbcontroller.h"
#include <QCoreApplication>
#include <QProcess>

#include <QJsonDocument>
#include <QDebug>

class MBControllerPrivate {
public:
    MBController *q;
};

MBController::MBController()
{
    d=new MBControllerPrivate;
    d->q=this;
}

MBController::~MBController()
{
    delete d;
}

QString MBController::loadLocalStudy(QString file_path)
{
    //Witold, it would be great if we could return a javascript object directly here
    return read_text_file(file_path);
}

void MBController::openSortingResult(QString json)
{
    MBExperiment E;
    E.json=QJsonDocument::fromJson(json.toUtf8()).object();
    E.exp_id=E.json["exp_id"].toString();
    QString exp_type=E.json["exp_type"].toString();
    QString basepath=E.json["basepath"].toString();
    if (!basepath.isEmpty()) basepath+="/";
    if (exp_type=="sorting_result") {
        QString pre=basepath+E.json["pre"].toString();
        QString filt=basepath+E.json["filt"].toString();
        QString raw=basepath+E.json["raw"].toString();
        QString firings=basepath+E.json["firings"].toString();
        QStringList args;
        args << "--mode=overview2" << "--pre="+pre << "--filt="+filt << "--raw="+raw  << "--firings="+firings;
        qDebug() << args;
        QString mv_exe=qApp->applicationDirPath()+"/../../mountainview/bin/mountainview";
        QProcess::startDetached(mv_exe,args);
    }
}
