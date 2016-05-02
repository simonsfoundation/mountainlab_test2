/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/30/2016
*******************************************************/

#ifndef TASKPROGRESSVIEW_H
#define TASKPROGRESSVIEW_H

#include <QWidget>

class TaskProgressViewPrivate;
class TaskProgressView : public QWidget {
    Q_OBJECT
public:
    friend class TaskProgressViewPrivate;
    TaskProgressView();
    virtual ~TaskProgressView();
private slots:
    void slot_refresh();

private:
    TaskProgressViewPrivate* d;
};

#endif // TASKPROGRESSVIEW_H
