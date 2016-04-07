/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/6/2016
*******************************************************/

#include "mbmainwindow.h"

#include <QHBoxLayout>
#include <QTreeWidget>
#include <QDebug>
#include <QJsonDocument>
#include <QProcess>
#include "mbexperimentlistwidget2.h"
#include <QCoreApplication>

class MBMainWindowPrivate {
public:
	MBMainWindow *q;
    MBExperimentManager *m_experiment_manager;

    MBExperimentListWidget2 *m_experiment_list;
};

MBMainWindow::MBMainWindow()
{
	d=new MBMainWindowPrivate;
	d->q=this;
    d->m_experiment_manager=0;

    d->m_experiment_list=new MBExperimentListWidget2;

    QHBoxLayout *hlayout=new QHBoxLayout;
    hlayout->addWidget(d->m_experiment_list);
    this->setLayout(hlayout);

    connect(d->m_experiment_list,SIGNAL(experimentActivated(QString)),this,SLOT(slot_experiment_activated(QString)));
}

MBMainWindow::~MBMainWindow()
{
    delete d;
}

void MBMainWindow::setExperimentManager(MBExperimentManager *M)
{
    d->m_experiment_manager=M;
    d->m_experiment_list->setExperimentManager(M);
}

void MBMainWindow::slot_experiment_activated(QString id)
{
    MBExperiment E=d->m_experiment_manager->experiment(id);
    QString exp_type=E.json["exp_type"].toString();
    QString basepath=E.json["basepath"].toString();
    if (!basepath.isEmpty()) basepath+="/";
    if (exp_type=="sorting_result") {
        QString pre=basepath+E.json["pre"].toString();
        QString firings=basepath+E.json["firings"].toString();
        QStringList args;
        args << "--mode=overview2" << "--pre="+pre << "--firings="+firings;
        QString mv_exe=qApp->applicationDirPath()+"/../../mountainview/bin/mountainview";
        QProcess::startDetached(mv_exe,args);
    }
}

