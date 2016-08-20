/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 8/18/2016
*******************************************************/
#ifndef COMPARECLUSTERVIEW_H
#define COMPARECLUSTERVIEW_H

#include <mccontext.h>
#include <mvabstractview.h>

class CompareClusterViewPrivate;
class CompareClusterView : public MVAbstractView {
public:
    friend class CompareClusterViewPrivate;
    CompareClusterView(MCContext* context);
    virtual ~CompareClusterView();

    void prepareCalculation() Q_DECL_OVERRIDE;
    void runCalculation() Q_DECL_OVERRIDE;
    void onCalculationFinished() Q_DECL_OVERRIDE;

    MCContext* mcContext();

private:
    CompareClusterViewPrivate* d;
};

#endif // COMPARECLUSTERVIEW_H
