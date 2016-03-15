#ifndef MVCLIPSVIEW_H
#define MVCLIPSVIEW_H

#include "sstimeseriesview.h"
#include "mvutils.h"

class MVClipsViewPrivate;
class MVClipsView : public SSTimeSeriesView
{
    Q_OBJECT
public:
    friend class MVClipsViewPrivate;
    MVClipsView();
    virtual ~MVClipsView();
    void setClips(const Mda &clips);
    void setTimes(const QList<double> &times);
	void setLabels(const QList<int> &labels);
    int currentClipIndex();
	//double currentClipTimepoint();
	MVEvent currentEvent();
	void setCurrentEvent(MVEvent evt);
signals:
	void currentEventChanged();
public slots:
	void slot_current_x_changed();
private:
    MVClipsViewPrivate *d;
};

#endif // MVCLIPSVIEW_H
