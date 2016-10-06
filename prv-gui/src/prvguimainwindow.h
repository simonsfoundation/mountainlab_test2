/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 9/29/2016
*******************************************************/
#ifndef PRVGUIMAINWINDOW_H
#define PRVGUIMAINWINDOW_H

#include <QJsonObject>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QThread>
#include <QWidget>
#include <QVariant>

#include "prvgui.h"
#include "prvguitreewidget.h"

class PrvGuiMainWindowPrivate;
class PrvGuiMainWindow : public QWidget {
    Q_OBJECT
public:
    friend class PrvGuiMainWindowPrivate;
    PrvGuiMainWindow();
    virtual ~PrvGuiMainWindow();
    bool loadPrv(QString prv_file_name);
    bool savePrv(QString prv_file_name);
    void setServerNames(QStringList names);
    void refresh();
    void startAllSearches();
    PrvGuiTreeWidget* tree();
    QString prvFileName() const;
    void setPrvFileName(QString fname);

    void searchAgain(QString checksum,long size,QString server);

protected:
    void closeEvent(QCloseEvent* event) Q_DECL_OVERRIDE;

signals:
    void prvFileNameChanged();
private slots:
    void slot_update_window_title();

protected:
    void resizeEvent(QResizeEvent* evt);

private:
    PrvGuiMainWindowPrivate* d;
};

#endif // PRVGUIMAINWINDOW_H
