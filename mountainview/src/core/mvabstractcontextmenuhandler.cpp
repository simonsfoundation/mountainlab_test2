#include "mvabstractcontextmenuhandler.h"

MVAbstractContextMenuHandler::MVAbstractContextMenuHandler(MVContext* context, MVMainWindow* mw)
{
    m_context = context;
    m_main_window = mw;
}

MVContext* MVAbstractContextMenuHandler::mvContext() const
{
    return m_context;
}

MVMainWindow* MVAbstractContextMenuHandler::mainWindow() const
{
    return m_main_window;
}
