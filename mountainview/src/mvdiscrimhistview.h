#ifndef MVDISCRIMHISTVIEW_H
#define MVDISCRIMHISTVIEW_H

#include "mvhistogramgrid.h"

class MVDiscrimHistViewPrivate;
class MVDiscrimHistView : public MVHistogramGrid {
    Q_OBJECT
public:
    friend class MVDiscrimHistViewPrivate;
    MVDiscrimHistView(MVViewAgent* view_agent);
    virtual ~MVDiscrimHistView();

    void setClusterNumbers(const QList<int>& cluster_numbers);

    void prepareCalculation() Q_DECL_OVERRIDE;
    void runCalculation() Q_DECL_OVERRIDE;
    void onCalculationFinished() Q_DECL_OVERRIDE;

    void wheelEvent(QWheelEvent* evt);

signals:

private
slots:

private:
    MVDiscrimHistViewPrivate* d;
};

#endif // MVDISCRIMHISTVIEW_H
