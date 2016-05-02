/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/30/2016
*******************************************************/

#include "taskprogressview.h"
#include "taskprogress.h"

class TaskProgressViewPrivate {
public:
    TaskProgressView *q;
};

TaskProgressView::TaskProgressView()
{
    d=new TaskProgressViewPrivate;
    d->q=this;
}

TaskProgressView::~TaskProgressView()
{
    delete d;
}
