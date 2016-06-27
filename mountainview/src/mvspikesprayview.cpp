/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/8/2016
*******************************************************/

#include "mvspikesprayview.h"
#include "extract_clips.h"
#include "mountainprocessrunner.h"
#include "mvmainwindow.h"

#include <QMessageBox>
#include <QPainter>
#include <taskprogress.h>
#include "mlutils.h"
#include "msmisc.h"

/// TODO (LOW) spike spray should respond to mouse wheel and show current position with marker
/// TODO (LOW) much more responsive rendering of spike spray

class MVSpikeSprayComputer {
public:
    //input
    DiskReadMda timeseries;
    DiskReadMda firings;
    QString mlproxy_url;
    MVEventFilter filter;
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
    MVViewAgent* m_view_agent;
    QList<int> m_labels_to_use;

    double m_amplitude_factor;
    int m_num_channels;

    Mda m_clips_to_render;
    QList<int> m_labels_to_render;
    MVSpikeSprayComputer m_computer;

    void render_clip(QPainter* painter, long M, long T, double* ptr, QColor col);
    QColor get_label_color(int label);
    QPointF coord2pix(int m, double t, double val);
};

MVSpikeSprayView::MVSpikeSprayView(MVViewAgent* view_agent)
    : MVAbstractView(view_agent)
{
    d = new MVSpikeSprayViewPrivate;
    d->q = this;
    d->m_view_agent = view_agent;
    d->m_amplitude_factor = 0;
    d->m_num_channels = 1;

    recalculateOnOptionChanged("clip_size");
    recalculateOn(viewAgent(), SIGNAL(timeseriesNamesChanged()));
    recalculateOn(viewAgent(), SIGNAL(filteredFiringsChanged()));

    /// TODO (LOW) should we put this in the abstract view?
    this->setFocusPolicy(Qt::StrongFocus);
}

MVSpikeSprayView::~MVSpikeSprayView()
{
    delete d;
}

void MVSpikeSprayView::setLabelsToUse(const QList<int>& labels)
{
    d->m_labels_to_use = labels;
    recalculate();
}

void MVSpikeSprayView::prepareCalculation()
{
    d->m_num_channels = d->m_view_agent->currentTimeseries().N1(); //important for rendering
    d->m_labels_to_render.clear();
    d->m_computer.mlproxy_url = d->m_view_agent->mlProxyUrl();
    d->m_computer.timeseries = d->m_view_agent->currentTimeseries();
    d->m_computer.firings = d->m_view_agent->firings();
    d->m_computer.filter = d->m_view_agent->eventFilter();
    d->m_computer.labels_to_use = d->m_labels_to_use;
    d->m_computer.clip_size = d->m_view_agent->option("clip_size").toInt();
}

void MVSpikeSprayView::runCalculation()
{
    d->m_computer.compute();
}

void MVSpikeSprayView::onCalculationFinished()
{
    d->m_clips_to_render = d->m_computer.clips_to_render;
    d->m_labels_to_render = d->m_computer.labels_to_render;
    update();
}

void MVSpikeSprayView::paintEvent(QPaintEvent* evt)
{
    Q_UNUSED(evt)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (isCalculating()) {
        QFont font = painter.font();
        font.setPointSize(20);
        painter.setFont(font);
        painter.fillRect(QRectF(0, 0, width(), height()), viewAgent()->color("calculation-in-progress"));
        painter.drawText(QRectF(0, 0, width(), height()), Qt::AlignCenter | Qt::AlignVCenter, "Calculating...");
        return;
    }

    /// TODO (LOW) this should be a configured color to match the cluster view
    painter.fillRect(0, 0, width(), height(), QBrush(QColor(60, 60, 60)));

    if (d->m_clips_to_render.N3() != d->m_labels_to_render.count()) {
        qWarning() << "Number of clips to render does not match the number of labels to render" << d->m_clips_to_render.N3() << d->m_labels_to_render.count();
        return;
    }

    if (!d->m_amplitude_factor) {
        double maxval = qMax(qAbs(d->m_clips_to_render.minimum()), qAbs(d->m_clips_to_render.maximum()));
        if (maxval)
            d->m_amplitude_factor = 1.5 / maxval;
    }

    int K = compute_max(d->m_labels_to_render);
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
        }
        else
            alphas[k] = 255;
    }

    long M = d->m_clips_to_render.N1();
    long T = d->m_clips_to_render.N2();
    double* ptr = d->m_clips_to_render.dataPtr();
    for (long i = 0; i < d->m_labels_to_render.count(); i++) {
        int label0 = d->m_labels_to_render[i];
        QColor col = d->get_label_color(label0);
        if (label0 >= 0) {
            col.setAlpha(alphas[label0]);
        }
        d->render_clip(&painter, M, T, &ptr[M * T * i], col);
    }

    //legend
    {
        double W = width();
        double spacing = 6;
        double margin = 10;
        QSet<int> labels_used = QSet<int>::fromList(d->m_labels_to_use);
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

void MVSpikeSprayView::keyPressEvent(QKeyEvent* evt)
{
    if (evt->key() == Qt::Key_Up) {
        d->m_amplitude_factor *= 1.2;
        update();
    }
    else if (evt->key() == Qt::Key_Down) {
        d->m_amplitude_factor /= 1.2;
        update();
    }
    else {
        QWidget::keyPressEvent(evt);
    }
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
    return m_view_agent->clusterColor(label);
}

QPointF MVSpikeSprayViewPrivate::coord2pix(int m, double t, double val)
{
    long M = m_num_channels;
    int clip_size = m_view_agent->option("clip_size").toInt();
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

void MVSpikeSprayComputer::compute()
{
    TaskProgress task("Spike spray computer");

    firings = compute_filtered_firings_remotely(mlproxy_url, firings, filter);

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
        params["firings"] = firings.makePath();
        params["labels"] = labels_str;
        params["max_per_label"] = 512;
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
        params["timeseries"] = timeseries.makePath();
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
    task.log(QString("%1x%2 from %3x%4 (%5x%6x%7) (%8)").arg(firings2.N1()).arg(firings2.N2()).arg(firings.N1()).arg(firings.N2()).arg(clips0.N1()).arg(clips0.N2()).arg(clips0.N3()).arg(clips_to_render.N3()));
    Mda firings0;
    firings2.readChunk(firings0, 0, 0, firings2.N1(), firings2.N2());
    task.setProgress(0.9);
    for (long i = 0; i < firings0.N2(); i++) {
        int label0 = (int)firings0.value(2, i);
        labels_to_render << label0;
    }
}

MVSpikeSprayFactory::MVSpikeSprayFactory(MVViewAgent *context, QObject* parent)
    : MVAbstractViewFactory(context, parent)
{
    connect(context, SIGNAL(selectedClustersChanged()),
        this, SLOT(updateEnabled()));
    updateEnabled();
}

QString MVSpikeSprayFactory::id() const
{
    return QStringLiteral("open-spike-spray");
}

QString MVSpikeSprayFactory::name() const
{
    return tr("Spike Spray");
}

QString MVSpikeSprayFactory::title() const
{
    return tr("Spike Spray");
}

MVAbstractView* MVSpikeSprayFactory::createView(QWidget* parent)
{
    QList<int> ks = mvContext()->selectedClusters();
    qSort(ks);
    if (ks.isEmpty()) {
        QMessageBox::information(MVMainWindow::instance(), "Unable to open spike spray", "You must select at least one cluster.");
        return Q_NULLPTR;
    }
    MVSpikeSprayView* X = new MVSpikeSprayView(mvContext());
    X->setLabelsToUse(ks);
    return X;
}

void MVSpikeSprayFactory::updateEnabled()
{
    setEnabled(!MVMainWindow::instance()->viewAgent()->selectedClusters().isEmpty());
}
