#ifndef MVUNITWIDGET_H
#define MVUNITWIDGET_H

#include <QWidget>
#include <QThread>
#include <QWheelEvent>
#include "diskarraymodel.h"
#include "mda.h"

class MVUnitWidgetPrivate;

class MVUnitWidget : public QWidget
{
	Q_OBJECT
public:
	friend class MVUnitWidgetPrivate;
	explicit MVUnitWidget(QWidget *parent = 0);
	~MVUnitWidget();

	void setElectrodeLocations(const Mda &L);
	void setTemplates(const Mda &X);
	void setPrimaryChannels(const Mda &X);
	void setRaw(DiskArrayModel *X,bool own_it);
	void setTimesLabels(const Mda &times,const Mda &labels);
	void setCrossCorrelogramsPath(const QString &path);
	int currentClipNumber();

	void setClips(DiskArrayModel *C,bool own_it);
	void setUnitNumber(int num);

	void updateWidgets();

signals:
	void currentClipNumberChanged();

protected:
	void resizeEvent(QResizeEvent *evt);

private slots:
	void slot_compute_template();
	void slot_clips_view_current_x_changed();
	void slot_selected_data_points_changed();


private:
	MVUnitWidgetPrivate *d;

signals:

public slots:
};



#endif // MVUNITWIDGET_H
