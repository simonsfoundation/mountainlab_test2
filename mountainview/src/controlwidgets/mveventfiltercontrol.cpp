/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/27/2016
*******************************************************/

#include "mveventfiltercontrol.h"
#include "mvcontext.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QTimer>

class MVEventFilterControlPrivate {
public:
    MVEventFilterControl* q;

    void update_enabled();
};

MVEventFilterControl::MVEventFilterControl(MVContext* context, MVMainWindow* mw)
    : MVAbstractControl(context, mw)
{
    d = new MVEventFilterControlPrivate;
    d->q = this;

    QGridLayout* glayout = new QGridLayout;
    int row = 0;
    {
        QCheckBox* X = this->createCheckBoxControl("use_event_filter");
        X->setText("Use event filter");
        context->onOptionChanged("use_event_filter", this, SLOT(updateControls()));
        glayout->addWidget(X, row, 0, 1, 2);
        row++;
    }
    {
        QWidget* X = this->createDoubleControl("min_detectability_score");
        context->onOptionChanged("min_detectability_score", this, SLOT(updateControls()));
        glayout->addWidget(new QLabel("Min. detectability score:"), row, 0);
        glayout->addWidget(X, row, 1);
        row++;
    }
    {
        QWidget* X = this->createDoubleControl("max_outlier_score");
        context->onOptionChanged("max_outlier_score", this, SLOT(updateControls()));
        glayout->addWidget(new QLabel("Max. outlier score:"), row, 0);
        glayout->addWidget(X, row, 1);
        row++;
    }
    this->setLayout(glayout);

    updateControls();
}

MVEventFilterControl::~MVEventFilterControl()
{
    delete d;
}

QString MVEventFilterControl::title()
{
    return "Event Filter";
}

void MVEventFilterControl::updateContext()
{
    MVEventFilter EF;
    EF.use_event_filter = this->controlValue("use_event_filter").toBool();
    EF.min_detectability_score = this->controlValue("min_detectability_score").toDouble();
    EF.max_outlier_score = this->controlValue("max_outlier_score").toDouble();
    mvContext()->setEventFilter(EF);
    d->update_enabled();
}

void MVEventFilterControl::updateControls()
{
    MVEventFilter EF = mvContext()->eventFilter();
    this->setControlValue("use_event_filter", EF.use_event_filter);
    this->setControlValue("min_detectability_score", EF.min_detectability_score);
    this->setControlValue("max_outlier_score", EF.max_outlier_score);
    d->update_enabled();
}

void MVEventFilterControlPrivate::update_enabled()
{
    MVEventFilter EF = q->mvContext()->eventFilter();
    q->setControlEnabled("min_detectability_score", EF.use_event_filter);
    q->setControlEnabled("max_outlier_score", EF.use_event_filter);
}
