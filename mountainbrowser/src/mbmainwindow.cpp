/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/6/2016
*******************************************************/

#include "mbmainwindow.h"

#include <QHBoxLayout>
#include <QTreeWidget>

class MBMainWindowPrivate {
public:
	MBMainWindow *q;
    MBExperimentManager *m_experiment_manager;

    QTreeWidget *m_experiment_list;
    void refresh_experiment_list();
};

MBMainWindow::MBMainWindow()
{
	d=new MBMainWindowPrivate;
	d->q=this;
    d->m_experiment_manager=0;

    d->m_experiment_list=new QTreeWidget;

    QHBoxLayout *hlayout=new QHBoxLayout;
    hlayout->addWidget(d->m_experiment_list);
    this->setLayout(hlayout);
}

MBMainWindow::~MBMainWindow()
{
    delete d;
}

void MBMainWindow::setExperimentManager(MBExperimentManager *M)
{
    d->m_experiment_manager=M;
    d->refresh_experiment_list();
}


void MBMainWindowPrivate::refresh_experiment_list()
{
    QTreeWidget *X=m_experiment_list;
    X->clear();
    QStringList labels; labels << "Experiments";
    X->setHeaderLabels(labels);
    if (!m_experiment_manager) return;
    QStringList ids=m_experiment_manager->allExperimentIds();
    foreach (QString id,ids) {
        QStringList strings; strings << id;
        QTreeWidgetItem *item=new QTreeWidgetItem(strings);
        X->addTopLevelItem(item);
    }
}
