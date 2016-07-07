#include "mvclusterpaircontextmenuhandler.h"
#include "mvmainwindow.h"
#include <QAction>
#include <QMenu>
#include <QSet>
#include <QSignalMapper>

MVClusterPairContextMenuHandler::MVClusterPairContextMenuHandler(MVContext* context, MVMainWindow* mw, QObject* parent)
    : QObject(parent)
    , MVAbstractContextMenuHandler(context, mw)
{
}

bool MVClusterPairContextMenuHandler::canHandle(const QMimeData& md) const
{
    return md.hasFormat("application/x-mv-cluster-pairs");
}

QSet<ClusterPair> string_list_to_cluster_pair_set(QStringList list)
{
    QSet<ClusterPair> ret;
    foreach (QString str, list) {
        ret.insert(ClusterPair::fromString(str));
    }
    return ret;
}

QList<QAction*> MVClusterPairContextMenuHandler::actions(const QMimeData& md)
{
    QStringList clusters_strlist;
    QDataStream ds(md.data("application/x-mv-cluster-pairs"));
    ds >> clusters_strlist;
    QSet<ClusterPair> cluster_pairs = string_list_to_cluster_pair_set(clusters_strlist);
    QList<QAction*> actions;

    MVContext* context = this->mvContext();
    MVMainWindow* mw = this->mainWindow();
    Q_UNUSED(mw)

    //TAGS
    {
        actions << addTagMenu(cluster_pairs);
        actions << removeTagMenu(cluster_pairs);
        QAction* action = new QAction("Clear tags", 0);
        connect(action, &QAction::triggered, [cluster_pairs, context]() {
            foreach (ClusterPair cluster_pair, cluster_pairs) {
                context->setClusterPairTags(cluster_pair, QSet<QString>());
            }
        });
        actions << action;
    }

    //Separator
    {
        QAction* action = new QAction(0);
        action->setSeparator(true);
        actions << action;
    }

    //CROSS-CORRELOGRAMS
    {
        /// TODO (HIGH) implement cross-correlograms for selected pairs
    }

    //Separator
    {
        QAction* action = new QAction(0);
        action->setSeparator(true);
        actions << action;
    }

    //DISCRIMINATION HISTOGRAMS
    {
        /// TODO (HIGH) implement discrim hists for selected pairs
    }

    return actions;
}

QAction* MVClusterPairContextMenuHandler::addTagMenu(const QSet<ClusterPair>& cluster_pairs) const
{
    MVContext* context = mvContext();

    QMenu* M = new QMenu;
    M->setTitle("Add tag");
    QSignalMapper* mapper = new QSignalMapper(M);
    foreach (const QString& tag, validTags()) {
        QAction* a = new QAction(tag, M);
        a->setData(tag);
        M->addAction(a);
        mapper->setMapping(a, tag);
        connect(a, SIGNAL(triggered()), mapper, SLOT(map()));
    }
    /// Witold, I am a bit nervous about passing the context pointer into the lambda function below
    connect(mapper, static_cast<void (QSignalMapper::*)(const QString&)>(&QSignalMapper::mapped),
        [cluster_pairs, context](const QString& tag) {
        foreach(ClusterPair cluster_pair, cluster_pairs) {
            QSet<QString> clTags = context->clusterPairTags(cluster_pair);
            clTags.insert(tag);
            context->setClusterPairTags(cluster_pair, clTags);
        }
        });

    return M->menuAction();
}

QAction* MVClusterPairContextMenuHandler::removeTagMenu(const QSet<ClusterPair>& cluster_pairs) const
{
    MVContext* context = mvContext();

    QSet<QString> tags_set;
    foreach (ClusterPair cluster_pair, cluster_pairs) {
        QJsonObject attributes = mvContext()->clusterPairAttributes(cluster_pair);
        QJsonArray tags = attributes["tags"].toArray();
        for (int i = 0; i < tags.count(); i++) {
            tags_set.insert(tags[i].toString());
        }
    }
    QStringList tags_list = tags_set.toList();
    qSort(tags_list);
    QMenu* M = new QMenu;
    M->setTitle("Remove tag");
    QSignalMapper* mapper = new QSignalMapper(M);
    foreach (const QString& tag, tags_list) {
        QAction* a = new QAction(tag, M);
        a->setData(tag);
        M->addAction(a);
        mapper->setMapping(a, tag);
        connect(a, SIGNAL(triggered()), mapper, SLOT(map()));
    }
    connect(mapper, static_cast<void (QSignalMapper::*)(const QString&)>(&QSignalMapper::mapped),
        [cluster_pairs, context](const QString& tag) {
        foreach(ClusterPair cluster_pair, cluster_pairs) {
            QSet<QString> clTags = context->clusterPairTags(cluster_pair);
            clTags.remove(tag);
            context->setClusterPairTags(cluster_pair, clTags);
        }
        });
    M->setEnabled(!tags_list.isEmpty());
    return M->menuAction();
}

QStringList MVClusterPairContextMenuHandler::validTags() const
{
    QSet<QString> set = this->mvContext()->allClusterPairTags();
    /// TODO (LOW) these go in a configuration file
    set << "to_merge";
    QStringList ret = set.toList();
    qSort(ret);
    return ret;
}
