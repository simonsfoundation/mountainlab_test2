/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/20/2016
*******************************************************/

#ifndef TABBERFRAME_H
#define TABBERFRAME_H

#include "mvabstractview.h"

class TabberFramePrivate;
class TabberFrame : public QWidget {
    Q_OBJECT
public:
    friend class TabberFramePrivate;
    TabberFrame(MVAbstractView* view);
    virtual ~TabberFrame();
    MVAbstractView* view();
private slots:
    void slot_update_action_visibility();

private:
    TabberFramePrivate* d;
};

#endif // TABBERFRAME_H
