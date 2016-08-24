#include "mvabstractviewfactory.h"

MVAbstractViewFactory::MVAbstractViewFactory(QObject* parent)
    : QObject(parent)
{
}

QString MVAbstractViewFactory::group() const { return QString(); }

QString MVAbstractViewFactory::toolTip() const { return QString(); }

QString MVAbstractViewFactory::title() const { return name(); }

bool MVAbstractViewFactory::isEnabled(MVContext* context) const
{
    Q_UNUSED(context)
    return true;
}
