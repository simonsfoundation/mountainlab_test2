/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/27/2016
*******************************************************/

#include "flowlayout.h"
#include "mvabstractview.h"
#include "mvabstractviewfactory.h"
#include "mvopenviewscontrol.h"
#include "mvmainwindow.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QTimer>
#include <QSignalMapper>
#include <QToolButton>

class MVOpenViewsControlPrivate {
public:
    MVOpenViewsControl* q;
    FlowLayout* m_flow_layout;
    QSignalMapper* m_viewMapper;
};

MVOpenViewsControl::MVOpenViewsControl(MVContext* context, MVMainWindow* mw)
    : MVAbstractControl(context, mw)
{
    d = new MVOpenViewsControlPrivate;
    d->q = this;

    d->m_viewMapper = new QSignalMapper(this);
    connect(d->m_viewMapper, SIGNAL(mapped(QObject*)),
        this, SLOT(slot_open_view(QObject*)));

    d->m_flow_layout = new FlowLayout;

    QList<MVAbstractViewFactory*> factories = mw->viewFactories();
    foreach (MVAbstractViewFactory* f, factories) {
        QToolButton* button = new QToolButton;
        QFont font = button->font();
        font.setPixelSize(14);
        button->setFont(font);
        button->setText(f->name());
        button->setProperty("action_name", f->id());
        button->setEnabled(f->isEnabled());
        d->m_flow_layout->addWidget(button);
        d->m_viewMapper->setMapping(button, f);
        QObject::connect(button, SIGNAL(clicked()), d->m_viewMapper, SLOT(map()));
        QObject::connect(f, SIGNAL(enabledChanged(bool)), button, SLOT(setEnabled(bool)));
    }

    this->setLayout(d->m_flow_layout);
}

MVOpenViewsControl::~MVOpenViewsControl()
{
    delete d;
}

QString MVOpenViewsControl::title()
{
    return "Open Views";
}

void MVOpenViewsControl::updateContext()
{
}

void MVOpenViewsControl::updateControls()
{
}

void MVOpenViewsControl::slot_open_view(QObject* obj)
{
    MVAbstractViewFactory* factory = qobject_cast<MVAbstractViewFactory*>(obj);
    if (!factory)
        return;
    this->mainWindow()->openView(factory->id());
}
