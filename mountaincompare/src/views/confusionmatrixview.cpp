/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/29/2016
*******************************************************/

#include "confusionmatrixview.h"

#include <QHBoxLayout>
#include <mountainprocessrunner.h>
#include <taskprogress.h>
#include "actionfactory.h"
#include "matrixview.h"

class ConfusionMatrixViewCalculator {
public:
    //input
    QString mlproxy_url;
    DiskReadMda firings1;
    DiskReadMda firings2;

    //output
    DiskReadMda confusion_matrix;

    virtual void compute();

    bool loaded_from_static_output = false;
    QJsonObject exportStaticOutput();
    void loadStaticOutput(const QJsonObject& X);
};

class ConfusionMatrixViewPrivate {
public:
    ConfusionMatrixView* q;

    ConfusionMatrixViewCalculator m_calculator;
    Mda m_confusion_matrix;
    MatrixView *m_matrix_view; //raw numbers
    MatrixView *m_matrix_view_rn; //row normalized
    MatrixView *m_matrix_view_cn; //column normalized
    Mda row_normalize(const Mda &A);
    Mda column_normalize(const Mda &A);
};

ConfusionMatrixView::ConfusionMatrixView(MVContext* mvcontext)
    : MVAbstractView(mvcontext)
{
    d = new ConfusionMatrixViewPrivate;
    d->q = this;

    d->m_matrix_view=new MatrixView;
    d->m_matrix_view->setMode(MatrixView::CountsMode);
    d->m_matrix_view_rn=new MatrixView;
    d->m_matrix_view_rn->setMode(MatrixView::PercentMode);
    d->m_matrix_view_cn=new MatrixView;
    d->m_matrix_view_cn->setMode(MatrixView::PercentMode);


    QHBoxLayout *hlayout=new QHBoxLayout;
    this->setLayout(hlayout);
    hlayout->addWidget(d->m_matrix_view);
    hlayout->addWidget(d->m_matrix_view_rn);
    hlayout->addWidget(d->m_matrix_view_cn);

    if (!mcContext()) {
        qCritical() << "mcContext is null" << __FILE__ << __LINE__;
        return;
    }

    this->recalculateOn(mcContext(), SIGNAL(firingsChanged()), false);
    this->recalculateOn(mcContext(), SIGNAL(firings2Changed()), false);

    this->recalculate();
}

ConfusionMatrixView::~ConfusionMatrixView()
{
    this->stopCalculation();
    delete d;
}

void ConfusionMatrixView::prepareCalculation()
{
    if (!mcContext()) return;

    d->m_calculator.mlproxy_url = mcContext()->mlProxyUrl();
    d->m_calculator.firings1 = mcContext()->firings();
    d->m_calculator.firings2 = mcContext()->firings2();
}

void ConfusionMatrixView::runCalculation()
{
    d->m_calculator.compute();
}

void ConfusionMatrixView::onCalculationFinished()
{
    int A1=d->m_calculator.confusion_matrix.N1();
    int A2=d->m_calculator.confusion_matrix.N2();
    d->m_calculator.confusion_matrix.readChunk(d->m_confusion_matrix,0,0,A1,A2);

    d->m_matrix_view->setMatrix(d->m_confusion_matrix);
    d->m_matrix_view->setValueRange(0,d->m_confusion_matrix.maximum());
    d->m_matrix_view_rn->setMatrix(d->row_normalize(d->m_confusion_matrix));
    d->m_matrix_view_cn->setMatrix(d->column_normalize(d->m_confusion_matrix));
}

MCContext *ConfusionMatrixView::mcContext()
{
    return qobject_cast<MCContext *>(mvContext());
}

void ConfusionMatrixView::keyPressEvent(QKeyEvent* evt)
{
    /*
    if (evt->key() == Qt::Key_Up) {
        slot_vertical_zoom_in();
    }
    if (evt->key() == Qt::Key_Down) {
        slot_vertical_zoom_out();
    }
    */
}

void ConfusionMatrixView::prepareMimeData(QMimeData& mimeData, const QPoint& pos)
{
    /*
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << mvContext()->selectedClusters();
    mimeData.setData("application/x-mv-clusters", ba); // selected cluster data
    */

    MVAbstractView::prepareMimeData(mimeData, pos); // call base class implementation
}

void ConfusionMatrixViewCalculator::compute()
{
    TaskProgress task(TaskProgress::Calculate, "Compute confusion matrix");
    if (this->loaded_from_static_output) {
        task.log("Loaded from static output");
        return;
    }

    QTime timer;
    timer.start();
    task.setProgress(0.1);

    MountainProcessRunner MPR;
    MPR.setProcessorName("confusion_matrix");

    QMap<QString, QVariant> params;
    params["firings1"] = firings1.makePath();
    params["firings2"] = firings2.makePath();
    params["max_matching_offset"] = 6;
    MPR.setInputParameters(params);
    MPR.setMLProxyUrl(mlproxy_url);

    task.log() << "Firings 1/2 dimensions" << firings1.N1() << firings1.N2() << firings2.N1() << firings2.N2();

    QString output_path = MPR.makeOutputFilePath("output");

    MPR.runProcess();

    if (MLUtil::threadInterruptRequested()) {
        task.error(QString("Halted while running process."));
        return;
    }

    confusion_matrix.setPath(output_path);
    task.log() << "Output path:" << output_path;
    task.log() << "Confusion matrix dimensions:" << confusion_matrix.N1() << confusion_matrix.N2();
}

ConfusionMatrixViewFactory::ConfusionMatrixViewFactory(MVContext* context, QObject* parent)
    : MVAbstractViewFactory(context, parent)
{
}

QString ConfusionMatrixViewFactory::id() const
{
    return QStringLiteral("open-confusion-matrix");
}

QString ConfusionMatrixViewFactory::name() const
{
    return tr("Confusion Matrix");
}

QString ConfusionMatrixViewFactory::title() const
{
    return tr("Confusion Matrix");
}

MVAbstractView* ConfusionMatrixViewFactory::createView(QWidget* parent)
{
    Q_UNUSED(parent)
    ConfusionMatrixView* X = new ConfusionMatrixView(mvContext());

    if (!X->mcContext()) {
        qCritical() << "mcContext is null" << __FILE__ << __LINE__;
        delete X;
        return 0;
    }

    return X;
}

Mda ConfusionMatrixViewPrivate::row_normalize(const Mda &A)
{
    Mda B=A;
    int M=B.N1();
    int N=B.N2();
    for (int m=0; m<M; m++) {
        double sum=0;
        for (int n=0; n<N; n++) {
            sum+=B.value(m,n);
        }
        if (sum) {
            for (int n=0; n<N; n++) {
                B.setValue(B.value(m,n)/sum,m,n);
            }
        }
    }
    return B;
}

Mda ConfusionMatrixViewPrivate::column_normalize(const Mda &A)
{
    Mda B=A;
    int M=B.N1();
    int N=B.N2();
    for (int n=0; n<N; n++) {
        double sum=0;
        for (int m=0; m<M; m++) {
            sum+=B.value(m,n);
        }
        if (sum) {
            for (int m=0; m<M; m++) {
                B.setValue(B.value(m,n)/sum,m,n);
            }
        }
    }
    return B;
}
