#ifndef SSCONTROLLER_H
#define SSCONTROLLER_H

#include <QObject>

class SSController : public QObject
{
	Q_OBJECT
public:
	SSController();
	~SSController();
	Q_INVOKABLE QWidget *createTimeSeriesWidget();
	Q_INVOKABLE QWidget *createTimeSeriesView();
	Q_INVOKABLE QWidget *createLabelView();
	Q_INVOKABLE QObject *loadArray(QString path);
	Q_INVOKABLE QObject *readArray(QString path);
};


#endif // SSCONTROLLER_H
