/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/8/2016
*******************************************************/

#include "mvspikesprayview.h"
#include "computationthread.h"
#include "extract_clips.h"
#include "mountainprocessrunner.h"

#include <QPainter>
#include <taskprogress.h>
#include "mlutils.h"
#include "msmisc.h"

class MVSpikeSprayComputer : public ComputationThread {
public:
    //input
    QString mlproxy_url;
    DiskReadMda timeseries;
    DiskReadMda firings;
    QList<int> labels_to_use;
    int clip_size;

    //output
    Mda clips_to_render;
    QList<int> labels_to_render;

    void compute();
};

class MVSpikeSprayViewPrivate {
public:
    MVSpikeSprayView* q;
    QString m_mlproxy_url;
    DiskReadMda m_timeseries;
    DiskReadMda m_firings;
    QList<int> m_labels_to_use;
    int m_clip_size;
    QList<QColor> m_label_colors;
    bool m_compute_needed;
    double m_amplitude_factor;

    Mda m_clips_to_render;
    QList<int> m_labels_to_render;
    MVSpikeSprayComputer m_computer;

    void schedule_compute();
    void render_clip(QPainter* painter, long M, long T, double* ptr, QColor col);
    QColor get_label_color(int label);
    QPointF coord2pix(int m, double t, double val);
};

MVSpikeSprayView::MVSpikeSprayView()
{
    d = new MVSpikeSprayViewPrivate;
    d->q = this;
    d->m_clip_size = 100;
    d->m_compute_needed = false;
    d->m_amplitude_factor = 1.0 / 5;

    QObject::connect(&d->m_computer, SIGNAL(computationFinished()), this, SLOT(slot_computation_finished()));
}

MVSpikeSprayView::~MVSpikeSprayView()
{
    d->m_computer.stopComputation();
    delete d;
}

void MVSpikeSprayView::setMLProxyUrl(const QString& url)
{
    d->m_mlproxy_url = url;
}

void MVSpikeSprayView::setTimeseries(DiskReadMda& X)
{
    d->m_timeseries = X;
    /// TODO address: the following is a hack so that the array info is not downloaded during the paintEvent which seems to cause a crash
    d->m_timeseries.N1();
    d->schedule_compute();
}

void MVSpikeSprayView::setFirings(DiskReadMda& F)
{
    d->m_firings = F;
    /// TODO address: the following is a hack so that the array info is not downloaded during the paintEvent which seems to cause a crash
    d->m_firings.N1();
    d->schedule_compute();
}

void MVSpikeSprayView::setLabelsToUse(const QList<int>& labels)
{
    d->m_labels_to_use = labels;
    d->schedule_compute();
}

void MVSpikeSprayView::setClipSize(int clip_size)
{
    d->m_clip_size = clip_size;
    d->schedule_compute();
}

void MVSpikeSprayView::setLabelColors(const QList<QColor>& colors)
{
    d->m_label_colors = colors;
    update();
}

void MVSpikeSprayView::slot_computation_finished()
{
    d->m_computer.stopComputation(); //because I'm paranoid
    d->m_clips_to_render = d->m_computer.clips_to_render;
    d->m_labels_to_render = d->m_computer.labels_to_render;
    update();
}

void MVSpikeSprayView::paintEvent(QPaintEvent* evt)
{
    Q_UNUSED(evt)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (d->m_compute_needed) {
        d->m_labels_to_render.clear();
        d->m_computer.stopComputation();
        d->m_computer.mlproxy_url = d->m_mlproxy_url;
        d->m_computer.timeseries = d->m_timeseries;
        d->m_computer.firings = d->m_firings;
        d->m_computer.labels_to_use = d->m_labels_to_use;
        d->m_computer.clip_size = d->m_clip_size;
        d->m_computer.startComputation();
        d->m_compute_needed = false;
        return;
    }
    if (d->m_computer.isComputing())
        return;

    if (d->m_clips_to_render.N3() != d->m_labels_to_render.count())
        return;

    double maxval = qMax(qAbs(d->m_clips_to_render.minimum()), qAbs(d->m_clips_to_render.maximum()));
    if (maxval)
        d->m_amplitude_factor = 1. / maxval;

    int K=compute_max(d->m_labels_to_render);
    if (!K) return;
    int counts[K+1];
    for (int k=0; k<K+1; k++) counts[k]=0;
    for (long i = 0; i < d->m_labels_to_render.count(); i++) {
        int label0=d->m_labels_to_render[i];
        if (label0>=0) counts[label0]++;
    }
    int alphas[K+1];
    for (int k=0; k<=K; k++) {
        if (counts[k]) {
            alphas[k]=255/counts[k];
            alphas[k]=qMin(255,qMax(5,alphas[k]));
        }
        else alphas[k]=255;
    }

    long M = d->m_clips_to_render.N1();
    long T = d->m_clips_to_render.N2();
    double* ptr = d->m_clips_to_render.dataPtr();
    for (long i = 0; i < d->m_labels_to_render.count(); i++) {
        int label0=d->m_labels_to_render[i];
        QColor col = d->get_label_color(label0);
        if (label0>=0) {
            col.setAlpha(alphas[label0]);
        }
        d->render_clip(&painter, M, T, &ptr[M * T * i], col);
    }
}

void MVSpikeSprayViewPrivate::schedule_compute()
{
    m_compute_needed = true;
    q->update();
}

void MVSpikeSprayViewPrivate::render_clip(QPainter* painter, long M, long T, double* ptr, QColor col)
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
            }
            else {
                path.lineTo(pt);
            }
        }
        painter->drawPath(path);
    }
}

QColor MVSpikeSprayViewPrivate::get_label_color(int label)
{
    if (!m_label_colors.size())
        return Qt::black;
    return m_label_colors[label % m_label_colors.size()];
}

QPointF MVSpikeSprayViewPrivate::coord2pix(int m, double t, double val)
{
    long M = m_timeseries.N1();
    double margin_left = 100, margin_right = 100;
    double margin_top = 100, margin_bottom = 100;
    QRectF rect(margin_left, margin_top, q->width() - margin_left - margin_right, q->height() - margin_top - margin_bottom);
    double pctx = (t + 0.5) / m_clip_size;
    double pcty = (m + 0.5) / M - val / M * m_amplitude_factor;
    return QPointF(rect.left() + pctx * rect.width(), rect.top() + pcty * rect.height());
}

void MVSpikeSprayComputer::compute()
{
    TaskProgress task("Spike spray computer");
    QString firings_out_path;
    {
        QString labels_str;
        foreach (int x, labels_to_use) {
            if (!labels_str.isEmpty())
                labels_str += ",";
            labels_str += QString("%1").arg(x);
        }

        MountainProcessRunner MT;
        QString processor_name = "mv_subfirings";
        MT.setProcessorName(processor_name);

        QMap<QString, QVariant> params;
        params["firings"] = firings.path();
        params["labels"] = labels_str;
        params["max_per_label"] = 256;
        MT.setInputParameters(params);
        MT.setMLProxyUrl(mlproxy_url);

        firings_out_path = MT.makeOutputFilePath("firings_out");

        MT.runProcess();

        if (thread_interrupt_requested()) {
            task.error(QString("Halted while running process: " + processor_name));
            return;
        }
    }

    task.setProgress(0.25);
    task.log("firings_out_path: " + firings_out_path);

    QString clips_path;
    {
        MountainProcessRunner MT;
        QString processor_name = "extract_clips";
        MT.setProcessorName(processor_name);

        QMap<QString, QVariant> params;
        params["timeseries"] = timeseries.path();
        params["firings"] = firings_out_path;
        params["clip_size"] = clip_size;
        MT.setInputParameters(params);
        MT.setMLProxyUrl(mlproxy_url);

        clips_path = MT.makeOutputFilePath("clips");

        MT.runProcess();
        if (thread_interrupt_requested()) {
            task.error(QString("Halted while running process: " + processor_name));
            return;
        }
    }

    task.setProgress(0.5);
    task.log("clips_path: " + clips_path);

    DiskReadMda clips0(clips_path);
    clips0.readChunk(clips_to_render, 0, 0, 0, clips0.N1(), clips0.N2(), clips0.N3());

    task.setProgress(0.75);

    DiskReadMda firings2(firings_out_path);
    task.log(QString("%1x%2 from %3x%4").arg(firings2.N1()).arg(firings2.N2()).arg(firings.N1()).arg(firings.N2()));
    Mda firings0;
    firings2.readChunk(firings0, 0, 0, firings2.N1(), firings2.N2());
    task.setProgress(0.9);
    for (long i = 0; i < firings0.N2(); i++) {
        int label0 = (int)firings0.value(2, i);
        labels_to_render << label0;
    }
}
