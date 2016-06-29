#ifndef MVABSTRACTCONTEXTMENUHANDLER_H
#define MVABSTRACTCONTEXTMENUHANDLER_H

#include <QMimeData>

/*!
 * Context menu handler is an object that can populate context menu with
 * context sensitive actions. QMimeData is used as a carrier for determining
 * the context. A view can request a menu for a given context by creating
 * a QMimeData object and adding custom MIME types as well as associating
 * extra data with a given MIME type. Registered context menu handlers are
 * queried with the MIME type to return a list of supported actions for that
 * type and its data. Actions from all handlers are then assembled and
 * shown in form of a context menu at a given position on the screen.
 *
 * Actions should make use of the associated data to trigger appropriate
 * functionality.
 */

class QAction;

class MVAbstractContextMenuHandler {
public:
    virtual ~MVAbstractContextMenuHandler() {}
    virtual bool canHandle(const QMimeData &md) const { return false; }
    virtual QList<QAction*> actions(const QMimeData &md) = 0;
};
#endif // MVABSTRACTCONTEXTMENUHANDLER_H

