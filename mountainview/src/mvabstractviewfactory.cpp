#include "mvabstractviewfactory.h"

MVAbstractViewFactory::MVAbstractViewFactory(QObject *parent)
    : QObject(parent)
    , m_enabled(true)
{

}

bool MVAbstractViewFactory::isEnabled() const
{
    return m_enabled;
}

QString MVAbstractViewFactory::group() const { return QString(); }

QString MVAbstractViewFactory::toolTip() const { return QString(); }

void MVAbstractViewFactory::setEnabled(bool e)
{
    if (isEnabled() == e) return;
    m_enabled = e;
    emit enabledChanged(e);
}
