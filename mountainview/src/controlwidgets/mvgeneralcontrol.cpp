/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/27/2016
*******************************************************/

#include "mvgeneralcontrol.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QTimer>

class MVGeneralControlPrivate {
public:
    MVGeneralControl* q;
};

MVGeneralControl::MVGeneralControl(MVContext* context, MVMainWindow* mw)
    : MVAbstractControl(context, mw)
{
    d = new MVGeneralControlPrivate;
    d->q = this;

    QGridLayout* glayout = new QGridLayout;
    int row = 0;
    {
        QWidget* X = this->createChoicesControl("timeseries");
        connect(context, SIGNAL(timeseriesNamesChanged()), this, SLOT(updateControls()));
        connect(context, SIGNAL(currentTimeseriesChanged()), this, SLOT(updateControls()));
        glayout->addWidget(new QLabel("Timeseries:"), row, 0);
        glayout->addWidget(X, row, 1);
        row++;
    }
    {
        QWidget* X = this->createIntControl("clip_size");
        context->onOptionChanged("clip_size", this, SLOT(updateControls()));
        glayout->addWidget(new QLabel("Clip size:"), row, 0);
        glayout->addWidget(X, row, 1);
        row++;
    }
    {
        QWidget* X = this->createDoubleControl("cc_max_dt_msec");
        context->onOptionChanged("cc_max_dt_msec", this, SLOT(updateControls()));
        glayout->addWidget(new QLabel("Max. dt (ms):"), row, 0);
        glayout->addWidget(X, row, 1);
        row++;
    }
    this->setLayout(glayout);

    updateControls();
}

MVGeneralControl::~MVGeneralControl()
{
    delete d;
}

QString MVGeneralControl::title()
{
    return "General Options";
}

void MVGeneralControl::updateContext()
{
    mvContext()->setCurrentTimeseriesName(this->controlValue("timeseries").toString());
    mvContext()->setOption("clip_size", this->controlValue("clip_size").toInt());
    mvContext()->setOption("cc_max_dt_msec", this->controlValue("cc_max_dt_msec").toDouble());
}

void MVGeneralControl::updateControls()
{
    this->setChoices("timeseries", mvContext()->timeseriesNames());
    this->setControlValue("timeseries", mvContext()->currentTimeseriesName());
    this->setControlValue("clip_size", mvContext()->option("clip_size").toInt());
    this->setControlValue("cc_max_dt_msec", mvContext()->option("cc_max_dt_msec").toDouble());
}
