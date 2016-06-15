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
    void setLabelsToUse(const QList<int>& labels);
private slots:
    void slot_computation_finished();
    void slot_view_agent_option_changed(QString name);
    void slot_restart_calculation();

protected:
    void paintEvent(QPaintEvent* evt);
    void keyPressEvent(QKeyEvent* evt);

private:
    MVSpikeSprayViewPrivate* d;
};

#endif // MVSPIKESPRAYVIEW_H
