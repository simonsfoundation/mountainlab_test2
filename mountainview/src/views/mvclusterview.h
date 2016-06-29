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
#include "mvcontext.h"

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
    MVClusterView(MVContext* context, QWidget* parent = 0);
    virtual ~MVClusterView();
    void setData(const Mda& X);
    bool hasData();
    void setTimes(const QVector<double>& times);
    void setLabels(const QVector<int>& labels);
    void setAmplitudes(const QVector<double>& amps);

    void setScores(const QVector<double>& detectability_scores, const QVector<double>& outlier_scores);

    void setMode(int mode);
    void setCurrentEvent(MVEvent evt, bool do_emit = false);
    MVEvent currentEvent();
    int currentEventIndex();
    AffineTransformation transformation();
    void setTransformation(const AffineTransformation& T);

    void setEventFilter(FilterInfo F);

    QSet<int> activeClusterNumbers() const;
    void setActiveClusterNumbers(const QSet<int>& A);

    QImage renderImage(int W, int H);
signals:
    void currentEventChanged();
    void transformationChanged();
    void activeClusterNumberToggled();

private slots:
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

class MVClusterLegend {
public:
    void setClusterColors(const QList<QColor>& colors);
    void setClusterNumbers(const QVector<int>& numbers);
    void setParentWindowSize(QSize size);
    void draw(QPainter* painter);
    int clusterNumberAt(QPointF pos) const;

    int hoveredClusterNumber() const;
    void setHoveredClusterNumber(int num);
    QSet<int> activeClusterNumbers() const;
    void setActiveClusterNumbers(const QSet<int>& A);
    void toggleActiveClusterNumber(int num);

private:
    QList<QColor> m_cluster_colors;
    QVector<int> m_cluster_numbers;
    QSize m_parent_window_size;
    QList<QRectF> m_cluster_number_rects;
    int m_hovered_cluster_number = -1;
    QSet<int> m_active_cluster_numbers;

    QColor cluster_color(int k) const;
};

#endif // MVCLUSTERVIEW_H
