#ifndef MVAMPHISTVIEW_H
#define MVAMPHISTVIEW_H

#include "mvabstractview.h"


class MVAmpHistViewPrivate;
class MVAmpHistView : public MVAbstractView
{
public:
    friend class MVAmpHistViewPrivate;
    MVAmpHistView(MVViewAgent *view_agent);
    virtual ~MVAmpHistView();

    void prepareCalculation() Q_DECL_OVERRIDE;
    void runCalculation() Q_DECL_OVERRIDE;
    void onCalculationFinished() Q_DECL_OVERRIDE;


private:
    MVAmpHistViewPrivate *d;
};

#endif // MVAMPHISTVIEW_H

