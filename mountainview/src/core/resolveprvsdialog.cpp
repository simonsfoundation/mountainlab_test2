/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 10/1/2016
*******************************************************/

#include "resolveprvsdialog.h"

class ResolvePrvsDialogPrivate {
public:
    ResolvePrvsDialog* q;
};

ResolvePrvsDialog::ResolvePrvsDialog()
{
    d = new ResolvePrvsDialogPrivate;
    d->q = this;
}

ResolvePrvsDialog::~ResolvePrvsDialog()
{
    delete d;
}
