#ifndef MVCLIPSVIEW_H
#define MVCLIPSVIEW_H

#include "sstimeseriesview.h"
#include "mvutils.h"

/** \class MVClipsView
 *  \brief View a set of clips. Usually each clip contains a single spike.
 *
 *  What is a clip? you may ask. TODO: explain this somewhere in the docs
 *
 * TODO: don't piggy back on SSTimeSeriesView for this widget -- make a new special widget for this purpose
 * TODO: do we want to handle huge arrays? (e.g., like DiskArrayModel?)
 */

class MVClipsViewPrivate;
class MVClipsView : public SSTimeSeriesView
{
    Q_OBJECT
public:
    friend class MVClipsViewPrivate;
    MVClipsView();
    virtual ~MVClipsView();
	///Set MxTxL array of clips to view
    void setClips(const Mda &clips);
	///Set the associated times, for purpose of currentEvent()
    void setTimes(const QList<double> &times);
	///Set the associated labels, for purpose of currentEvent()
	void setLabels(const QList<int> &labels);
	///The index of the currently selected clip
    int currentClipIndex();
	//double currentClipTimepoint();
	///The currently selected event -- this has the time/label
	MVEvent currentEvent();
	///Set the current event, which searches times/labels to move the cursor to the corresponding clip
	void setCurrentEvent(MVEvent evt);
signals:
	///The user has selected a new current clip, which affects the outputs of currentClipIndex() and currentEvent()
	void currentEventChanged();
public slots:
	void slot_current_x_changed();
private:
    MVClipsViewPrivate *d;
};

#endif // MVCLIPSVIEW_H
