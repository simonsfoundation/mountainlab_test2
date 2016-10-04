/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 10/1/2016
*******************************************************/

#include "prvguiuploaddialog.h"
#include "ui_prvguiuploaddialog.h"

class PrvGuiUploadDialogPrivate {
public:
    PrvGuiUploadDialog* q;
    Ui_PrvGuiUploadDialog ui;
};

PrvGuiUploadDialog::PrvGuiUploadDialog()
{
    d = new PrvGuiUploadDialogPrivate;
    d->q = this;
    d->ui.setupUi(this);
}

PrvGuiUploadDialog::~PrvGuiUploadDialog()
{
    delete d;
}

void PrvGuiUploadDialog::setServerNames(QStringList server_names)
{
    foreach (QString name, server_names) {
        d->ui.servers->addItem(name);
    }
}

QString PrvGuiUploadDialog::selectedServer() const
{
    return d->ui.servers->currentText();
}
