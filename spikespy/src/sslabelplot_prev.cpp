#include "sslabelplot_prev.h"
#include "sslabelsmodel1_prev.h"
#include "plotarea_prev.h"
#include <QDebug>
#include <QObject>

class SSLabelPlotPrivate {
public:
    SSLabelPlot* q;

    SSLabelsModel* m_labels;
    bool m_labels_is_owner;
    int m_max_timepoint;
    int m_max_label;

    PlotArea m_plot_area;
    QVector<double> m_plot_offsets;
    QList<QColor> m_label_colors;
    QPixmap m_image;
    bool m_image_needs_update;
    int m_margins[4];
    bool m_show_zoom_message;

    void set_labels2();
    void setup_plot_area();
    QColor get_label_color(int ll);
};

SSLabelPlot::SSLabelPlot()
{
    d = new SSLabelPlotPrivate;
    d->q = this;

    d->m_labels = 0;
    d->m_labels_is_owner = false;

    d->m_image_needs_update = false;
    d->m_max_timepoint = 0;
    d->m_max_label = 0;

    QList<QString> color_strings;

    color_strings.clear();
    color_strings << "#F7977A"
                  << "#FDC68A"
                  << "#C4DF9B"
                  << "#82CA9D"
                  << "#6ECFF6"
                  << "#8493CA"
                  << "#A187BE"
                  << "#F49AC2"
                  << "#F9AD81"
                  << "#FFF79A"
                  << "#A2D39C"
                  << "#7BCDC8"
                  << "#7EA7D8"
                  << "#8882BE"
                  << "#BC8DBF"
                  << "#F6989D";
    for (int i = 0; i < color_strings.size(); i++) {
        d->m_label_colors << QColor(color_strings[i]);
    }

    d->m_plot_area.setMarkerColors(d->m_label_colors);

    d->m_margins[0] = d->m_margins[1] = d->m_margins[2] = d->m_margins[3] = 0;
    d->m_show_zoom_message = false;

    d->m_plot_area.setPlotBaselines(true);

    connect(this, SIGNAL(replotNeeded()), this, SLOT(slot_replot_needed()));
}

SSLabelPlot::~SSLabelPlot()
{
    if (d->m_labels) {
        if (d->m_labels_is_owner)
            delete d->m_labels;
    }
    delete d;
}

void SSLabelPlot::updateSize()
{
    d->m_plot_area.setPosition(d->m_margins[0], d->m_margins[2]);
    d->m_plot_area.setSize(width() - d->m_margins[0] - d->m_margins[1], height() - d->m_margins[1] - d->m_margins[3]);
}

Vec2 SSLabelPlot::coordToPix(Vec2 coord)
{
    return d->m_plot_area.coordToPix(coord);
}

Vec2 SSLabelPlot::pixToCoord(Vec2 pix)
{
    return d->m_plot_area.pixToCoord(pix);
}

void SSLabelPlot::setXRange(const Vec2& range)
{
    if ((xRange().x != range.x) || (xRange().y != range.y)) {
        SSAbstractPlot::setXRange(range);
        d->setup_plot_area();
    }
}

void SSLabelPlot::setYRange(const Vec2& range)
{
    d->m_plot_area.setYRange(range.x, range.y);
    SSAbstractPlot::setYRange(range);
}

void SSLabelPlot::initialize()
{
    refresh();
}

void SSLabelPlot::refresh()
{
    d->setup_plot_area();
}

void SSLabelPlot::setLabels(DiskReadMdaOld* TL, bool is_owner)
{
    SSLabelsModel1* L = new SSLabelsModel1;
    L->setTimepointsLabels(TL, is_owner);
    setLabels(L, true);
}

void SSLabelPlot::setLabels(SSLabelsModel* L, bool is_owner)
{
    d->m_labels = L;
    d->m_labels_is_owner = is_owner;

    d->m_max_timepoint = L->getMaxTimepoint();
    d->m_max_label = L->getMaxLabel();

    d->set_labels2();

    d->m_image_needs_update = true;
    update();
}

void SSLabelPlot::setMargins(int left, int right, int top, int bottom)
{
    d->m_margins[0] = left;
    d->m_margins[1] = right;
    d->m_margins[2] = top;
    d->m_margins[3] = bottom;
}

void SSLabelPlot::slot_replot_needed()
{
    this->refresh();
}

void SSLabelPlot::paintPlot(QPainter* painter)
{
    updateSize();

    if ((width() != d->m_image.width()) || (height() != d->m_image.height())) {
        d->m_image_needs_update = true;
    }

    if (d->m_image_needs_update) {
        d->m_image = QPixmap(width(), height());
        d->m_image.fill(QColor(0, 0, 0, 0));
        QPainter painter2(&d->m_image);
        d->m_plot_area.refresh(&painter2);
        d->m_image_needs_update = false;
    }

    painter->drawPixmap(0, 0, d->m_image);

    if (d->m_show_zoom_message) {
        QString str = "Zoom in to view labels";
        painter->setFont(QFont("Arial", 15));
        QRect RR(0, 0, width(), height() - 30);
        painter->drawText(RR, Qt::AlignCenter | Qt::AlignVCenter, str);
    }
}

void SSLabelPlotPrivate::set_labels2()
{
    int N = m_max_timepoint + 1;

    q->setXRange(vec2(0, N - 1));

    q->update();
}

void SSLabelPlotPrivate::setup_plot_area()
{
    if (!m_labels)
        return;

    m_plot_offsets.clear();

    double offset = 0;
    offset += 0.5;
    for (int ii = 0; ii < m_max_label; ii++) {
        m_plot_offsets << offset;
        offset += 1;
    }
    q->setYRange(vec2(0, offset - 0.5));
    if (q->channelFlip()) {
        QVector<double> tmp = m_plot_offsets;
        for (int i = 0; i < tmp.count(); i++) {
            m_plot_offsets[i] = tmp[tmp.count() - 1 - i];
        }
    }

    m_plot_area.clearSeries();

    int xrange_min = q->xRange().x;
    int xrange_max = q->xRange().y;

    m_plot_area.setXRange(xrange_min - 1, xrange_max + 1);

    int x1, x2;
    x1 = xrange_min;
    x2 = xrange_max;

    int max_range_for_showing_labels = 1e6;
    if (x2 - x1 + 1 > max_range_for_showing_labels) {
        m_image_needs_update = true;
        m_show_zoom_message = true;
        q->update();
        return;
    }
    m_show_zoom_message = false;

    MemoryMda TL = m_labels->getTimepointsLabels(x1, x2);
    int K = TL.size(1);

    for (int ii = 0; ii < m_max_label; ii++) {

        QVector<int> T;
        for (int j = 0; j < K; j++) {
            int t0 = (int)TL.value(0, j);
            int l0 = (int)TL.value(1, j);
            if (l0 == ii + 1) {
                T << t0;
            }
        }

        Mda xvals, yvals;
        xvals.allocate(1, T.count() * 2);
        yvals.allocate(1, T.count() * 2);
        for (int jj = 0; jj < T.count(); jj++) {
            xvals.setValue(T[jj], 0, jj * 2);
            yvals.setValue(-0.45, 0, jj * 2);
            xvals.setValue(T[jj], 0, jj * 2 + 1);
            yvals.setValue(0.45, 0, jj * 2 + 1);
        }

        QColor color = get_label_color(ii);
        PlotSeries SS;
        SS.xvals = xvals;
        SS.yvals = yvals;
        SS.color = color;
        SS.offset = m_plot_offsets[ii];
        SS.plot_pairs = true;
        SS.name = QString("%1").arg(ii + 1);
        m_plot_area.addSeries(SS);
    }

    m_image_needs_update = true;
    q->update();
}

QColor SSLabelPlotPrivate::get_label_color(int ll)
{
    if (m_label_colors.size() == 0)
        return QColor(0, 0, 0);
    return m_label_colors[ll % m_label_colors.size()];
}
