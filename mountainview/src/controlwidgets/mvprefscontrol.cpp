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
        QWidget* X = this->createChoicesControl("timeseries_for_spikespray");
        connect(context, SIGNAL(timeseriesNamesChanged()), this, SLOT(updateControls()));
        connect(context, SIGNAL(currentTimeseriesChanged()), this, SLOT(updateControls()));
        glayout->addWidget(new QLabel("For spikespray:"), row, 0);
        glayout->addWidget(X, row, 1);
        row++;
    }
    {
        QWidget* X = this->createIntControl("clip_size");
        X->setToolTip("The clip size used for all the views");
        context->onOptionChanged("clip_size", this, SLOT(updateControls()));
        glayout->addWidget(new QLabel("Clip size:"), row, 0);
        glayout->addWidget(X, row, 1);
        row++;
    }
    {
        QWidget* X = this->createDoubleControl("cc_max_dt_msec");
        X->setToolTip("Maximum dt in milliseconds for cross-correlograms.");
        context->onOptionChanged("cc_max_dt_msec", this, SLOT(updateControls()));
        glayout->addWidget(new QLabel("CC max. dt (msec):"), row, 0);
        glayout->addWidget(X, row, 1);
        row++;
    }
    {
        QWidget* X = this->createDoubleControl("cc_log_time_constant_msec");
        context->onOptionChanged("cc_log_time_constant_msec", this, SLOT(updateControls()));
        glayout->addWidget(new QLabel("CC log time constant (msec):"), row, 0);
        glayout->addWidget(X, row, 1);
        row++;
    }
    {
        QWidget* X = this->createDoubleControl("cc_bin_size_msec");
        context->onOptionChanged("cc_bin_size_msec", this, SLOT(updateControls()));
        glayout->addWidget(new QLabel("CC bin size (msec):"), row, 0);
        glayout->addWidget(X, row, 1);
        row++;
    }
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
    QString ts4ss = this->controlValue("timeseries_for_spikespray").toString();
    if (ts4ss == "Default timeseries")
        ts4ss = "";
    mvContext()->setOption("timeseries_for_spikespray", ts4ss);
    mvContext()->setOption("clip_size", this->controlValue("clip_size").toInt());
    mvContext()->setOption("cc_max_dt_msec", this->controlValue("cc_max_dt_msec").toDouble());
    mvContext()->setOption("cc_log_time_constant_msec", this->controlValue("cc_log_time_constant_msec").toDouble());
    mvContext()->setOption("cc_bin_size_msec", this->controlValue("cc_bin_size_msec").toDouble());
    mvContext()->setOption("amp_thresh_display", this->controlValue("amp_thresh_display").toDouble());
}

void MVPrefsControl::updateControls()
{
    QStringList choices = mvContext()->timeseriesNames();
    choices.insert(0, "Default timeseries");
    this->setChoices("timeseries_for_spikespray", choices);
    this->setControlValue("clip_size", mvContext()->option("clip_size").toInt());
    this->setControlValue("cc_max_dt_msec", mvContext()->option("cc_max_dt_msec").toDouble());
    this->setControlValue("cc_log_time_constant_msec", mvContext()->option("cc_log_time_constant_msec").toDouble());
    this->setControlValue("cc_bin_size_msec", mvContext()->option("cc_bin_size_msec").toDouble());
    this->setControlValue("amp_thresh_display", mvContext()->option("amp_thresh_display").toDouble());
}
