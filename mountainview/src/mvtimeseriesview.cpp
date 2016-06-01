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

struct mvtsv_segment {
    long ds_factor;
    long i1, i2;
    QImage image;
    QString makeCode()
    {
        return QString("%1--%2--%3").arg(ds_factor).arg(i1).arg(i2);
    }
};

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
    static mvtsv_coord from_t(double t)
    {
        return mvtsv_coord(0, t, 0);
    }
};

struct mvtsv_channel {
    long channel;
    QString label;
    QColor color;
    QRectF geometry;
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
    QList<mvtsv_channel> m_channels;
    long m_ds_factor;
    double m_current_t;
    MVRange m_selected_t_range;
    bool m_activated;

    mvtsv_layout_settings m_layout_settings;

    bool m_layout_needed;

    QPointF m_left_click_anchor_pix;
    mvtsv_coord m_left_click_anchor_coord;
    MVRange m_left_click_anchor_t_range;
    bool m_left_click_dragging;

    QMap<QString, mvtsv_segment> m_segments;

    QList<mvtsv_channel> make_channel_layout(double W, double H, long M);
    void paint_cursor(QPainter* painter, double W, double H);
    QPointF coord2pix(mvtsv_coord C);
    mvtsv_coord pix2coord(long channel, QPointF pix);

    void zoom_out(mvtsv_coord about_coord, double frac = 0.8);
    void zoom_in(mvtsv_coord about_coord, double frac = 0.8);

    QList<mvtsv_segment> get_required_segments(long ds_factor);
    void load_segment_image(mvtsv_segment& S);
    void paint_segment(QPainter* painter, mvtsv_segment& S);
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
    d->m_layout_needed = true;
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
    d->m_layout_needed = true;
    update();
}

MVRange MVTimeSeriesView::timeRange() const
{
    return MVRange(d->m_view_t1, d->m_view_t2);
}

void MVTimeSeriesView::resizeEvent(QResizeEvent* evt)
{
    d->m_layout_needed = true;
    QWidget::resizeEvent(evt);
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

    if (d->m_layout_needed) {
        d->m_layout_needed=false;
        d->m_channels = d->make_channel_layout(W0, H0, M);
        d->m_segments.clear();
        for (long m = 0; m < M; m++) {
            mvtsv_channel* CH = &d->m_channels[m];
            CH->channel = m;
            CH->label = QString("%1").arg(m + 1);
            CH->color = Qt::black;
        }
    }

    d->paint_cursor(&painter, W0, H0);

    long minimum_num_timepoints = W0;
    if (minimum_num_timepoints > 5000)
        minimum_num_timepoints = 5000; /// TODO this should be a constant

    long ds_factor;
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
    }
    d->m_ds_factor = ds_factor;

    QList<mvtsv_segment> required_segments = d->get_required_segments(ds_factor);

    //Mda Xmin, Xmax;
    //d->m_ts.getData(Xmin, Xmax, data_i1 / ds_factor, data_i2 / ds_factor, ds_factor);

    for (int i = 0; i < required_segments.count(); i++) {
        d->load_segment_image(required_segments[i]);
        d->paint_segment(&painter, required_segments[i]);
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

    mvtsv_channel* CH = &m_channels[C.channel];

    double xpct = (C.t - m_view_t1) / (m_view_t2 - m_view_t1);
    double px = CH->geometry.x() + xpct * CH->geometry.width();
    double py = CH->geometry.y() + CH->geometry.height() / 2 + CH->geometry.height() / 2 * (C.y * m_vertical_scale_factor);
    return QPointF(px, py);
}

mvtsv_coord MVTimeSeriesViewPrivate::pix2coord(long channel, QPointF pix)
{
    if (channel >= m_channels.count())
        return mvtsv_coord(0, 0, 0);

    mvtsv_channel* CH = &m_channels[channel];

    mvtsv_coord C;
    double xpct = 0;
    if (CH->geometry.width()) {
        xpct = (pix.x() - CH->geometry.x()) / (CH->geometry.width());
    }
    C.t = m_view_t1 + xpct * (m_view_t2 - m_view_t1);
    if (m_vertical_scale_factor) {
        C.y = (pix.y() - (CH->geometry.y() + CH->geometry.height() / 2)) / m_vertical_scale_factor / (CH->geometry.height() / 2);
    }
    return C;
}

void MVTimeSeriesViewPrivate::zoom_out(mvtsv_coord about_coord, double frac)
{
    QPointF about_pix = coord2pix(about_coord);
    q->setTimeRange(q->timeRange() * (1 / frac));
    mvtsv_coord new_coord = pix2coord(0, about_pix);
    double dt = about_coord.t - new_coord.t;
    q->setTimeRange(q->timeRange() + (dt));
}

void MVTimeSeriesViewPrivate::zoom_in(mvtsv_coord about_coord, double frac)
{
    zoom_out(about_coord, 1 / frac);
}

QList<mvtsv_segment> MVTimeSeriesViewPrivate::get_required_segments(long ds_factor)
{
    long i1 = floor(m_view_t1 - m_data_t0);
    long i2 = ceil(m_view_t2 - m_data_t0);

    long j1 = floor(i1 * 1.0 / ds_factor);
    long j2 = ceil(i2 * 1.0 / ds_factor);

    long chunk_size = 100;
    int chunk_num1 = j1 / chunk_size;
    int chunk_num2 = j2 / chunk_size;

    QList<mvtsv_segment> ret;
    for (int chunk_num = chunk_num1; chunk_num <= chunk_num2; chunk_num++) {
        mvtsv_segment SS;
        SS.ds_factor = ds_factor;
        SS.i1 = chunk_num * chunk_size;
        SS.i2 = (chunk_num + 1) * chunk_size;
        ret << SS;
    }

    return ret;
}

void MVTimeSeriesViewPrivate::load_segment_image(mvtsv_segment& S)
{
    QString code = S.makeCode();
    if (m_segments.contains(code)) {
        S.image = m_segments[code].image;
        return;
    }

    qDebug() << "DEBUG" << __FUNCTION__ << __FILE__ << __LINE__;
    long NN = S.i2 - S.i1 + 1;

    int oversamp=5;

    int W = NN*oversamp;
    int H = q->height();
    QImage img = QImage(W, H, QImage::Format_ARGB32);
    QColor transparent(0, 0, 0, 0);
    img.fill(transparent);

    QPainter painter(&img);
    painter.setRenderHint(QPainter::Antialiasing);

    Mda Xmin, Xmax;
    m_ts.getData(Xmin, Xmax, S.i1, S.i2, S.ds_factor);

    for (int m = 0; m < m_channels.count(); m++) {
        mvtsv_channel* CH = &m_channels[m];

        QVector<double> min_values, max_values;
        for (int ii = 0; ii < NN; ii++) {
            min_values << Xmin.value(m, ii);
            max_values << Xmax.value(m, ii);
        }

        //draw data
        QPen pen = painter.pen();
        pen.setColor(CH->color);
        painter.setPen(pen);
        QPainterPath path;
        for (long n = 0; n < NN; n++) {
            double y1 = CH->geometry.top() + CH->geometry.height() / 2 + CH->geometry.height() / 2 * (min_values[n] * m_vertical_scale_factor);
            double y2 = CH->geometry.top() + CH->geometry.height() / 2 + CH->geometry.height() / 2 * (max_values[n] * m_vertical_scale_factor);
            QPointF pt1 = QPointF((n+0.5)*oversamp, y1);
            QPointF pt2 = QPointF((n+0.5)*oversamp, y2);
            if (n == 0) {
                path.moveTo(pt1);
                path.lineTo(pt2);
            }
            else {
                path.lineTo(pt1);
                path.lineTo(pt2);
            }
        }
        painter.drawPath(path);
    }
    S.image = img;

    m_segments[code] = S;

    qDebug() << "DEBUG" << __FUNCTION__ << __FILE__ << __LINE__;
}

void MVTimeSeriesViewPrivate::paint_segment(QPainter* painter, mvtsv_segment& S)
{
    mvtsv_layout_settings L = m_layout_settings;
    double xfactor = (q->width() - L.margin_left - L.margin_right) * 1.0 / (m_view_t2 - m_view_t1);
    double x0 = L.margin_left + (S.i1 * S.ds_factor - m_view_t1) * xfactor;
    double y0 = 0;
    double w0 = (S.i2 - S.i1 + 1) * S.ds_factor * xfactor;
    double h0 = q->height();
    QImage img=S.image.scaled(w0,h0,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    painter->drawImage(x0,y0,img);
    //QRectF R(x0, y0, w0, h0);
    //painter->drawPixmap(R, S.image, QRectF(0, 0, S.image.width(), S.image.height()));
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
    double center = (min + max) / 2;
    double span = (max - min);
    return MVRange(center - span / 2 * scale, center + span / 2 * scale);
}
