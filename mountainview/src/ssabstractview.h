#ifndef SSAbstractView_H
#define SSAbstractView_H

#include <QWidget>
#include "plotarea.h" //for Vec2
#include "sslabelsmodel.h"
#include "ssabstractplot.h"
#include "sstimeseriesplot.h"
#include "diskreadmda.h"

class SSAbstractViewPrivate;
class SSAbstractViewUnderlayPainter;
class SSAbstractView : public QWidget
{
	Q_OBJECT
public:
	friend class SSAbstractViewPrivate;
	friend class SSAbstractViewUnderlayPainter;
	explicit SSAbstractView(QWidget *parent = 0);
	virtual ~SSAbstractView();

	Q_INVOKABLE void setTitle(QString title);
	Q_INVOKABLE void initialize();
	Q_INVOKABLE void setTimepointMapping(const DiskReadMda &X);
	Q_INVOKABLE void setSamplingFrequency(float val);

	QString title();
	void setCurrentX(double x);
	double currentX();
	void setCurrentTimepoint(int tt);
	int currentTimepoint();
	int currentChannel();
	void setCurrentChannel(int channel);
	Vec2 xRange();
	void setXRange(const Vec2 &range);
	Vec2 selectionRange() const;
	void setSelectionRange(const Vec2 &range);
	QString getTimeLabelForX(double x);
	long getTimepointForX(int x);
	void enableSignals();
	void disableSignals();
	void setTimelineVisible(bool val);
	void setVerticalZoomFactor(float val);
	float verticalZoomFactor();
	void setActivated(bool val);
	void centerOnCursor();
	
	virtual double currentValue();
	virtual QString viewType()=0;

protected:
	void mousePressEvent(QMouseEvent *evt);
	void mouseReleaseEvent(QMouseEvent *evt);
	void mouseMoveEvent(QMouseEvent *evt);
	void keyPressEvent(QKeyEvent *);
	void wheelEvent(QWheelEvent *);

	void setMaxTimepoint(int t);
	virtual SSAbstractPlot *plot()=0;


private slots:
	void on_x_range_changed();
	void on_replot_needed();

private:
	SSAbstractViewPrivate *d;

signals:
	void currentXChanged();
	void currentChannelChanged();
	void xRangeChanged();
	void selectionRangeChanged();
	void replotNeeded();
	void clicked();

private slots:
};

#endif // SSAbstractView_H
