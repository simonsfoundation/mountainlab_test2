#ifndef MVDISCRIMHISTVIEW_H
#define MVDISCRIMHISTVIEW_H

#include "mvabstractviewfactory.h"
#include "mvhistogramgrid.h"

class MVDiscrimHistViewPrivate;
class MVDiscrimHistView : public MVHistogramGrid {
    Q_OBJECT
public:
    friend class MVDiscrimHistViewPrivate;
    MVDiscrimHistView(MVContext* view_agent);
    virtual ~MVDiscrimHistView();

    void setClusterNumbers(const QList<int>& cluster_numbers);

    void prepareCalculation() Q_DECL_OVERRIDE;
    void runCalculation() Q_DECL_OVERRIDE;
    void onCalculationFinished() Q_DECL_OVERRIDE;

    void wheelEvent(QWheelEvent* evt);

signals:

private slots:

private:
    MVDiscrimHistViewPrivate* d;
};

class MVDiscrimHistFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    MVDiscrimHistFactory(MVContext* context, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(QWidget* parent) Q_DECL_OVERRIDE;
private slots:
    void updateEnabled();
};

#endif // MVDISCRIMHISTVIEW_H
