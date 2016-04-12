#include "extractclipsdialog.h"
#include "ui_extractclipsdialog.h"

#include <QMessageBox>

ExtractClipsDialog::ExtractClipsDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ExtractClipsDialog)
{
	setModal(true);
	ui->setupUi(this);
	this->setWindowTitle("Extract Clips");

	ui->fixed_clipsize->setChecked(true);
}

ExtractClipsDialog::~ExtractClipsDialog()
{
	delete ui;
}

QVariant ExtractClipsDialog::param(QString name)
{
	if (name=="labels") return ui->labels->text();
	else if (name=="clipsize") return ui->clipsize->text().toInt();
	else if (name=="fixed_clipsize") return ui->fixed_clipsize->isChecked();
	else if (name=="save_to_disk") return ui->save_to_disk->isChecked();
	else return "";
}

void ExtractClipsDialog::accept()
{
	if ((param("save_to_disk").toBool())&&(!param("fixed_clipsize").toBool())) {
		QMessageBox::information(0,"Extract clips","You must select a fixed clip size for saving to disk.");
		return;
	}
	QDialog::accept();
}

void ExtractClipsDialog::setComparisonMode(bool val)
{
	if (!val) {
		ui->save_to_disk->show();
		ui->fixed_clipsize->show();
	}
	else {
		ui->save_to_disk->hide();
		ui->fixed_clipsize->hide();
		if (ui->labels->text().split(",").count()!=2) {
			ui->labels->setText("1,1");
		}
	}
}
