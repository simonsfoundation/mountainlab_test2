/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/27/2016
*******************************************************/

#ifndef MVEVENTFILTERCONTROL_H
#define MVEVENTFILTERCONTROL_H

#include "mvabstractcontrol.h"

class MVEventFilterControlPrivate;
class MVEventFilterControl : public MVAbstractControl {
    Q_OBJECT
public:
    friend class MVEventFilterControlPrivate;
    MVEventFilterControl(MVContext* context, MVMainWindow* mw);
    virtual ~MVEventFilterControl();

    QString title() const Q_DECL_OVERRIDE;
public slots:
    virtual void updateContext() Q_DECL_OVERRIDE;
    virtual void updateControls() Q_DECL_OVERRIDE;

private slots:

private:
    MVEventFilterControlPrivate* d;
};

#endif // MVEVENTFILTERCONTROL_H
