/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef MVCLUSTERWIDGET_H
#define MVCLUSTERWIDGET_H

#include <QWidget>
#include "mvutils.h"
#include "affinetransformation.h"
#include "mvclusterview.h" //for FilterInfo
#include "mvviewagent.h"

/** \class MVClusterWidget
 *  \brief Presents one or more cluster views and a synchronized clip view
 *
 */

class MVClusterWidgetPrivate;
class MVClusterWidget : public QWidget {
    Q_OBJECT
public:
    friend class MVClusterWidgetPrivate;
    MVClusterWidget(MVViewAgent* view_agent);
    virtual ~MVClusterWidget();

    void setMLProxyUrl(const QString& url);

    void setClipSize(int clip_size);
    ///Set the time series for purpose of computing the clip to display when corresponding datapoint is selected
    void setTimeseries(const DiskReadMda& X);
    ///Just as in MVClusterView::setTransformation()
    void setFirings(const DiskReadMda& F);
    void setLabelsToUse(const QList<int>& labels);

    ///Just as in MVClusterView::setCurrentEvent()
    void setCurrentEvent(const MVEvent& evt);
    ///For purpose of displaying the clip associated with the current datapoint computed from array of setTimeSeries()
    void setTransformation(const AffineTransformation& T);
    ///Just as in MVClusterView::currentEvent()
    MVEvent currentEvent();

    void setEventFilter(FilterInfo info);

private:
    ///Just as in MVClusterView::setData()
    void setData(const Mda& X);
    ///Just as in MVClusterView::setTimes()
    void setTimes(const QList<double>& times);
    ///Just as in MVClusterView::setLabels()
    void setLabels(const QList<int>& labels);
    ///Just as in MVClusterView::setAmplitudes()
    void setAmplitudes(const QList<double>& amps);
    void setScores(const QList<double>& detectability_scores, const QList<double>& outlier_scores);

signals:
    ///Just as in MVClusterView::currentEventChanged()
    void currentEventChanged();
private slots:
    void slot_view_current_event_changed();
    void slot_view_transformation_changed();
    void slot_show_clip_view_toggled(bool val);
    void slot_show_view_toggled(bool val);
    void slot_computation_finished();

private:
    MVClusterWidgetPrivate* d;
};

#endif // MVCLUSTERWIDGET_H
