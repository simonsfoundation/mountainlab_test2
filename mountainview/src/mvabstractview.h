/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/15/2016
*******************************************************/

#ifndef MVABSTRACTVIEW_H
#define MVABSTRACTVIEW_H

#include "mvviewagent.h"
#include "mlutils.h"
#include <QWidget>

class MVAbstractViewPrivate;
class CalculationThread;
class MVAbstractView : public QWidget {
    Q_OBJECT
public:
    friend class MVAbstractViewPrivate;
    friend class CalculationThread;
    MVAbstractView(MVViewAgent* view_agent);
    virtual ~MVAbstractView();

    bool isCalculating() const;
    bool recalculateSuggested() const;
    void stopCalculation();

public
slots:
    void recalculate();
    void neverSuggestRecalculate();

signals:
    void calculationStarted();
    void calculationFinished();
    void recalculateSuggestedChanged();

protected:
    virtual void prepareCalculation() = 0;
    virtual void runCalculation() = 0;
    virtual void onCalculationFinished() = 0;

    MVViewAgent* viewAgent();
    void recalculateOnOptionChanged(QString name, bool suggest_only = true);
    void recalculateOn(QObject*, const char* signal, bool suggest_only = true);

private
slots:
    void slot_do_calculation();
    void slot_calculation_finished();
    void slot_view_agent_option_changed(QString name);
    void slot_suggest_recalculate();

private:
    MVAbstractViewPrivate* d;
};

#endif // MVABSTRACTVIEW_H
