/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/27/2016
*******************************************************/

#ifndef MVFIRINGEVENTVIEW2_H
#define MVFIRINGEVENTVIEW2_H

#include <QWidget>
#include <diskreadmda.h>
#include "mvabstractviewfactory.h"
#include "mvtimeseriesviewbase.h"

class MVFiringEventView2Private;
class MVFiringEventView2 : public MVTimeSeriesViewBase {
    Q_OBJECT
public:
    friend class MVFiringEventView2Private;
    MVFiringEventView2(MVViewAgent* view_agent);
    virtual ~MVFiringEventView2();

    void prepareCalculation() Q_DECL_OVERRIDE;
    void runCalculation() Q_DECL_OVERRIDE;
    void onCalculationFinished() Q_DECL_OVERRIDE;

    void setLabelsToUse(const QSet<int>& labels_to_use);

    void paintContent(QPainter* painter);

    void setAmplitudeRange(MVRange range);
    void autoSetAmplitudeRange();

private slots:

private:
    MVFiringEventView2Private* d;
};

class MVFiringEventsFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    MVFiringEventsFactory(MVViewAgent* context, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(QWidget* parent) Q_DECL_OVERRIDE;
private slots:
    void updateEnabled();
};

#endif // MVFiringEventView2_H
