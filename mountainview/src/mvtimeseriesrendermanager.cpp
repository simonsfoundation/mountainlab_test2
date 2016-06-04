/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/1/2016
*******************************************************/

#include "mvtimeseriesrendermanager.h"

#include <QImage>
#include <QPainter>
#include <QThread>
#include <QTimer>
#include <QCoreApplication>

#define PANEL_NUM_POINTS 1200
#define PANEL_WIDTH PANEL_NUM_POINTS * 2
#define PANEL_HEIGHT_PER_CHANNEL 30
#define MIN_PANEL_HEIGHT 600
#define MAX_PANEL_HEIGHT 1800
#define PANEL_HEIGHT(M) (long) qMin(MAX_PANEL_HEIGHT * 1.0, qMax(MIN_PANEL_HEIGHT * 1.0, PANEL_HEIGHT_PER_CHANNEL* M * 1.0))

#define MAX_NUM_IMAGE_PIXELS 50 * 1e6

struct ImagePanel {
    long ds_factor;
    long index;
    double amp_factor;
    QImage image;
    QString make_code();
};

class MVTimeSeriesRenderManagerPrivate {
public:
    MVTimeSeriesRenderManager* q;
    MultiScaleTimeSeries* m_ts;
    QMap<QString, ImagePanel> m_image_panels;
    QSet<QString> m_running_panel_codes;
    ThreadManager m_thread_manager;
    double m_total_num_image_pixels;
    QList<QColor> m_channel_colors;

    QImage render_panel(ImagePanel p);
    void start_compute_panel(ImagePanel p);
    void stop_compute_panel(const QString& code);
    ImagePanel* closest_ancestor_panel(ImagePanel p);
    void cleanup_images(double t1, double t2, double amp_factor);
};

MVTimeSeriesRenderManager::MVTimeSeriesRenderManager()
{
    d = new MVTimeSeriesRenderManagerPrivate;
    d->q = this;
    d->m_ts = 0;
    d->m_total_num_image_pixels = 0;
}

MVTimeSeriesRenderManager::~MVTimeSeriesRenderManager()
{
    delete d;
}

void MVTimeSeriesRenderManager::setMultiScaleTimeSeries(MultiScaleTimeSeries* ts)
{
    d->m_ts = ts;
    /// TODO clear all the records and stop all the running threads
}

void MVTimeSeriesRenderManager::setChannelColors(const QList<QColor>& colors)
{
    d->m_channel_colors = colors;
}

QImage MVTimeSeriesRenderManager::getImage(double t1, double t2, double amp_factor, double W, double H)
{
    if (!d->m_ts)
        return QImage();

    QImage ret(W, H, QImage::Format_ARGB32);
    QColor transparent(0, 0, 0, 6);
    ret.fill(transparent);
    QPainter painter(&ret);

    long ds_factor = 1;
    //points per pixel should be around 1
    while ((t2 - t1) / ds_factor / W > 3) {
        ds_factor *= 3;
    }

    QSet<QString> panel_codes_needed;
    QList<ImagePanel> panels_to_start;

    long ind1 = (long)(t1 / (ds_factor * PANEL_NUM_POINTS));
    long ind2 = (long)(t2 / (ds_factor * PANEL_NUM_POINTS));
    for (long iii = ind1; iii <= ind2; iii++) {
        ImagePanel p;
        p.amp_factor = amp_factor;
        p.ds_factor = ds_factor;
        p.index = iii;
        panel_codes_needed.insert(p.make_code());
        if (!d->m_image_panels.contains(p.make_code())) {
            if (!d->m_running_panel_codes.contains(p.make_code())) {
                panels_to_start << p;
            }
        }
        QImage img = d->render_panel(p);
        if (img.width()) {
            double a1 = (iii * PANEL_NUM_POINTS * ds_factor - t1) * 1.0 / (t2 - t1) * W;
            double a2 = ((iii + 1) * PANEL_NUM_POINTS * ds_factor - t1) * 1.0 / (t2 - t1) * W;
            painter.drawImage(a1, 0, img.scaled(a2 - a1, H, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        }
    }

    //stop the threads that aren't needed
    foreach(QString code, d->m_running_panel_codes)
    {
        if (!panel_codes_needed.contains(code)) {
            d->stop_compute_panel(code);
        }
    }

    for (int i = 0; i < panels_to_start.count(); i++) {
        d->start_compute_panel(panels_to_start[i]);
    }

    if (d->m_total_num_image_pixels > MAX_NUM_IMAGE_PIXELS) {
        d->cleanup_images(t1, t2, amp_factor);
    }

    return ret;
}

void MVTimeSeriesRenderManager::slot_thread_finished()
{
    MVTimeSeriesRenderManagerThread* thread = qobject_cast<MVTimeSeriesRenderManagerThread*>(sender());
    if (!thread)
        return;
    ImagePanel p;
    p.amp_factor = thread->amp_factor;
    p.ds_factor = thread->ds_factor;
    p.index = thread->index;
    QString code = p.make_code();
    d->m_running_panel_codes.remove(code);

    if (thread->image.width()) {
        d->m_image_panels[code] = p;
        d->m_image_panels[code].image = thread->image;
        d->m_total_num_image_pixels += thread->image.width() * thread->image.height();
        emit updated();
    } else {
    }

    thread->deleteLater();
}

QString ImagePanel::make_code()
{
    return QString("amp=%1.ds=%2.ind=%3").arg(this->amp_factor).arg(this->ds_factor).arg(this->index);
}

void MVTimeSeriesRenderManagerPrivate::start_compute_panel(ImagePanel p)
{
    if (m_running_panel_codes.contains(p.make_code()))
        return;
    MVTimeSeriesRenderManagerThread* thread = new MVTimeSeriesRenderManagerThread;
    QObject::connect(thread, SIGNAL(finished()), q, SLOT(slot_thread_finished()));
    thread->amp_factor = p.amp_factor;
    thread->ds_factor = p.ds_factor;
    thread->index = p.index;
    thread->ts = m_ts;
    thread->channel_colors = m_channel_colors;
    m_running_panel_codes.insert(p.make_code());
    m_thread_manager.start(p.make_code(), thread);
}

void MVTimeSeriesRenderManagerPrivate::stop_compute_panel(const QString& code)
{
    m_running_panel_codes.remove(code);
    m_thread_manager.stop(code);
}

ImagePanel* MVTimeSeriesRenderManagerPrivate::closest_ancestor_panel(ImagePanel p)
{
    QList<ImagePanel*> candidates;
    QStringList keys = m_image_panels.keys();
    foreach(QString key, keys)
    {
        ImagePanel* pp = &m_image_panels[key];
        if (pp->amp_factor == p.amp_factor) {
            double t1 = p.index * p.ds_factor * PANEL_NUM_POINTS;
            double t2 = (p.index + 1) * p.ds_factor * PANEL_NUM_POINTS;
            double s1 = pp->index * pp->ds_factor * PANEL_NUM_POINTS;
            double s2 = (pp->index + 1) * pp->ds_factor * PANEL_NUM_POINTS;
            if ((s1 <= t1) && (t2 <= s2)) {
                candidates << pp;
            }
        }
    }
    if (candidates.isEmpty())
        return 0;
    ImagePanel* ret = candidates[0];
    long best_ds_factor = ret->ds_factor;
    for (int i = 0; i < candidates.count(); i++) {
        if (candidates[i]->ds_factor < best_ds_factor) {
            ret = candidates[i];
            best_ds_factor = ret->ds_factor;
        }
    }
    return ret;
}

void MVTimeSeriesRenderManagerPrivate::cleanup_images(double t1, double t2, double amp_factor)
{
    QStringList keys = m_image_panels.keys();
    foreach(QString key, keys)
    {
        ImagePanel* P = &m_image_panels[key];
        double s1 = P->index * P->ds_factor * PANEL_NUM_POINTS;
        double s2 = (P->index + 1) * P->ds_factor * PANEL_NUM_POINTS;
        if (((s2 < t1) || (s1 > t2)) || (P->amp_factor != amp_factor)) {
            //does not intersect or different amplitude
            m_total_num_image_pixels -= P->image.width() * P->image.height();
            m_image_panels.remove(key);
        }
    }
}

QColor MVTimeSeriesRenderManagerThread::get_channel_color(long m)
{
    if (channel_colors.isEmpty())
        return Qt::black;
    return channel_colors[m % channel_colors.count()];
}

void MVTimeSeriesRenderManagerThread::run()
{
    long M = ts->N1();
    if (!M)
        return;

    if (thread_interrupt_requested())
        return;

    QImage image0 = QImage(PANEL_WIDTH, PANEL_HEIGHT(M), QImage::Format_ARGB32);
    QColor transparent(0, 0, 0, 0);
    image0.fill(transparent);

    if (thread_interrupt_requested())
        return;

    if (!ts)
        return;

    if (thread_interrupt_requested())
        return;

    QPainter painter(&image0);
    painter.setRenderHint(QPainter::Antialiasing);

    long t1 = index * PANEL_NUM_POINTS;
    long t2 = (index + 1) * PANEL_NUM_POINTS;

    Mda Xmin, Xmax;
    ts->getData(Xmin, Xmax, t1, t2, ds_factor);

    if (thread_interrupt_requested())
        return;

    double space = 0;
    double channel_height = (PANEL_HEIGHT(M) - (M - 1) * space) / M;
    long y0 = 0;
    QPen pen = painter.pen();
    for (int m = 0; m < M; m++) {
        if (thread_interrupt_requested())
            return;
        pen.setColor(get_channel_color(m));
        painter.setPen(pen);
        QRectF geom(0, y0, PANEL_WIDTH, channel_height);
        /*
        QPainterPath path;
        for (long ii = t1; ii <= t2; ii++) {
            double val_min = Xmin.value(m, ii - t1);
            double val_max = Xmax.value(m, ii - t1);
            double pctx = (ii - t1) * 1.0 / (t2 - t1);
            double pcty_min = 1 - (val_min * amp_factor + 1) / 2;
            double pcty_max = 1 - (val_max * amp_factor + 1) / 2;
            QPointF pt_min = QPointF(geom.x() + pctx * geom.width(), geom.y() + pcty_min * geom.height());
            QPointF pt_max = QPointF(geom.x() + pctx * geom.width(), geom.y() + pcty_max * geom.height());
            if (ii == t1)
                path.moveTo(pt_min);
            else
                path.lineTo(pt_min);
            path.lineTo(pt_max);
        }
        painter.drawPath(path);
        */
        QPainterPath path;
        QPointF first;
        for (long ii = t1; ii <= t2; ii++) {
            double val_min = Xmin.value(m, ii - t1);
            double pctx = (ii - t1) * 1.0 / (t2 - t1);
            double pcty_min = 1 - (val_min * amp_factor + 1) / 2;
            QPointF pt_min = QPointF(geom.x() + pctx * geom.width(), geom.y() + pcty_min * geom.height());
            if (ii == t1) {
                first = pt_min;
                path.moveTo(pt_min);
            } else
                path.lineTo(pt_min);
        }
        for (long ii = t2; ii >= t1; ii--) {
            double val_max = Xmax.value(m, ii - t1);
            double pctx = (ii - t1) * 1.0 / (t2 - t1);
            double pcty_max = 1 - (val_max * amp_factor + 1) / 2;
            QPointF pt_max = QPointF(geom.x() + pctx * geom.width(), geom.y() + pcty_max * geom.height());
            path.lineTo(pt_max);
        }
        path.lineTo(first);
        painter.fillPath(path, painter.pen().color());

        if (thread_interrupt_requested())
            return;

        painter.drawPath(path);

        y0 += channel_height + space;
    }

    if (thread_interrupt_requested())
        return;
    image = image0; //only copy on successful exit
}

ThreadManager::ThreadManager()
{
    QTimer::singleShot(100, this, SLOT(slot_timer()));
}

ThreadManager::~ThreadManager()
{
    m_queued_threads.clear();
    QStringList keys=m_running_threads.keys();
    foreach (QString key,keys) {
        this->stop(key);
    }
    while (!m_running_threads.isEmpty()) {
        qApp->processEvents();
    }
}

void ThreadManager::start(QString id, QThread* thread)
{
    if (m_queued_threads.contains(id))
        return;
    m_queued_threads[id] = thread;
    thread->setProperty("threadmanager_id", id);
    QObject::connect(thread, SIGNAL(finished()), this, SLOT(slot_thread_finished()));
}

void ThreadManager::stop(QString id)
{
    if (m_queued_threads.contains(id))
        m_queued_threads.remove(id);
    if (m_running_threads.contains(id)) {
        m_running_threads[id]->requestInterruption();
    }
}

void ThreadManager::slot_timer()
{
    while ((m_running_threads.count() < 4) && (!m_queued_threads.isEmpty())) {
        QString key = m_queued_threads.keys().value(0);
        QThread* T = m_queued_threads[key];
        m_queued_threads.remove(key);
        m_running_threads[key] = T;
        T->start();
    }
    QTimer::singleShot(100, this, SLOT(slot_timer()));
}

void ThreadManager::slot_thread_finished()
{
    QThread* thread = qobject_cast<QThread*>(sender());
    if (!thread)
        return;
    m_running_threads.remove(thread->property("threadmanager_id").toString());
}

QImage MVTimeSeriesRenderManagerPrivate::render_panel(ImagePanel p)
{
    QString code = p.make_code();
    if (m_image_panels.contains(code)) {
        return m_image_panels[code].image;
    } else {
        QImage ret;
        ImagePanel* p2 = closest_ancestor_panel(p);
        if (p2) {
            double s1 = p2->ds_factor * p2->index * PANEL_NUM_POINTS;
            double s2 = p2->ds_factor * (p2->index + 1) * PANEL_NUM_POINTS;
            double t1 = p.ds_factor * p.index * PANEL_NUM_POINTS;
            double t2 = p.ds_factor * (p.index + 1) * PANEL_NUM_POINTS;
            double a1 = (t1 - s1) / (s2 - s1) * p2->image.width();
            double a2 = (t2 - s1) / (s2 - s1) * p2->image.width();
            ret = p2->image.copy(a1, 0, a2 - a1, p2->image.height());
            QPainter painter(&ret);
            QColor col(255, 0, 0, 6);
            painter.fillRect(0, 0, ret.width(), ret.height(), col);
        } else {
            ret = QImage(100, 100, QImage::Format_ARGB32);
            QColor col(0, 0, 0, 10);
            ret.fill(col);
        }
        return ret;
    }
}
