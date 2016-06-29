/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/27/2016
*******************************************************/

#include "mvcontrolpanel2.h"
#include "qAccordion/qaccordion.h"

#include <QPushButton>
//#include
#include "mvmainwindow.h"

class MVControlPanel2Private {
public:
    MVControlPanel2* q;
    QAccordion* m_accordion;
    QList<MVAbstractControl*> m_controls;
    MVContext* m_context;
    MVMainWindow* m_main_window;
};

MVControlPanel2::MVControlPanel2(MVContext* context, MVMainWindow* mw)
{
    d = new MVControlPanel2Private;
    d->q = this;
    d->m_context = context;
    d->m_main_window = mw;

    QHBoxLayout* top_layout = new QHBoxLayout;
    top_layout->setSpacing(20);
    top_layout->setMargin(20);
    {
        QPushButton* B = new QPushButton("Recalc Visible");
        top_layout->addWidget(B);
        QObject::connect(B, SIGNAL(clicked(bool)), this, SLOT(slot_recalc_visible()));
    }
    {
        QPushButton* B = new QPushButton("Recalc All");
        top_layout->addWidget(B);
        QObject::connect(B, SIGNAL(clicked(bool)), this, SLOT(slot_recalc_all()));
    }

    d->m_accordion = new QAccordion;
    d->m_accordion->setMultiActive(true);

    QScrollArea* SA = new QScrollArea;
    SA->setWidget(d->m_accordion);
    SA->setWidgetResizable(true);

    QVBoxLayout* vlayout = new QVBoxLayout;
    vlayout->setSpacing(0);
    vlayout->setMargin(0);
    vlayout->addLayout(top_layout);
    vlayout->addWidget(SA);
    this->setLayout(vlayout);
}

MVControlPanel2::~MVControlPanel2()
{
    delete d;
}

void MVControlPanel2::addControl(MVAbstractControl* mvcontrol, bool start_open)
{
    d->m_controls << mvcontrol;

    QFrame* frame = new QFrame;
    QHBoxLayout* frame_layout = new QHBoxLayout;
    frame_layout->addWidget(mvcontrol);
    frame->setLayout(frame_layout);
    ContentPane* CP = new ContentPane(mvcontrol->title(), frame);
    d->m_accordion->addContentPane(CP);
    if (start_open) {
        CP->openContentPane();
    }
}

void MVControlPanel2::slot_recalc_visible()
{
    d->m_main_window->recalculateViews(SuggestedVisible);
}

void MVControlPanel2::slot_recalc_all()
{
    d->m_main_window->recalculateViews(Suggested);
}
