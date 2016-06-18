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

class MVAbstractViewFactory;
class MVAbstractViewPrivate;
class CalculationThread;
class MVAbstractView : public QWidget {
    Q_OBJECT
public:
    enum ViewFeature {
        NoFeatures = 0x0,
        RenderView = 0x1    // can render itself on a given painter (e.g. for export)
    };
    Q_DECLARE_FLAGS(ViewFeatures, ViewFeature)

    friend class MVAbstractViewPrivate;
    friend class CalculationThread;
    MVAbstractView(MVViewAgent* view_agent);
    virtual ~MVAbstractView();

    bool isCalculating() const;
    bool recalculateSuggested() const;

public
slots:
    void recalculate();
    void neverSuggestRecalculate();

    virtual MVAbstractViewFactory* viewFactory() const;
    MVViewAgent* viewAgent();

    virtual ViewFeatures viewFeatures() const;
    virtual void renderView(QPainter *painter); // add render opts

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

protected
slots:
    void recalculate();

private
slots:
    void slot_do_calculation();
    void slot_calculation_finished();
    void slot_view_agent_option_changed(QString name);
    void slot_suggest_recalculate();

private:
    MVAbstractViewPrivate* d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(MVAbstractView::ViewFeatures)

#endif // MVABSTRACTVIEW_H
