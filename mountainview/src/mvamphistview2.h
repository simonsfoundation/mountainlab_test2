#ifndef MVAMPHISTVIEW2_H
#define MVAMPHISTVIEW2_H

#include "mvhistogramgrid.h"

class MVAmpHistView2Private;
class MVAmpHistView2 : public MVHistogramGrid {
    Q_OBJECT
public:
    friend class MVAmpHistView2Private;
    MVAmpHistView2(MVViewAgent* view_agent);
    virtual ~MVAmpHistView2();

    void prepareCalculation() Q_DECL_OVERRIDE;
    void runCalculation() Q_DECL_OVERRIDE;
    void onCalculationFinished() Q_DECL_OVERRIDE;

    void wheelEvent(QWheelEvent* evt);

signals:

private
slots:

private:
    MVAmpHistView2Private* d;
};

#endif // MVAMPHISTVIEW2_H
