/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/29/2016
*******************************************************/

#include "curationprogramview.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QTextEdit>
#include <mountainprocessrunner.h>
#include <taskprogress.h>
#include "actionfactory.h"
#include <QJSEngine>

class CurationProgramViewPrivate {
public:
    CurationProgramView* q;
    QTextEdit* m_input_editor;
    QTextEdit* m_output_editor;

    void refresh_editor();
};

CurationProgramView::CurationProgramView(MVContext* mvcontext)
    : MVAbstractView(mvcontext)
{
    d = new CurationProgramViewPrivate;
    d->q = this;

    QHBoxLayout* editor_layout = new QHBoxLayout;

    d->m_input_editor = new QTextEdit;
    d->m_input_editor->setLineWrapMode(QTextEdit::NoWrap);
    d->m_input_editor->setTabStopWidth(30);
    editor_layout->addWidget(d->m_input_editor);

    d->m_output_editor = new QTextEdit;
    d->m_output_editor->setLineWrapMode(QTextEdit::NoWrap);
    d->m_output_editor->setTabStopWidth(30);
    d->m_output_editor->setReadOnly(true);
    editor_layout->addWidget(d->m_output_editor);

    QHBoxLayout* button_layout = new QHBoxLayout;
    {
        QPushButton* B = new QPushButton("Apply");
        button_layout->addWidget(B);
        QObject::connect(B, SIGNAL(clicked(bool)), this, SLOT(slot_apply()));
    }
    button_layout->addStretch();

    QVBoxLayout* vlayout = new QVBoxLayout;
    this->setLayout(vlayout);
    vlayout->addLayout(editor_layout);
    vlayout->addLayout(button_layout);

    this->recalculateOnOptionChanged("curation_program", false);

    QObject::connect(d->m_input_editor, SIGNAL(textChanged()), this, SLOT(slot_text_changed()), Qt::QueuedConnection);

    this->recalculate();
}

CurationProgramView::~CurationProgramView()
{
    this->stopCalculation();
    delete d;
}

void CurationProgramView::prepareCalculation()
{
}

void CurationProgramView::runCalculation()
{
}

void CurationProgramView::onCalculationFinished()
{
    d->refresh_editor();
}

void CurationProgramView::slot_text_changed()
{
    QString txt = d->m_input_editor->toPlainText();
    mvContext()->setOption("curation_program", txt);
}

void CurationProgramView::slot_apply()
{
    QJSEngine engine;
    QString program = d->m_input_editor->toPlainText();
    QJSValue ret = engine.evaluate(program);
    QString output;
    if (ret.isError()) {
        output += "ERROR: " + ret.toString() + "\n";
    }
    d->m_output_editor->setText(output);
}

void CurationProgramViewPrivate::refresh_editor()
{
    QString txt = q->mvContext()->option("curation_program").toString();
    if (txt == m_input_editor->toPlainText())
        return;
    m_input_editor->setText(txt);
}
