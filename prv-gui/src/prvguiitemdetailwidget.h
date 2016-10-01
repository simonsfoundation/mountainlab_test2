/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 10/1/2016
*******************************************************/
#ifndef PRVGUIITEMDETAILWIDGET_H
#define PRVGUIITEMDETAILWIDGET_H

#include "prvguitreewidget.h"

class PrvGuiItemDetailWidgetPrivate;
class PrvGuiItemDetailWidget : public QWidget {
    Q_OBJECT
public:
    friend class PrvGuiItemDetailWidgetPrivate;
    PrvGuiItemDetailWidget(PrvGuiTreeWidget* TW);
    virtual ~PrvGuiItemDetailWidget();
private slots:
    void slot_refresh();

private:
    PrvGuiItemDetailWidgetPrivate* d;
};

#endif // PRVGUIITEMDETAILWIDGET_H
