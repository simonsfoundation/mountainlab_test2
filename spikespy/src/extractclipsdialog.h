#ifndef EXTRACTCLIPSDIALOG_H
#define EXTRACTCLIPSDIALOG_H

#include <QDialog>

namespace Ui {
class ExtractClipsDialog;
}

class ExtractClipsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ExtractClipsDialog(QWidget *parent = 0);
	~ExtractClipsDialog();
	QVariant param(QString name);
	virtual void accept();
	void setComparisonMode(bool val);

private:
	Ui::ExtractClipsDialog *ui;
};

#endif // EXTRACTCLIPSDIALOG_H
