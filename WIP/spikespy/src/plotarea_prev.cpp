#include "plotarea_prev.h"
#include <QDebug>
#include <QTime>

struct PAMarker {
    int t;
    int l;
    int vpos;
};

class PlotAreaPrivate {
public:
    PlotArea* q;
    double m_xmin, m_xmax;
    double m_ymin, m_ymax;
    QRect m_plot_rect;
    int m_line_width;
    //bool m_connect_zeros;
    QList<PlotSeries> m_series;
    QList<PAMarker> m_markers;
    QList<QColor> m_marker_colors;
    QList<QString> m_marker_labels;
    int m_left_panel_width;
    int m_right_panel_width;
    bool m_plot_baselines;
    int m_marker_alpha;
    bool m_show_marker_lines;

    void do_refresh(QPainter* P);
};

PlotArea::PlotArea()
{
    d = new PlotAreaPrivate;
    d->q = this;

    d->m_xmin = d->m_xmax = 0;
    d->m_ymin = d->m_ymax = 0;
    d->m_line_width = 1;
    //d->m_connect_zeros=true;
    d->m_left_panel_width = 10;
    d->m_right_panel_width = 10;
    d->m_plot_baselines = false;
    d->m_marker_alpha = 255;
    d->m_show_marker_lines = true;
}

PlotArea::~PlotArea()
{
    delete d;
}

void PlotArea::setSize(double W, double H)
{
    d->m_plot_rect = QRect(d->m_plot_rect.left(), d->m_plot_rect.top(), (int)W, (int)H);
}
void PlotArea::setPosition(double x0, double y0)
{
    d->m_plot_rect = QRect((int)x0, (int)y0, d->m_plot_rect.width(), d->m_plot_rect.height());
}

void PlotArea::clearSeries()
{
    d->m_series.clear();
}

void PlotArea::setXRange(double xmin, double xmax)
{
    d->m_xmin = xmin;
    d->m_xmax = xmax;
}

void PlotArea::setYRange(double ymin, double ymax)
{
    d->m_ymin = ymin;
    d->m_ymax = ymax;
}

void PlotArea::addSeries(const PlotSeries& SS)
{
    d->m_series.append(SS);
}

void PlotArea::addMarker(int t, int l)
{
    PAMarker X;
    X.t = t;
    X.l = l;
    X.vpos = 0;
    d->m_markers << X;
}

void PlotArea::addCompareMarker(int t, int l)
{
    PAMarker X;
    X.t = t;
    X.l = l;
    X.vpos = 1;
    d->m_markers << X;
}

void PlotArea::clearMarkers()
{
    d->m_markers.clear();
}

void PlotArea::refresh(QPainter* P)
{
    d->do_refresh(P);
}

void PlotAreaPrivate::do_refresh(QPainter* P)
{

    QTime timer;
    timer.start();

    /*
	Vec2 PP0=q->coordToPix(vec2(xmin,m_ymin));
	Vec2 PP1=q->coordToPix(vec2(xmax,m_ymax));
	P->fillRect(qMin(PP0.x,PP1.x),qMin(PP0.y,PP1.y), qAbs(PP0.x-PP1.x), qAbs(PP0.y-PP1.y),QBrush(QColor(0,0,0,0)));
	*/

    m_left_panel_width = qMax(40, qMin(20, m_plot_rect.width() / 100));
    m_right_panel_width = m_left_panel_width;

    P->fillRect(QRect(m_left_panel_width, m_plot_rect.top(), m_plot_rect.width() - m_left_panel_width - m_right_panel_width, m_plot_rect.height()), QBrush(QColor(240, 240, 240)));

    //left/right panel
    for (int pass = 1; pass <= 2; pass++) {
        for (int ss = 0; ss < m_series.count(); ss++) {
            PlotSeries* SS = &m_series[ss];
            Vec2 pix = q->coordToPix(vec2(0, SS->offset));
            QRect RR;
            if (pass == 1) {
                RR = QRect(0, pix.y - 30, m_left_panel_width, 60);
            }
            else if (pass == 2) {
                RR = QRect(m_plot_rect.width() - m_right_panel_width, pix.y - 30, m_right_panel_width, 60);
            }
            P->setPen(QPen(QColor(150, 150, 255)));
            P->setFont(QFont("Arial", qMin(m_left_panel_width - 1, 9)));
            P->drawText(RR, Qt::AlignCenter | Qt::AlignVCenter, SS->name);
        }
    }

    //baselines
    if (m_plot_baselines) {
        for (int ss = 0; ss < m_series.count(); ss++) {
            PlotSeries* SS = &m_series[ss];
            Vec2 pix = q->coordToPix(vec2(0, SS->offset));
            P->drawLine(m_left_panel_width, pix.y, m_plot_rect.width() - m_left_panel_width - m_right_panel_width, pix.y);
        }
    }

    //plot the markers
    for (int pass = 1; pass <= 2; pass++) {
        P->setFont(QFont("Arial", 8));
        QSet<int> marker_x_positions;
        for (int i = 0; i < m_markers.count(); i++) {
            int t0 = m_markers[i].t;
            int l0 = m_markers[i].l;
            int vpos0 = m_markers[i].vpos;
            Vec2 pix = q->coordToPix(vec2(t0, 0));
            QColor col = QColor(0, 0, 0);
            if (l0 == 0) {
                col = Qt::gray;
            }
            else {
                if (m_marker_colors.count() > 0) {
                    col = m_marker_colors.value((l0 - 1) % m_marker_colors.count());
                }
            }
            float pixels_per_marker = m_plot_rect.width() * 1.0 / m_markers.count();
            float marker_width = qMin(0.5F, pixels_per_marker / 20);
            P->setPen(QPen(col, marker_width));
            if (m_show_marker_lines) {
                P->drawLine(pix.x, m_plot_rect.bottom(), pix.x, m_plot_rect.top());
            }

            if ((pixels_per_marker > 1) && (l0 > 0)) {
                if (l0 < m_marker_labels.count()) {
                    int x0 = pix.x;
                    int offset = 0;
                    Q_UNUSED(offset)
                    //don't do the offsets for now, because it disrupts the comparison markers
                    /*
                    for (int dx=-6; dx<=0; dx++) {
                        if (marker_x_positions.contains(x0+dx)) offset=dx+6;
                    }
                    if (offset>0) {
                        x0+=offset;
                    }
                    */
                    QRect RR;
                    if (pass == 1) {
                        RR = QRect(x0 - 50, m_plot_rect.bottom() + 15 * vpos0, 100, 15);
                    }
                    else if (pass == 2) {
                        RR = QRect(x0 - 50, m_plot_rect.top() - 15 * vpos0 - 15, 100, 15);
                    }
                    marker_x_positions.insert(x0);
                    //float tmp=qMin(200.0F,(pixels_per_marker-1)*1.0F/6*255);
                    //P->setPen(QPen(QColor(255-tmp,255-tmp,255-tmp),0));
                    //QColor color_of_text=col;
                    QColor color_of_text = QColor(20, 20, 20);
                    color_of_text.setAlpha((int)qMin(255.0F, pixels_per_marker * 256.0F / 20));
                    P->setPen(color_of_text);
                    P->drawText(RR, Qt::AlignCenter | Qt::AlignVCenter, m_marker_labels[l0]);
                }
            }
        }
    }

    //plot the actual data
    int num_line_segments = 0;
    for (int ss = 0; ss < m_series.size(); ss++) {
        PlotSeries SS = m_series[ss];
        bool is_first = true;
        int line_width = m_line_width;
        if (SS.plot_pairs)
            line_width++; //this is a major hack!
        P->setPen(QPen(QBrush(SS.color), line_width));
        QPainterPath path;
        //double last_val=0;
        if (!SS.plot_pairs) {
            for (int i = 0; i < SS.xvals.N2(); i++) {
                double x0 = SS.xvals.value(0, i);
                double val = SS.yvals.value(0, i);
                Vec2 pix1 = q->coordToPix(vec2(x0, val + SS.offset));
                //if ((val==0)&&(last_val==0)/*&&(!m_connect_zeros)*/) {
                if ((val == 0) /*&&(!m_connect_zeros)*/) {
                    path.moveTo(pix1.x, pix1.y);
                    //is_first=false;
                    is_first = true;
                }
                else {
                    if (is_first) {
                        is_first = false;
                        path.moveTo(pix1.x, pix1.y);
                    }
                    else {
                        path.lineTo(pix1.x, pix1.y);
                        num_line_segments++;
                    }
                }
                //last_val=val;
            }
        }
        else {
            for (int i = 0; i + 1 < SS.xvals.N2(); i += 2) {
                double x1 = SS.xvals.value(0, i);
                double val1 = SS.yvals.value(0, i);
                double x2 = SS.xvals.value(0, i + 1);
                double val2 = SS.yvals.value(0, i + 1);
                Vec2 pix1 = q->coordToPix(vec2(x1, val1 + SS.offset));
                Vec2 pix2 = q->coordToPix(vec2(x2, val2 + SS.offset));
                path.moveTo(pix1.x, pix1.y);
                path.lineTo(pix2.x, pix2.y);
                num_line_segments++;
            }
        }
        P->drawPath(path);
    }

    //qDebug()  << "num_segments = " << num_segments << "elapsed: " << timer.elapsed();
}

Vec2 PlotArea::coordToPix(Vec2 coord)
{
    if (d->m_xmax <= d->m_xmin) {
        return vec2(0, 0);
    }
    if (d->m_ymax <= d->m_ymin) {
        return vec2(0, 0);
    }
    double x0 = coord.x;
    double y0 = coord.y;
    double pctx = (x0 - d->m_xmin) / (d->m_xmax - d->m_xmin);
    double pcty = (y0 - d->m_ymin) / (d->m_ymax - d->m_ymin);
    double x1 = (d->m_plot_rect.left() + d->m_left_panel_width + 5) + (d->m_plot_rect.width() - d->m_left_panel_width - d->m_right_panel_width - 10) * pctx;
    double y1 = d->m_plot_rect.top() + d->m_plot_rect.height() * (1 - pcty);
    return vec2(x1, y1);
}

Vec2 PlotArea::pixToCoord(Vec2 pix)
{
    if (d->m_plot_rect.width() <= 0) {
        return vec2(0, 0);
    }
    if (d->m_plot_rect.height() <= 0) {
        return vec2(0, 0);
    }
    double x0 = pix.x;
    double y0 = pix.y;
    double pctx = (x0 - (d->m_plot_rect.left() + d->m_left_panel_width + 5)) / (d->m_plot_rect.width() - d->m_left_panel_width - d->m_right_panel_width - 10);
    double pcty = (y0 - d->m_plot_rect.top()) / d->m_plot_rect.height();
    double x1 = d->m_xmin + (d->m_xmax - d->m_xmin) * pctx;
    double y1 = d->m_ymax - (d->m_ymax - d->m_ymin) * pcty;
    return vec2(x1, y1);
}

void PlotArea::setMarkerColors(const QList<QColor>& colors)
{
    d->m_marker_colors = colors;
}

void PlotArea::setMarkerLabels(const QList<QString>& labels)
{
    d->m_marker_labels = labels;
}

/*void PlotArea::setConnectZeros(bool val)
{
	d->m_connect_zeros=val;
}*/

void PlotArea::setPlotBaselines(bool val)
{
    d->m_plot_baselines = val;
}

void PlotArea::setMarkerAlpha(int val)
{
    d->m_marker_alpha = val;
}

void PlotArea::setShowMarkerLines(bool val)
{
    d->m_show_marker_lines = val;
}

QRect PlotArea::plotRect()
{
    return d->m_plot_rect;
}
Vec2 PlotArea::yRange() const
{
    return vec2(d->m_ymin, d->m_ymax);
}
