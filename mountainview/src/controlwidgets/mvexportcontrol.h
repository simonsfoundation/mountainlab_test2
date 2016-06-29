/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/27/2016
*******************************************************/

#ifndef MVEXPORTCONTROL_H
#define MVEXPORTCONTROL_H

#include "mvabstractcontrol.h"

class MVExportControlPrivate;
class MVExportControl : public MVAbstractControl {
    Q_OBJECT
public:
    friend class MVExportControlPrivate;
    MVExportControl(MVContext* context, MVMainWindow* mw);
    virtual ~MVExportControl();

    QString title() Q_DECL_OVERRIDE;
public slots:
    virtual void updateContext() Q_DECL_OVERRIDE;
    virtual void updateControls() Q_DECL_OVERRIDE;

private slots:
    void slot_export_mv_document();
    void slot_export_firings_array();

private:
    MVExportControlPrivate* d;
};

#endif // MVExportControl_H
