#ifndef MVOVERVIEWWIDGET_H
#define MVOVERVIEWWIDGET_H

#include <QWidget>
#include <QWheelEvent>
#include "diskarraymodel.h"
#include "mda.h"

class MVOverviewWidgetPrivate;

class MVOverviewWidget : public QWidget
{
	Q_OBJECT
public:
	friend class MVOverviewWidgetPrivate;
	explicit MVOverviewWidget(QWidget *parent = 0);
	~MVOverviewWidget();

	void setElectrodeLocations(const Mda &L);
	void setTemplates(const Mda &X);
	void setPrimaryChannels(const Mda &X);
	void setRaw(DiskArrayModel *X,bool own_it);
	void setTimesLabels(const Mda &times,const Mda &labels);
	void setCrossCorrelogramsPath(const QString &path);

	void setClips(DiskArrayModel *X,bool own_it);
	void setClipsIndex(const Mda &X);

	void updateWidgets();

protected:
	void resizeEvent(QResizeEvent *evt);

private slots:
	void slot_spike_templates_current_x_changed();
	void slot_cross_correlograms_current_unit_changed();
	void slot_cross_correlograms_selected_units_changed();
	void slot_statistics_widget_current_unit_changed();
	void slot_statistics_widget_selected_units_changed();
	void slot_unit_activated(int num);
	void slot_current_clip_number_changed();
	void slot_cdf_view_current_label_changed();
	void slot_cdf_view_current_timepoint_changed();
	void slot_current_raw_timepoint_changed();
	void slot_compare_neurons();
	void slot_explore_neuron();

private:
	MVOverviewWidgetPrivate *d;

signals:

public slots:
};

#endif // MVOVERVIEWWIDGET_H
