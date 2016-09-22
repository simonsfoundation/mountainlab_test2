#ifndef PLOTAREA_H
#define PLOTAREA_H

#include <QColor>
#include <QPainter>
#include "sscommon.h"

struct PlotSeries {
    Mda xvals, yvals;
    QColor color;
    double offset;
    bool plot_pairs; //this is a hack :(
    QString name;
};

class PlotAreaPrivate;
class PlotArea {
public:
    friend class PlotAreaPrivate;
    PlotArea();
    ~PlotArea();
    void setSize(double W, double H);
    void setPosition(double x, double y);
    void clearSeries();
    void setXRange(double xmin, double xmax);
    void setYRange(double ymin, double ymax);
    Vec2 yRange() const;
    void addSeries(const PlotSeries& SS);
    void addMarker(int t, int l);
    void addCompareMarker(int t, int l);
    void clearMarkers();
    void refresh(QPainter* P);
    Vec2 coordToPix(Vec2 coord);
    Vec2 pixToCoord(Vec2 pix);
    void setMarkerColors(const QList<QColor>& colors);
    void setMarkerLabels(const QList<QString>& labels);
    //void setConnectZeros(bool val);
    void setPlotBaselines(bool val);
    void setMarkerAlpha(int val);
    void setShowMarkerLines(bool val);
    QRect plotRect();

private:
    PlotAreaPrivate* d;
};

#endif // PLOTAREA_H
