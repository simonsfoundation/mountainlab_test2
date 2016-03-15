#ifndef MVNEURONCOMPARISONWIDGET_H
#define MVNEURONCOMPARISONWIDGET_H

#include <QWidget>
#include <QThread>
#include <QWheelEvent>
#include "diskarraymodel.h"
#include "mda.h"

class MVNeuronComparisonWidgetPrivate;

class MVNeuronComparisonWidget : public QWidget
{
	Q_OBJECT
public:
    friend class MVNeuronComparisonWidgetPrivate;
    explicit MVNeuronComparisonWidget(QWidget *parent = 0);
    ~MVNeuronComparisonWidget();

	void setElectrodeLocations(const Mda &L);
	void setRaw(DiskArrayModel *X,bool own_it);
	void setTimesLabels(const Mda &times,const Mda &labels);
	void setCrossCorrelogramsPath(const QString &path);

	void setClips(const QList<DiskArrayModel *> &C,bool own_it);
	void setUnitNumbers(const QList<int> &numbers);

	void updateWidgets();

signals:
	void currentClipNumberChanged();

protected:
	void resizeEvent(QResizeEvent *evt);

private slots:
	void slot_compute_templates();
	void slot_clips_view_current_x_changed();
	void slot_selected_data_points_changed();


private:
    MVNeuronComparisonWidgetPrivate *d;

signals:

public slots:
};



#endif // MVNEURONCOMPARISONWIDGET_H
