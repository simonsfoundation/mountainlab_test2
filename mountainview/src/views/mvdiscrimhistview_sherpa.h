#ifndef MVDISCRIMHISTVIEW_SHERPA_H
#define MVDISCRIMHISTVIEW_SHERPA_H

#include "mvabstractviewfactory.h"
#include "mvhistogramgrid.h"

class MVDiscrimHistViewSherpaPrivate;
class MVDiscrimHistViewSherpa : public MVHistogramGrid {
    Q_OBJECT
public:
    friend class MVDiscrimHistViewSherpaPrivate;
    MVDiscrimHistViewSherpa(MVContext* context);
    virtual ~MVDiscrimHistViewSherpa();

    void setNumHistograms(int num);

    void prepareCalculation() Q_DECL_OVERRIDE;
    void runCalculation() Q_DECL_OVERRIDE;
    void onCalculationFinished() Q_DECL_OVERRIDE;

    void wheelEvent(QWheelEvent* evt);

signals:

private slots:

private:
    MVDiscrimHistViewSherpaPrivate* d;
};

class MVDiscrimHistSherpaFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    MVDiscrimHistSherpaFactory(MVContext* context, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(QWidget* parent) Q_DECL_OVERRIDE;
private slots:
    void updateEnabled();
};

#endif // MVDISCRIMHISTVIEW_SHERPA_H
