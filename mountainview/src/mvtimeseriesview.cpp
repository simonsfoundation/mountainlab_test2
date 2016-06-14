/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 5/27/2016
*******************************************************/

#include "mvtimeseriesview.h"
#include "multiscaletimeseries.h"
#include "mvtimeseriesrendermanager.h"
#include <math.h>

#include <QImageWriter>
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
    static mvtsv_coord from_t(double t)
    {
        return mvtsv_coord(0, t, 0);
    }
};

struct mvtsv_channel {
    long channel;
    QString label;
    QRectF geometry;
};

struct mvtsv_prefs {
    mvtsv_prefs()
    {
        num_label_levels = 3;
        label_font_height = 12;
        mtop = 40;
        mbottom = 60;
        mleft = 40;
        mright = 20;
        marker_color = QColor(200, 0, 0, 120);
    }

    int num_label_levels;
    int label_font_height;
    QColor marker_color;
    int mleft, mright, mtop, mbottom;

    QList<QColor> channel_colors;
};

struct TickStruct {
    TickStruct(QString str0, long min_pixel_spacing_between_ticks0, double tick_height0, double timepoint_interval0)
    {
        str = str0;
        min_pixel_spacing_between_ticks = min_pixel_spacing_between_ticks0;
        tick_height = tick_height0;
        timepoint_interval = timepoint_interval0;
        show_scale = false;
    }

    QString str;
    long min_pixel_spacing_between_ticks;
    double tick_height;
    double timepoint_interval;
    bool show_scale;
};

class MVTimeSeriesViewPrivate {
public:
    MVTimeSeriesView* q;
    MultiScaleTimeSeries m_msts;

    double m_samplerate;
    DiskReadMda m_timeseries;
    QVector<double> m_times;
    QVector<int> m_labels;

    mvtsv_prefs m_prefs;

    double m_amplitude_factor;
    QList<mvtsv_channel> m_channels;
    MVRange m_selected_t_range;
    bool m_activated;
    MVViewAgent* m_view_agent;

    bool m_layout_needed;

    MVTimeSeriesRenderManager m_render_manager;

    QPointF m_left_click_anchor_pix;
    mvtsv_coord m_left_click_anchor_coord;
    MVRange m_left_click_anchor_t_range;
    bool m_left_click_dragging;

    QList<mvtsv_channel> make_channel_layout(double W, double H, long M);
    void paint_cursor(QPainter* painter, double W, double H);
    void paint_markers(QPainter* painter, const QVector<double>& t0, const QVector<int>& labels, double W, double H);
    void paint_message_at_top(QPainter* painter, QString msg, double W, double H);
    void paint_time_axis(QPainter* painter, double W, double H);
    void paint_time_axis_unit(QPainter* painter, double W, double H, TickStruct TS);
    void paint_channel_labels(QPainter* painter, double W, double H);
    void paint_status_string(QPainter* painter, double W, double H, QString str);

    QPointF coord2pix(mvtsv_coord C);
    mvtsv_coord pix2coord(long channel, QPointF pix);

    void zoom_out(mvtsv_coord about_coord, double frac = 0.8);
    void zoom_in(mvtsv_coord about_coord, double frac = 0.8);
    void scroll_to_current_timepoint();

    QString format_time(double tp);
    void update_cursor();
};

MVTimeSeriesView::MVTimeSeriesView(MVViewAgent* view_agent)
{
    d = new MVTimeSeriesViewPrivate;
    d->q = this;
    d->m_selected_t_range = MVRange(-1, -1);
    d->m_activated = true;
    d->m_amplitude_factor = 1.0;
    d->m_left_click_anchor_pix = QPointF(-1, -1);
    d->m_left_click_dragging = false;
    d->m_layout_needed = true;
    this->setMouseTracking(true);
    d->m_render_manager.setMultiScaleTimeSeries(&d->m_msts);
    d->m_samplerate = 0;

    d->m_view_agent = view_agent;
    QObject::connect(view_agent, SIGNAL(currentTimepointChanged()), this, SLOT(update()));
    QObject::connect(view_agent, SIGNAL(currentTimeRangeChanged()), this, SLOT(update()));
    QObject::connect(view_agent, SIGNAL(currentTimepointChanged()), this, SLOT(slot_scroll_to_current_timepoint()));

    this->setFocusPolicy(Qt::StrongFocus);

    QObject::connect(&d->m_render_manager, SIGNAL(updated()), this, SLOT(update()));
}

MVTimeSeriesView::~MVTimeSeriesView()
{
    delete d;
}

void MVTimeSeriesView::setSampleRate(double samplerate)
{
    d->m_samplerate = samplerate;
    update();
}

void MVTimeSeriesView::setTimeseries(const DiskReadMda& X)
{
    d->m_timeseries = X;
    /// TODO address: the following is a hack so that the array info is not downloaded during the paintEvent which seems to cause a crash
    d->m_timeseries.N1();
    d->m_msts.setData(X);
    d->m_layout_needed = true;
    this->setTimeRange(MVRange(0, d->m_timeseries.N2() - 1)); //above hack not strictly needed because we now call N2() here.
    this->autoSetAmplitudeFactor();
    update();
}

void MVTimeSeriesView::setMLProxyUrl(const QString& url)
{
    d->m_msts.setMLProxyUrl(url);
    update();
}

void MVTimeSeriesView::setTimesLabels(const QVector<double>& times, const QVector<int>& labels)
{
    d->m_times = times;
    d->m_labels = labels;
    update();
}

void MVTimeSeriesView::setChannelColors(const QList<QColor>& colors)
{
    d->m_prefs.channel_colors = colors;
    d->m_render_manager.setChannelColors(colors);
    update();
}

MVRange MVTimeSeriesView::timeRange() const
{
    return d->m_view_agent->currentTimeRange();
}

double MVTimeSeriesView::amplitudeFactor() const
{
    return d->m_amplitude_factor;
}

DiskReadMda MVTimeSeriesView::timeseries()
{
    return d->m_timeseries;
}

void MVTimeSeriesView::resizeEvent(QResizeEvent* evt)
{
    d->m_layout_needed = true;
    QWidget::resizeEvent(evt);
}

void MVTimeSeriesView::setTimeRange(MVRange range)
{
    if (range.min < 0) {
        range = range + (0 - range.min);
    }
    if (range.max >= d->m_timeseries.N2()) {
        range = range + (d->m_timeseries.N2() - range.max);
    }
    if ((range.min < 0) || (range.max >= d->m_timeseries.N2())) {
        range = MVRange(0, d->m_timeseries.N2() - 1);
    }
    d->m_view_agent->setCurrentTimeRange(range);
}

void MVTimeSeriesView::setCurrentTimepoint(double t)
{
    d->m_view_agent->setCurrentTimepoint(t);
    update();
}

void MVTimeSeriesView::setSelectedTimeRange(MVRange range)
{
    if (range == d->m_selected_t_range)
        return;
    d->m_selected_t_range = range;
    update();
}

void MVTimeSeriesView::setAmplitudeFactor(double factor)
{
    d->m_amplitude_factor = factor;
    update();
}

class AutoSetAmplitudeFactorThread : public QThread {
public:
    //input
    MultiScaleTimeSeries* msts;

    //output
    double min, max;
    void run()
    {
        min = msts->minimum();
        max = msts->maximum();
    }
};

void MVTimeSeriesView::autoSetAmplitudeFactor()
{
    if (!in_gui_thread()) {
        qWarning() << "Can only call autoSetAmplitudeFactor in gui thread";
        return;
    }
    //we can't actually do this in the gui thread, which is where it will be called

    /// Witold we should be sure this thread is stopped in the rare case that the object is deleted while it is still running
    AutoSetAmplitudeFactorThread* thread = new AutoSetAmplitudeFactorThread;
    thread->msts = &d->m_msts;
    QObject::connect(thread, &AutoSetAmplitudeFactorThread::finished, [this, thread]() {
        double max_range = qMax(qAbs(d->m_msts.minimum()), qAbs(d->m_msts.maximum()));
        if (max_range) {
            this->setAmplitudeFactor(1.5 / max_range);
        }
        else {
            qWarning() << "Problem in autoSetAmplitudeFactor: range is null";
        }
    });
    thread->start();
}

void MVTimeSeriesView::autoSetAmplitudeFactorWithinTimeRange()
{
    double min0 = d->m_render_manager.visibleMinimum();
    double max0 = d->m_render_manager.visibleMaximum();
    double factor = qMax(qAbs(min0), qAbs(max0));
    if (factor)
        this->setAmplitudeFactor(1.5 / factor);
}

void MVTimeSeriesView::setActivated(bool val)
{
    if (d->m_activated == val)
        return;
    d->m_activated = val;
    update();
}

double MVTimeSeriesView::currentTimepoint() const
{
    return d->m_view_agent->currentTimepoint();
}

void MVTimeSeriesView::paintEvent(QPaintEvent* evt)
{
    Q_UNUSED(evt)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor back_col = QColor(240, 240, 240);
    if (d->m_activated)
        back_col = Qt::white;
    painter.fillRect(0, 0, width(), height(), back_col);

    double mleft = d->m_prefs.mleft;
    double mright = d->m_prefs.mright;
    double mtop = d->m_prefs.mtop;
    double mbottom = d->m_prefs.mbottom;

    double W0 = this->width();
    double H0 = this->height();

    long M = d->m_timeseries.N1();
    if (!M)
        return;

    // Geometry of channels
    if (d->m_layout_needed) {
        d->m_layout_needed = false;
        d->m_channels = d->make_channel_layout(W0, H0, M);
        for (long m = 0; m < M; m++) {
            mvtsv_channel* CH = &d->m_channels[m];
            CH->channel = m;
            CH->label = QString("%1").arg(m + 1);
        }
    }

    double view_t1 = d->m_view_agent->currentTimeRange().min;
    double view_t2 = d->m_view_agent->currentTimeRange().max;

    // Event markers
    QVector<double> times0;
    QVector<int> labels0;
    for (long i = 0; i < d->m_times.count(); i++) {
        double t0 = d->m_times[i];
        int l0 = d->m_labels[i];
        if ((view_t1 <= t0) && (t0 <= view_t2)) {
            times0 << t0;
            labels0 << l0;
        }
    }

    /// TODO add this to prefs
    double min_avg_pixels_per_marker = 10;
    if ((times0.count()) && (W0 / times0.count() >= min_avg_pixels_per_marker)) {
        d->paint_markers(&painter, times0, labels0, W0, H0);
    }
    else {
        if (times0.count()) {
            d->paint_message_at_top(&painter, "Zoom in to view markers", W0, H0);
        }
    }

    // Cursor
    d->paint_cursor(&painter, W0, H0);

    double WW = W0 - mleft - mright;
    double HH = H0 - mtop - mbottom;
    QImage img = d->m_render_manager.getImage(view_t1, view_t2, d->m_amplitude_factor, WW, HH);
    painter.drawImage(mleft, mtop, img);

    // Time axis
    d->paint_time_axis(&painter, W0, H0);

    // Channel labels
    d->paint_channel_labels(&painter, W0, H0);

    // Status
    {
        QString str;
        if (d->m_samplerate) {
            str = QString("%1 (tp: %2)").arg(d->format_time(d->m_view_agent->currentTimepoint())).arg((long)d->m_view_agent->currentTimepoint());
        }
        else {
            str = QString("Sample rate is null (tp: %2)").arg((long)d->m_view_agent->currentTimepoint());
        }
        d->paint_status_string(&painter, W0, H0, str);
    }
}

void MVTimeSeriesView::mousePressEvent(QMouseEvent* evt)
{
    if (evt->button() == Qt::LeftButton) {
        d->m_left_click_anchor_pix = evt->pos();
        d->m_left_click_anchor_t_range = this->timeRange();
        d->m_left_click_anchor_coord = d->pix2coord(0, evt->pos());
        d->m_left_click_dragging = false;
        emit clicked();
    }
    d->update_cursor();
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
    d->update_cursor();
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

    d->update_cursor();
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

void MVTimeSeriesView::keyPressEvent(QKeyEvent* evt)
{
    if (evt->key() == Qt::Key_Up) {
        d->m_amplitude_factor *= 1.2;
        update();
    }
    else if (evt->key() == Qt::Key_Down) {
        d->m_amplitude_factor /= 1.2;
        update();
    }
    else if (evt->key() == Qt::Key_A) {
        autoSetAmplitudeFactorWithinTimeRange();
    }
    else if (evt->key() == Qt::Key_Left) {
        MVRange trange = this->timeRange();
        double range = trange.max - trange.min;
        this->setCurrentTimepoint(this->currentTimepoint() - range / 10);
        d->scroll_to_current_timepoint();
    }
    else if (evt->key() == Qt::Key_Right) {
        MVRange trange = this->timeRange();
        double range = trange.max - trange.min;
        this->setCurrentTimepoint(this->currentTimepoint() + range / 10);
        d->scroll_to_current_timepoint();
    }
    else if (evt->key() == Qt::Key_Home) {
        this->setCurrentTimepoint(0);
        d->scroll_to_current_timepoint();
    }
    else if (evt->key() == Qt::Key_End) {
        this->setCurrentTimepoint(d->m_timeseries.N2() - 1);
        d->scroll_to_current_timepoint();
    }
    else if (evt->key() == Qt::Key_Equal) {
        d->zoom_in(mvtsv_coord::from_t(this->currentTimepoint()));
    }
    else if (evt->key() == Qt::Key_Minus) {
        d->zoom_out(mvtsv_coord::from_t(this->currentTimepoint()));
    }
    else {
        QWidget::keyPressEvent(evt);
    }
}

void MVTimeSeriesView::unit_test()
{

    /*
    DiskReadMda X1("/home/magland/sorting_results/axellab/datafile001_datafile002_66_mn_butter_500-6000_trimmin80/pre2.mda");
    DiskReadMda X2("http://datalaboratory.org:8020/mdaserver/axellab/datafile001_datafile002_66_mn_butter_500-6000_trimmin80/pre2.mda");

    Mda A1,A2;
    long index=8e7+1;
    X1.readChunk(A1,0,index,X1.N1(),1);
    X2.readChunk(A2,0,index,X2.N1(),1);

    A1.write32("/home/magland/tmp/A1.mda");
    A2.write32("/home/magland/tmp/A2.mda");
    return;

    */

    /*
    long M = 40;
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
    */

    //DiskReadMda X0("/home/magland/sorting_results/franklab/results/ex001_20160424/pre2.mda");

    QString proxy_url = "http://datalaboratory.org:8020";
    //DiskReadMda X0("http://datalaboratory.org:8020/mdaserver/franklab/results/ex001_20160424/pre2.mda");
    DiskReadMda X0("http://datalaboratory.org:8020/mdaserver/axellab/datafile001_datafile002_66_mn_butter_500-6000_trimmin80/pre2.mda");
    //DiskReadMda X0("/home/magland/sorting_results/axellab/datafile001_datafile002_66_mn_butter_500-6000_trimmin80/pre2.mda");

    MVTimeSeriesView* W = new MVTimeSeriesView(new MVViewAgent); //note that the view agent does not get deleted. :(
    W->setTimeseries(X0);
    W->setMLProxyUrl(proxy_url);
    //W->setTimeRange(MVRange(0, X0.N2()-1));
    W->setTimeRange(MVRange(0, 1000));
    W->show();
}

void MVTimeSeriesView::slot_scroll_to_current_timepoint()
{
    d->scroll_to_current_timepoint();
}

QList<mvtsv_channel> MVTimeSeriesViewPrivate::make_channel_layout(double W, double H, long M)
{
    QList<mvtsv_channel> channels;
    if (!M)
        return channels;
    double mleft = m_prefs.mleft;
    double mright = m_prefs.mright;
    double mtop = m_prefs.mtop;
    double mbottom = m_prefs.mbottom;
    double space = 0;
    double channel_height = (H - mbottom - mtop - (M - 1) * space) / M;
    double y0 = mtop;
    for (int m = 0; m < M; m++) {
        mvtsv_channel X;
        X.geometry = QRectF(mleft, y0, W - mleft - mright, channel_height);
        channels << X;
        y0 += channel_height + space;
    }
    return channels;
}

void MVTimeSeriesViewPrivate::paint_cursor(QPainter* painter, double W, double H)
{
    Q_UNUSED(W)
    Q_UNUSED(H)

    double mtop = m_prefs.mtop;
    double mbottom = m_prefs.mbottom;

    if (m_selected_t_range.min < 0) {
        QPointF p0 = coord2pix(mvtsv_coord(0, m_view_agent->currentTimepoint(), 0));
        QPointF p1 = coord2pix(mvtsv_coord(0, m_view_agent->currentTimepoint(), 0));
        p0.setY(mtop);
        p1.setY(H - mbottom);

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
            QColor col = QColor(50, 50, 220);
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
        p0.setY(mtop);
        p1.setY(H - mbottom);

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

struct MarkerRecord {
    double xpix;
    int label;
    int level;
};

struct MarkerRecord_comparer {
    bool operator()(const MarkerRecord& a, const MarkerRecord& b) const
    {
        if (a.xpix < b.xpix)
            return true;
        else if (a.xpix == b.xpix)
            return (a.level < b.level);
        else
            return false;
    }
};

void sort_by_xpix(QList<MarkerRecord>& records)
{
    qSort(records.begin(), records.end(), MarkerRecord_comparer());
}

void MVTimeSeriesViewPrivate::paint_markers(QPainter* painter, const QVector<double>& times, const QVector<int>& labels, double W, double H)
{
    Q_UNUSED(W)
    double mtop = m_prefs.mtop;
    double mbottom = m_prefs.mbottom;

    QList<MarkerRecord> marker_recs;

    int min_dist = 20;

    for (long i = 0; i < times.count(); i++) {
        double t0 = times[i];
        int l0 = labels[i];
        QPointF p0 = coord2pix(mvtsv_coord(0, t0, 0));
        MarkerRecord MR;
        MR.xpix = p0.x();
        MR.label = l0;
        MR.level = 0;
        marker_recs << MR;
    }
    sort_by_xpix(marker_recs);
    for (long i = 1; i < marker_recs.count(); i++) {
        if (marker_recs[i - 1].xpix + min_dist >= marker_recs[i].xpix) {
            marker_recs[i].level = (marker_recs[i - 1].level + 1) % m_prefs.num_label_levels;
        }
    }
    QPen pen = painter->pen();
    pen.setColor(m_prefs.marker_color);
    painter->setPen(pen);
    QFont font = painter->font();
    font.setPixelSize(m_prefs.label_font_height);
    painter->setFont(font);
    for (long i = 0; i < marker_recs.count(); i++) {
        MarkerRecord MR = marker_recs[i];
        QPointF p0(MR.xpix, mtop);
        QPointF p1(MR.xpix, H - mbottom);
        painter->drawLine(p0, p1);
        QRectF rect(MR.xpix - 30, mtop - 3 - m_prefs.label_font_height * (MR.level + 1), 60, m_prefs.label_font_height);
        painter->drawText(rect, Qt::AlignCenter | Qt::AlignVCenter, QString("%1").arg(MR.label));
    }
}

void MVTimeSeriesViewPrivate::paint_message_at_top(QPainter* painter, QString msg, double W, double H)
{
    Q_UNUSED(H)
    QPen pen = painter->pen();
    pen.setColor(m_prefs.marker_color);
    painter->setPen(pen);
    QFont font = painter->font();
    font.setPixelSize(m_prefs.label_font_height);
    painter->setFont(font);

    QRectF rect(0, 0, W, m_prefs.mtop);
    painter->drawText(rect, Qt::AlignCenter | Qt::AlignVCenter, msg);
}

void MVTimeSeriesViewPrivate::paint_time_axis(QPainter* painter, double W, double H)
{
    double samplerate = m_samplerate;
    long min_pixel_spacing_between_ticks = 30;

    double view_t1 = m_view_agent->currentTimeRange().min;
    double view_t2 = m_view_agent->currentTimeRange().max;

    QPen pen = painter->pen();
    pen.setColor(Qt::black);
    painter->setPen(pen);

    QPointF pt1(m_prefs.mleft, H - m_prefs.mbottom);
    QPointF pt2(W - m_prefs.mright, H - m_prefs.mbottom);
    painter->drawLine(pt1, pt2);

    QList<TickStruct> structs;

    structs << TickStruct("1 ms", min_pixel_spacing_between_ticks, 4, 1e-3 * samplerate);
    structs << TickStruct("10 ms", min_pixel_spacing_between_ticks, 6, 10 * 1e-3 * samplerate);
    structs << TickStruct("100 ms", min_pixel_spacing_between_ticks, 8, 100 * 1e-3 * samplerate);
    structs << TickStruct("1 s", min_pixel_spacing_between_ticks, 10, 1 * samplerate);
    structs << TickStruct("10 s", min_pixel_spacing_between_ticks, 12, 10 * samplerate);
    structs << TickStruct("1 m", min_pixel_spacing_between_ticks, 14, 60 * samplerate);
    structs << TickStruct("10 m", min_pixel_spacing_between_ticks, 16, 10 * 60 * samplerate);
    structs << TickStruct("1 h", min_pixel_spacing_between_ticks, 18, 60 * 60 * samplerate);
    structs << TickStruct("1 day", min_pixel_spacing_between_ticks, 20, 24 * 60 * 60 * samplerate);

    for (int i = 0; i < structs.count(); i++) {
        double scale_pixel_width = W / (view_t2 - view_t1) * structs[i].timepoint_interval;
        if ((scale_pixel_width >= 60) && (!structs[i].str.isEmpty())) {
            structs[i].show_scale = true;
            break;
        }
    }

    for (int i = 0; i < structs.count(); i++) {
        paint_time_axis_unit(painter, W, H, structs[i]);
    }
}

/// TODO, change W,H to size throughout
void MVTimeSeriesViewPrivate::paint_time_axis_unit(QPainter* painter, double W, double H, TickStruct TS)
{
    Q_UNUSED(W)

    double view_t1 = m_view_agent->currentTimeRange().min;
    double view_t2 = m_view_agent->currentTimeRange().max;

    double pixel_interval = W / (view_t2 - view_t1) * TS.timepoint_interval;

    if (pixel_interval >= TS.min_pixel_spacing_between_ticks) {
        long i1 = (long)ceil(view_t1 / TS.timepoint_interval);
        long i2 = (long)floor(view_t2 / TS.timepoint_interval);
        for (long i = i1; i <= i2; i++) {
            QPointF p1 = coord2pix(mvtsv_coord(0, i * TS.timepoint_interval, 0));
            p1.setY(H - m_prefs.mbottom);
            QPointF p2 = p1;
            p2.setY(H - m_prefs.mbottom + TS.tick_height);
            painter->drawLine(p1, p2);
        }
    }
    if (TS.show_scale) {
        int label_height = 10;
        long j1 = view_t1 + 1;
        if (j1 < 1)
            j1 = 1;
        long j2 = j1 + TS.timepoint_interval;
        QPointF p1 = coord2pix(mvtsv_coord(0, j1, 0));
        QPointF p2 = coord2pix(mvtsv_coord(0, j2, 0));
        p1.setY(H - m_prefs.mbottom + TS.tick_height);
        p2.setY(H - m_prefs.mbottom + TS.tick_height);

        painter->drawLine(p1, p2);

        QRectF rect(p1.x(), p1.y() + 5, p2.x() - p1.x(), label_height);
        QFont font = painter->font();
        font.setPixelSize(label_height);
        painter->drawText(rect, Qt::AlignCenter | Qt::AlignVCenter, TS.str);
    }
}

void MVTimeSeriesViewPrivate::paint_channel_labels(QPainter* painter, double W, double H)
{
    Q_UNUSED(W)
    Q_UNUSED(H)
    QPen pen = painter->pen();
    pen.setColor(Qt::black);
    painter->setPen(pen);

    QFont font = painter->font();
    font.setPixelSize(13);
    painter->setFont(font);

    long M = m_timeseries.N1();
    for (int m = 0; m < M; m++) {
        QPointF pt = coord2pix(mvtsv_coord(m, 0, 0));
        QRectF rect(0, pt.y() - 30, m_prefs.mleft - 5, 60);
        QString str = QString("%1").arg(m + 1);
        painter->drawText(rect, Qt::AlignRight | Qt::AlignVCenter, str);
    }
}

void MVTimeSeriesViewPrivate::paint_status_string(QPainter* painter, double W, double H, QString str)
{
    QPen pen = painter->pen();
    pen.setColor(Qt::black);
    painter->setPen(pen);
    double status_height = 12;
    double voffset = 4;
    QRectF rect(m_prefs.mleft, H - voffset - status_height, W - m_prefs.mleft - m_prefs.mright, status_height);
    painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, str);
}

QPointF MVTimeSeriesViewPrivate::coord2pix(mvtsv_coord C)
{
    if (C.channel >= m_channels.count())
        return QPointF(0, 0);

    double view_t1 = m_view_agent->currentTimeRange().min;
    double view_t2 = m_view_agent->currentTimeRange().max;

    if (view_t2 <= view_t1)
        return QPointF(0, 0);

    mvtsv_channel* CH = &m_channels[C.channel];

    double xpct = (C.t - view_t1) / (view_t2 - view_t1);
    double px = CH->geometry.x() + xpct * CH->geometry.width();
    double py = CH->geometry.y() + CH->geometry.height() / 2 + CH->geometry.height() / 2 * (C.y * m_amplitude_factor);
    return QPointF(px, py);
}

mvtsv_coord MVTimeSeriesViewPrivate::pix2coord(long channel, QPointF pix)
{
    if (channel >= m_channels.count())
        return mvtsv_coord(0, 0, 0);

    double view_t1 = m_view_agent->currentTimeRange().min;
    double view_t2 = m_view_agent->currentTimeRange().max;

    mvtsv_channel* CH = &m_channels[channel];

    mvtsv_coord C;
    double xpct = 0;
    if (CH->geometry.width()) {
        xpct = (pix.x() - CH->geometry.x()) / (CH->geometry.width());
    }
    C.t = view_t1 + xpct * (view_t2 - view_t1);
    if (m_amplitude_factor) {
        C.y = (pix.y() - (CH->geometry.y() + CH->geometry.height() / 2)) / m_amplitude_factor / (CH->geometry.height() / 2);
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

void MVTimeSeriesViewPrivate::scroll_to_current_timepoint()
{
    double t = q->currentTimepoint();
    MVRange trange = q->timeRange();
    if ((trange.min < t) && (t < trange.max))
        return;
    double range = trange.max - trange.min;
    if (t < trange.min) {
        q->setTimeRange(trange + (t - trange.min - range / 10));
    }
    else {
        q->setTimeRange(trange + (t - trange.max + range / 10));
    }
}

QString MVTimeSeriesViewPrivate::format_time(double tp)
{
    double samplerate = m_samplerate;
    double sec = tp / samplerate;
    long day = (long)floor(sec / (24 * 60 * 60));
    sec -= day * 24 * 60 * 60;
    long hour = (long)floor(sec / (60 * 60));
    sec -= hour * 60 * 60;
    long minute = (long)floor(sec / (60));
    sec -= minute * 60;

    QString str;
    if (day)
        str += QString("%1 days ").arg(day);
    QString tmp_sec = QString("%1").arg(sec);
    if (sec < 10) {
        tmp_sec = QString("0%1").arg(sec);
    }
    str += QString("%1:%2:%3").arg(hour).arg(minute, 2, 10, QChar('0')).arg(tmp_sec);

    return str;
}

void MVTimeSeriesViewPrivate::update_cursor()
{
    if (m_left_click_dragging) {
        q->setCursor(Qt::OpenHandCursor);
    }
    else {
        q->setCursor(Qt::ArrowCursor);
    }
}
