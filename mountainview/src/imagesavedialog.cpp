#include "imagesavedialog.h"
#include <QImage>
#include <QLabel>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QSettings>
#include <QFileDialog>
#include <QImageWriter>
#include <QMessageBox>

class ImageSaveDialogPrivate {
public:
    ImageSaveDialog* q;
    QImage m_image;
    QLabel* m_label;
    QLineEdit* m_directory_edit;
    QLineEdit* m_file_edit;
};

ImageSaveDialog::ImageSaveDialog()
{
    d = new ImageSaveDialogPrivate;
    d->q = this;

    d->m_label = new QLabel;
    d->m_directory_edit = new QLineEdit;
    d->m_directory_edit->setReadOnly(true);
    d->m_file_edit = new QLineEdit;
    d->m_file_edit->setMaximumWidth(150);
    d->m_file_edit->setText("img.jpg");

    QSettings settings("SCDA", "mountainview");
    QString last_image_save_dir = settings.value("last_image_save_dir", QDir::homePath()).toString();
    d->m_directory_edit->setText(last_image_save_dir);

    QToolButton* save_button = new QToolButton;
    save_button->setText("Save");
    save_button->setToolTip("Save image");
    connect(save_button, SIGNAL(clicked(bool)), this, SLOT(slot_save()));

    QToolButton* save_as_button = new QToolButton;
    save_as_button->setText("Save As...");
    save_as_button->setToolTip("Save image as...");
    connect(save_as_button, SIGNAL(clicked(bool)), this, SLOT(slot_save_as()));

    QToolButton* cancel_button = new QToolButton;
    cancel_button->setText("Cancel");
    cancel_button->setToolTip("Cancel image export");
    connect(cancel_button, SIGNAL(clicked(bool)), this, SLOT(slot_cancel()));

    QVBoxLayout* vlayout = new QVBoxLayout;
    vlayout->addWidget(d->m_label);
    QHBoxLayout* hlayout = new QHBoxLayout;
    vlayout->addLayout(hlayout);
    hlayout->addWidget(d->m_directory_edit);
    hlayout->addWidget(d->m_file_edit);
    hlayout->addWidget(save_button);
    hlayout->addWidget(save_as_button);
    hlayout->addWidget(cancel_button);
    this->setLayout(vlayout);
}

ImageSaveDialog::~ImageSaveDialog()
{
    delete d;
}

void ImageSaveDialog::presentImage(const QImage& img)
{
    ImageSaveDialog dlg;
    dlg.setImage(img);
    dlg.setModal(true);
    dlg.exec();
}

void ImageSaveDialog::setImage(const QImage& img)
{
    d->m_image = img;
    d->m_label->setPixmap(QPixmap::fromImage(img));
    d->m_label->resize(img.width(), img.height());
}

void ImageSaveDialog::slot_save()
{
    QString fname = d->m_file_edit->text();
    QString dirname = d->m_directory_edit->text();
    QString full_fname;
    if (fname.isEmpty()) {
        full_fname = QFileDialog::getSaveFileName(0, "Save image", dirname, "Image Files (*.png *.jpg *.bmp)");
    } else {
        full_fname = dirname + "/" + fname;
    }
    QSettings settings("SCDA", "mountainview");
    settings.setValue("last_image_save_dir", QFileInfo(full_fname).path());
    QImageWriter writer(full_fname);
    if (!writer.write(d->m_image)) {
        QMessageBox::critical(0, "Error writing image", "Unable to write image. Please contact us for help with this option.");
    }
    this->accept();
}

void ImageSaveDialog::slot_save_as()
{
    d->m_file_edit->setText("");
    slot_save();
}

void ImageSaveDialog::slot_cancel()
{
    this->reject();
}
