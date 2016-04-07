/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/6/2016
*******************************************************/

#include "mbexperimentlistwidget.h"

class MBExperimentListWidgetPrivate {
public:
    MBExperimentListWidget *q;
    MBExperimentManager *m_experiment_manager;
};

MBExperimentListWidget::MBExperimentListWidget()
{
    d=new MBExperimentListWidgetPrivate;
    d->q=this;

    this->setExpandsOnDoubleClick(false);
    connect(this,SIGNAL(itemActivated(QTreeWidgetItem*,int)),this,SLOT(slot_item_activated(QTreeWidgetItem*)));
}

MBExperimentListWidget::~MBExperimentListWidget()
{
    delete d;
}


void MBExperimentListWidget::setExperimentManager(MBExperimentManager *EM)
{
    d->m_experiment_manager=EM;
    this->refresh();
}

QString strlist2str(const QStringList &X) {
    QString ret;
    foreach (QString str,X) {
        if (!ret.isEmpty()) ret+=", ";
        ret+=str;
    }
    return ret;
}

void MBExperimentListWidget::refresh()
{
    QTreeWidget *X=this;
    X->clear();
    QStringList labels; labels << "Experiments" << "Type" << "Base Path";
    X->setHeaderLabels(labels);
    if (!d->m_experiment_manager) return;
    QStringList ids=d->m_experiment_manager->allExperimentIds();
    foreach (QString id,ids) {
        QStringList strings;
        MBExperiment E=d->m_experiment_manager->experiment(id);
        strings << id << E.json["exp_type"].toString() << E.json["basepath"].toString() << strlist2str(E.json.keys());
        QTreeWidgetItem *item=new QTreeWidgetItem(strings);
        item->setData(0,Qt::UserRole,id);
        X->addTopLevelItem(item);
        QStringList keys=E.json.keys();
        qSort(keys);
        foreach (QString key,keys) {
            QString str0=QString("%1=%2").arg(key,E.json[key].toString());
            QTreeWidgetItem *item2=new QTreeWidgetItem(QStringList(str0));
            item->addChild(item2);
        }
    }
    for (int j=0; j<X->columnCount(); j++) {
        X->resizeColumnToContents(j);
    }
    X->setColumnWidth(0,qMax(400,X->columnWidth(0)));
}

void MBExperimentListWidget::slot_item_activated(QTreeWidgetItem *item)
{
    QString id=item->data(0,Qt::UserRole).toString();
    emit experimentActivated(id);
}
