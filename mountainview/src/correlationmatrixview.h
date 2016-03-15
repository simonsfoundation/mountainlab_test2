#ifndef CORRELATIONMATRIXVIEW_H
#define CORRELATIONMATRIXVIEW_H

#include "mda.h"
#include <QWidget>

class CorrelationMatrixViewPrivate;
class CorrelationMatrixView : public QWidget
{
public:
	friend class CorrelationMatrixViewPrivate;
	CorrelationMatrixView(QWidget *parent=0);
	virtual ~CorrelationMatrixView();
	void setMatrix(const Mda &CM);

protected:
	void paintEvent(QPaintEvent *evt);
	void mouseMoveEvent(QMouseEvent *evt);

signals:

public slots:

private:
	CorrelationMatrixViewPrivate *d;
};

#endif // CORRELATIONMATRIXVIEW_H
