#ifndef MVSTATISTICSWIDGET_H
#define MVSTATISTICSWIDGET_H

#include <QWidget>
#include "mda.h"
#include "diskarraymodel.h"

class QTreeWidgetItem;
class MVStatisticsWidgetPrivate;
class MVStatisticsWidget : public QWidget
{
	Q_OBJECT
public:
	friend class MVStatisticsWidgetPrivate;
	MVStatisticsWidget();
	virtual ~MVStatisticsWidget();

	void setTimesLabels(const Mda &times,const Mda &labels);
	void setPrimaryChannels(const Mda &primary_channels);
	void setRaw(DiskArrayModel *X);
	void updateStatistics();

	QList<int> selectedUnits();
	void setSelectedUnits(const QList<int> &units);
	int currentUnit();
	void setCurrentUnit(int unit);

signals:
	void selectedUnitsChanged();
	void currentUnitChanged();
	void unitActivated(int num);

private slots:
	void slot_item_clicked();
	void slot_item_activated(QTreeWidgetItem *item);
	void slot_item_selection_changed();

private:
	MVStatisticsWidgetPrivate *d;
};

#endif // MVSTATISTICSWIDGET_H
