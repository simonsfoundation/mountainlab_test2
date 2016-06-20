#ifndef MVAMPHISTVIEW_H
#define MVAMPHISTVIEW_H

#include "mvabstractview.h"

class MVAmpHistViewPrivate;
class MVAmpHistView : public MVAbstractView {
    Q_OBJECT
public:
    friend class MVAmpHistViewPrivate;
    MVAmpHistView(MVViewAgent* view_agent);
    virtual ~MVAmpHistView();

    void prepareCalculation() Q_DECL_OVERRIDE;
    void runCalculation() Q_DECL_OVERRIDE;
    void onCalculationFinished() Q_DECL_OVERRIDE;

    QImage renderImage(int W = 0, int H = 0);
    void wheelEvent(QWheelEvent* evt);

signals:
    void histogramActivated();

private slots:
    void slot_histogram_view_control_clicked();
    void slot_histogram_view_clicked();
    void slot_histogram_view_activated();
    void slot_export_image();
    void slot_update_highlighting();

private:
    MVAmpHistViewPrivate* d;
};

#endif // MVAMPHISTVIEW_H
