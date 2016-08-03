#ifndef MVSPIKESPRAYPANEL_H
#define MVSPIKESPRAYPANEL_H

#include <QList>
#include <QMutex>
#include <QThread>
#include <QWidget>
#include <mvcontext.h>

class MVSpikeSprayPanelPrivate;
class MVSpikeSprayPanel : public QWidget {
public:
    friend class MVSpikeSprayPanelPrivate;
    MVSpikeSprayPanel(MVContext* context);
    virtual ~MVSpikeSprayPanel();
    void setLabelsToUse(const QSet<int>& labels);
    void setClipsToRender(Mda* X);
    void setLabelsToRender(const QVector<int>& X);
    void setAmplitudeFactor(double X);
    void setBrightnessFactor(double factor);
    void setWeightFactor(double factor);
    void setLegendVisible(bool val);

protected:
    void paintEvent(QPaintEvent* evt);

private:
    MVSpikeSprayPanelPrivate* d;
};

class MVSSRenderThread : public QThread {
    Q_OBJECT
public:
    //input
    Mda clips;
    QList<QColor> colors;
    double amplitude_factor;
    int W, H;

    //output
    QImage image;

    QImage image_in_progress;
    QMutex image_in_progress_mutex;

    void run();
    void render_clip(QPainter* painter, long M, long T, double* ptr, QColor col);
    QPointF coord2pix(int m, double t, double val);
signals:
    void signalImageInProgressUpdated();
};

#endif // MVSPIKESPRAYPANEL_H
