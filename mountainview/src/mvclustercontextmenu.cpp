#include "mvclustercontextmenu.h"

#include <QJsonArray>

class MVClusterContextMenuPrivate {
public:
    MVClusterContextMenu* q;
    MVContext* m_context;
    QSet<int> m_cluster_numbers;
};

MVClusterContextMenu::MVClusterContextMenu(MVContext* mvcontext, QSet<int> cluster_numbers)
{
    d = new MVClusterContextMenuPrivate;
    d->q = this;
    d->m_context = mvcontext;
    d->m_cluster_numbers = cluster_numbers;

    QStringList valid_tags;
    valid_tags << "accept"
               << "reject"
               << "noise"
               << "mua"
               << "artifact";

    QSet<QString> tags_set;
    foreach (int cluster_number, d->m_cluster_numbers) {
        QJsonObject attributes = d->m_context->clusterAttributes(cluster_number);
        QJsonArray tags = attributes["tags"].toArray();
        for (int i = 0; i < tags.count(); i++) {
            tags_set.insert(tags[i].toString());
        }
    }
    QStringList tags_list = tags_set.toList();
    qSort(tags_list);

    QMenu* menu = this;
    {
        //add tag
        QMenu* M = new QMenu;
        M->setTitle("Add tag");
        foreach(QString tag, valid_tags)
        {
            QAction* A = new QAction(this);
            A->setText(tag);
            A->setProperty("tag", tag);
            M->addAction(A);
            QObject::connect(A, SIGNAL(triggered(bool)), this, SLOT(slot_add_tag()));
        }
        menu->addMenu(M);
    }
    {
        //remove tag
        QMenu* M = new QMenu;
        M->setTitle("Remove tag");
        foreach(QString tag, tags_list)
        {
            QAction* A = new QAction(this);
            A->setText(tag);
            A->setProperty("tag", tag);
            M->addAction(A);
            QObject::connect(A, SIGNAL(triggered(bool)), this, SLOT(slot_remove_tag()));
        }
        M->setEnabled(!tags_list.isEmpty());
        menu->addMenu(M);
    }

    {
        //clear tags
        QAction* A = new QAction(this);
        A->setText("Clear tags");
        QObject::connect(A, SIGNAL(triggered(bool)), this, SLOT(slot_clear_tags()));
        menu->addAction(A);
    }
}

MVClusterContextMenu::~MVClusterContextMenu()
{
    delete d;
}

QJsonArray stringset2jsonarray(QSet<QString> set)
{
    QJsonArray ret;
    QStringList list = set.toList();
    qSort(list);
    foreach(QString str, list)
    {
        ret.append(str);
    }
    return ret;
}

QSet<QString> jsonarray2stringset(QJsonArray X)
{
    QSet<QString> ret;
    for (int i = 0; i < X.count(); i++) {
        ret.insert(X[i].toString());
    }
    return ret;
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
        QJsonObject attributes = d->m_context->clusterAttributes(cluster_number);
        QJsonArray tags = attributes["tags"].toArray();
        QSet<QString> tags_set = jsonarray2stringset(tags);
        tags_set.insert(tag);
        attributes["tags"] = stringset2jsonarray(tags_set);
        d->m_context->setClusterAttributes(cluster_number, attributes);
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
        QJsonObject attributes = d->m_context->clusterAttributes(cluster_number);
        QJsonArray tags = attributes["tags"].toArray();
        QSet<QString> tags_set = jsonarray2stringset(tags);
        tags_set.insert(tag);
        attributes["tags"] = stringset2jsonarray(tags_set);
        d->m_context->setClusterAttributes(cluster_number, attributes);
    }
}

void MVClusterContextMenu::slot_clear_tags()
{
    foreach (int cluster_number, d->m_cluster_numbers) {
        QJsonObject attributes = d->m_context->clusterAttributes(cluster_number);
        attributes["tags"] = QJsonArray();
        d->m_context->setClusterAttributes(cluster_number, attributes);
    }
}
