/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/29/2016
*******************************************************/

#include "clustermetricsview.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QTreeWidget>
#include <mountainprocessrunner.h>
#include <taskprogress.h>
#include "actionfactory.h"

class ClusterMetricsViewPrivate {
public:
    ClusterMetricsView* q;
    QTreeWidget* m_tree;

    void refresh_tree();
};

ClusterMetricsView::ClusterMetricsView(MVContext* mvcontext)
    : MVAbstractView(mvcontext)
{
    d = new ClusterMetricsViewPrivate;
    d->q = this;

    QHBoxLayout* hlayout = new QHBoxLayout;
    this->setLayout(hlayout);

    d->m_tree = new QTreeWidget;
    d->m_tree->setSortingEnabled(true);
    d->m_tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    hlayout->addWidget(d->m_tree);

    this->recalculateOn(mvContext(), SIGNAL(clusterAttributesChanged(int)), false);
    this->recalculateOn(mvContext(), SIGNAL(clusterVisibilityChanged()), false);

    d->refresh_tree();
    this->recalculate();

    QObject::connect(d->m_tree, SIGNAL(itemSelectionChanged()), this, SLOT(slot_item_selection_changed()));
    QObject::connect(d->m_tree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(slot_current_item_changed()));
    QObject::connect(mvContext(), SIGNAL(currentClusterChanged()), this, SLOT(slot_update_current_cluster()));
    QObject::connect(mvContext(), SIGNAL(selectedClustersChanged()), this, SLOT(slot_update_selected_clusters()));
}

ClusterMetricsView::~ClusterMetricsView()
{
    this->stopCalculation();
    delete d;
}

void ClusterMetricsView::prepareCalculation()
{
    if (!mvContext())
        return;
}

void ClusterMetricsView::runCalculation()
{
}

void ClusterMetricsView::onCalculationFinished()
{
    d->refresh_tree();
}

void ClusterMetricsView::keyPressEvent(QKeyEvent* evt)
{
    Q_UNUSED(evt)
}

void ClusterMetricsView::prepareMimeData(QMimeData& mimeData, const QPoint& pos)
{
    /*
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << mvContext()->selectedClusters();
    mimeData.setData("application/x-msv-clusters", ba); // selected cluster data
    */

    MVAbstractView::prepareMimeData(mimeData, pos); // call base class implementation
}

void ClusterMetricsView::slot_current_item_changed()
{
    QTreeWidgetItem* it = d->m_tree->currentItem();
    if (it) {
        int k = it->data(0, Qt::UserRole).toInt();
        mvContext()->setCurrentCluster(k);
    }
    else {
        mvContext()->setCurrentCluster(-1);
    }
}

void ClusterMetricsView::slot_item_selection_changed()
{
    //QList<QTreeWidgetItem*> items=d->m_tree->selectedItems();

    QSet<int> selected = mvContext()->selectedClusters().toSet();

    for (int i = 0; i < d->m_tree->topLevelItemCount(); i++) {
        QTreeWidgetItem* it = d->m_tree->topLevelItem(i);
        int k = it->data(0, Qt::UserRole).toInt();
        if (it->isSelected()) {
            selected.insert(k);
        }
        else {
            selected.remove(k);
        }
    }

    mvContext()->setSelectedClusters(selected.toList());
}

void ClusterMetricsView::slot_update_current_cluster()
{
    int current = mvContext()->currentCluster();

    for (int i = 0; i < d->m_tree->topLevelItemCount(); i++) {
        QTreeWidgetItem* it = d->m_tree->topLevelItem(i);
        int k = it->data(0, Qt::UserRole).toInt();
        if (k == current) {
            d->m_tree->setCurrentItem(it);
        }
    }
}

void ClusterMetricsView::slot_update_selected_clusters()
{
    QSet<int> selected = mvContext()->selectedClusters().toSet();

    for (int i = 0; i < d->m_tree->topLevelItemCount(); i++) {
        QTreeWidgetItem* it = d->m_tree->topLevelItem(i);
        int k = it->data(0, Qt::UserRole).toInt();
        it->setSelected(selected.contains(k));
    }
}

class NumericSortTreeWidgetItem : public QTreeWidgetItem {
public:
    NumericSortTreeWidgetItem(QTreeWidget* parent)
        : QTreeWidgetItem(parent)
    {
    }

private:
    bool operator<(const QTreeWidgetItem& other) const
    {
        int column = treeWidget()->sortColumn();
        return text(column).toDouble() < other.text(column).toDouble();
    }
};

void ClusterMetricsViewPrivate::refresh_tree()
{
    m_tree->clear();

    QList<int> keys = q->mvContext()->clusterAttributesKeys();

    QSet<QString> metric_names_set;
    for (int ii = 0; ii < keys.count(); ii++) {
        QJsonObject metrics = q->mvContext()->clusterAttributes(keys[ii])["metrics"].toObject();
        QStringList nnn = metrics.keys();
        foreach (QString name, nnn) {
            metric_names_set.insert(name);
        }
    }

    QStringList metric_names = metric_names_set.toList();
    qSort(metric_names);

    QStringList headers;
    headers << "Cluster";
    headers.append(metric_names);
    m_tree->setHeaderLabels(headers);

    for (int ii = 0; ii < keys.count(); ii++) {
        int k = keys[ii];
        if (q->mvContext()->clusterIsVisible(k)) {
            NumericSortTreeWidgetItem* it = new NumericSortTreeWidgetItem(m_tree);
            QJsonObject metrics = q->mvContext()->clusterAttributes(k)["metrics"].toObject();
            it->setText(0, QString("%1").arg(k));
            it->setData(0, Qt::UserRole, k);
            for (int j = 0; j < metric_names.count(); j++) {
                it->setText(j + 1, QString("%1").arg(metrics[metric_names[j]].toDouble()));
            }
            m_tree->addTopLevelItem(it);
        }
    }

    for (int c = 0; c < m_tree->columnCount(); c++) {
        m_tree->resizeColumnToContents(c);
    }
}
