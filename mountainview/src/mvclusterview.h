/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef MVCLUSTERVIEW_H
#define MVCLUSTERVIEW_H

#include <QWidget>
#include "mda.h"
#include "mvutils.h"
#include "affinetransformation.h"
#include "mvviewagent.h"

#define MVCV_MODE_HEAT_DENSITY 1
#define MVCV_MODE_LABEL_COLORS 2
#define MVCV_MODE_TIME_COLORS 3
#define MVCV_MODE_AMPLITUDE_COLORS 4

/** \class MVClusterView
 *  \brief A rotatable view of datapoints in 3D space
 *
 *  Several modes are available
 */

/// TODO, maybe put this where it belongs
struct FilterInfo {
    FilterInfo()
    {
        use_filter = false;
        min_detectability_score = 0;
        max_outlier_score = 0;
    }

    bool use_filter;
    double min_detectability_score;
    double max_outlier_score;
};

class MVClusterViewPrivate;
class MVClusterView : public QWidget {
    Q_OBJECT
public:
    friend class MVClusterViewPrivate;
    MVClusterView(MVViewAgent* view_agent, QWidget* parent = 0);
    virtual ~MVClusterView();
    void setData(const Mda& X);
    bool hasData();
    void setTimes(const QList<double>& times);
    void setLabels(const QList<int>& labels);
    void setAmplitudes(const QList<double>& amps);

    void setScores(const QList<double>& detectability_scores, const QList<double>& outlier_scores);

    void setMode(int mode);
    void setCurrentEvent(MVEvent evt, bool do_emit = false);
    MVEvent currentEvent();
    int currentEventIndex();
    AffineTransformation transformation();
    void setTransformation(const AffineTransformation& T);

    void setEventFilter(FilterInfo F);

    QImage renderImage(int W, int H);
signals:
    void currentEventChanged();
    void transformationChanged();

private
slots:
    void slot_context_menu(const QPoint& pos);

protected:
    void paintEvent(QPaintEvent* evt);
    void mouseMoveEvent(QMouseEvent* evt);
    void mousePressEvent(QMouseEvent* evt);
    void mouseReleaseEvent(QMouseEvent* evt);
    void wheelEvent(QWheelEvent* evt);

private:
    MVClusterViewPrivate* d;
};

#endif // MVCLUSTERVIEW_H
