/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/13/2016
*******************************************************/

#include "mvmergecontrol.h"

class MVMergeControlPrivate {
public:
    MVMergeControl* q;
};

MVMergeControl::MVMergeControl(MVContext* context, MVMainWindow* mw)
    : MVAbstractControl(context, mw)
{
    d = new MVMergeControlPrivate;
    d->q = this;
}

MVMergeControl::~MVMergeControl()
{
    delete d;
}

void MVMergeControl::updateContext()
{
}

void MVMergeControl::updateControls()
{
}
