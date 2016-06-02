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

struct ImageRecord {
    double t1, t2, amp_factor;
    double W, H;
    QImage image;
    QString make_code();
};

class MVTimeSeriesRenderManagerPrivate {
public:
    MVTimeSeriesRenderManager* q;
    MultiScaleTimeSeries* m_ts;
    QMap<QString, ImageRecord> m_image_records;
    QSet<QString> m_running_record_codes;
    ThreadManager m_thread_manager;

    long get_preferred_data_ds_factor(double t1, double t2, double amp_factor, double W, double H);
    ImageRecord find_closest_record_matching_t1t2amp(double t1, double t2, double amp_factor, double W, double H);
    ImageRecord find_closest_record_containing_t1t2amp(double t1, double t2, double amp_factor, double W, double H);
    void start_compute_image(ImageRecord rec);
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
    ImageRecord rec;
    rec.t1 = t1;
    rec.t2 = t2;
    rec.amp_factor = amp_factor;
    rec.W = W;
    rec.H = H;
    QString code = rec.make_code();
    if (d->m_image_records.contains(code)) {
        return d->m_image_records[code].image;
    } else {
        ImageRecord rec0 = d->find_closest_record_matching_t1t2amp(t1, t2, amp_factor, W, H);
        if (rec0.W) {
            d->start_compute_image(rec);
            return rec0.image.scaled(W, H, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        } else {
            rec0 = d->find_closest_record_containing_t1t2amp(t1, t2, amp_factor, W, H);
            if (rec0.W) {
                QImage img = rec0.image;
                double a1 = (t1 - rec0.t1) / (rec0.t2 - rec0.t1) * rec0.W;
                double a2 = (t2 - rec0.t1) / (rec0.t2 - rec0.t1) * rec0.W;
                img = img.copy(a1, 0, a2 - a1, rec0.H);
                d->start_compute_image(rec);
                return img.scaled(W, H, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            } else {
                ImageRecord rec1 = rec;
                d->start_compute_image(rec);
                return QImage();
            }
        }
    }
}

void MVTimeSeriesRenderManager::slot_thread_finished()
{
    MVTimeSeriesRenderManagerThread* thread = qobject_cast<MVTimeSeriesRenderManagerThread*>(sender());
    if (!thread)
        return;
    ImageRecord rec;
    rec.t1 = thread->t1;
    rec.t2 = thread->t2;
    rec.amp_factor = thread->amp_factor;
    rec.W = thread->W;
    rec.H = thread->H;
    QString code = rec.make_code();
    d->m_image_records[code] = rec;
    d->m_image_records[code].image = thread->image;
    d->m_running_record_codes.remove(code);
    thread->deleteLater();
    emit updated();
}

QString ImageRecord::make_code()
{
    return QString("t1=%1.t2=%2.amp=%3.W=%4.H=%5").arg(this->t1).arg(this->t2).arg(this->amp_factor).arg(this->W).arg(this->H);
}

long MVTimeSeriesRenderManagerPrivate::get_preferred_data_ds_factor(double t1, double t2, double amp_factor, double W, double H)
{
    Q_UNUSED(H)
    Q_UNUSED(amp_factor)
    //we want one timepoint per pixel
    double timepoints_per_pixel = (t2 - t1) / W;
    if (!timepoints_per_pixel)
        return 1;
    return MultiScaleTimeSeries::smallest_power_of_3_larger_than(timepoints_per_pixel / 3); //err on the side of less downsampling
}

ImageRecord MVTimeSeriesRenderManagerPrivate::find_closest_record_matching_t1t2amp(double t1, double t2, double amp_factor, double W, double H)
{
    QList<ImageRecord*> candidates;
    QStringList keys = m_image_records.keys();
    foreach(QString key, keys)
    {
        ImageRecord* R = &m_image_records[key];
        if ((R->t1 == t1) && (R->t2 == t2) && (R->amp_factor == amp_factor)) {
            candidates << R;
        }
    }
    if (candidates.isEmpty()) {
        ImageRecord ret;
        ret.t1 = ret.t2 = ret.amp_factor = ret.W = ret.H = 0;
        return ret;
    } else {
        double best_dist = 0;
        ImageRecord* best;
        foreach(ImageRecord * C, candidates)
        {
            double dist = qMax(qAbs(C->W - W), qAbs(C->H - H));
            if ((!best_dist) || (dist < best_dist)) {
                best = C;
                best_dist = dist;
            }
        }
        return *best;
    }
}

ImageRecord MVTimeSeriesRenderManagerPrivate::find_closest_record_containing_t1t2amp(double t1, double t2, double amp_factor, double W, double H)
{
    QList<ImageRecord*> candidates;
    QStringList keys = m_image_records.keys();
    foreach(QString key, keys)
    {
        ImageRecord* R = &m_image_records[key];
        if ((R->t1 <= t1) && (R->t2 >= t2) && (R->amp_factor == amp_factor)) {
            candidates << R;
        }
    }
    if (candidates.isEmpty()) {
        ImageRecord ret;
        ret.t1 = ret.t2 = ret.amp_factor = ret.W = ret.H = 0;
        return ret;
    } else {
        double best_dist = 0;
        ImageRecord* best;
        foreach(ImageRecord * C, candidates)
        {
            double a1 = (t1 - C->t1) / (C->t2 - C->t1) * C->W;
            double a2 = (t2 - C->t1) / (C->t2 - C->t1) * C->W;
            double dist = qMax(qAbs((a2 - a1) - W), qAbs(C->H - H));
            if ((!best_dist) || (dist < best_dist)) {
                best = C;
                best_dist = dist;
            }
        }
        return *best;
    }
}

void MVTimeSeriesRenderManagerPrivate::start_compute_image(ImageRecord rec)
{
    if (m_running_record_codes.contains(rec.make_code()))
        return;
    MVTimeSeriesRenderManagerThread* thread = new MVTimeSeriesRenderManagerThread;
    QObject::connect(thread, SIGNAL(finished()), q, SLOT(slot_thread_finished()));
    thread->t1 = rec.t1;
    thread->t2 = rec.t2;
    thread->amp_factor = rec.amp_factor;
    thread->W = rec.W;
    thread->H = rec.H;
    thread->data_ds_factor = get_preferred_data_ds_factor(rec.t1, rec.t2, rec.amp_factor, rec.W, rec.H);
    thread->ts = m_ts;
    m_running_record_codes.insert(rec.make_code());
    m_thread_manager.start(thread);
}

void MVTimeSeriesRenderManagerThread::run()
{
    image = QImage(W, H, QImage::Format_ARGB32);
    QColor transparent(0, 0, 0, 0);
    image.fill(transparent);

    if (!ts)
        return;
    long M = ts->N1();
    if (!M)
        return;

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    long t1i = (long)(t1 / data_ds_factor);
    long t2i = (long)(t2 / data_ds_factor);

    Mda Xmin, Xmax;
    ts->getData(Xmin, Xmax, t1i, t2i, data_ds_factor);

    double space = 0;
    double channel_height = (H - (M - 1) * space) / M;
    long y0 = 0;
    for (int m = 0; m < M; m++) {
        QRectF geom(0, y0, W, channel_height);
        QPainterPath path;
        for (long ii = t1i; ii <= t2i; ii++) {
            double val_min = Xmin.value(m, ii - t1i);
            double val_max = Xmax.value(m, ii - t1i);
            double pctx = (ii * data_ds_factor - t1) / (t2 - t1);
            double pcty_min = 1 - (val_min * amp_factor + 1) / 2;
            double pcty_max = 1 - (val_max * amp_factor + 1) / 2;
            QPointF pt_min = QPointF(geom.x() + pctx * geom.width(), geom.y() + pcty_min * geom.height());
            QPointF pt_max = QPointF(geom.x() + pctx * geom.width(), geom.y() + pcty_max * geom.height());
            if (ii == t1i)
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
