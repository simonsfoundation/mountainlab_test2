/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 10/1/2016
*******************************************************/
#ifndef PRVGUIDOWNLOADDIALOG_H
#define PRVGUIDOWNLOADDIALOG_H

#include <QDialog>

class PrvGuiDownloadDialogPrivate;
class PrvGuiDownloadDialog : public QDialog {
public:
    friend class PrvGuiDownloadDialogPrivate;
    PrvGuiDownloadDialog();
    virtual ~PrvGuiDownloadDialog();
    void setServerNames(QStringList server_names);
    QString selectedServer() const;

private:
    PrvGuiDownloadDialogPrivate* d;
};

#endif // PRVGUIDOWNLOADDIALOG_H
