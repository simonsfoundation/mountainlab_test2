#ifndef CVVIEW_H
#define CVVIEW_H

#include <QWidget>
#include <QList>
#include "cvcommon.h"
#include <QColor>

class CVViewPrivate;
class CVView : public QWidget
{
	Q_OBJECT
public:
	friend class CVViewPrivate;
	explicit CVView(QWidget *parent = 0);
	~CVView();
	void addDataPoints(const QList<CVDataPoint> &points);
	void setLabelColors(const QList<QColor> colors);
	CVLine selectedLine();
	QList<int> selectedDataPointIndices();
	void setSelectedDataPointIndices(const QList<int> &L);
	void setNumDataPointsToSelect(int num);
    void setLabelStrings(const QStringList &strings);

protected:
	void paintEvent(QPaintEvent *evt);
	void mouseMoveEvent(QMouseEvent *evt);
	void mousePressEvent(QMouseEvent *evt);
	void mouseReleaseEvent(QMouseEvent *evt);
	void keyPressEvent(QKeyEvent *evt);
	void wheelEvent(QWheelEvent *evt);

signals:
	void selectedLineChanged();
	void selectedDataPointsChanged();

private:
	CVViewPrivate *d;

private slots:
	void slot_update_view(long code,long data_point_index);

signals:
	void signal_do_update_view(long code,long data_point_index);

signals:

public slots:
};

#endif // CVVIEW_H
