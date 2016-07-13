#ifndef SSABSTRACTPLOT_H
#define SSABSTRACTPLOT_H

#include <QWidget>
#include "sscommon.h"
#include "diskreadmdaold.h"

class SSAbstractPlotPrivate;
class OverlayPainter;
class SSAbstractPlot : public QWidget {
    Q_OBJECT
public:
    friend class SSAbstractPlotPrivate;
    explicit SSAbstractPlot(QWidget* parent = 0);
    ~SSAbstractPlot();

    virtual void updateSize() = 0;
    virtual Vec2 coordToPix(Vec2 coord) = 0;
    virtual Vec2 pixToCoord(Vec2 pix) = 0;
    virtual void initialize() = 0;

    virtual void setXRange(const Vec2& range);
    virtual void setYRange(const Vec2& range);
    virtual void setUnderlayPainter(OverlayPainter* X);
    virtual void setOverlayPainter(OverlayPainter* X);
    virtual void setVerticalZoomFactor(float val);

    Vec2 yRange() const;
    Vec2 xRange() const;
    float verticalZoomFactor();
    void setChannelFlip(bool val); //ahb
    bool channelFlip(); //ahb
    void setTimepointMapping(const DiskReadMdaOld& TM);
    DiskReadMdaOld timepointMapping();

signals:
    void xRangeChanged();
    void replotNeeded();

protected:
    void paintEvent(QPaintEvent* evt);
    virtual void paintPlot(QPainter* painter) = 0;

private:
    SSAbstractPlotPrivate* d;

signals:

public slots:
};

class OverlayPainter {
public:
    virtual void paint(QPainter* painter) = 0;
};

#endif // SSABSTRACTPLOT_H
