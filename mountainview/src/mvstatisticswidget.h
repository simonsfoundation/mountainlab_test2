#ifndef MVSTATISTICSWIDGET_H
#define MVSTATISTICSWIDGET_H

#include <QWidget>
#include "mda.h"
#include "diskarraymodel.h"

/** \class MVStatisticsWidget
 *  \brief -- Not used anymore
 */

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

	QList<int> selectedLabels();
	void setSelectedLabels(const QList<int> &labels);
	int currentLabel();
	void setCurrentLabel(int label);

signals:
	void selectedLabelsChanged();
	void currentLabelChanged();
	void labelActivated(int num);

private slots:
	void slot_item_clicked();
	void slot_item_activated(QTreeWidgetItem *item);
	void slot_item_selection_changed();

private:
	MVStatisticsWidgetPrivate *d;
};

#endif // MVSTATISTICSWIDGET_H
