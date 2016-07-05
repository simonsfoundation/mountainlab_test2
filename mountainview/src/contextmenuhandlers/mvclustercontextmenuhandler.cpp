#include "mvclustercontextmenuhandler.h"
#include "mvmainwindow.h"
#include <QAction>
#include <QMenu>
#include <QSet>
#include <QSignalMapper>

MVClusterContextMenuHandler::MVClusterContextMenuHandler(MVContext* context, MVMainWindow* mw, QObject* parent)
    : QObject(parent)
    , MVAbstractContextMenuHandler(context, mw)
{
}

bool MVClusterContextMenuHandler::canHandle(const QMimeData& md) const
{
    return md.hasFormat("application/x-mv-cluster");
}

QList<QAction*> MVClusterContextMenuHandler::actions(const QMimeData& md)
{
    QSet<int> clusters;
    QDataStream ds(md.data("application/x-mv-cluster"));
    ds >> clusters;
    QList<QAction*> actions;

    MVContext* context = this->mvContext();
    MVMainWindow* mw = this->mainWindow();

    /// Witold, is there a reason that the clusterList is being added as data to some of the actions below? If not, let's remove this.
    QVariantList clusterList;
    foreach (int c, clusters)
        clusterList << c;
    int first_cluster=clusters.values().first();

    //TAGS
    {
        actions << addTagMenu(clusters);
        actions << removeTagMenu(clusters);
        QAction* action = new QAction("Clear tags", 0);
        connect(action, &QAction::triggered, [clusters, context]() {
            foreach (int cluster_number, clusters) {
                context->setClusterTags(cluster_number, QSet<QString>());
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

    //MERGE
    {
        {
            QAction* A = new QAction(0);
            A->setEnabled(clusters.count() >= 2);
            A->setText("Merge selected");
            QObject::connect(A, &QAction::triggered, [clusters, context]() {
                ClusterMerge CM = context->clusterMerge();
                CM.merge(clusters);
                context->setClusterMerge(CM);
            });
            actions << A;
        }
        {
            QAction* A = new QAction(0);
            A->setEnabled(can_unmerge_selected_clusters(context, clusters));
            A->setText("Unmerge selected");
            QObject::connect(A, &QAction::triggered, [clusters, context]() {
                ClusterMerge CM = context->clusterMerge();
                CM.unmerge(clusters);
                context->setClusterMerge(CM);
            });
            actions << A;
        }
    }

    //Separator
    {
        QAction* action = new QAction(0);
        action->setSeparator(true);
        actions << action;
    }

    //CROSS-CORRELOGRAMS
    {
        {
            QAction* action = new QAction("Auto-correlograms", 0);
            action->setToolTip("Open auto-correlograms for these clusters");
            action->setEnabled(clusters.count() >= 1);
            action->setData(first_cluster);
            connect(action, &QAction::triggered, [mw]() {
                mw->openView("open-selected-auto-correlograms");
            });
            actions << action;
        }
        {
            QAction* action = new QAction("Cross-correlograms", 0);
            action->setToolTip("Open cross-correlograms associated with this cluster");
            action->setEnabled(clusters.count() == 1);
            action->setData(first_cluster);
            connect(action, &QAction::triggered, [mw]() {
                mw->openView("open-cross-correlograms");
            });
            actions << action;
        }
        {
            QAction* action = new QAction("Matrix of cross-correlograms", 0);
            action->setEnabled(clusters.count() >= 2);
            action->setData(clusterList);
            connect(action, &QAction::triggered, [mw]() {
                mw->openView("open-matrix-of-cross-correlograms");
            });
            actions << action;
        }
    }

    //Separator
    {
        QAction* action = new QAction(0);
        action->setSeparator(true);
        actions << action;
    }

    //DISCRIMINATION HISTOGRAMS
    {
        QAction* action = new QAction("Discrim Hist", 0);
        action->setToolTip("Open discrimination histograms for these clusters");
        action->setEnabled(clusters.count() >= 2);

        connect(action, &QAction::triggered, [mw]() {
            mw->openView("open-discrim-histograms");
        });
        actions << action;
    }

    //SPIKE SPRAY
    {
        QAction* action = new QAction("Spike spray", 0);
        action->setToolTip("Open spike spray for these clusters");
        action->setEnabled(clusters.count() >= 1);
        action->setData(clusterList);
        connect(action, &QAction::triggered, [mw]() {
            mw->openView("open-spike-spray");
        });
        actions << action;
    }

    //SPIKE SPRAY
    {
        QAction* action = new QAction("Clips", 0);
        action->setToolTip("View all spike clips for these clusters");
        action->setEnabled(clusters.count() >= 1);
        action->setData(clusterList);
        connect(action, &QAction::triggered, [mw]() {
            mw->openView("open-clips");
        });
        actions << action;
    }

    return actions;
}

bool MVClusterContextMenuHandler::can_unmerge_selected_clusters(MVContext* context, const QSet<int>& clusters)
{
    ClusterMerge CM = context->clusterMerge();
    foreach (int k, clusters) {
        if (CM.representativeLabel(k) != k)
            return true;
        if (CM.getMergeGroup(k).count() > 1)
            return true;
    }
    return false;
}

QAction* MVClusterContextMenuHandler::addTagMenu(const QSet<int>& clusters) const
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
        [clusters, context](const QString& tag) {
        foreach(int cluster, clusters) {
            QSet<QString> clTags = context->clusterTags(cluster);
            clTags.insert(tag);
            context->setClusterTags(cluster, clTags);
        }
        });

    return M->menuAction();
}

QAction* MVClusterContextMenuHandler::removeTagMenu(const QSet<int>& clusters) const
{
    MVContext* context = mvContext();

    QSet<QString> tags_set;
    foreach (int cluster_number, clusters) {
        QJsonObject attributes = mvContext()->clusterAttributes(cluster_number);
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
        [clusters, context](const QString& tag) {
        foreach(int cluster, clusters) {
            QSet<QString> clTags = context->clusterTags(cluster);
            clTags.remove(tag);
            context->setClusterTags(cluster, clTags);
        }
        });
    M->setEnabled(!tags_list.isEmpty());
    return M->menuAction();
}

QStringList MVClusterContextMenuHandler::validTags() const
{
    /// TODO (LOW) these go in a configuration file
    return QStringList() << "accepted"
                         << "rejected"
                         << "noise"
                         << "mua"
                         << "artifact";
}
