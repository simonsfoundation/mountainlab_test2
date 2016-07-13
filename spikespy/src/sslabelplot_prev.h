#ifndef SSLABELPLOT_H
#define SSLABELPLOT_H

#include "ssabstractplot_prev.h"
#include "sslabelsmodel_prev.h"

class SSLabelPlotPrivate;
class SSLabelPlot : public SSAbstractPlot {
    Q_OBJECT
public:
    friend class SSLabelPlotPrivate;
    SSLabelPlot();
    ~SSLabelPlot();

    void updateSize();
    Vec2 coordToPix(Vec2 coord);
    Vec2 pixToCoord(Vec2 pix);
    void setXRange(const Vec2& range);
    void setYRange(const Vec2& range);
    void initialize();
    void refresh();

    void setLabels(DiskReadMdaOld* TL, bool is_owner);
    void setLabels(SSLabelsModel* L, bool is_owner = false);
    void setMargins(int left, int right, int top, int bottom);

private slots:
    void slot_replot_needed();

private:
    void paintPlot(QPainter* painter);

    SSLabelPlotPrivate* d;
};

#endif // SSLABELPLOT_H
