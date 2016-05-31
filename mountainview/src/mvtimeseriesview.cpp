/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/27/2016
*******************************************************/

#include "mvtimeseriesview.h"
#include "multiscaletimeseries.h"
#include <math.h>

#include <QMouseEvent>
#include <QPainter>

struct mvtsv_coord {
    mvtsv_coord(long channel0 = 0, double t0 = 0, double y0 = 0)
    {
        channel = channel0;
        t = t0;
        y = y0;
    }
    long channel;
    double t;
    double y;
    static mvtsv_coord from_t(double t) {
        return mvtsv_coord(0,t,0);
    }
};

struct mvtsv_channel {
    mvtsv_channel()
    {
        vertical_scale_factor = 1;
    }
    long channel;
    QString label;
    QColor color;
    QVector<double> min_values;
    QVector<double> max_values;
    QRectF geometry;
    double vertical_scale_factor;

    QPointF coord2pix(mvtsv_coord C, MVRange view_t_range);
    mvtsv_coord pix2coord(QPointF pix, MVRange view_t_range);
};
struct mvtsv_layout_settings {
    mvtsv_layout_settings()
    {
        margin_left = 30;
        margin_right = 30;
        margin_top = 30;
        margin_bottom = 30;
        vertical_space_between_channel_rects = 4;
    }

    double margin_left, margin_right;
    double margin_top, margin_bottom;
    double vertical_space_between_channel_rects;
};

class MVTimeSeriesViewPrivate {
public:
    MVTimeSeriesView* q;
    MultiScaleTimeSeries m_ts;
    DiskReadMda m_data;
    double m_data_t0;
    double m_view_t1, m_view_t2;
    double m_vertical_scale_factor;
    long m_channel_i1, m_ds_factor;
    QList<mvtsv_channel> m_channels;
    mvtsv_layout_settings m_layout_settings;
    double m_current_t;
    MVRange m_selected_t_range;
    bool m_activated;

    QPointF m_left_click_anchor_pix;
    mvtsv_coord m_left_click_anchor_coord;
    MVRange m_left_click_anchor_t_range;
    bool m_left_click_dragging;

    QList<mvtsv_channel> make_channel_layout(double W, double H, long M);
    void paint_channel(QPainter* painter, mvtsv_channel* CH);
    void paint_cursor(QPainter* painter, double W, double H);
    QPointF coord2pix(mvtsv_coord C);
    mvtsv_coord pix2coord(long channel, QPointF pix);

    void zoom_out(mvtsv_coord about_coord, double frac = 0.8);
    void zoom_in(mvtsv_coord about_coord, double frac = 0.8);
};

MVTimeSeriesView::MVTimeSeriesView()
{
    d = new MVTimeSeriesViewPrivate;
    d->q = this;
    d->m_current_t = 0;
    d->m_selected_t_range = MVRange(-1, -1);
    d->m_activated = true; /// TODO set activated only when window is active (like in sstimeseriesview, I think)
    d->m_vertical_scale_factor = 1;
    d->m_left_click_anchor_pix = QPointF(-1, -1);
    d->m_left_click_dragging = false;
    this->setMouseTracking(true);
}

MVTimeSeriesView::~MVTimeSeriesView()
{
    delete d;
}

void MVTimeSeriesView::setData(double t0, const DiskReadMda& X)
{
    d->m_data_t0 = t0;
    d->m_data = X;
    d->m_ts.setData(X);
    update();
}

MVRange MVTimeSeriesView::timeRange() const
{
    return MVRange(d->m_view_t1, d->m_view_t2);
}

void MVTimeSeriesView::setTimeRange(MVRange range)
{
    d->m_view_t1 = range.min;
    d->m_view_t2 = range.max;
    update();
}

void MVTimeSeriesView::setCurrentTimepoint(double t)
{
    if (t == d->m_current_t)
        return;
    d->m_current_t = t;
    update();
}

void MVTimeSeriesView::setSelectedTimeRange(MVRange range)
{
    if (range == d->m_selected_t_range)
        return;
    d->m_selected_t_range = range;
    update();
}

double MVTimeSeriesView::currentTimepoint() const
{
    return d->m_current_t;
}

void MVTimeSeriesView::paintEvent(QPaintEvent* evt)
{
    Q_UNUSED(evt)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    double W0 = this->width();
    double H0 = this->height();
    long M = d->m_data.N1();
    if (!M)
        return;
    d->m_channels = d->make_channel_layout(W0, H0, M);

    long minimum_num_timepoints = W0;
    if (minimum_num_timepoints > 5000)
        minimum_num_timepoints = 5000; /// TODO this should be a constant

    long ds_factor;
    long data_i1, data_i2;
    /*
     * What we want:
     *   data_i1,data_i2 must be divisible by ds_factor
     *   (data_i2-data_i1)/ds_factor >= minimum_num_timepoints (but minimal)
     *   data_i1 < (d->m_view_t1-d->m_data_t0)
     *   data_i2 > (d->m_view_t2-d->m_data_t0)
     */

    {
        long i1 = floor(d->m_view_t1 - d->m_data_t0);
        long i2 = ceil(d->m_view_t2 - d->m_data_t0);
        ds_factor = MultiScaleTimeSeries::smallest_power_of_3_larger_than((i2 - i1) / minimum_num_timepoints);
        data_i1 = (i1 / ds_factor) * ds_factor;
        data_i2 = (i2 / ds_factor + 1) * ds_factor;
    }
    d->m_channel_i1 = data_i1;
    d->m_ds_factor = ds_factor;

    qDebug() << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << d->m_ds_factor << (data_i2-data_i1+1)/d->m_ds_factor;

    Mda Xmin, Xmax;
    d->m_ts.getData(Xmin, Xmax, data_i1 / ds_factor, data_i2 / ds_factor, ds_factor);
    Xmin.write32("debug_min.mda");
    Xmax.write32("debug_max.mda");

    for (long m = 0; m < M; m++) {
        mvtsv_channel* CH = &d->m_channels[m];
        CH->channel = m;
        CH->label = QString("%1").arg(m + 1);
        CH->vertical_scale_factor = d->m_vertical_scale_factor;
        CH->color = Qt::black;
        for (long i = 0; i < Xmin.N2(); i++) {
            CH->min_values << Xmin.value(m, i);
            CH->max_values << Xmax.value(m, i);
        }
    }

    d->paint_cursor(&painter, W0, H0);

    for (long m = 0; m < M; m++) {
        mvtsv_channel* CH = &d->m_channels[m];
        d->paint_channel(&painter, CH);
    }
}

void MVTimeSeriesView::mousePressEvent(QMouseEvent* evt)
{
    if (evt->button() == Qt::LeftButton) {
        d->m_left_click_anchor_pix = evt->pos();
        d->m_left_click_anchor_t_range = this->timeRange();
        d->m_left_click_anchor_coord = d->pix2coord(0, evt->pos());
        d->m_left_click_dragging = false;
    }
    //update_cursor();
}

void MVTimeSeriesView::mouseReleaseEvent(QMouseEvent* evt)
{
    d->m_left_click_anchor_pix = QPointF(-1, -1);
    if (evt->button() == Qt::LeftButton) {
        if (!d->m_left_click_dragging) {
            mvtsv_coord coord = d->pix2coord(0, evt->pos());
            this->setCurrentTimepoint(coord.t);
        }
        d->m_left_click_dragging = false;
    }
    //update_cursor();
}

void MVTimeSeriesView::mouseMoveEvent(QMouseEvent* evt)
{
    if (!d->m_left_click_dragging) {
        if (d->m_left_click_anchor_pix.x() >= 0) {
            double dist = qAbs(d->m_left_click_anchor_pix.x() - evt->pos().x());
            if (dist >= 5) {
                d->m_left_click_dragging = true;
            }
        }
    }

    if (d->m_left_click_dragging) {
        // we want the evt->pos() to correspond to d->m_left_click_anchor_coord.t
        double t0 = d->pix2coord(0, evt->pos()).t;
        double dt = t0 - d->m_left_click_anchor_coord.t;
        this->setTimeRange(this->timeRange() + (-dt));
    }

    /// TODO set the mouse pointer cursor
    //set_cursor();
}

void MVTimeSeriesView::wheelEvent(QWheelEvent* evt)
{
    int delta = evt->delta();
    if (!(evt->modifiers() & Qt::ControlModifier)) {
        if (delta < 0) {
            d->zoom_out(mvtsv_coord::from_t(this->currentTimepoint()));
        }
        else if (delta > 0) {
            d->zoom_in(mvtsv_coord::from_t(this->currentTimepoint()));
        }
    }
    else {
        //This used to allow zooming at hover position -- probably not needed
        /*
        float frac = 1;
        if (delta < 0)
            frac = 1 / 0.8;
        else if (delta > 0)
            frac = 0.8;
        Vec2 coord = this->plot()->pixToCoord(vec2(evt->pos().x(), evt->pos().y()));
        d->do_zoom(coord.x, frac);
        */
    }
}

void MVTimeSeriesView::unit_test()
{
    long M = 4;
    long N = 100000;
    Mda X(M, N);
    for (long n = 0; n < N; n++) {
        for (long m = 0; m < M; m++) {
            double period = (m + 1) * 105.8;
            double val = sin(n * 2 * M_PI / period);
            val *= (N - n) * 1.0 / N;
            val += (qrand() % 10000) * 1.0 / 10000 * 0.4;
            X.setValue(val, m, n);
        }
    }
    DiskReadMda X0(X);
    MVTimeSeriesView* W = new MVTimeSeriesView;
    W->setData(0, X0);
    W->setTimeRange(MVRange(0, 1000));
    W->show();
}

QList<mvtsv_channel> MVTimeSeriesViewPrivate::make_channel_layout(double W, double H, long M)
{
    QList<mvtsv_channel> channels;
    if (!M)
        return channels;
    mvtsv_layout_settings L = m_layout_settings;
    double channel_height = (H - L.margin_bottom - L.margin_top - (M - 1) * L.vertical_space_between_channel_rects) / M;
    double y0 = L.margin_top;
    for (int m = 0; m < M; m++) {
        mvtsv_channel X;
        X.geometry = QRectF(L.margin_left, y0, W - L.margin_left - L.margin_right, channel_height);
        channels << X;
        y0 += channel_height + L.vertical_space_between_channel_rects;
    }
    return channels;
}

void MVTimeSeriesViewPrivate::paint_channel(QPainter* painter, mvtsv_channel* CH)
{
    QPen pen = painter->pen();

    //draw border
    pen.setColor(Qt::black); /// TODO this color should be configured
    painter->setPen(pen);
    painter->drawRect(CH->geometry);

    long NN = CH->min_values.count();

    //draw data
    pen.setColor(CH->color);
    painter->setPen(pen);
    QPainterPath path;
    for (long n = 0; n < NN; n++) {
        double tt = m_channel_i1 - m_data_t0 + n * m_ds_factor;
        double yy1 = CH->min_values[n];
        double yy2 = CH->max_values[n];
        QPointF pt1 = CH->coord2pix(mvtsv_coord(CH->channel, tt, yy1),q->timeRange());
        QPointF pt2 = CH->coord2pix(mvtsv_coord(CH->channel, tt, yy2),q->timeRange());
        if (n == 0) {
            path.moveTo(pt1);
            path.lineTo(pt2);
        }
        else {
            path.lineTo(pt1);
            path.lineTo(pt2);
        }
    }
    painter->drawPath(path);
}

void MVTimeSeriesViewPrivate::paint_cursor(QPainter* painter, double W, double H)
{
    Q_UNUSED(W)
    Q_UNUSED(H)
    mvtsv_layout_settings L = m_layout_settings;

    if (m_selected_t_range.min < 0) {
        QPointF p0 = coord2pix(mvtsv_coord(0, m_current_t, 0));
        QPointF p1 = coord2pix(mvtsv_coord(0, m_current_t, 0));
        p0.setY(L.margin_top);
        p1.setY(H - L.margin_bottom);

        for (int pass = 1; pass <= 2; pass++) {
            QPainterPath path;
            QPointF pp = p0;
            int sign = -1;
            if (pass == 2) {
                pp = p1;
                sign = 1;
            }
            path.moveTo(pp.x(), pp.y() - 10 * sign);
            path.lineTo(pp.x() - 8, pp.y() - 2 * sign);
            path.lineTo(pp.x() + 8, pp.y() - 2 * sign);
            path.lineTo(pp.x(), pp.y() - 10 * sign);
            QColor col = QColor(60, 80, 60);
            if (m_activated) {
                //col=Qt::gray;
                col = QColor(50, 50, 220);
            }
            painter->fillPath(path, QBrush(col));
        }

        QPainterPath path2;
        path2.moveTo(p0.x(), p0.y() + 10);
        path2.lineTo(p1.x(), p1.y() - 10);
        painter->setPen(QPen(QBrush(QColor(50, 50, 220, 60)), 0));
        painter->drawPath(path2);

        //painter->setPen(QPen(QBrush(QColor(50,50,220,180)),1));
        //painter->drawPath(path2);
    }

    if (m_selected_t_range.min >= 0) {
        QPointF p0 = coord2pix(mvtsv_coord(0, m_selected_t_range.min, 0));
        QPointF p1 = coord2pix(mvtsv_coord(0, m_selected_t_range.max, 0));
        p0.setY(L.margin_top);
        p1.setY(H - L.margin_bottom);

        QPainterPath path;
        path.moveTo(p0.x(), p0.y());
        path.lineTo(p1.x(), p0.y());
        path.lineTo(p1.x(), p1.y());
        path.lineTo(p0.x(), p1.y());
        path.lineTo(p0.x(), p0.y());

        int pen_width = 6;
        QColor pen_color = QColor(150, 150, 150);
        painter->setPen(QPen(QBrush(pen_color), pen_width));
        painter->drawPath(path);
    }
}

QPointF MVTimeSeriesViewPrivate::coord2pix(mvtsv_coord C)
{
    if (C.channel >= m_channels.count())
        return QPointF(0, 0);
    return m_channels[C.channel].coord2pix(C,q->timeRange());
}

mvtsv_coord MVTimeSeriesViewPrivate::pix2coord(long channel, QPointF pix)
{
    if (channel >= m_channels.count())
        return mvtsv_coord(0, 0, 0);
    return m_channels[channel].pix2coord(pix,q->timeRange());
}

void MVTimeSeriesViewPrivate::zoom_out(mvtsv_coord about_coord, double frac)
{
    QPointF about_pix = coord2pix(about_coord);
    q->setTimeRange(q->timeRange() * (1/frac));
    mvtsv_coord new_coord = pix2coord(0,about_pix);
    double dt = about_coord.t - new_coord.t;
    q->setTimeRange(q->timeRange() + (dt));
}

void MVTimeSeriesViewPrivate::zoom_in(mvtsv_coord about_coord, double frac)
{
    zoom_out(about_coord, 1 / frac);
}

QPointF mvtsv_channel::coord2pix(mvtsv_coord C, MVRange view_t_range)
{
    double xpct = (C.t - view_t_range.min) / (view_t_range.max - view_t_range.min);
    double px = geometry.x() + xpct * geometry.width();
    double py = geometry.y() + geometry.height() / 2 + geometry.height() / 2 * (C.y * vertical_scale_factor);
    return QPointF(px, py);
}

mvtsv_coord mvtsv_channel::pix2coord(QPointF pix, MVRange view_t_range)
{
    mvtsv_coord C;
    double xpct = 0;
    if (geometry.height()) {
        xpct = (pix.x() - geometry.x()) / (geometry.width());
    }
    C.t = view_t_range.min + xpct * (view_t_range.max - view_t_range.min);
    if (vertical_scale_factor) {
        C.y = (pix.y() - (geometry.y() + geometry.height() / 2)) / vertical_scale_factor / (geometry.height() / 2);
    }
    return C;
}

bool MVRange::operator==(const MVRange& other)
{
    return ((other.min == min) && (other.max == max));
}

MVRange MVRange::operator+(double offset)
{
    return MVRange(min + offset, max + offset);
}

MVRange MVRange::operator*(double scale)
{
    double center=(min+max)/2;
    double span=(max-min);
    return MVRange(center-span/2*scale,center+span/2*scale);
}
