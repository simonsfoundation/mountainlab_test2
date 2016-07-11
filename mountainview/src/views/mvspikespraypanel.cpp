#include "mvspikespraypanel.h"

#include <QList>
#include <QMutex>
#include <QPainter>
#include <QPen>
#include <QThread>
#include <QTime>
#include <QTimer>
#include "mda.h"
#include "mlcommon.h"

class MVSpikeSprayPanelPrivate {
public:
    MVSpikeSprayPanel* q;
    QSet<int> m_labels_to_use;
    MVContext* m_context;
    double m_amplitude_factor = 1;
    Mda* m_clips_to_render = 0;
    QVector<int> m_labels_to_render;
    bool m_legend_visible = true;
    MVSSRenderThread m_render_thread;
    bool m_start_render_required=false;

    QColor get_label_color(int label);
    void start_render();
};

MVSpikeSprayPanel::MVSpikeSprayPanel(MVContext* context)
{
    d = new MVSpikeSprayPanelPrivate;
    d->q = this;
    d->m_context = context;

    QObject::connect(&d->m_render_thread,SIGNAL(finished()),this,SLOT(update()));
    QObject::connect(&d->m_render_thread,SIGNAL(signalImageInProgressUpdated()),this,SLOT(update()));
}

MVSpikeSprayPanel::~MVSpikeSprayPanel()
{
    delete d;
}

void MVSpikeSprayPanel::setLabelsToUse(const QSet<int>& labels)
{
    d->m_labels_to_use = labels;
    d->m_start_render_required = true;
    update();
}

void MVSpikeSprayPanel::setClipsToRender(Mda* X)
{
    d->m_clips_to_render = X;
    d->m_start_render_required = true;
    update();
}

void MVSpikeSprayPanel::setLabelsToRender(const QVector<int>& X)
{
    d->m_labels_to_render = X;
    d->m_start_render_required = true;
    update();
}

void MVSpikeSprayPanel::setAmplitudeFactor(double X)
{
    d->m_amplitude_factor = X;
    d->m_start_render_required = true;
    update();
}

void MVSpikeSprayPanel::setLegendVisible(bool val)
{
    d->m_legend_visible = val;
}

void MVSpikeSprayPanel::paintEvent(QPaintEvent* evt)
{
    Q_UNUSED(evt)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    /// TODO (LOW) this should be a configured color to match the cluster view
    painter.fillRect(0, 0, width(), height(), QBrush(QColor(60, 60, 60)));

    if (d->m_start_render_required) {
        d->start_render();
    }
    {
        QImage img;
        {
            QMutexLocker locker(&d->m_render_thread.image_in_progress_mutex);
            img=d->m_render_thread.image_in_progress;
        }
        if (!img.isNull()) {
            QImage img_scaled=img.scaled(this->size(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
            painter.drawImage(0,0,img_scaled);
        }
    }

    //legend
    if (d->m_legend_visible) {
        double W = width();
        double spacing = 6;
        double margin = 10;
        QSet<int> labels_used = d->m_labels_to_use;
        QList<int> list = labels_used.toList();
        double text_height = qMax(12.0, qMin(25.0, W * 1.0 / 10));
        qSort(list);
        double y0 = margin;
        QFont font = painter.font();
        font.setPixelSize(text_height - 1);
        painter.setFont(font);
        for (int i = 0; i < list.count(); i++) {
            QRectF rect(0, y0, W - margin, text_height);
            QString str = QString("%1").arg(list[i]);
            QPen pen = painter.pen();
            pen.setColor(d->get_label_color(list[i]));
            painter.setPen(pen);
            painter.drawText(rect, Qt::AlignRight, str);
            y0 += text_height + spacing;
        }
    }

}

QColor MVSpikeSprayPanelPrivate::get_label_color(int label)
{
    return m_context->clusterColor(label);
}

void MVSpikeSprayPanelPrivate::start_render()
{
    m_start_render_required=false;
    if (!m_clips_to_render)
        return;
    if (m_clips_to_render->N3() != m_labels_to_render.count()) {
        qWarning() << "Number of clips to render does not match the number of labels to render" << m_clips_to_render->N3() << m_labels_to_render.count();
        return;
    }

    if (!m_amplitude_factor) {
        double maxval = qMax(qAbs(m_clips_to_render->minimum()), qAbs(m_clips_to_render->maximum()));
        if (maxval)
            m_amplitude_factor = 1.5 / maxval;
    }

    int K = MLCompute::max(m_labels_to_render);
    if (!K)
        return;
    int counts[K + 1];
    for (int k = 0; k < K + 1; k++)
        counts[k] = 0;
    QList<long> inds;
    for (long i = 0; i < m_labels_to_render.count(); i++) {
        int label0 = m_labels_to_render[i];
        if (label0 >= 0) {
            if (this->m_labels_to_use.contains(label0)) {
                counts[label0]++;
                inds << i;
            }
        }
    }
    int alphas[K + 1];
    for (int k = 0; k <= K; k++) {
        if (counts[k]) {
            alphas[k] = 255 / counts[k];
            alphas[k] = qMin(255, qMax(5, alphas[k]));
        } else
            alphas[k] = 255;
    }

    m_render_thread.requestInterruption();
    m_render_thread.wait();

    long M=m_clips_to_render->N1();
    long T=m_clips_to_render->N2();
    m_render_thread.clips.allocate(M,T,inds.count());
    m_render_thread.colors.clear();
    for (long j=0; j<inds.count(); j++) {
        int label0=m_labels_to_render[inds[j]];
        QColor col = get_label_color(label0);
        if (label0 >= 0) {
            col.setAlpha(alphas[label0]);
        }
        for (long t=0; t<T; t++) {
            for (long m=0; m<M; m++) {
                m_render_thread.clips.setValue(m_clips_to_render->value(m,t,inds[j]),m,t,j);
            }
        }
        m_render_thread.colors << col;
    }

    m_render_thread.amplitude_factor=m_amplitude_factor;
    m_render_thread.W=T*2;
    m_render_thread.H=500;

    m_render_thread.start();
}


void MVSSRenderThread::run()
{
    image=QImage(W,H,QImage::Format_ARGB32);
    QColor transparent(0,0,0,0);
    image.fill(transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    long M = clips.N1();
    long T = clips.N2();
    long L=clips.N3();
    if (L!=colors.count()) {
        qWarning() << "Unexpected sizes: " << colors.count() << L;
        return;
    }
    double* ptr = clips.dataPtr();
    QTime timer; timer.start();
    for (long i = 0; i < L; i++) {
        if (timer.elapsed()>300) {
            {
                QMutexLocker locker(&image_in_progress_mutex);
                image_in_progress=image;
                emit signalImageInProgressUpdated();
            }
            timer.restart();
        }
        if (this->isInterruptionRequested()) return;
        QColor col=colors[i];
        render_clip(&painter, M, T, &ptr[M * T * i], col);
    }
    {
        QMutexLocker locker(&image_in_progress_mutex);
        image_in_progress=image;
        emit signalImageInProgressUpdated();
    }
}

void MVSSRenderThread::render_clip(QPainter* painter, long M, long T, double* ptr, QColor col)
{
    QPen pen = painter->pen();
    pen.setColor(col);
    painter->setPen(pen);
    QPainterPath path;
    for (long m = 0; m < M; m++) {
        QPainterPath path;
        for (long t = 0; t < T; t++) {
            double val = ptr[m + M * t];
            QPointF pt = coord2pix(m, t, val);
            if (t == 0) {
                path.moveTo(pt);
            } else {
                path.lineTo(pt);
            }
        }
        painter->drawPath(path);
    }
}

QPointF MVSSRenderThread::coord2pix(int m, double t, double val)
{
    long M = clips.N1();
    int clip_size = clips.N2();
    double margin_left = 20, margin_right = 20;
    double margin_top = 20, margin_bottom = 20;
    /*
    double max_width = 300;
    if (q->width() - margin_left - margin_right > max_width) {
        double diff = (q->width() - margin_left - margin_right) - max_width;
        margin_left += diff / 2;
        margin_right += diff / 2;
    }
    */
    QRectF rect(margin_left, margin_top, W - margin_left - margin_right, H - margin_top - margin_bottom);
    double pctx = (t + 0.5) / clip_size;
    double pcty = (m + 0.5) / M - val / M * amplitude_factor;
    return QPointF(rect.left() + pctx * rect.width(), rect.top() + pcty * rect.height());
}
