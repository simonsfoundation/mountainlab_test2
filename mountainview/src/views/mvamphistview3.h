#ifndef MVAMPHISTVIEW3_H
#define MVAMPHISTVIEW3_H

#include "mvabstractview.h"
#include "mvabstractviewfactory.h"

class MVAmpHistView3Private;
class MVAmpHistView3 : public MVAbstractView {
    Q_OBJECT
public:
    friend class MVAmpHistView3Private;
    MVAmpHistView3(MVContext* context);
    virtual ~MVAmpHistView3();

    void prepareCalculation() Q_DECL_OVERRIDE;
    void runCalculation() Q_DECL_OVERRIDE;
    void onCalculationFinished() Q_DECL_OVERRIDE;

protected:
    void wheelEvent(QWheelEvent* evt);
    void prepareMimeData(QMimeData& mimeData, const QPoint& pos);
    void keyPressEvent(QKeyEvent* evt);

private slots:
    void slot_zoom_in_horizontal(double factor = 1.2);
    void slot_zoom_out_horizontal(double factor = 1.2);
    void slot_pan_left(double units = 0.1);
    void slot_pan_right(double units = 0.1);
    void slot_panel_clicked(int index, Qt::KeyboardModifiers modifiers);
    void slot_update_highlighting_and_captions();
    void slot_current_cluster_changed();
    void slot_update_bins();

private:
    MVAmpHistView3Private* d;
};

class MVAmplitudeHistograms3Factory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    MVAmplitudeHistograms3Factory(MVContext* context, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(QWidget* parent) Q_DECL_OVERRIDE;
private slots:
};

#endif // MVAMPHISTVIEW3_H
