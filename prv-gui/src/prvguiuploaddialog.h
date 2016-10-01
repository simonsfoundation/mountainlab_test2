/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 10/1/2016
*******************************************************/
#ifndef PRVGUIUPLOADDIALOG_H
#define PRVGUIUPLOADDIALOG_H

#include <QDialog>

class PrvGuiUploadDialogPrivate;
class PrvGuiUploadDialog : public QDialog {
public:
    friend class PrvGuiUploadDialogPrivate;
    PrvGuiUploadDialog();
    virtual ~PrvGuiUploadDialog();
    void setServerNames(QStringList server_names);
    QString selectedServer() const;

private:
    PrvGuiUploadDialogPrivate* d;
};

#endif // PRVGUIUPLOADDIALOG_H
