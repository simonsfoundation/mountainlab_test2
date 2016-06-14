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
#include "mvviewagent.h"

class MVSpikeSprayViewPrivate;
class MVSpikeSprayView : public QWidget {
    Q_OBJECT
public:
    friend class MVSpikeSprayViewPrivate;
    MVSpikeSprayView(MVViewAgent* view_agent);
    virtual ~MVSpikeSprayView();

    void setMLProxyUrl(const QString& url);

    void setTimeseries(const DiskReadMda& X);
    void setFirings(const DiskReadMda& F);
    void setLabelsToUse(const QList<int>& labels);
    void setClipSize(int clip_size);
private slots:
    void slot_computation_finished();

protected:
    void paintEvent(QPaintEvent* evt);
    void keyPressEvent(QKeyEvent* evt);

private:
    MVSpikeSprayViewPrivate* d;
};

#endif // MVSPIKESPRAYVIEW_H
