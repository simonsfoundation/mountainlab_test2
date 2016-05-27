#ifndef TOOLBUTTONMENU_H
#define TOOLBUTTONMENU_H

#include <QObject>
#include <QToolButton>
#include <QEvent>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QMenu>
#include <QDebug>

class ToolButtonMenu : public QObject {
        Q_OBJECT
public:
        ToolButtonMenu(QObject *parent = 0) : QObject(parent) {

        }
        QToolButton *activateOn(QWidget *w) {
                w->installEventFilter(this);
                w->setAttribute(Qt::WA_Hover);
                QToolButton *tb = new QToolButton(w);
                connect(tb, SIGNAL(clicked()), this, SLOT(openMenu()));
                QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(tb);
                tb->setGraphicsEffect(eff);
                m_widgets.insert(w, tb);
                tb->setVisible(true);
                eff->setOpacity(0);
                QSize sh = tb->sizeHint();
                //tb->setGeometry(w->width() - sh.width() - 10, 10, sh.width(), sh.height());
                tb->setGeometry(w->width() - sh.width(), 0, sh.width() + 4, sh.height());
                return tb;
        }
protected:
        bool eventFilter(QObject *o, QEvent *e) {
                if (o->isWidgetType()) {
                        QWidget *w = static_cast<QWidget*>(o);
                        if (e->type() == QEvent::HoverEnter) {
                                QToolButton *tb = m_widgets.value(w);
                                QGraphicsOpacityEffect *eff
                                                = qobject_cast<QGraphicsOpacityEffect*>(tb->graphicsEffect());
                                QPropertyAnimation *anim = new QPropertyAnimation(eff, "opacity");
                                anim->setDuration(250);
                                anim->setStartValue(0);
                                anim->setEndValue(1);
                                anim->start();

                        } else if (e->type() == QEvent::HoverLeave) {
                                QToolButton *tb = m_widgets.value(w);
                                QGraphicsOpacityEffect *eff
                                                = qobject_cast<QGraphicsOpacityEffect*>(tb->graphicsEffect());
                                QPropertyAnimation *anim = new QPropertyAnimation(eff, "opacity");
                                anim->setDuration(250);
                                anim->setStartValue(1);
                                anim->setEndValue(0);
                                anim->start();
                        } else if (e->type() == QEvent::Resize) {
                                QToolButton *tb = m_widgets.value(w);
                                QSize sh = tb->sizeHint();
                                tb->setGeometry(w->width() - sh.width() + 4, 0, sh.width(), sh.height());
                        }
                }
                return false;
        }
private slots:
        void openMenu() {
                QToolButton *tb = qobject_cast<QToolButton*>(sender());
                if (!tb) return;
                QWidget *w = m_widgets.key(tb);
                if (!w) return;
                QMenu m;
                m.addActions(w->actions());
                m.exec(tb->mapToGlobal(tb->rect().bottomLeft()));
        }

private:
        QHash<QWidget*, QToolButton *> m_widgets;
};


#endif // TOOLBUTTONMENU_H

