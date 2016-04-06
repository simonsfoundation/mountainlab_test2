/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/6/2016
*******************************************************/

#ifndef MBMAINWINDOW_H
#define MBMAINWINDOW_H

#include "mbexperimentmanager.h"

#include <QWidget>


class MBMainWindowPrivate;
class MBMainWindow : public QWidget
{
	Q_OBJECT
public:
	friend class MBMainWindowPrivate;
	MBMainWindow();
	virtual ~MBMainWindow();
    void setExperimentManager(MBExperimentManager *M);
private:
	MBMainWindowPrivate *d;
};

#endif // MBMAINWINDOW_H

