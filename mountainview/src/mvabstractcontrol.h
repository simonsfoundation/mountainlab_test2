/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/27/2016
*******************************************************/

#ifndef MVABSTRACTCONTROL_H
#define MVABSTRACTCONTROL_H

#include <QCheckBox>
#include <QComboBox>

#include "mvviewagent.h"
#include "mvmainwindow.h"

class MVAbstractControlPrivate;
class MVAbstractControl : public QWidget {
public:
    friend class MVAbstractControlPrivate;
    MVAbstractControl(MVContext* context, MVMainWindow* mw);
    virtual ~MVAbstractControl();

    virtual QString title() = 0;
public slots:
    virtual void updateContext() = 0;
    virtual void updateControls() = 0;

protected:
    MVContext* mvContext();
    MVMainWindow* mainWindow();

    QVariant controlValue(QString name) const;
    void setControlValue(QString name, QVariant val);
    void setChoices(QString name,const QStringList &choices);
    void setControlEnabled(QString name, bool val);

    QWidget* createIntControl(QString name);
    QWidget* createDoubleControl(QString name);
    QComboBox* createChoicesControl(QString name);
    QCheckBox* createCheckBoxControl(QString name);

private slots:
    void slot_controls_changed();

private:
    MVAbstractControlPrivate* d;
};

#endif // MVABSTRACTCONTROL_H
