/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/31/2016
*******************************************************/

#ifndef MVCROSSCORRELOGRAMSWIDGET3_H
#define MVCROSSCORRELOGRAMSWIDGET3_H

#include "mvhistogramgrid.h"

#include <QWidget>

enum CrossCorrelogramMode3 {
    Undefined3,
    All_Auto_Correlograms3,
    Cross_Correlograms3,
    Matrix_Of_Cross_Correlograms3
};

struct CrossCorrelogramOptions3 {
    CrossCorrelogramMode3 mode = Undefined3;
    QList<int> ks;
};

class MVCrossCorrelogramsWidget3Private;
class MVCrossCorrelogramsWidget3 : public MVHistogramGrid {
    Q_OBJECT
public:
    friend class MVCrossCorrelogramsWidget3Private;
    MVCrossCorrelogramsWidget3(MVViewAgent* view_agent);
    virtual ~MVCrossCorrelogramsWidget3();

    void prepareCalculation() Q_DECL_OVERRIDE;
    void runCalculation() Q_DECL_OVERRIDE;
    void onCalculationFinished() Q_DECL_OVERRIDE;

    void setOptions(CrossCorrelogramOptions3 opts);
signals:
private slots:

private:
    MVCrossCorrelogramsWidget3Private* d;
};

#endif // MVCROSSCORRELOGRAMSWIDGET3_H
