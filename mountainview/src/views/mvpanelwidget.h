/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/29/2016
*******************************************************/

#ifndef MVPANELWIDGET_H
#define MVPANELWIDGET_H

#include "paintlayer.h"
#include <QWidget>

class MVPanelWidgetPrivate;
class MVPanelWidget : public QWidget {
    Q_OBJECT
public:
    friend class MVPanelWidgetPrivate;
    MVPanelWidget();
    virtual ~MVPanelWidget();

    void clearPanels(bool delete_layers);
    void addPanel(int row, int col, PaintLayer* layer);
    int rowCount() const;
    int columnCount() const;
    void setSpacing(int row_spacing, int col_spacing);
    void setMargins(int row_margin, int col_margin);
    void setMinimumPanelWidth(int w);
    void setMinimumPanelHeight(int h);
    void setScrollable(bool h_scrollable, bool v_scrollable);

    int currentPanelIndex() const;
    void setCurrentPanelIndex(int index); //for zooming

    void setViewportGeometry(QRectF geom);

public slots:
    void zoomIn();
    void zoomOut();

protected:
    void paintEvent(QPaintEvent* evt);
    void wheelEvent(QWheelEvent* evt);
    void mousePressEvent(QMouseEvent* evt);
    void mouseReleaseEvent(QMouseEvent* evt);
    void mouseMoveEvent(QMouseEvent* evt);

signals:
    void signalPanelClicked(int index, Qt::KeyboardModifiers modifiers);

private:
    MVPanelWidgetPrivate* d;
};

#endif // MVPANELWIDGET_H
