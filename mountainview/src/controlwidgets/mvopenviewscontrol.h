/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/27/2016
*******************************************************/

#ifndef MVOPENVIEWSCONTROL_H
#define MVOPENVIEWSCONTROL_H

#include "mvabstractcontrol.h"

class MVOpenViewsControlPrivate;
class MVOpenViewsControl : public MVAbstractControl {
    Q_OBJECT
public:
    friend class MVOpenViewsControlPrivate;
    MVOpenViewsControl(MVContext* context, MVMainWindow* mw);
    virtual ~MVOpenViewsControl();

    QString title() const Q_DECL_OVERRIDE;
public slots:
    virtual void updateContext() Q_DECL_OVERRIDE;
    virtual void updateControls() Q_DECL_OVERRIDE;

private slots:
    void slot_open_view(QObject* obj);

private:
    MVOpenViewsControlPrivate* d;
};

#endif // MVOPENVIEWSCONTROL_H
