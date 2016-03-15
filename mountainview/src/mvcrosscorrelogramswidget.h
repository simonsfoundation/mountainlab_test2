#ifndef MVCROSSCORRELOGRAMSWIDGET_H
#define MVCROSSCORRELOGRAMSWIDGET_H

#include <QWidget>
#include "diskreadmda.h"
#include <QMap>
#include <QColor>

class MVCrossCorrelogramsWidgetPrivate;
class MVCrossCorrelogramsWidget : public QWidget
{
	Q_OBJECT
public:
	friend class MVCrossCorrelogramsWidgetPrivate;
	MVCrossCorrelogramsWidget();
	virtual ~MVCrossCorrelogramsWidget();

	void setCrossCorrelogramsPath(const QString &path);
	void setCrossCorrelogramsData(const DiskReadMda &X);
    void setLabels(const QStringList &labels);
	void setColors(const QMap<QString,QColor> &colors);
	void updateWidget();

	int currentUnit();
	QList<int> selectedUnits();
	void setCurrentUnit(int num);
	void setSelectedUnits(const QList<int> &nums);
	int baseUnit();
	void setBaseUnit(int num);

	void setUnitNumbers(const QList<int> &numbers);

signals:
	void currentUnitChanged();
	void unitActivated(int num);
	void selectedUnitsChanged();

private slots:
	void slot_histogram_view_clicked();
	void slot_histogram_view_control_clicked();
	void slot_histogram_view_activated();

protected:
	void keyPressEvent(QKeyEvent *);

private:
	MVCrossCorrelogramsWidgetPrivate *d;
};

#endif // MVCROSSCORRELOGRAMSWIDGET_H
