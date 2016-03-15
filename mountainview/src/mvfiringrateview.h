#ifndef MVFIRINGRATEVIEW_H
#define MVFIRINGRATEVIEW_H

#include <QWidget>
#include "diskreadmda.h"
#include "mvutils.h"

class MVFiringRateViewPrivate;
class MVFiringRateView : public QWidget
{
    Q_OBJECT
public:
    friend class MVFiringRateViewPrivate;
    MVFiringRateView();
    virtual ~MVFiringRateView();
    void setFirings(const Mda &firings);
    void setSamplingFreq(double ff); //in Hz
    void setCurrentEvent(MVEvent evt);
    void setEpochs(const QList<Epoch> &epochs);
signals:

protected:
    void paintEvent(QPaintEvent *evt);
	void mouseReleaseEvent(QMouseEvent *evt);

private slots:
    void slot_update();

private:
    MVFiringRateViewPrivate *d;
};

#endif // MVFIRINGRATEVIEW_H
