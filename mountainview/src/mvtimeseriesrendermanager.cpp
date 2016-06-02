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

#define PANEL_WIDTH 1200
#define PANEL_HEIGHT 1200
#define PANEL_NUM_POINTS 300

struct ImagePanel {
    long ds_factor;
    long index;
    double amp_factor;
    QImage image;
    QString make_code();
};

/// TODO stop threads when no longer needed

class MVTimeSeriesRenderManagerPrivate {
public:
    MVTimeSeriesRenderManager* q;
    MultiScaleTimeSeries* m_ts;
    QMap<QString, ImagePanel> m_image_panels;
    QSet<QString> m_running_panel_codes;
    ThreadManager m_thread_manager;

    QImage render_panel(ImagePanel p);
    void start_compute_panel(double amp_factor, long ds_factor, long t_index);
    ImagePanel* closest_ancestor_panel(ImagePanel p);
};

MVTimeSeriesRenderManager::MVTimeSeriesRenderManager()
{
    d = new MVTimeSeriesRenderManagerPrivate;
    d->q = this;
    d->m_ts = 0;
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

QImage MVTimeSeriesRenderManager::getImage(double t1, double t2, double amp_factor, double W, double H)
{
    if (!d->m_ts)
        return QImage();

    QImage ret(W, H, QImage::Format_ARGB32);
    QColor transparent(0, 0, 0, 0);
    ret.fill(transparent);
    QPainter painter(&ret);

    long ds_factor = 1;
    //we want no more than 3 panels
    while ((t2 - t1) / ds_factor / PANEL_NUM_POINTS > 3)
        ds_factor *= 3;

    long ind1 = (long)(t1 / (ds_factor * PANEL_NUM_POINTS));
    long ind2 = (long)(t2 / (ds_factor * PANEL_NUM_POINTS));
    for (long iii = ind1; iii <= ind2; iii++) {
        ImagePanel p;
        p.amp_factor = amp_factor;
        p.ds_factor = ds_factor;
        p.index = iii;
        QImage img = d->render_panel(p);
        if (img.width()) {
            double a1 = (iii * PANEL_NUM_POINTS * ds_factor - t1) * 1.0 / (t2 - t1) * W;
            double a2 = ((iii + 1) * PANEL_NUM_POINTS * ds_factor - t1) * 1.0 / (t2 - t1) * W;
            painter.drawImage(a1, 0, img.scaled(a2 - a1, H, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        }
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
    d->m_image_panels[code] = p;
    d->m_image_panels[code].image = thread->image;
    d->m_running_panel_codes.remove(code);
    thread->deleteLater();
    emit updated();
}

QString ImagePanel::make_code()
{
    return QString("amp=%1.ds=%2.ind=%3").arg(this->amp_factor).arg(this->ds_factor).arg(this->index);
}

void MVTimeSeriesRenderManagerPrivate::start_compute_panel(double amp_factor, long ds_factor, long index)
{
    ImagePanel p;
    p.amp_factor = amp_factor;
    p.ds_factor = ds_factor;
    p.index = index;
    if (m_running_panel_codes.contains(p.make_code()))
        return;
    MVTimeSeriesRenderManagerThread* thread = new MVTimeSeriesRenderManagerThread;
    QObject::connect(thread, SIGNAL(finished()), q, SLOT(slot_thread_finished()));
    thread->amp_factor = p.amp_factor;
    thread->ds_factor = p.ds_factor;
    thread->index = p.index;
    thread->ts = m_ts;
    m_running_panel_codes.insert(p.make_code());
    m_thread_manager.start(thread);
}

ImagePanel* MVTimeSeriesRenderManagerPrivate::closest_ancestor_panel(ImagePanel p)
{
    QList<ImagePanel*> candidates;
    QStringList keys = m_image_panels.keys();
    foreach (QString key, keys) {
        ImagePanel* pp = &m_image_panels[key];
        if (pp->amp_factor == p.amp_factor) {
            double t1 = p.index * p.ds_factor * PANEL_NUM_POINTS;
            double t2 = (p.index + 1) * p.ds_factor * PANEL_NUM_POINTS;
            double s1 = pp->index * pp->ds_factor * PANEL_NUM_POINTS;
            double s2 = (pp->index+1) * pp->ds_factor * PANEL_NUM_POINTS;
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

void MVTimeSeriesRenderManagerThread::run()
{
    image = QImage(PANEL_WIDTH, PANEL_HEIGHT, QImage::Format_ARGB32);
    QColor transparent(0, 0, 0, 0);
    image.fill(transparent);

    if (!ts)
        return;
    long M = ts->N1();
    if (!M)
        return;

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    long t1 = index * PANEL_NUM_POINTS;
    long t2 = (index + 1) * PANEL_NUM_POINTS;

    Mda Xmin, Xmax;
    qDebug() << "getData:::::::::::::::::::::::::::" << t1 << t2 << ds_factor;
    ts->getData(Xmin, Xmax, t1, t2, ds_factor);
    qDebug() << Xmin.N1() << Xmin.N2();

    double space = 0;
    double channel_height = (PANEL_HEIGHT - (M - 1) * space) / M;
    long y0 = 0;
    for (int m = 0; m < M; m++) {
        QRectF geom(0, y0, PANEL_WIDTH, channel_height);
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

        y0 += channel_height + space;
    }
}

ThreadManager::ThreadManager()
{
    QTimer::singleShot(100, this, SLOT(slot_timer()));
}

void ThreadManager::start(QThread* thread)
{
    m_queued_threads << thread;
    QObject::connect(thread, SIGNAL(finished()), this, SLOT(slot_thread_finished()));
}

void ThreadManager::slot_timer()
{
    while ((m_running_threads.count() < 20) && (!m_queued_threads.isEmpty())) {
        QThread* T = m_queued_threads[0];
        m_queued_threads.removeFirst();
        m_running_threads.insert(T);
        T->start();
    }
    QTimer::singleShot(100, this, SLOT(slot_timer()));
}

void ThreadManager::slot_thread_finished()
{
    QThread* thread = qobject_cast<QThread*>(sender());
    if (!thread)
        return;
    m_running_threads.remove(thread);
}

QImage MVTimeSeriesRenderManagerPrivate::render_panel(ImagePanel p)
{
    QString code = p.make_code();
    if (m_image_panels.contains(code)) {
        return m_image_panels[code].image;
    }
    else {
        start_compute_panel(p.amp_factor, p.ds_factor, p.index);
        QImage ret;
        ImagePanel* p2 = closest_ancestor_panel(p);
        if (p2) {
            qDebug() << "Found ancestor panel:" << p.make_code() << p2->make_code();
            double s1 = p2->ds_factor * p2->index * PANEL_NUM_POINTS;
            double s2 = p2->ds_factor * (p2->index + 1) * PANEL_NUM_POINTS;
            double t1 = p.ds_factor * p.index * PANEL_NUM_POINTS;
            double t2 = p.ds_factor * (p.index + 1) * PANEL_NUM_POINTS;
            double a1 = (t1 - s1) / (s2 - s1) * p2->image.width();
            double a2 = (t2 - s1) / (s2 - s1) * p2->image.width();
            qDebug() << a1 << a2;
            qDebug() << s1 << t1 << t2 << s2;
            ret = p2->image.copy(a1, 0, a2 - a1, p2->image.height());
        }
        else {
            qDebug() << "Did not find ancestor panel" << p.make_code();
        }
        return ret;
    }
}
