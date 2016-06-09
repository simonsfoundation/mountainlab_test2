/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/8/2016
*******************************************************/

#ifndef MVSPIKESPRAYVIEW_H
#define MVSPIKESPRAYVIEW_H

#include "diskreadmda.h"

#include <QPaintEvent>
#include <QWidget>

class MVSpikeSprayViewPrivate;
class MVSpikeSprayView : public QWidget {
    Q_OBJECT
public:
    friend class MVSpikeSprayViewPrivate;
    MVSpikeSprayView();
    virtual ~MVSpikeSprayView();

    void setMLProxyUrl(const QString& url);

    void setTimeseries(DiskReadMda& X);
    void setFirings(DiskReadMda& F);
    void setLabelsToUse(const QList<int>& labels);
    void setClipSize(int clip_size);
    void setLabelColors(const QList<QColor>& colors);
private slots:
    void slot_computation_finished();

protected:
    void paintEvent(QPaintEvent* evt);

private:
    MVSpikeSprayViewPrivate* d;
};

#endif // MVSPIKESPRAYVIEW_H
