/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 10/1/2016
*******************************************************/

#include "prvguiitemdetailwidget.h"

#include <QTextEdit>
#include <QVBoxLayout>

class PrvGuiItemDetailWidgetPrivate {
public:
    PrvGuiItemDetailWidget* q;
    PrvGuiTreeWidget* m_tree;
    QTextEdit* m_text_edit;
};

PrvGuiItemDetailWidget::PrvGuiItemDetailWidget(PrvGuiTreeWidget* TW)
{
    d = new PrvGuiItemDetailWidgetPrivate;
    d->q = this;
    d->m_tree = TW;

    d->m_text_edit = new QTextEdit;
    d->m_text_edit->setReadOnly(true);

    QVBoxLayout* vlayout = new QVBoxLayout;
    vlayout->addWidget(d->m_text_edit);
    this->setLayout(vlayout);

    QObject::connect(d->m_tree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(slot_refresh()));
}

PrvGuiItemDetailWidget::~PrvGuiItemDetailWidget()
{
    delete d;
}

void PrvGuiItemDetailWidget::slot_refresh()
{
    QVariantMap details = d->m_tree->currentItemDetails();
    QString txt;
    txt += details["local_path"].toString() + "\n";
    QVariantMap server_urls = details["server_urls"].toMap();
    QStringList servers = server_urls.keys();
    foreach (QString server, servers) {
        QString url = server_urls[server].toString();
        if (!url.isEmpty()) {
            txt += url + "\n";
        }
    }
    d->m_text_edit->setPlainText(txt);
}
