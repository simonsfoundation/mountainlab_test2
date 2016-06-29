/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef HISTOGRAMVIEW_H
#define HISTOGRAMVIEW_H

#include "mvcontext.h"

#include <QWidget>

/** \class HistogramView
 *  \brief View of a histogram from an arbitrary data array
 */
class HistogramViewPrivate;
class HistogramView : public QWidget {
    Q_OBJECT
public:
    friend class HistogramViewPrivate;
    explicit HistogramView(QWidget* parent = 0);
    virtual ~HistogramView();

    void setData(const QList<double>& values); // The data to view
    void setData(int N, double* values); // Alternative specification
    void setSecondData(const QList<double>& values);
    void setBins(double bin_min, double bin_max, int num_bins); //Set evenly spaced bins
    void autoSetBins(int num_bins); // auto set evenly spaced bins based on range of data (call setData first)
    void setFillColor(const QColor& col); // The color for filling the histogram bars
    void setLineColor(const QColor& col); // The edge color for the bars
    void setTitle(const QString& title);
    void setColors(const QMap<QString, QColor>& colors); // Controls background and highlighting colors. For consistent app look.

    MVRange xRange() const;
    void setXRange(MVRange range);
    void autoCenterXRange(); //centers range based on data
    void setDrawVerticalAxisAtZero(bool val);

    void setCurrent(bool val); // Set this as the current histogram (affects highlighting)
    void setSelected(bool val); // Set this as among the selected histograms (affects highlighting)

    QImage renderImage(int W, int H);

protected:
    void paintEvent(QPaintEvent* evt);
    void mousePressEvent(QMouseEvent* evt);
    void mouseMoveEvent(QMouseEvent* evt);
    void enterEvent(QEvent* evt);
    void leaveEvent(QEvent* evt);
    void mouseDoubleClickEvent(QMouseEvent* evt);
signals:
    void clicked(Qt::KeyboardModifiers modifiers);
    void rightClicked(Qt::KeyboardModifiers modifiers);
    void activated();

    void signalExportHistogramMatrixImage();

private slots:
    void slot_context_menu(const QPoint& pos);

private:
    HistogramViewPrivate* d;
};

#endif // HISTOGRAMVIEW_H
