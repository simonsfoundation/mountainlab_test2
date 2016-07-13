#include "moresultlistview.h"

#include <QHBoxLayout>
#include <QTreeWidget>

class MOResultListViewPrivate {
public:
    MOResultListView* q;
    MOFile* m_mof;
    QTreeWidget* m_list;

    void refresh_list();
};

MOResultListView::MOResultListView(MOFile* mof)
{
    d = new MOResultListViewPrivate;
    d->q = this;
    d->m_mof = mof;

    d->m_list = new QTreeWidget;

    QVBoxLayout* vlayout = new QVBoxLayout;
    vlayout->addWidget(d->m_list);
    setLayout(vlayout);

    d->refresh_list();

    QObject::connect(mof, SIGNAL(resultsChanged()), this, SLOT(slot_refresh()));
    QObject::connect(d->m_list, SIGNAL(itemActivated(QTreeWidgetItem*, int)), this, SLOT(slot_item_activated(QTreeWidgetItem*)));
}

MOResultListView::~MOResultListView()
{
    delete d;
}

void MOResultListView::slot_refresh()
{
    d->refresh_list();
}

void MOResultListView::slot_item_activated(QTreeWidgetItem* it)
{
    QString name = it->data(0, Qt::UserRole).toString();
    if (name.isEmpty())
        return;
    emit resultActivated(name);
}

void MOResultListViewPrivate::refresh_list()
{
    QStringList labels;
    labels << "Result"
           << "Type";
    m_list->setHeaderLabels(labels);
    m_list->clear();
    QStringList names = m_mof->resultNames();
    foreach (QString name, names) {
        QJsonObject X = m_mof->result(name);
        QTreeWidgetItem* it = new QTreeWidgetItem;
        it->setData(0, Qt::UserRole, name);
        it->setText(0, name);
        it->setText(1, X["result_type"].toString());
        m_list->addTopLevelItem(it);
    }
    for (int i = 0; i < m_list->columnCount(); i++) {
        m_list->resizeColumnToContents(i);
    }
}
