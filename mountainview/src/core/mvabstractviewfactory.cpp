#include "mvabstractviewfactory.h"

MVAbstractViewFactory::MVAbstractViewFactory(MVContext* context, QObject* parent)
    : QObject(parent)
    , m_enabled(true)
    , m_context(context)
{
}

bool MVAbstractViewFactory::isEnabled() const
{
    return m_enabled;
}

QString MVAbstractViewFactory::group() const { return QString(); }

QString MVAbstractViewFactory::toolTip() const { return QString(); }

QString MVAbstractViewFactory::title() const { return name(); }

void MVAbstractViewFactory::setEnabled(bool e)
{
    if (isEnabled() == e)
        return;
    m_enabled = e;
    emit enabledChanged(e);
}

MVContext* MVAbstractViewFactory::mvContext()
{
    return m_context;
}
