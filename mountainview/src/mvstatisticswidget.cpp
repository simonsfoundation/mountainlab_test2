#include "mvstatisticswidget.h"
#include <QFile>
#include <QHBoxLayout>
#include "diskarraymodel.h"
#include <QTreeWidget>
#include <QDebug>

class MVStatisticsWidgetPrivate {
public:
	MVStatisticsWidget *q;
	QTreeWidget *m_tree;
	Mda m_times;
	Mda m_labels;
	Mda m_primary_channels;
	DiskArrayModel *m_raw;

	void update_statistics();
};


class TreeWidgetItem : public QTreeWidgetItem {
public:
  TreeWidgetItem(QTreeWidget* parent=0):QTreeWidgetItem(parent){}
  private:
  bool operator<(const QTreeWidgetItem &other)const {
	 int column = treeWidget()->sortColumn();
	 return text(column).toFloat() < other.text(column).toFloat();
  }
};

struct SpikeStats {
	int count;
};

MVStatisticsWidget::MVStatisticsWidget()
{
	d=new MVStatisticsWidgetPrivate;
	d->q=this;

	d->m_raw=0;

	d->m_tree=new QTreeWidget;
	d->m_tree->setSelectionMode(QTreeWidget::ExtendedSelection);
	d->m_tree->setSortingEnabled(true);
	QFont font=d->m_tree->font();
	font.setPointSize(10);
	d->m_tree->setFont(font);
	connect(d->m_tree,SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),this,SLOT(slot_item_clicked()));
	connect(d->m_tree,SIGNAL(itemActivated(QTreeWidgetItem*,int)),this,SLOT(slot_item_activated(QTreeWidgetItem*)));
	connect(d->m_tree,SIGNAL(itemSelectionChanged()),this,SLOT(slot_item_selection_changed()));

	QVBoxLayout *layout=new QVBoxLayout;
	layout->addWidget(d->m_tree);
	this->setLayout(layout);
}

MVStatisticsWidget::~MVStatisticsWidget()
{
	delete d;
}

void MVStatisticsWidget::setTimesLabels(const Mda &times, const Mda &labels)
{
	d->m_times=times;
	d->m_labels=labels;
}

void MVStatisticsWidget::setPrimaryChannels(const Mda &primary_channels)
{
	d->m_primary_channels=primary_channels;
}

void MVStatisticsWidget::setRaw(DiskArrayModel *X)
{
	d->m_raw=X;
}

void MVStatisticsWidget::updateStatistics()
{
	d->update_statistics();
}

QList<int> MVStatisticsWidget::selectedUnits()
{
	QList<QTreeWidgetItem *> items=d->m_tree->selectedItems();
	QList<int> ret;
	for (int i=0; i<items.count(); i++) {
		ret << items[i]->data(0,Qt::UserRole).toInt();
	}
	return ret;
}

void MVStatisticsWidget::setSelectedUnits(const QList<int> &units)
{
	QSet<int> the_set=units.toSet();
	for (int j=0; j<d->m_tree->topLevelItemCount(); j++) {
		QTreeWidgetItem *item=d->m_tree->topLevelItem(j);
		if (the_set.contains(item->data(0,Qt::UserRole).toInt())) {
			item->setSelected(true);
		}
		else {
			item->setSelected(false);
		}
	}
}

int MVStatisticsWidget::currentUnit()
{
	QTreeWidgetItem *item=d->m_tree->currentItem();
	if (!item) return 0;
	int unit_number=item->data(0,Qt::UserRole).toInt();
	return unit_number;
}

void MVStatisticsWidget::setCurrentUnit(int unit)
{
	if (unit==currentUnit()) return;
	for (int j=0; j<d->m_tree->topLevelItemCount(); j++) {
		QTreeWidgetItem *item=d->m_tree->topLevelItem(j);
		if (item->data(0,Qt::UserRole).toInt()==unit) {
			d->m_tree->setCurrentItem(item);
		}
	}
}

void MVStatisticsWidget::slot_item_clicked()
{
	emit currentUnitChanged();
}

void MVStatisticsWidget::slot_item_activated(QTreeWidgetItem *item)
{
	emit unitActivated(item->data(0,Qt::UserRole).toInt());
}

void MVStatisticsWidget::slot_item_selection_changed()
{
	emit selectedUnitsChanged();
}

QString read_text_file_2(QString path) {
	QFile FF(path);
	if (!FF.open(QFile::Text|QFile::ReadOnly)) {
		return "";
	}
	QString ret=QString(FF.readAll());
	FF.close();
	return ret;
}


void MVStatisticsWidgetPrivate::update_statistics()
{
	if (!m_raw) return;

	QList<SpikeStats> spike_stats;
	int num=m_times.totalSize();
	for (int ii=0; ii<num; ii++) {
		//int t0=d->m_times.value1(ii);
		int label0=m_labels.get(ii);
		while (label0>=spike_stats.count()) {
			SpikeStats X;
			X.count=0;
			spike_stats << X;
		}
		spike_stats[label0].count++;
	}

	m_tree->clear();
	QStringList labels; labels << "" << "Neuron" << "Primary Channel" << "# Spikes" << "Spikes per minute";
	m_tree->setHeaderLabels(labels);
	for (int k=1; k<spike_stats.count(); k++) {
		SpikeStats X=spike_stats[k];
		int primary_channel=(int)m_primary_channels.value(0,k-1);
		float frequency=X.count*1.0/(m_raw->size(1)*1.0/30000/60);

		QTreeWidgetItem *item=new TreeWidgetItem;
		item->setData(0,Qt::UserRole,k);
		item->setText(0,"");
		item->setText(1,QString("%1").arg(k));
		item->setText(2,QString("%1").arg(primary_channel));
		item->setText(3,QString("%1").arg(X.count));
		item->setText(4,QString("%1").arg(frequency,0,'f',1,' '));

		for (int cc=0; cc<item->columnCount(); cc++) item->setTextAlignment(cc,Qt::AlignLeft);

		m_tree->addTopLevelItem(item);
	}
	for (int cc=0; cc<m_tree->columnCount(); cc++) m_tree->resizeColumnToContents(cc);
}
