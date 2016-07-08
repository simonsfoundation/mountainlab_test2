#include "mvspikespraypanel.h"

#include <QList>
#include <QPainter>
#include <QPen>
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

    void render_clip(QPainter* painter, long M, long T, double* ptr, QColor col);
    QColor get_label_color(int label);
    QPointF coord2pix(int m, double t, double val);
};

MVSpikeSprayPanel::MVSpikeSprayPanel(MVContext* context)
{
    d = new MVSpikeSprayPanelPrivate;
    d->q = this;
    d->m_context = context;
}

MVSpikeSprayPanel::~MVSpikeSprayPanel()
{
    delete d;
}

void MVSpikeSprayPanel::setLabelsToUse(const QSet<int>& labels)
{
    d->m_labels_to_use = labels;
    update();
}

void MVSpikeSprayPanel::setClipsToRender(Mda* X)
{
    d->m_clips_to_render = X;
    update();
}

void MVSpikeSprayPanel::setLabelsToRender(const QVector<int>& X)
{
    d->m_labels_to_render = X;
    update();
}

void MVSpikeSprayPanel::setAmplitudeFactor(double X)
{
    d->m_amplitude_factor = X;
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

    if (!d->m_clips_to_render)
        return;

    if (d->m_clips_to_render->N3() != d->m_labels_to_render.count()) {
        qWarning() << "Number of clips to render does not match the number of labels to render" << d->m_clips_to_render->N3() << d->m_labels_to_render.count();
        return;
    }

    if (!d->m_amplitude_factor) {
        double maxval = qMax(qAbs(d->m_clips_to_render->minimum()), qAbs(d->m_clips_to_render->maximum()));
        if (maxval)
            d->m_amplitude_factor = 1.5 / maxval;
    }

    int K = MLCompute::max(d->m_labels_to_render);
    if (!K)
        return;
    int counts[K + 1];
    for (int k = 0; k < K + 1; k++)
        counts[k] = 0;
    for (long i = 0; i < d->m_labels_to_render.count(); i++) {
        int label0 = d->m_labels_to_render[i];
        if (label0 >= 0)
            counts[label0]++;
    }
    int alphas[K + 1];
    for (int k = 0; k <= K; k++) {
        if (counts[k]) {
            alphas[k] = 255 / counts[k];
            alphas[k] = qMin(255, qMax(5, alphas[k]));
        } else
            alphas[k] = 255;
    }

    long M = d->m_clips_to_render->N1();
    long T = d->m_clips_to_render->N2();
    double* ptr = d->m_clips_to_render->dataPtr();
    for (long i = 0; i < d->m_labels_to_render.count(); i++) {
        int label0 = d->m_labels_to_render[i];
        if (d->m_labels_to_use.contains(label0)) {
            QColor col = d->get_label_color(label0);
            if (label0 >= 0) {
                col.setAlpha(alphas[label0]);
            }
            d->render_clip(&painter, M, T, &ptr[M * T * i], col);
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

void MVSpikeSprayPanelPrivate::render_clip(QPainter* painter, long M, long T, double* ptr, QColor col)
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

QColor MVSpikeSprayPanelPrivate::get_label_color(int label)
{
    return m_context->clusterColor(label);
}

QPointF MVSpikeSprayPanelPrivate::coord2pix(int m, double t, double val)
{
    long M = 1;
    if (m_clips_to_render)
        M = m_clips_to_render->N1();
    int clip_size = m_clips_to_render->N2();
    double margin_left = 20, margin_right = 20;
    double margin_top = 20, margin_bottom = 20;
    double max_width = 300;
    if (q->width() - margin_left - margin_right > max_width) {
        double diff = (q->width() - margin_left - margin_right) - max_width;
        margin_left += diff / 2;
        margin_right += diff / 2;
    }
    QRectF rect(margin_left, margin_top, q->width() - margin_left - margin_right, q->height() - margin_top - margin_bottom);
    double pctx = (t + 0.5) / clip_size;
    double pcty = (m + 0.5) / M - val / M * m_amplitude_factor;
    return QPointF(rect.left() + pctx * rect.width(), rect.top() + pcty * rect.height());
}
