/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/6/2016
*******************************************************/

#ifndef MBEXPERIMENTLISTWIDGET2_H
#define MBEXPERIMENTLISTWIDGET2_H

#include <QWebView>
#include "mbexperimentmanager.h"

class MBExperimentListWidget2Private;
class MBExperimentListWidget2 : public QWebView
{
    Q_OBJECT
public:
    friend class MBExperimentListWidget2Private;
    MBExperimentListWidget2();
    virtual ~MBExperimentListWidget2();
    void setExperimentManager(MBExperimentManager *EM);
    void refresh();
signals:
    void experimentActivated(QString id);
private slots:
    void slot_link_clicked(QUrl url);
private:
    MBExperimentListWidget2Private *d;
};

#endif // MBEXPERIMENTLISTWIDGET2_H

