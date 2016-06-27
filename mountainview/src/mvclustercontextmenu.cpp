#include "mvclustercontextmenu.h"

#include <QJsonArray>
#include "mvmainwindow.h"

class MVClusterContextMenuPrivate {
public:
    MVClusterContextMenu* q;
    MVContext* m_context;
    MVMainWindow* m_main_window;
    QSet<int> m_cluster_numbers;

    void add_tag_options();
    void add_merge_options();
    void add_cross_correlogram_options();

    bool can_unmerge_selected_clusters();
};

MVClusterContextMenu::MVClusterContextMenu(MVContext* mvcontext, MVMainWindow* mw, const QSet<int>& cluster_numbers)
{
    d = new MVClusterContextMenuPrivate;
    d->q = this;
    d->m_context = mvcontext;
    d->m_main_window = mw;
    d->m_cluster_numbers = cluster_numbers;

    d->add_tag_options();
    this->addSeparator();
    d->add_merge_options();
    this->addSeparator();
    d->add_cross_correlogram_options();
}

MVClusterContextMenu::~MVClusterContextMenu()
{
    delete d;
}

void MVClusterContextMenu::slot_add_tag()
{
    QAction* A = qobject_cast<QAction*>(sender());
    if (!A)
        return;
    QString tag = A->property("tag").toString();
    if (tag.isEmpty())
        return;

    foreach (int cluster_number, d->m_cluster_numbers) {
        QSet<QString> tags = d->m_context->clusterTags(cluster_number);
        tags.insert(tag);
        d->m_context->setClusterTags(cluster_number, tags);
    }
}

void MVClusterContextMenu::slot_remove_tag()
{
    QAction* A = qobject_cast<QAction*>(sender());
    if (!A)
        return;
    QString tag = A->property("tag").toString();
    if (tag.isEmpty())
        return;

    foreach (int cluster_number, d->m_cluster_numbers) {
        QSet<QString> tags = d->m_context->clusterTags(cluster_number);
        tags.remove(tag);
        d->m_context->setClusterTags(cluster_number, tags);
    }
}

void MVClusterContextMenu::slot_clear_tags()
{
    foreach (int cluster_number, d->m_cluster_numbers) {
        QSet<QString> tags = d->m_context->clusterTags(cluster_number);
        tags.clear();
        d->m_context->setClusterTags(cluster_number, tags);
    }
}

void MVClusterContextMenu::slot_open_cross_correlograms()
{
    d->m_main_window->openView("open-cross-correlograms");
}

void MVClusterContextMenu::slot_open_matrix_of_cross_correlograms()
{
    d->m_main_window->openView("open-matrix-of-cross-correlograms");
}

void MVClusterContextMenu::slot_merge_selected_clusters()
{
    ClusterMerge CM = d->m_context->clusterMerge();
    QList<int> ks = d->m_context->selectedClusters();
    CM.merge(ks);
    d->m_context->setClusterMerge(CM);
}

void MVClusterContextMenu::slot_unmerge_selected_clusters()
{
    ClusterMerge CM = d->m_context->clusterMerge();
    QList<int> ks = d->m_context->selectedClusters();
    CM.unmerge(ks);
    d->m_context->setClusterMerge(CM);
}

void MVClusterContextMenuPrivate::add_tag_options()
{
    /// TODO (LOW) these go in a configuration file
    QStringList valid_tags;
    valid_tags << "accepted"
               << "rejected"
               << "noise"
               << "mua"
               << "artifact";

    QSet<QString> tags_set;
    foreach (int cluster_number, m_cluster_numbers) {
        QJsonObject attributes = m_context->clusterAttributes(cluster_number);
        QJsonArray tags = attributes["tags"].toArray();
        for (int i = 0; i < tags.count(); i++) {
            tags_set.insert(tags[i].toString());
        }
    }
    QStringList tags_list = tags_set.toList();
    qSort(tags_list);

    QMenu* menu = q;
    {
        //add tag
        QMenu* M = new QMenu;
        M->setTitle("Add tag");
        foreach (QString tag, valid_tags) {
            QAction* A = new QAction(q);
            A->setText(tag);
            A->setProperty("tag", tag);
            M->addAction(A);
            QObject::connect(A, SIGNAL(triggered(bool)), q, SLOT(slot_add_tag()));
        }
        menu->addMenu(M);
    }
    {
        //remove tag
        QMenu* M = new QMenu;
        M->setTitle("Remove tag");
        foreach (QString tag, tags_list) {
            QAction* A = new QAction(q);
            A->setText(tag);
            A->setProperty("tag", tag);
            M->addAction(A);
            QObject::connect(A, SIGNAL(triggered(bool)), q, SLOT(slot_remove_tag()));
        }
        M->setEnabled(!tags_list.isEmpty());
        menu->addMenu(M);
    }

    {
        //clear tags
        QAction* A = new QAction(q);
        A->setText("Clear tags");
        QObject::connect(A, SIGNAL(triggered(bool)), q, SLOT(slot_clear_tags()));
        menu->addAction(A);
    }
}

void MVClusterContextMenuPrivate::add_merge_options()
{
    QMenu* menu = q;
    if (m_cluster_numbers.count() >= 1) {
        {
            QAction* A = new QAction(q);
            A->setText("Merge selected clusters");
            QObject::connect(A, &QAction::triggered, q, &MVClusterContextMenu::slot_merge_selected_clusters);
            menu->addAction(A);
            A->setEnabled(m_cluster_numbers.count() >= 2);
        }
        {
            QAction* A = new QAction(q);
            A->setText("Unmerge selected clusters");
            QObject::connect(A, &QAction::triggered, q, &MVClusterContextMenu::slot_unmerge_selected_clusters);
            menu->addAction(A);
            A->setEnabled(can_unmerge_selected_clusters());
        }
    }
}

void MVClusterContextMenuPrivate::add_cross_correlogram_options()
{
    QMenu* menu = q;
    if (m_cluster_numbers.count() == 1) {
        {
            QAction* A = new QAction(q);
            A->setText("Open cross-correlograms associated with this cluster");
            QObject::connect(A, &QAction::triggered, q, &MVClusterContextMenu::slot_open_cross_correlograms);
            menu->addAction(A);
        }
    }
    else {
        {
            QAction* A = new QAction(q);
            A->setText("Open matrix of cross-correlograms");
            QObject::connect(A, &QAction::triggered, q, &MVClusterContextMenu::slot_open_matrix_of_cross_correlograms);
            menu->addAction(A);
        }
    }
}

bool MVClusterContextMenuPrivate::can_unmerge_selected_clusters()
{
    ClusterMerge CM = m_context->clusterMerge();
    QList<int> ks = m_context->selectedClusters();
    foreach (int k, ks) {
        if (CM.representativeLabel(k) != k)
            return true;
    }
    return false;
}
