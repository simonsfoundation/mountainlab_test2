/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/31/2016
*******************************************************/

#ifndef MVCROSSCORRELOGRAMSWIDGET2_H
#define MVCROSSCORRELOGRAMSWIDGET2_H

#include "diskreadmda.h"
#include "mvabstractview.h"
#include "mvabstractviewfactory.h"

#include <QWidget>

enum CrossCorrelogramMode {
    Undefined,
    All_Auto_Correlograms,
    Cross_Correlograms,
    Matrix_Of_Cross_Correlograms
};

struct CrossCorrelogramOptions {
    CrossCorrelogramOptions()
    {
        mode = Undefined;
    }

    CrossCorrelogramMode mode;
    QVector<int> ks;
};

class MVCrossCorrelogramsWidget2Private;
class MVCrossCorrelogramsWidget2 : public MVAbstractView {
    Q_OBJECT
public:
    friend class MVCrossCorrelogramsWidget2Private;
    MVCrossCorrelogramsWidget2(MVContext* context);
    virtual ~MVCrossCorrelogramsWidget2();

    void prepareCalculation() Q_DECL_OVERRIDE;
    void runCalculation() Q_DECL_OVERRIDE;
    void onCalculationFinished() Q_DECL_OVERRIDE;

    void setOptions(CrossCorrelogramOptions opts);
    int currentLabel1();
    int currentLabel2();
    void setCurrentLabel1(int k);
    void setCurrentLabel2(int k);
    QVector<int> selectedLabels1();
    QVector<int> selectedLabels2();
    void setSelectedLabels1(const QVector<int>& L);
    void setSelectedLabels2(const QVector<int>& L);
    QImage renderImage(int W = 0, int H = 0);

    void paintEvent(QPaintEvent* evt);
    void keyPressEvent(QKeyEvent* evt);
signals:
    void histogramActivated();
private slots:
    void slot_histogram_view_clicked(Qt::KeyboardModifiers modifiers);
    void slot_histogram_view_activated();
    void slot_export_image();
    void slot_cluster_attributes_changed(int cluster_number);
    void slot_update_highlighting();

private:
    MVCrossCorrelogramsWidget2Private* d;
};

#endif // MVCROSSCORRELOGRAMSWIDGET2_H
