#ifndef TABBER_H
#define TABBER_H

#include <QTabWidget>
#include <QString>

class TabberPrivate;
class Tabber : public QObject
{
	Q_OBJECT
public:
	friend class TabberPrivate;
	Tabber();
	virtual ~Tabber();
	void addTabWidget(const QString &container_name,QTabWidget *TW);
	void addWidget(const QString &container_name,const QString &label,QWidget *W);
	void addWidget(QTabWidget *TW,const QString &label,QWidget *W);
	QString currentContainerName();
	QList<QWidget *> allWidgets();
private slots:
	void slot_tab_close_requested(int index);
private:
	TabberPrivate *d;
};

#endif // TABBER_H

