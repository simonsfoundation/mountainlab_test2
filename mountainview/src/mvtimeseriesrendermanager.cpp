/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/1/2016
*******************************************************/

#include "mvtimeseriesrendermanager.h"

#include <QImage>
#include <QPainter>
#include <QThread>

struct ImageRecord {
    double t1, t2, amp_factor;
    double W, H;
    long data_ds_factor;
    QImage image;
    QString make_code();
};

class MVTimeSeriesRenderManagerPrivate {
public:
    MVTimeSeriesRenderManager* q;
    MultiScaleTimeSeries* m_ts;
    QMap<QString, ImageRecord> m_image_records;
    MVTimeSeriesRenderManagerPrefs m_prefs;

    long get_preferred_data_ds_factor(double t1, double t2, double amp_factor, double W, double H);
    ImageRecord find_record_with_best_data_ds_factor(double t1, double t2, double amp_factor, double W, double H);
    ImageRecord find_closest_record_with_t1t2amp(double t1, double t2, double amp_factor, double W, double H);
    void start_compute_image(ImageRecord rec);
    void eliminate_obsolete_records(QString code);
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

void MVTimeSeriesRenderManager::setPrefs(MVTimeSeriesRenderManagerPrefs prefs)
{
    d->m_prefs = prefs;
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
    rec.data_ds_factor = d->get_preferred_data_ds_factor(t1, t2, amp_factor, W, H);
    QString code = rec.make_code();
    if (d->m_image_records.contains(code)) {
        return d->m_image_records[code].image;
    } else {
        ImageRecord rec0 = d->find_record_with_best_data_ds_factor(t1, t2, amp_factor, W, H);
        if (rec0.data_ds_factor) {
            ImageRecord rec1 = rec0;
            rec1.data_ds_factor = rec0.data_ds_factor / 3;
            d->start_compute_image(rec1);
            return rec0.image;
        } else {
            ImageRecord rec1;
            rec1.t1 = t1;
            rec1.t2 = t2;
            rec1.amp_factor = amp_factor;
            rec1.W = W;
            rec1.H = H;
            /// TODO optimize this choice
            rec1.data_ds_factor = rec.data_ds_factor * 27; //start with a low ds factor
            d->start_compute_image(rec1);

            rec0 = d->find_closest_record_with_t1t2amp(t1, t2, amp_factor, W, H);
            if (rec0.data_ds_factor) {
                return rec0.image.scaled(W, H, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            } else {
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
    rec.data_ds_factor = thread->data_ds_factor;
    qDebug() << "slot_thread_finished" << rec.make_code();
    QString code = rec.make_code();
    d->m_image_records[code] = rec;
    d->m_image_records[code].image = thread->image;
    d->eliminate_obsolete_records(code);
    thread->deleteLater();
    emit updated();
}

QString ImageRecord::make_code()
{
    return QString("%1.%2.%3.%4.%5.%6").arg(this->t1).arg(this->t2).arg(this->amp_factor).arg(this->W).arg(this->H).arg(this->data_ds_factor);
}

long MVTimeSeriesRenderManagerPrivate::get_preferred_data_ds_factor(double t1, double t2, double amp_factor, double W, double H)
{
    Q_UNUSED(H)
    Q_UNUSED(amp_factor)
    //we want one timepoint per pixel
    double timepoints_per_pixel = (t2 - t1) / W;
    if (!timepoints_per_pixel)
        return 1;
    return MultiScaleTimeSeries::smallest_power_of_3_larger_than((1 / timepoints_per_pixel) / 3); //err on the side of less downsampling
}

ImageRecord MVTimeSeriesRenderManagerPrivate::find_record_with_best_data_ds_factor(double t1, double t2, double amp_factor, double W, double H)
{
    long best_data_ds_factor = 0;
    ImageRecord* best_R = 0;
    QStringList keys = m_image_records.keys();
    foreach(QString key, keys)
    {
        ImageRecord* R = &m_image_records[key];
        if ((R->t1 == t1) && (R->t2 == t2) && (R->amp_factor == amp_factor) && (R->W == W) && (R->H == H)) {
            if ((!best_R) || (R->data_ds_factor < best_data_ds_factor)) {
                best_data_ds_factor = R->data_ds_factor;
                best_R = R;
            }
        }
    }
    if (best_R) {
        return *best_R;
    } else {
        ImageRecord ret;
        ret.t1 = ret.t2 = ret.amp_factor = ret.W = ret.H = 0;
        ret.data_ds_factor = 0;
        return ret;
    }
}

ImageRecord MVTimeSeriesRenderManagerPrivate::find_closest_record_with_t1t2amp(double t1, double t2, double amp_factor, double W, double H)
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
        ret.data_ds_factor = 0;
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

void MVTimeSeriesRenderManagerPrivate::start_compute_image(ImageRecord rec)
{
    qDebug() << "start_compute_image" << rec.make_code();
    MVTimeSeriesRenderManagerThread* thread = new MVTimeSeriesRenderManagerThread;
    QObject::connect(thread, SIGNAL(finished()), q, SLOT(slot_thread_finished()));
    thread->t1 = rec.t1;
    thread->t2 = rec.t2;
    thread->amp_factor = rec.amp_factor;
    thread->W = rec.W;
    thread->H = rec.H;
    thread->prefs = m_prefs;
    thread->data_ds_factor = rec.data_ds_factor;
    thread->ts = m_ts;
    thread->start();
}

void MVTimeSeriesRenderManagerPrivate::eliminate_obsolete_records(QString code)
{
    if (!m_image_records.contains(code))
        return;
    ImageRecord* R0 = &m_image_records[code];
    QStringList keys = m_image_records.keys();
    foreach(QString key, keys)
    {
        ImageRecord* R = &m_image_records[key];
        if ((R->t1 == R0->t1) && (R->t2 == R0->t2) && (R->amp_factor == R0->amp_factor) && (R->W == R0->W) && (R->H == R0->H)) {
            if (R->data_ds_factor > R0->data_ds_factor) {
                m_image_records.remove(key);
            }
        }
    }
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

    double mleft = this->prefs.margins[0];
    double mright = this->prefs.margins[1];
    double mtop = this->prefs.margins[2];
    double mbottom = this->prefs.margins[3];
    double space = this->prefs.space_between_channels;

    long t1i=(long)t1;
    long t2i=(long)t2;

    Mda Xmin, Xmax;
    ts->getData(Xmin, Xmax, t1i, t2i, data_ds_factor);

    double channel_height = (H - mtop - mbottom - (M - 1) * space) / M;
    long y0 = mtop;
    for (int m = 0; m < M; m++) {
        QRectF geom(mleft, y0, W - mleft - mright, channel_height);
        QPainterPath path;
        for (long t = t1i; t <= t2i; t++) {
            double val_min = Xmin.value(m, t - t1);
            double val_max = Xmax.value(m, t - t1);
            double pctx = (t - t1) / (t2 - t1);
            double pcty_min = 1 - (val_min * amp_factor + 1) / 2;
            double pcty_max = 1 - (val_max * amp_factor + 1) / 2;
            QPointF pt_min = QPointF(geom.x() + pctx * geom.width(), geom.y() + pcty_min * geom.height());
            QPointF pt_max = QPointF(geom.x() + pctx * geom.width(), geom.y() + pcty_max * geom.height());
            if (t == t1)
                path.moveTo(pt_min);
            else
                path.lineTo(pt_min);
            path.lineTo(pt_max);
        }
        painter.drawPath(path);

        y0 += channel_height + space;
    }
}

