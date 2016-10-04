/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 10/1/2016
*******************************************************/

#include "prvguidownloaddialog.h"
#include "ui_prvguidownloaddialog.h"

class PrvGuiDownloadDialogPrivate {
public:
    PrvGuiDownloadDialog* q;
    Ui_PrvGuiDownloadDialog ui;
};

PrvGuiDownloadDialog::PrvGuiDownloadDialog()
{
    d = new PrvGuiDownloadDialogPrivate;
    d->q = this;
    d->ui.setupUi(this);
}

PrvGuiDownloadDialog::~PrvGuiDownloadDialog()
{
    delete d;
}

void PrvGuiDownloadDialog::setServerNames(QStringList server_names)
{
    foreach (QString name, server_names) {
        d->ui.servers->addItem(name);
    }
}

QString PrvGuiDownloadDialog::selectedServer() const
{
    return d->ui.servers->currentText();
}
