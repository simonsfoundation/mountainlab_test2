#ifndef MVCLUSTERDETAILWIDGET_H
#define MVCLUSTERDETAILWIDGET_H

#include "diskreadmda.h"
#include "mda.h"
#include <QWidget>
#include <QScrollArea>

class MVClusterDetailWidgetPrivate;
class MVClusterDetailWidget : public QWidget
{
	Q_OBJECT
public:
	friend class MVClusterDetailWidgetPrivate;
	friend class MVClusterDetailWidgetScrollArea;
	MVClusterDetailWidget(QWidget *parent=0);
	virtual ~MVClusterDetailWidget();
	void setSignal(DiskReadMda &X);
    void setFirings(const DiskReadMda &X);
    void setClipSize(int T);
    void setGroupNumbers(const QList<int> &group_numbers);
	void setSamplingFrequency(double freq);
	void setChannelColors(const QList<QColor> &colors);
	void setColors(const QMap<QString,QColor> &colors);
	int currentK();
	QList<int> selectedKs();
	void setCurrentK(int k);
	void setSelectedKs(const QList<int> &ks);
protected:
	void paintEvent(QPaintEvent *evt);
	void keyPressEvent(QKeyEvent *evt);
	void mousePressEvent(QMouseEvent *evt);
	void mouseReleaseEvent(QMouseEvent *evt);
	void mouseMoveEvent(QMouseEvent *evt);
    void mouseDoubleClickEvent(QMouseEvent *evt);
	void wheelEvent(QWheelEvent *evt);
signals:
    void signalTemplateActivated();
	void signalCurrentKChanged();
	void signalSelectedKsChanged();
	void signalZoomedIn();
private:
	MVClusterDetailWidgetPrivate *d;
};

#endif // MVCLUSTERDETAILWIDGET_H
