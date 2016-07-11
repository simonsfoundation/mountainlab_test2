/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/8/2016
*******************************************************/

#ifndef MVSPIKESPRAYVIEW_H
#define MVSPIKESPRAYVIEW_H

#include "diskreadmda.h"
#include "mvabstractviewfactory.h"

#include <QPaintEvent>
#include <QWidget>
#include "mvabstractview.h"

class MVSpikeSprayViewPrivate;
class MVSpikeSprayView : public MVAbstractView {
    Q_OBJECT
public:
    friend class MVSpikeSprayViewPrivate;
    MVSpikeSprayView(MVContext* context);
    virtual ~MVSpikeSprayView();
    void setLabelsToUse(const QSet<int>& labels);

    void prepareCalculation() Q_DECL_OVERRIDE;
    void runCalculation() Q_DECL_OVERRIDE;
    void onCalculationFinished() Q_DECL_OVERRIDE;

protected:
    void paintEvent(QPaintEvent* evt);
    void keyPressEvent(QKeyEvent* evt);
    void wheelEvent(QWheelEvent* evt);

private slots:
    void slot_zoom_in();
    void slot_zoom_out();
    void slot_vertical_zoom_in();
    void slot_vertical_zoom_out();

private:
    MVSpikeSprayViewPrivate* d;
};

class MVSpikeSprayFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    MVSpikeSprayFactory(MVContext* context, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(QWidget* parent) Q_DECL_OVERRIDE;
private
slots:
    void updateEnabled();
};

#endif // MVSPIKESPRAYVIEW_H
