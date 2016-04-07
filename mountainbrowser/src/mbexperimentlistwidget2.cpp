/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/6/2016
*******************************************************/

#include "mbexperimentlistwidget2.h"

#include <QJsonDocument>
#include <QWebFrame>
#include <QCoreApplication>
#include <QJsonArray>

class MBExperimentListWidget2Private {
public:
    MBExperimentListWidget2 *q;
    MBExperimentManager *m_experiment_manager;
    void load_and_wait(QUrl url);
};

MBExperimentListWidget2::MBExperimentListWidget2()
{
    d=new MBExperimentListWidget2Private;
    d->q=this;
    d->m_experiment_manager=0;
    this->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(this,SIGNAL(linkClicked(QUrl)),this,SLOT(slot_link_clicked(QUrl)));
}

MBExperimentListWidget2::~MBExperimentListWidget2()
{
    delete d;
}

void MBExperimentListWidget2::setExperimentManager(MBExperimentManager *EM)
{
    d->m_experiment_manager=EM;
    this->refresh();
}

QString abbreviate(const QString &str,int max_size) {
    if (str.count()<=max_size) return str;
    else return str.mid(0,max_size-3)+"...";
}

QString create_experiment_html(MBExperiment E) {

    QString exp_id=E.json["exp_id"].toString();
    QString exp_type=E.json["exp_type"].toString();
    QString description=E.json["description"].toString();

    QString html;

    html+="<head";

    html+="<body>\n";
    html+="<ul>\n";
    html+="<li>\n";
    html+=QString("<a href=open_experiment/%1>%1 (%2)</a><br>\n").arg(E.id).arg(exp_type);
    html+=QString("%1\n").arg(abbreviate(description,100).toHtmlEscaped());
    html+="</li>\n";
    html+="</ul>\n";
    html+="</body>\n";

    return html;
}

void MBExperimentListWidget2::refresh()
{
    //this->load(QUrl::fromLocalFile("/home/magland/dev/mountainlab/mountainbrowser/src/html/template.html"));
    d->load_and_wait(QUrl("qrc:/html/mbexperimentlist.html"));
    QJsonArray experiments;
    QStringList ids=d->m_experiment_manager->allExperimentIds();
    foreach (QString id,ids) {
        experiments.append(d->m_experiment_manager->experiment(id).json);
    }
    QString experiments_json=QJsonDocument(experiments).toJson();
    experiments_json=experiments_json.split("\n").join(" ");
    QString js=QString("refresh_experiment_list(JSON.parse('%1'));").arg(experiments_json);
    QVariant res=this->page()->mainFrame()->evaluateJavaScript(js);
    qDebug() << res.toString();
}

void MBExperimentListWidget2::slot_link_clicked(QUrl url)
{
    qDebug() << url.path();
    QStringList vals=url.path().split("/");
    QString val1=vals.value(1);
    QString val2=vals.value(2);
    if (val1=="open_experiment") {
        emit experimentActivated(val2);
    }
}


void MBExperimentListWidget2Private::load_and_wait(QUrl url)
{
    QEventLoop loop;
    QObject::connect(q,SIGNAL(loadFinished(bool)),&loop,SLOT(quit()));
    q->load(url);
    loop.exec();
}
