#include "mvprefscontrol.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QTimer>

class MVPrefsControlPrivate {
public:
    MVPrefsControl* q;
};

MVPrefsControl::MVPrefsControl(MVContext* context, MVMainWindow* mw)
    : MVAbstractControl(context, mw)
{
    d = new MVPrefsControlPrivate;
    d->q = this;

    QGridLayout* glayout = new QGridLayout;
    int row = 0;
    {
        QWidget* X = this->createDoubleControl("amp_thresh_display");
        context->onOptionChanged("amp_thresh_display", this, SLOT(updateControls()));
        glayout->addWidget(new QLabel("Amp. thresh. (display):"), row, 0);
        glayout->addWidget(X, row, 1);
        row++;
    }
    this->setLayout(glayout);

    updateControls();
}

MVPrefsControl::~MVPrefsControl()
{
    delete d;
}

QString MVPrefsControl::title() const
{
    return "Preferences";
}

void MVPrefsControl::updateContext()
{
    mvContext()->setCurrentTimeseriesName(this->controlValue("timeseries").toString());
    mvContext()->setOption("amp_thresh_display", this->controlValue("amp_thresh_display").toDouble());
}

void MVPrefsControl::updateControls()
{
    this->setChoices("timeseries", mvContext()->timeseriesNames());
    this->setControlValue("amp_thresh_display", mvContext()->option("amp_thresh_display").toDouble());
}
