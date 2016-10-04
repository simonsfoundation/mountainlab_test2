/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 10/1/2016
*******************************************************/
#ifndef PRVGUIMAINCONTROLWIDGET_H
#define PRVGUIMAINCONTROLWIDGET_H

#include <QWidget>
#include "prvguitreewidget.h"

class PrvGuiMainControlWidgetPrivate;
class PrvGuiMainControlWidget : public QWidget {
    Q_OBJECT
public:
    friend class PrvGuiMainControlWidgetPrivate;
    PrvGuiMainControlWidget(PrvGuiTreeWidget* TW);
    virtual ~PrvGuiMainControlWidget();

private slots:
    void slot_refresh_tree();
    void slot_update_enabled();
    void slot_upload();
    void slot_download();
    void slot_regenerate();

private:
    PrvGuiMainControlWidgetPrivate* d;
};

#endif // PRVGUIMAINCONTROLWIDGET_H
