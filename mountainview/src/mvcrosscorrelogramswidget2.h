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
    QList<int> ks;
};

class MVCrossCorrelogramsWidget2Private;
class MVCrossCorrelogramsWidget2 : public MVAbstractView {
    Q_OBJECT
public:
    friend class MVCrossCorrelogramsWidget2Private;
    MVCrossCorrelogramsWidget2(MVViewAgent* view_agent);
    virtual ~MVCrossCorrelogramsWidget2();

    void prepareCalculation() Q_DECL_OVERRIDE;
    void runCalculation() Q_DECL_OVERRIDE;
    void onCalculationFinished() Q_DECL_OVERRIDE;

    void setOptions(CrossCorrelogramOptions opts);
    int currentLabel1();
    int currentLabel2();
    void setCurrentLabel1(int k);
    void setCurrentLabel2(int k);
    QList<int> selectedLabels1();
    QList<int> selectedLabels2();
    void setSelectedLabels1(const QList<int>& L);
    void setSelectedLabels2(const QList<int>& L);
    QImage renderImage(int W = 0, int H = 0);

    void paintEvent(QPaintEvent* evt);
signals:
    void histogramActivated();
private slots:
    void slot_histogram_view_control_clicked();
    void slot_histogram_view_clicked();
    void slot_histogram_view_activated();
    void slot_export_image();
    void slot_cluster_attributes_changed();
    void slot_update_highlighting();

private:
    MVCrossCorrelogramsWidget2Private* d;
};

class MVAutoCorrelogramsFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    MVAutoCorrelogramsFactory(QObject *parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView *createView(MVViewAgent *agent, QWidget *parent) Q_DECL_OVERRIDE;
private slots:
    void slot_auto_correlogram_activated();
};

class MVMatrixOfCrossCorrelogramsFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    MVMatrixOfCrossCorrelogramsFactory(QObject *parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView *createView(MVViewAgent *agent, QWidget *parent) Q_DECL_OVERRIDE;
private slots:
    void updateEnabled();
};

#endif // MVCROSSCORRELOGRAMSWIDGET2_H
