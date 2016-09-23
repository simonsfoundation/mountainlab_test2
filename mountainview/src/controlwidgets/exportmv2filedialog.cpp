#include "exportmv2filedialog.h"
#include "ui_exportmv2filedialog.h"
#include "mlcommon.h"

#include <QJsonArray>
#include <QJsonObject>

ExportMV2FileDialog::ExportMV2FileDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ExportMV2FileDialog)
{
    ui->setupUi(this);

    QJsonArray servers = MLUtil::configValue("prv", "servers").toArray();
    for (int i = 0; i < servers.count(); i++) {
        QString server_name = servers[i].toObject()["name"].toString();
        ui->server->addItem(server_name, server_name);
    }

    QObject::connect(ui->ensure_remote, SIGNAL(stateChanged(int)), this, SLOT(slot_update_enabled()));

    slot_update_enabled();
}

ExportMV2FileDialog::~ExportMV2FileDialog()
{
    delete ui;
}

bool ExportMV2FileDialog::ensureLocal() const
{
    return ui->ensure_local->isChecked();
}

bool ExportMV2FileDialog::ensureRemote() const
{
    return ui->ensure_remote->isChecked();
}

bool ExportMV2FileDialog::rawOnly() const
{
    return ui->raw_only->isChecked();
}

QString ExportMV2FileDialog::server() const
{
    return ui->server->currentData().toString();
}

void ExportMV2FileDialog::slot_update_enabled()
{
    ui->server->setEnabled(ui->ensure_remote->isChecked());
}
