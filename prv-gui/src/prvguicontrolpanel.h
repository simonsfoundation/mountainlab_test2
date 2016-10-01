/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 10/1/2016
*******************************************************/
#ifndef PRVGUICONTROLPANEL_H
#define PRVGUICONTROLPANEL_H

#include <QWidget>

class PrvGuiControlPanelPrivate;
class PrvGuiControlPanel : public QWidget {
public:
    friend class PrvGuiControlPanelPrivate;
    PrvGuiControlPanel();
    virtual ~PrvGuiControlPanel();
    void addControlWidget(QString label, QWidget* W);

private:
    PrvGuiControlPanelPrivate* d;
};

#endif // PRVGUICONTROLPANEL_H
