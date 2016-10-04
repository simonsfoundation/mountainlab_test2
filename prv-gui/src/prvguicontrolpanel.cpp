/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 10/1/2016
*******************************************************/

#include "prvguicontrolpanel.h"
#include "qAccordion/qaccordion.h"

class PrvGuiControlPanelPrivate {
public:
    PrvGuiControlPanel* q;
    QAccordion* m_accordion;
};

PrvGuiControlPanel::PrvGuiControlPanel()
{
    d = new PrvGuiControlPanelPrivate;
    d->q = this;

    d->m_accordion = new QAccordion;
    d->m_accordion->setMultiActive(true);

    QScrollArea* SA = new QScrollArea;
    SA->setWidget(d->m_accordion);
    SA->setWidgetResizable(true);

    QVBoxLayout* vlayout = new QVBoxLayout;
    vlayout->addWidget(SA);
    this->setLayout(vlayout);
}

PrvGuiControlPanel::~PrvGuiControlPanel()
{
    delete d;
}

void PrvGuiControlPanel::addControlWidget(QString label, QWidget* W)
{
    QFrame* frame = new QFrame;
    QHBoxLayout* frame_layout = new QHBoxLayout;
    frame_layout->addWidget(W);
    frame->setLayout(frame_layout);
    ContentPane* CP = new ContentPane(label, frame);
    CP->setMaximumHeight(1000);
    d->m_accordion->addContentPane(CP);
    bool start_open = true;
    if (start_open) {
        CP->openContentPane();
    }
}
