#include "mvamphistview.h"

class MVAmpHistViewPrivate {
public:
    MVAmpHistView *q;
};

MVAmpHistView::MVAmpHistView(MVViewAgent *view_agent) : MVAbstractView(view_agent)
{
    d=new MVAmpHistViewPrivate;
    d->q=this;
}

MVAmpHistView::~MVAmpHistView()
{
    delete d;
}
