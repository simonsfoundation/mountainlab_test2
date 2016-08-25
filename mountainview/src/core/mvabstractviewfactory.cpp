#include "mvabstractviewfactory.h"

MVAbstractViewFactory::MVAbstractViewFactory(MVMainWindow* mw, QObject* parent)
    : QObject(parent)
{
    m_main_window = mw;
}

QString MVAbstractViewFactory::group() const { return QString(); }

QString MVAbstractViewFactory::toolTip() const { return QString(); }

QString MVAbstractViewFactory::title() const { return name(); }

QList<QAction*> MVAbstractViewFactory::actions(const QMimeData& md)
{
    Q_UNUSED(md)
    return QList<QAction*>();
}

bool MVAbstractViewFactory::isEnabled(MVContext* context) const
{
    Q_UNUSED(context)
    return true;
}

MVMainWindow* MVAbstractViewFactory::mainWindow()
{
    return m_main_window;
}
