#ifndef MVOVERVIEW2WIDGET_H
#define MVOVERVIEW2WIDGET_H

#include <QWidget>
#include "mda.h"
#include <QTabWidget>
#include "mvutils.h"

class MVOverview2WidgetPrivate;
class CustomTabWidget;
class MVOverview2Widget : public QWidget
{
	Q_OBJECT
public:
	friend class MVOverview2WidgetPrivate;
	friend class CustomTabWidget;
	MVOverview2Widget(QWidget *parent=0);
	virtual ~MVOverview2Widget();
    void addRawPath(const QString &name,const QString &path);
    void setCurrentRawDataName(const QString &name);
	void setFiringsPath(const QString &firings);
	void setSamplingFrequency(float freq);
	void setDefaultInitialization();
    void setEpochs(const QList<Epoch> &epochs);

protected:
	void resizeEvent(QResizeEvent *evt);

signals:

public slots:

private slots:
	void slot_control_panel_button_clicked(QString str);
    void slot_control_panel_combobox_activated(QString str);
	void slot_auto_correlogram_activated(int k);
    //void slot_templates_clicked();
	void slot_details_current_k_changed();
	void slot_details_selected_ks_changed();
    void slot_details_template_activated();
    void slot_cross_correlogram_current_unit_changed();
	void slot_cross_correlogram_selected_units_changed();
	void slot_clips_view_current_event_changed();
	void slot_cluster_view_current_event_changed();

private:
	MVOverview2WidgetPrivate *d;
};

class CustomTabWidget : public QTabWidget {
	Q_OBJECT
public:
	MVOverview2Widget *q;
	CustomTabWidget(MVOverview2Widget *q);
protected:
	void mousePressEvent(QMouseEvent *evt);
private slots:
	void slot_tab_close_requested(int num);
	void slot_tab_bar_clicked();
	void slot_tab_bar_double_clicked();
	void slot_switch_to_other_tab_widget();
};


#endif // MVOVERVIEW2WIDGET_H
