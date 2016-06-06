/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/6/2016
*******************************************************/

#ifndef SPIKESPYWIDGET_H
#define SPIKESPYWIDGET_H

#include "diskreadmda.h"

#include <QWidget>

struct SpikeSpyViewData {
    DiskReadMda timeseries;
    DiskReadMda firings;
};

class SpikeSpyWidgetPrivate;
class SpikeSpyWidget : public QWidget {
    Q_OBJECT
public:
    friend class SpikeSpyWidgetPrivate;
    SpikeSpyWidget();
    virtual ~SpikeSpyWidget();
    void setSampleRate(double samplerate);
    void addView(const SpikeSpyViewData& data);

private:
    SpikeSpyWidgetPrivate* d;
};

#endif // SPIKESPYWIDGET_H
