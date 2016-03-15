#ifndef MOUNTAINVIEWWIDGET_H
#define MOUNTAINVIEWWIDGET_H

#include <QMainWindow>
#include <QWheelEvent>
#include "diskarraymodel.h"
#include "mda.h"

class MountainViewWidgetPrivate;

class MountainViewWidget : public QMainWindow
{
	Q_OBJECT
public:
	friend class MountainViewWidgetPrivate;
	explicit MountainViewWidget(QWidget *parent = 0);
	~MountainViewWidget();

	void setElectrodeLocations(const Mda &L);
    void setTemplates(const Mda &X);
	void setPrimaryChannels(const Mda &X);
    void setRaw(DiskArrayModel *X);
    void setTimesLabels(const Mda &times,const Mda &labels);
    void setCrossCorrelogramsPath(const QString &path);

protected:
	void resizeEvent(QResizeEvent *evt);

private slots:
    void slot_view_labeled_raw_data();
    void slot_view_spike_templates();
    void slot_view_spike_clips();
    void slot_cluster_view();
    void slot_firetrack();
	void slot_statistics();
    void slot_cross_correlograms(int k=0);
	void slot_quit();

    void slot_cross_correlogram_clicked();
    void slot_spike_templates_x_changed();
    void slot_object_destroyed(QObject*);


private:
	MountainViewWidgetPrivate *d;

signals:

public slots:
};

#endif // MOUNTAINVIEWWIDGET_H
