/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/31/2016
*******************************************************/

#ifndef MVCROSSCORRELOGRAMSWIDGET3_H
#define MVCROSSCORRELOGRAMSWIDGET3_H

#include "mvabstractviewfactory.h"
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

class MVAutoCorrelogramsFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    MVAutoCorrelogramsFactory(QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVViewAgent* agent, QWidget* parent) Q_DECL_OVERRIDE;
private slots:
    //void slot_auto_correlogram_activated();
};

/*
class MVCrossCorrelogramsFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    MVCrossCorrelogramsFactory(QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVViewAgent* agent, QWidget* parent) Q_DECL_OVERRIDE;
private slots:
};
*/

class MVMatrixOfCrossCorrelogramsFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    MVMatrixOfCrossCorrelogramsFactory(QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVViewAgent* agent, QWidget* parent) Q_DECL_OVERRIDE;
private slots:
    void updateEnabled();
};

#endif // MVCROSSCORRELOGRAMSWIDGET3_H
