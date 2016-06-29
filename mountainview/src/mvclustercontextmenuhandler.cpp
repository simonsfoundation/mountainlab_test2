#include "mvclustercontextmenuhandler.h"
#include "mvmainwindow.h"
#include <QAction>
#include <QMenu>
#include <QSet>
#include <QSignalMapper>

MVClusterContextMenuHandler::MVClusterContextMenuHandler(QObject *parent)
    : QObject(parent)
{

}

bool MVClusterContextMenuHandler::canHandle(const QMimeData &md) const {
    return md.hasFormat("application/x-mv-cluster");
}

QList<QAction *> MVClusterContextMenuHandler::actions(const QMimeData &md) {
    QSet<int> clusters;
    QDataStream ds(md.data("application/x-mv-cluster"));
    ds >> clusters;
    QList<QAction*> actions;
    QAction *action = 0;
    actions << addTagMenu(clusters);
    actions << removeTagMenu(clusters);
    action = new QAction("Clear tags", 0);
    connect(action, &QAction::triggered, [clusters]() {
        foreach (int cluster_number, clusters) {
            MVMainWindow::instance()->viewAgent()->setClusterTags(cluster_number, QSet<QString>());
        }
    });
    actions << action;

    action = new QAction(0);
    action->setSeparator(true);
    actions << action;
    if (clusters.size() == 1) {
        action = new QAction("Open cross-correlograms associated with this cluster", 0);
        action->setData(clusters.values().first());
        connect(action, &QAction::triggered, []() {
            MVMainWindow::instance()->openView("open-cross-correlograms");
        });
        actions << action;
    } else {
        action = new QAction("Open matrix of cross-correlograms", 0);
        QVariantList clusterList;
        foreach(int c, clusters) clusterList << c;
        action->setData(clusterList);
        connect(action, &QAction::triggered, []() {
            MVMainWindow::instance()->openView("open-matrix-of-cross-correlograms");
        });
        actions << action;
    }
    return actions;
}

QAction *MVClusterContextMenuHandler::addTagMenu(const QSet<int> &clusters) const
{
    QMenu *M = new QMenu;
    M->setTitle("Add tag");
    QSignalMapper *mapper = new QSignalMapper(M);
    foreach(const QString &tag, validTags()) {
        QAction *a = new QAction(tag, M);
        a->setData(tag);
        M->addAction(a);
        mapper->setMapping(a, tag);
        connect(a, SIGNAL(triggered()), mapper, SLOT(map()));
    }
    connect(mapper, static_cast<void(QSignalMapper::*)(const QString&)>(&QSignalMapper::mapped),
            [clusters](const QString &tag) {
        foreach(int cluster, clusters) {
            QSet<QString> clTags = MVMainWindow::instance()->viewAgent()->clusterTags(cluster);
            clTags.insert(tag);
            MVMainWindow::instance()->viewAgent()->setClusterTags(cluster, clTags);
        }
    });

    return M->menuAction();
}

QAction *MVClusterContextMenuHandler::removeTagMenu(const QSet<int> &clusters) const
{
    QSet<QString> tags_set;
    foreach (int cluster_number, clusters) {
        QJsonObject attributes = MVMainWindow::instance()->viewAgent()->clusterAttributes(cluster_number);
        QJsonArray tags = attributes["tags"].toArray();
        for (int i = 0; i < tags.count(); i++) {
            tags_set.insert(tags[i].toString());
        }
    }
    QStringList tags_list = tags_set.toList();
    qSort(tags_list);
    QMenu *M = new QMenu;
    M->setTitle("Remove tag");
    QSignalMapper *mapper = new QSignalMapper(M);
    foreach(const QString &tag, tags_list) {
        QAction *a = new QAction(tag, M);
        a->setData(tag);
        M->addAction(a);
        mapper->setMapping(a, tag);
        connect(a, SIGNAL(triggered()), mapper, SLOT(map()));
    }
    connect(mapper, static_cast<void(QSignalMapper::*)(const QString&)>(&QSignalMapper::mapped),
            [clusters](const QString &tag) {
        foreach(int cluster, clusters) {
            QSet<QString> clTags = MVMainWindow::instance()->viewAgent()->clusterTags(cluster);
            clTags.remove(tag);
            MVMainWindow::instance()->viewAgent()->setClusterTags(cluster, clTags);
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



