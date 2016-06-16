/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef MVCLIPSVIEW_H
#define MVCLIPSVIEW_H

#include "sstimeseriesview.h"
#include "mvutils.h"
#include "mvviewagent.h"

/** \class MVClipsView
 *  \brief View a set of clips. Usually each clip contains a single spike.
 */

class MVClipsViewPrivate;
class MVClipsView : public SSTimeSeriesView {
    Q_OBJECT
public:
    friend class MVClipsViewPrivate;
    MVClipsView(MVViewAgent* view_agent);
    virtual ~MVClipsView();

    ///Set MxTxL array of clips to view
    void setClips(const Mda& clips);

    ///Set the associated times, for purpose of currentEvent()
    void setTimes(const QList<double>& times);

    ///Set the associated labels, for purpose of currentEvent()
    void setLabels(const QList<int>& labels);

    ///The index of the currently selected clip
    int currentClipIndex();
signals:
    ///The user has selected a new current clip, which affects the outputs of currentClipIndex() and currentEvent()
    void currentEventChanged();
private
slots:
    void slot_current_x_changed();

private:
    MVClipsViewPrivate* d;
};

#endif // MVCLIPSVIEW_H
