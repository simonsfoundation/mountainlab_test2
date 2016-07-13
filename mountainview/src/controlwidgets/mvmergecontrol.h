/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/13/2016
*******************************************************/

#ifndef MVMERGECONTROL
#define MVMERGECONTROL

#include "mvabstractcontrol.h"

class MVMergeControlPrivate;
class MVMergeControl : public MVAbstractControl {
    Q_OBJECT
public:
    friend class MVMergeControlPrivate;
    MVMergeControl(MVContext* context, MVMainWindow* mw);
    virtual ~MVMergeControl();

public slots:
    virtual void updateContext() Q_DECL_OVERRIDE;
    virtual void updateControls() Q_DECL_OVERRIDE;

private:
    MVMergeControlPrivate* d;
};

#endif // MVMERGECONTROL
