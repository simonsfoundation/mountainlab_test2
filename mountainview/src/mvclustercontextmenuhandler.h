#ifndef MVCLUSTERCONTEXTMENUHANDLER_H
#define MVCLUSTERCONTEXTMENUHANDLER_H

#include <QObject>
#include "mvabstractcontextmenuhandler.h"

class MVClusterContextMenuHandler : public QObject, public MVAbstractContextMenuHandler {
public:
    MVClusterContextMenuHandler(QObject* parent = 0);

    bool canHandle(const QMimeData& md) const Q_DECL_OVERRIDE;
    QList<QAction*> actions(const QMimeData& md) Q_DECL_OVERRIDE;

private:
    QAction* addTagMenu(const QSet<int>& clusters) const;
    QAction* removeTagMenu(const QSet<int>& clusters) const;
    QStringList validTags() const;
};

#endif // MVCLUSTERCONTEXTMENUHANDLER_H
