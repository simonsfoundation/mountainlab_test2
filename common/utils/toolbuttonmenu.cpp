#include "toolbuttonmenu.h"
#include <QEvent>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QMenu>

ToolButtonMenu::ToolButtonMenu(QObject* parent)
    : QObject(parent)
    , m_offset(QSize(4, 0))
{
}

void ToolButtonMenu::setOffset(const QSize& off)
{
    if (offset() == off)
        return;
    m_offset = off;
    emit offsetChanged(m_offset);
}

QToolButton* ToolButtonMenu::activateOn(QWidget* w)
{
    w->installEventFilter(this);
    w->setAttribute(Qt::WA_Hover);
    QToolButton* tb = new QToolButton(w);
    tb->setAutoRaise(true);
    connect(tb, SIGNAL(clicked()), this, SLOT(openMenu()));
    QGraphicsOpacityEffect* eff = new QGraphicsOpacityEffect(tb);
    tb->setGraphicsEffect(eff);
    m_widgets.insert(w, tb);
    tb->setVisible(true);
    eff->setOpacity(0);
    updateButtonPosition(w, tb);
    return tb;
}

bool ToolButtonMenu::eventFilter(QObject* o, QEvent* e)
{
    if (o->isWidgetType()) {
        QWidget* w = static_cast<QWidget*>(o);
        if (e->type() == QEvent::HoverEnter) {
            QToolButton* tb = m_widgets.value(w);
            QGraphicsOpacityEffect* eff
                = qobject_cast<QGraphicsOpacityEffect*>(tb->graphicsEffect());
            QPropertyAnimation* anim = new QPropertyAnimation(eff, "opacity");
            anim->setDuration(250);
            anim->setStartValue(0);
            anim->setEndValue(1);
            anim->start();
        }
        else if (e->type() == QEvent::HoverLeave) {
            QToolButton* tb = m_widgets.value(w);
            QGraphicsOpacityEffect* eff
                = qobject_cast<QGraphicsOpacityEffect*>(tb->graphicsEffect());
            QPropertyAnimation* anim = new QPropertyAnimation(eff, "opacity");
            anim->setDuration(250);
            anim->setStartValue(1);
            anim->setEndValue(0);
            anim->start();
        }
        else if (e->type() == QEvent::Resize) {
            updateButtonPosition(w);
        }
    }
    return false;
}

void ToolButtonMenu::updateButtonPosition(QWidget* w, QToolButton* tb)
{
    if (!tb)
        tb = m_widgets.value(w);
    if (!w || !tb)
        return;
    QSize sh = tb->sizeHint();
    tb->setGeometry(w->width() - sh.width() - offset().width(), offset().height(),
        sh.width(), sh.height());
}

void ToolButtonMenu::openMenu()
{
    QToolButton* tb = qobject_cast<QToolButton*>(sender());
    if (!tb)
        return;
    QWidget* w = m_widgets.key(tb);
    if (!w)
        return;
    QMenu m;
    m.addActions(w->actions());
    m.exec(tb->mapToGlobal(tb->rect().bottomLeft()));
}
