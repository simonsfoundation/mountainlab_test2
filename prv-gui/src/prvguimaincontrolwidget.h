/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 10/1/2016
*******************************************************/
#ifndef PRVGUIMAINCONTROLWIDGET_H
#define PRVGUIMAINCONTROLWIDGET_H

#include <QWidget>
#include "prvguimainwindow.h"

class PrvGuiMainControlWidgetPrivate;
class PrvGuiMainControlWidget : public QWidget {
    Q_OBJECT
public:
    friend class PrvGuiMainControlWidgetPrivate;
    PrvGuiMainControlWidget(PrvGuiMainWindow* MW);
    virtual ~PrvGuiMainControlWidget();

private slots:
    void slot_search_again();
    void slot_update_enabled();
    void slot_upload();
    void slot_download();
    void slot_regenerate();
    void slot_save();
    void slot_save_as();

    void slot_uploader_finished();

private:
    PrvGuiMainControlWidgetPrivate* d;
};

#endif // PRVGUIMAINCONTROLWIDGET_H
