/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/1/2016
*******************************************************/

#include "mvtimeseriesrendermanager.h"

#include <QImage>
#include <QThread>

struct ImageRecord {
    MVTimeSeriesRenderManagerInfo info;
    int ds_x, ds_y; //downsampling factors
    bool is_updating;
    QImage image;
};

class MVTimeSeriesRenderManagerPrivate {
public:
    MVTimeSeriesRenderManager* q;
    MultiScaleTimeSeries* m_ts;
    QMap<QString, ImageRecord> m_image_records;
    MVTimeSeriesRenderManagerInfo m_info;
    double m_margins[4];
    double m_space_between_channels;

    void update(QString code);
    void do_update(MVTimeSeriesRenderManagerInfo info, int ds_x, int ds_y);
};

MVTimeSeriesRenderManager::MVTimeSeriesRenderManager()
{
    d = new MVTimeSeriesRenderManagerPrivate;
    d->q = this;
    d->m_ts = 0;
    d->m_margins[0] = d->m_margins[1] = d->m_margins[2] = d->m_margins[3] = 30;
    d->m_space_between_channels = 10;
}

MVTimeSeriesRenderManager::~MVTimeSeriesRenderManager()
{
    delete d;
}

void MVTimeSeriesRenderManager::setMultiScaleTimeSeries(MultiScaleTimeSeries* ts)
{
    d->m_ts = ts;
}

void MVTimeSeriesRenderManager::setMargins(double left, double right, double top, double bottom)
{
    if ((left == d->m_margins[0]) && (right == d->m_margins[1]) && (top == d->m_margins[2]) && (bottom == d->m_margins[3]))
        return;
    d->m_margins[0] = left;
    d->m_margins[1] = right;
    d->m_margins[2] = top;
    d->m_margins[3] = bottom;
    d->m_image_records.clear();
}

void MVTimeSeriesRenderManager::setSpaceBetweenChannels(double pix)
{
    if (d->m_space_between_channels == pix)
        return;
    d->m_space_between_channels = pix;
    d->m_image_records.clear();
}

void MVTimeSeriesRenderManager::setInfo(MVTimeSeriesRenderManagerInfo info)
{
    d->m_info = info;
}

void MVTimeSeriesRenderManager::getImage(QImage& img0)
{
    QString code = d->m_info.make_code();
    if (d->m_image_records.contains(code)) {
        ImageRecord* R = &d->m_image_records[code];
        if ((R->ds_x != 1) || (R->ds_y != 1)) {
            if (!R->is_updating) {
                d->update(code);
            }
            //finish!!!
        }
        img0 = d->m_image_records[code].image;
    }
    else {
        /// TODO find the best image we can use right now
        ImageRecord R;
        R.info = d->m_info;
        R.ds_x = 0;
        R.ds_y = 0;
        R.is_updating = false;
        d->m_image_records[code] = R;
        d->update(code);
        img0 = QImage();
    }
}

void MVTimeSeriesRenderManager::slot_update_thread_finished()
{
    MVTimeSeriesRenderManagerUpdateThread* thread = qobject_cast<MVTimeSeriesRenderManagerUpdateThread*>(sender());
    if (!thread) {
        qWarning() << __FUNCTION__ << __FILE__ << __LINE__ << "Unexpected problem";
        return;
    }
    QString code = thread->info.make_code();
    if (d->m_image_records.contains(code)) {
        ImageRecord* R = &d->m_image_records[code];
        R->ds_x = thread->ds_x;
        R->ds_y = thread->ds_y;
        R->is_updating = false;
        R->image = thread->image;
    }
    thread->deleteLater();
    emit updated(); //should I always do this?
}

QString MVTimeSeriesRenderManagerInfo::make_code()
{
    return QString("").arg(this->W).arg(this->H).arg(this->t1).arg(this->t2);
}

void MVTimeSeriesRenderManagerPrivate::update(QString code)
{
    if (!m_image_records.contains(code))
        return;
    ImageRecord* R = &m_image_records[code];
    if ((R->ds_x == 1) && (R->ds_y == 1))
        return;
    if (R->is_updating)
        return;
    double ds_x = 1, ds_y = 1;
    if (R->ds_x == 0)
        ds_x = 4;
    else
        R->ds_x = R->ds_x / 2;
    if (R->ds_y == 0)
        ds_y = 4;
    else
        ds_y = R->ds_y / 4;
    R->is_updating = true;
    do_update(R->info, ds_x, ds_y);
}

void MVTimeSeriesRenderManagerPrivate::do_update(MVTimeSeriesRenderManagerInfo info, int ds_x, int ds_y)
{
    MVTimeSeriesRenderManagerUpdateThread* thread = new MVTimeSeriesRenderManagerUpdateThread;
    QObject::connect(thread, SIGNAL(finished), q, SLOT(slot_update_thread_finished()));
    thread->info = info;
    thread->ds_x = ds_x;
    thread->ds_y = ds_y;
    thread->start();
}
