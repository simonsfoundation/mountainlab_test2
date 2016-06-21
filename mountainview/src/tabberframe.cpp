/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/20/2016
*******************************************************/

#include "tabberframe.h"

#include <QMenu>
#include <QToolBar>
#include <QAction>
#include <QVBoxLayout>
#include <QToolButton>

class TabberFramePrivate {
public:
    TabberFrame* q;
    MVAbstractView* m_view;
    QToolBar* m_toolbar;
    QAction* m_recalc_action;
    QAction* m_never_recalc_action;

    void update_action_visibility();
    static QList<QAction*> find_actions_of_type(QList<QAction*> actions, QString str);
};

TabberFrame::TabberFrame(MVAbstractView* view)
{
    d = new TabberFramePrivate;
    d->q = this;
    d->m_view = view;
    d->m_toolbar = new QToolBar;
    d->m_recalc_action = new QAction(QIcon(":/images/calculator.png"), "", this);
    d->m_recalc_action->setToolTip("Recalculate this view");
    QObject::connect(d->m_recalc_action, SIGNAL(triggered(bool)), view, SLOT(recalculate()));
    d->m_never_recalc_action = new QAction("Never", this);
    d->m_never_recalc_action->setToolTip("Never recalculate this widget");
    QObject::connect(d->m_never_recalc_action, SIGNAL(triggered(bool)), view, SLOT(neverSuggestRecalculate()));

    QToolButton* tool_button = new QToolButton;
    QMenu* menu = new QMenu;
    menu->addActions(d->find_actions_of_type(view->actions(), ""));
    tool_button->setMenu(menu);
    tool_button->setIcon(QIcon(":images/gear.png"));
    tool_button->setPopupMode(QToolButton::InstantPopup);
    QWidget* spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    d->m_toolbar->addAction(d->m_recalc_action);
    d->m_toolbar->addAction(d->m_never_recalc_action);
    d->m_to

    QList<QAction*> toolbar_actions = d->find_actions_of_type(view->actions(), "toolbar");
    foreach (QAction* a, toolbar_actions) {
        d->m_toolbar->addAction(a);
    }

    d->m_toolbar->addWidget(spacer);
    d->m_toolbar->addWidget(tool_button);

    QVBoxLayout* vlayout = new QVBoxLayout;
    vlayout->setSpacing(0);
    vlayout->setMargin(0);
    vlayout->addWidget(d->m_toolbar);
    vlayout->addWidget(view);
    this->setLayout(vlayout);

    QObject::connect(view, SIGNAL(recalculateSuggestedChanged()), this, SLOT(slot_update_action_visibility()));
    d->update_action_visibility();
}

TabberFrame::~TabberFrame()
{
    delete d;
}

MVAbstractView* TabberFrame::view()
{
    return d->m_view;
}

void TabberFrame::slot_update_action_visibility()
{
    d->update_action_visibility();
}

void TabberFramePrivate::update_action_visibility()
{
    if (m_view->recalculateSuggested()) {
        m_recalc_action->setVisible(true);
        m_never_recalc_action->setVisible(true);
    }
    else {
        m_recalc_action->setVisible(false);
        m_never_recalc_action->setVisible(false);
    }
    if (m_view->property("container_name").toString() == "north") {
    }
}

QList<QAction*> TabberFramePrivate::find_actions_of_type(QList<QAction*> actions, QString str)
{
    QList<QAction*> ret;
    for (int i = 0; i < actions.count(); i++) {
        if (actions[i]->property("action_type").toString() == str) {
            ret << actions[i];
        }
    }
    return ret;
}
