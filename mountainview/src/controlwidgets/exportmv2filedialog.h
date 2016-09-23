#ifndef EXPORTMV2FILEDIALOG_H
#define EXPORTMV2FILEDIALOG_H

#include <QDialog>

namespace Ui {
class ExportMV2FileDialog;
}

class ExportMV2FileDialog : public QDialog {
    Q_OBJECT

public:
    explicit ExportMV2FileDialog(QWidget* parent = 0);
    ~ExportMV2FileDialog();

    bool ensureLocal() const;
    bool ensureRemote() const;
    bool rawOnly() const;
    QString server() const;

private slots:
    void slot_update_enabled();

private:
    Ui::ExportMV2FileDialog* ui;
};

#endif // EXPORTMV2FILEDIALOG_H
