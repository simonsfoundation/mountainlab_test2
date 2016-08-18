/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/29/2016
*******************************************************/

#include "confusionmatrixview.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <mountainprocessrunner.h>
#include <taskprogress.h>
#include "actionfactory.h"
#include "matrixview.h"
#include "get_sort_indices.h"

class ConfusionMatrixViewCalculator {
public:
    //input
    QString mlproxy_url;
    DiskReadMda firings1;
    DiskReadMda firings2;

    //output
    DiskReadMda confusion_matrix;
    QList<int> optimal_assignments;
    DiskReadMda event_correspondence;

    virtual void compute();

    bool loaded_from_static_output = false;
    QJsonObject exportStaticOutput();
    void loadStaticOutput(const QJsonObject& X);
};

struct CMVControlBar {
    QWidget *widget;
    QMap<QString,QRadioButton*> permutation_buttons;
    CMVControlBar(ConfusionMatrixView *q) {
        widget=new QWidget;
        widget->setFixedHeight(50);
        widget->setFont(QFont("Arial",12));
        QHBoxLayout* hlayout=new QHBoxLayout;
        widget->setLayout(hlayout);

        hlayout->addWidget(new QLabel("Permutation:"));
        {
            QRadioButton *B=new QRadioButton("None");
            B->setProperty("name","none");
            hlayout->addWidget(B);
            permutation_buttons[B->property("name").toString()]=B;
        }
        {
            QRadioButton *B=new QRadioButton("Row");
            B->setProperty("name","row");
            hlayout->addWidget(B);
            permutation_buttons[B->property("name").toString()]=B;
        }
        {
            QRadioButton *B=new QRadioButton("Column");
            B->setProperty("name","column");
            hlayout->addWidget(B);
            permutation_buttons[B->property("name").toString()]=B;
        }
        {
            QRadioButton *B=new QRadioButton("Both1");
            B->setProperty("name","both1");
            hlayout->addWidget(B);
            permutation_buttons[B->property("name").toString()]=B;
        }
        {
            QRadioButton *B=new QRadioButton("Both2");
            B->setProperty("name","both2");
            hlayout->addWidget(B);
            permutation_buttons[B->property("name").toString()]=B;
        }
        foreach (QRadioButton *B,permutation_buttons) {
            QObject::connect(B,SIGNAL(clicked(bool)),q,SLOT(slot_permutation_mode_button_clicked()));
        }

        hlayout->addStretch();

        permutation_buttons["none"]->setChecked(true);
    }
    ConfusionMatrixView::PermutationMode permutationMode() {
        QString str;
        foreach (QRadioButton *B,permutation_buttons) {
            if (B->isChecked()) str=B->property("name").toString();
        }
        if (str=="none") return ConfusionMatrixView::NoPermutation;
        if (str=="row") return ConfusionMatrixView::RowPermutation;
        if (str=="column") return ConfusionMatrixView::ColumnPermutation;
        if (str=="both1") return ConfusionMatrixView::BothRowBasedPermutation;
        if (str=="both2") return ConfusionMatrixView::BothColumnBasedPermutation;
        return ConfusionMatrixView::NoPermutation;
    }
};

class ConfusionMatrixViewPrivate {
public:
    ConfusionMatrixView* q;

    ConfusionMatrixViewCalculator m_calculator;
    Mda m_confusion_matrix;
    QList<int> m_optimal_assignments;
    MatrixView *m_matrix_view; //raw numbers
    MatrixView *m_matrix_view_rn; //row normalized
    MatrixView *m_matrix_view_cn; //column normalized
    QList<MatrixView*> m_all_matrix_views;
    CMVControlBar *m_control_bar;

    Mda row_normalize(const Mda &A);
    Mda column_normalize(const Mda &A);
    void update_permutations();
    void set_current_clusters(int k1,int k2);
};



ConfusionMatrixView::ConfusionMatrixView(MVContext* mvcontext)
    : MVAbstractView(mvcontext)
{
    d = new ConfusionMatrixViewPrivate;
    d->q = this;

    d->m_matrix_view=new MatrixView;
    d->m_matrix_view->setMode(MatrixView::CountsMode);
    d->m_matrix_view->setTitle("Confusion matrix");
    d->m_matrix_view_rn=new MatrixView;
    d->m_matrix_view_rn->setMode(MatrixView::PercentMode);
    d->m_matrix_view_rn->setTitle("Row-normalized");
    d->m_matrix_view_cn=new MatrixView;
    d->m_matrix_view_cn->setMode(MatrixView::PercentMode);
    d->m_matrix_view_cn->setTitle("Column-normalized");

    d->m_all_matrix_views << d->m_matrix_view << d->m_matrix_view_cn << d->m_matrix_view_rn;

    d->m_control_bar=new CMVControlBar(this);

    QHBoxLayout *hlayout=new QHBoxLayout;
    hlayout->addWidget(d->m_matrix_view);
    hlayout->addWidget(d->m_matrix_view_rn);
    hlayout->addWidget(d->m_matrix_view_cn);

    QVBoxLayout *vlayout=new QVBoxLayout;
    this->setLayout(vlayout);
    vlayout->addLayout(hlayout);
    vlayout->addWidget(d->m_control_bar->widget);

    if (!mcContext()) {
        qCritical() << "mcContext is null" << __FILE__ << __LINE__;
        return;
    }

    foreach (MatrixView *MV,d->m_all_matrix_views) {
        QObject::connect(MV,SIGNAL(currentElementChanged()),this,SLOT(slot_matrix_view_current_element_changed()));
    }

    this->recalculateOn(mcContext(), SIGNAL(firingsChanged()), false);
    this->recalculateOn(mcContext(), SIGNAL(firings2Changed()), false);

    //Important to do a queued connection here! because we are changing two things at the same time
    QObject::connect(mcContext(),SIGNAL(currentClusterChanged()),this,SLOT(slot_update_current_elements_based_on_context()),Qt::QueuedConnection);
    QObject::connect(mcContext(),SIGNAL(currentCluster2Changed()),this,SLOT(slot_update_current_elements_based_on_context()),Qt::QueuedConnection);

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
    d->m_optimal_assignments=d->m_calculator.optimal_assignments;

    d->m_matrix_view->setMatrix(d->m_confusion_matrix);
    d->m_matrix_view->setValueRange(0,d->m_confusion_matrix.maximum());
    d->m_matrix_view_rn->setMatrix(d->row_normalize(d->m_confusion_matrix));
    d->m_matrix_view_cn->setMatrix(d->column_normalize(d->m_confusion_matrix));

    QStringList row_labels,col_labels;
    for (int m=0; m<A1-1; m++) {
        row_labels << QString("%1").arg(m+1);
    }
    for (int n=0; n<A2-1; n++) {
        col_labels << QString("%1").arg(n+1);
    }

    foreach (MatrixView *MV,d->m_all_matrix_views) {
        MV->setLabels(row_labels,col_labels);
    }

    d->update_permutations();
    slot_update_current_elements_based_on_context();
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

void ConfusionMatrixView::slot_permutation_mode_button_clicked()
{
    d->update_permutations();
}

void ConfusionMatrixView::slot_matrix_view_current_element_changed()
{
    MatrixView *MV=qobject_cast<MatrixView*>(sender());
    if (!MV) return;
    QPoint a=MV->currentElement();
    if ((a.x()>=0)&&(a.y()>=0)) {
        mcContext()->setCurrentCluster(a.x()+1);
        mcContext()->setCurrentCluster2(a.y()+1);
    }
    else {
        mcContext()->setCurrentCluster(-1);
        mcContext()->setCurrentCluster2(-1);
    }
}

void ConfusionMatrixView::slot_update_current_elements_based_on_context()
{
    foreach (MatrixView *MV,d->m_all_matrix_views) {
        MV->setCurrentElement(QPoint(mcContext()->currentCluster()-1,mcContext()->currentCluster2()-1));
    }
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
    QString optimal_assignments_path = MPR.makeOutputFilePath("optimal_assignments");
    QString event_correspondence_path = MPR.makeOutputFilePath("event_correspondence");

    MPR.runProcess();

    if (MLUtil::threadInterruptRequested()) {
        task.error(QString("Halted while running process."));
        return;
    }

    confusion_matrix.setPath(output_path);
    event_correspondence.setPath(event_correspondence_path);

    optimal_assignments.clear();
    {
        DiskReadMda tmp(optimal_assignments_path);
        for (int i=0; i<tmp.totalSize(); i++) {
            optimal_assignments << tmp.value(i);
        }
    }

    task.log() << "Output path:" << output_path;
    task.log() << "Optimal assignments path:" << optimal_assignments_path;
    task.log() << "Optimal assignments:" << optimal_assignments;
    task.log() << "Confusion matrix dimensions:" << confusion_matrix.N1() << confusion_matrix.N2();
    task.log() << "Event correspondence dimensions:" << event_correspondence.N1() << event_correspondence.N2();
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

void ConfusionMatrixViewPrivate::update_permutations()
{
    int M=m_confusion_matrix.N1();
    int N=m_confusion_matrix.N2();

    QVector<int> perm_rows;
    QVector<int> perm_cols;

    ConfusionMatrixView::PermutationMode permutation_mode=m_control_bar->permutationMode();

    if (permutation_mode==ConfusionMatrixView::RowPermutation) {
        perm_rows.fill(-1,M);
        for (int i=0; i<M-1; i++) {
            int val=m_optimal_assignments.value(i);
            if ((val>=0)&&(val<M-1)) {
                perm_rows[i]=val;
            }
            else {
                perm_rows[i]=-1; //will be filled in later
            }
        }
        for (int i=0; i<M-1; i++) {
            if (perm_rows[i]==-1) {
                for (int j=0; j<M-1; j++) {
                    if (!perm_rows.contains(j)) {
                        perm_rows[i]=j;
                        break;
                    }
                }
            }
        }
        perm_rows[M-1]=M-1; //unclassified row
    }

    if (permutation_mode==ConfusionMatrixView::ColumnPermutation) {
        perm_cols.fill(-1,N);
        for (int i=0; i<N-1; i++) {
            int val=m_optimal_assignments.indexOf(i);
            if ((val>=0)&&(val<N-1)) {
                perm_cols[i]=val;
            }
            else {
                perm_cols[i]=-1; //will be filled in later
            }
        }
        for (int i=0; i<N-1; i++) {
            if (perm_cols[i]==-1) {
                for (int j=0; j<N-1; j++) {
                    if (!perm_cols.contains(j)) {
                        perm_cols[i]=j;
                        break;
                    }
                }
            }
        }
        perm_cols[N-1]=N-1; //unclassified column
    }

    if (permutation_mode==ConfusionMatrixView::BothRowBasedPermutation) {
        Mda A=row_normalize(m_confusion_matrix);

        QVector<double> diag_entries(M-1);
        for (int m=0; m<M-1; m++) {
            if (m_optimal_assignments.value(m)>=0) {
                diag_entries[m]=A.value(m,m_optimal_assignments[m]);
            }
            else {
                diag_entries[m]=0;
            }
        }
        QList<long> sort_inds=get_sort_indices(diag_entries);
        perm_rows.fill(-1,M);
        perm_cols.fill(-1,N);

        for (int ii=0; ii<sort_inds.count(); ii++) {
            int m=sort_inds[sort_inds.count()-1-ii];
            perm_rows[m]=ii;
            int n=m_optimal_assignments.value(m);
            if (n>=0) {
                perm_cols[n]=ii;
            }
        }
        for (int i=0; i<N-1; i++) {
            if (perm_cols[i]==-1) {
                qDebug() << "@@@@" << i;
                for (int j=0; j<N-1; j++) {
                    printf("%d- [%d]",j,(int)perm_cols.contains(7));
                    if (!perm_cols.contains(j)) {
                        printf("! ");
                        perm_cols[i]=j;
                        break;
                    }
                }
                printf(" using %d\n",perm_cols[i]);
                qDebug() << perm_cols;
            }
        }
        perm_rows[M-1]=M-1; //unclassified row
        perm_cols[N-1]=N-1; //unclassified column
    }

    if (permutation_mode==ConfusionMatrixView::BothColumnBasedPermutation) {
        Mda A=column_normalize(m_confusion_matrix);

        QVector<double> diag_entries(N-1);
        for (int n=0; n<N-1; n++) {
            if (m_optimal_assignments.indexOf(n)>=0) {
                diag_entries[n]=A.value(m_optimal_assignments.indexOf(n),n);
            }
            else {
                diag_entries[n]=0;
            }
        }
        QList<long> sort_inds=get_sort_indices(diag_entries);
        perm_rows.fill(-1,M);
        perm_cols.fill(-1,N);

        for (int ii=0; ii<sort_inds.count(); ii++) {
            int n=sort_inds[sort_inds.count()-1-ii];
            perm_cols[n]=ii;
            int m=m_optimal_assignments.indexOf(n);
            if (m>=0) {
                perm_rows[m]=ii;
            }
        }
        for (int i=0; i<M-1; i++) {
            if (perm_rows[i]==-1) {
                for (int j=0; j<M-1; j++) {
                    if (!perm_rows.contains(j)) {
                        perm_rows[i]=j;
                        break;
                    }
                }
            }
        }
        perm_rows[M-1]=M-1; //unclassified row
        perm_cols[N-1]=N-1; //unclassified column
    }

    qDebug() << "assignments" << m_optimal_assignments;
    qDebug() << "perm_rows" << perm_rows;
    qDebug() << "perm_cols" << perm_cols;

    foreach (MatrixView *MV,m_all_matrix_views) {
        MV->setIndexPermutations(perm_rows,perm_cols);
    }
}

void ConfusionMatrixViewPrivate::set_current_clusters(int k1, int k2)
{
    if ((k1<0)||(k2<0)) {
        foreach (MatrixView *V,m_all_matrix_views) {
            V->setCurrentElement(QPoint(-1,-1));
        }
    }
    else {
        foreach (MatrixView *V,m_all_matrix_views) {
            V->setCurrentElement(QPoint(k1-1,k2-1));
        }
    }
}
