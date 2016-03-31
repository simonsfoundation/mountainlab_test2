/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/31/2016
*******************************************************/

#include <QProgressDialog>
#include <QTime>
#include <QCoreApplication>

void set_progress(QString title, QString text, float frac)
{
    static QProgressDialog *m_progress_dialog=0;
    static QTime* timer = 0;
    if (!m_progress_dialog) {
        m_progress_dialog = new QProgressDialog;
        m_progress_dialog->setCancelButton(0);
    }
    if (!timer) {
        timer = new QTime;
        timer->start();
        m_progress_dialog->show();
        m_progress_dialog->repaint();
        qApp->processEvents();
    }
    if (timer->elapsed() > 500) {
        timer->restart();
        if (!m_progress_dialog->isVisible()) {
            m_progress_dialog->show();
        }
        m_progress_dialog->setLabelText(text);
        m_progress_dialog->setWindowTitle(title);
        m_progress_dialog->setValue((int)(frac * 100));
        m_progress_dialog->repaint();
        qApp->processEvents();
    }
    if (frac >= 1) {
        delete m_progress_dialog;
        m_progress_dialog = 0;
    }
}

