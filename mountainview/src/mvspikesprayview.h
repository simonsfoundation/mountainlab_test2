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
#include "mvabstractview.h"

class MVSpikeSprayViewPrivate;
class MVSpikeSprayView : public MVAbstractView {
    Q_OBJECT
public:
    friend class MVSpikeSprayViewPrivate;
    MVSpikeSprayView(MVViewAgent* view_agent);
    virtual ~MVSpikeSprayView();
    void setLabelsToUse(const QList<int>& labels);

    void prepareCalculation() Q_DECL_OVERRIDE;
    void runCalculation() Q_DECL_OVERRIDE;
    void onCalculationFinished() Q_DECL_OVERRIDE;

protected:
    void paintEvent(QPaintEvent* evt);
    void keyPressEvent(QKeyEvent* evt);

private:
    MVSpikeSprayViewPrivate* d;
};

#endif // MVSPIKESPRAYVIEW_H
