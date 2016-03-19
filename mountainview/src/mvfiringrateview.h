#ifndef MVFIRINGRATEVIEW_H
#define MVFIRINGRATEVIEW_H

#include <QWidget>
#include "diskreadmda.h"
#include "mvutils.h"

/** \class MVFiringRateView
 *  \brief Shows the firing rates (in color) of a selected neuron (or cluster) as a function of time. And shows distribution of amplitudes as well.
 */

class MVFiringRateViewPrivate;
class MVFiringRateView : public QWidget
{
    Q_OBJECT
public:
    friend class MVFiringRateViewPrivate;
    MVFiringRateView();
    virtual ~MVFiringRateView();
	///Set the RxL array of firing event info
    void setFirings(const Mda &firings);
	///The sampling rate, (not sure if used right now)
    void setSampleRate(double ff); //in Hz
	///The currently selected event by user click (does it work?)
    void setCurrentEvent(MVEvent evt);
	///Controls display. Not explained in detail right now.
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
