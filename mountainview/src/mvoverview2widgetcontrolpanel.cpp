#include "mvoverview2widgetcontrolpanel.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QMap>
#include <QCheckBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QComboBox>

class MVOverview2WidgetControlPanelPrivate {
public:
	MVOverview2WidgetControlPanel *q;

	QMap<QString,QLineEdit *> m_lineedit_parameters;
    QMap<QString,QCheckBox *> m_checkbox_parameters;
	QMap<QString,QGroupBox *> m_groupbox_parameters;
    QMap<QString,QComboBox *> m_combobox_parameters;
    QMap<QString,QPushButton *> m_buttons;

	void add_group_label(QGridLayout *G,QString label);
    QCheckBox *add_check_box(QGridLayout *G,QString name,QString label,bool val);
    QComboBox *add_combo_box(QGridLayout *G, QString name, QString label);
	QLineEdit *add_int_box(QGridLayout *G,QString name,QString label,int val,int minval,int maxval);
	QLineEdit *add_float_box(QGridLayout *G,QString name,QString label,float val,float minval,float maxval);
	QGroupBox *add_radio_button_group(QGridLayout *G,QString name,QStringList options,QString val);
	QPushButton *add_button(QGridLayout *G,QString name,QString label);
    void add_horizontal_divider(QVBoxLayout *layout);
};

void MVOverview2WidgetControlPanelPrivate::add_group_label(QGridLayout *G,QString label) {
	int r=G->rowCount();
	QLabel *X=new QLabel(label);
	QFont f=X->font();
	f.setPointSize(10);
	X->setFont(f);
    G->addWidget(X,r,0,1,2);
}

QCheckBox *MVOverview2WidgetControlPanelPrivate::add_check_box(QGridLayout *G, QString name, QString label, bool val)
{
    int r=G->rowCount();
    QCheckBox *X=new QCheckBox;
    X->setChecked(val);
    X->setText(QString("%1").arg(label));
    G->addWidget(X,r,1);
    m_checkbox_parameters[name]=X;
    X->setProperty("name",name);
    q->connect(X,SIGNAL(toggled(bool)),q,SLOT(slot_checkbox_clicked(bool)));
    return X;
}

QComboBox *MVOverview2WidgetControlPanelPrivate::add_combo_box(QGridLayout *G, QString name, QString label)
{
    int r=G->rowCount();
    QComboBox *X=new QComboBox;
    X->setProperty("name",name);
    G->addWidget(new QLabel(label),r,0);
    G->addWidget(X,r,1);
    m_combobox_parameters[name]=X;
    q->connect(X,SIGNAL(activated(QString)),q,SLOT(slot_combobox_activated()));
    return X;
}

QLineEdit *MVOverview2WidgetControlPanelPrivate::add_int_box(QGridLayout *G,QString name,QString label,int val,int minval,int maxval) {
	Q_UNUSED(minval)
	Q_UNUSED(maxval)
	int r=G->rowCount();
	QLineEdit *X=new QLineEdit;
	X->setText(QString("%1").arg(val));
	G->addWidget(new QLabel(label),r,0);
	G->addWidget(X,r,1);
	m_lineedit_parameters[name]=X;
	return X;
}

QLineEdit *MVOverview2WidgetControlPanelPrivate::add_float_box(QGridLayout *G, QString name, QString label, float val, float minval, float maxval)
{
	Q_UNUSED(minval)
	Q_UNUSED(maxval)
	int r=G->rowCount();
	QLineEdit *X=new QLineEdit;
	X->setText(QString("%1").arg(val));
	G->addWidget(new QLabel(label),r,0);
	G->addWidget(X,r,1);
	m_lineedit_parameters[name]=X;
	return X;
}

QGroupBox *MVOverview2WidgetControlPanelPrivate::add_radio_button_group(QGridLayout *G, QString name, QStringList options, QString val)
{
	int r=G->rowCount();
	QGroupBox *box=new QGroupBox;
	QHBoxLayout *hlayout=new QHBoxLayout;
	foreach (QString option,options) {
		QRadioButton *B=new QRadioButton(option);
		if (option==val) B->setChecked(true);
		else B->setChecked(false);
        B->setProperty("name",name);
		hlayout->addWidget(B);
	}
	hlayout->addStretch();
	box->setLayout(hlayout);
	m_groupbox_parameters[name]=box;
	G->addWidget(box,r,0,1,2);
	QObject::connect(box,SIGNAL(clicked(bool)),q,SLOT(slot_radio_button_clicked()));
	return box;
}

QPushButton *MVOverview2WidgetControlPanelPrivate::add_button(QGridLayout *G,QString name,QString label) {
	int r=G->rowCount();
	QPushButton *X=new QPushButton(label);
	X->setFixedHeight(20);
	G->addWidget(X,r,1);
    X->setProperty("name",name);
	q->connect(X,SIGNAL(clicked(bool)),q,SLOT(slot_button_clicked()));
    m_buttons[name]=X;
    return X;
}

void MVOverview2WidgetControlPanelPrivate::add_horizontal_divider(QVBoxLayout *layout)
{
    QFrame *line = new QFrame;
    line->setFrameShape(QFrame::HLine); // Horizontal line
    line->setFrameShadow(QFrame::Sunken);
    line->setLineWidth(1);
    layout->addSpacing(25);
    layout->addWidget(line);
}

MVOverview2WidgetControlPanel::MVOverview2WidgetControlPanel(QWidget *parent) : QWidget(parent)
{
	d=new MVOverview2WidgetControlPanelPrivate;
	d->q=this;

	QFont fnt=this->font();
	fnt.setPixelSize(9);
	this->setFont(fnt);

	QVBoxLayout *layout=new QVBoxLayout;

    { // Raw/Preprocessed Data
        QGridLayout *G=new QGridLayout;
        layout->addLayout(G);
        d->add_group_label(G,"Raw/Preprocessed Data");
        d->add_combo_box(G,"raw_data_name","Use data:");
        d->add_horizontal_divider(layout);
    }

	{ // Cross-correlograms
		QGridLayout *G=new QGridLayout;
        layout->addLayout(G);
		d->add_group_label(G,"Cross-correlograms");
		d->add_int_box(G,"max_dt","Max. dt (ms)",100,1,3000);
		d->add_button(G,"update_cross_correlograms","Update");
        d->add_horizontal_divider(layout);
	}

	{ // Templates
		QGridLayout *G=new QGridLayout;
        layout->addLayout(G);

		d->add_group_label(G,"Templates");
        d->add_int_box(G,"clip_size","Clip Size",150,20,10000)->setToolTip("Number of time points in clips");
		QStringList options; options << "centroids" << "geometric medians";
		d->add_radio_button_group(G,"template_method",options,"centroids");
		d->add_button(G,"update_templates","Update");
        d->add_horizontal_divider(layout);
	}

	{ // Shell Splitting
		QGridLayout *G=new QGridLayout;
        layout->addLayout(G);

		d->add_group_label(G,"Shell splitting");
		d->add_check_box(G,"use_shell_split","Use shell split",false)->setToolTip("Split into peak amplitude shells.");
        d->add_float_box(G,"shell_width","Shell Width",1.5,0.1,20)->setToolTip("The width (in amplitude) of each shell");
        d->add_int_box(G,"min_per_shell","Min per shell",300,0,1500)->setToolTip("The minimum number of points in each shell");
		d->add_button(G,"update_shell_split","Update");
        d->add_horizontal_divider(layout);
	}

	{ // Event Filter
		QGridLayout *G=new QGridLayout;
		layout->addLayout(G);

		d->add_group_label(G,"Event Filter");
		d->add_check_box(G,"use_event_filter","Use event filter",false)->setToolTip("Filter the set of events by peak amplitude and/or outlier score.");
		d->add_float_box(G,"min_amplitude","Min amplitude",0,0,100)->setToolTip("Exclude events with absolute peak amplitude below this threshold");
		d->add_float_box(G,"max_outlier_score","Max outlier score",3,0,100)->setToolTip("Exclude events with outlier score above this threshold. Use zero for no filter.");
		d->add_button(G,"update_event_filter","Update");
		d->add_horizontal_divider(layout);
	}

	{ // Actions
        QGridLayout *G=new QGridLayout;
		layout->addLayout(G);

        d->add_group_label(G,"Actions");
        d->add_button(G,"open_cluster_details","Details")->setToolTip("Open a new window with auto-computed cluster details");
        //d->add_button(G,"open_templates","Open Templates")->setToolTip("Open a new window with auto-computed templates");
        d->add_button(G,"open_auto_correlograms","Auto-Correlograms")->setToolTip("Open a new auto-correlograms window");
        d->add_button(G,"open_matrix_of_cross_correlograms","Matrix of Cross-Correlograms")->setToolTip("Open a matrix of cross-correlograms for the set of selected clusters");
        d->add_button(G,"open_raw_data","Raw Data")->setToolTip("Open a window of raw data");
        d->add_button(G,"open_clips","Clips")->setToolTip("Open clips for currently selected cluster.");
		d->add_button(G,"open_clusters","Cluster(s)")->setToolTip("Open a view of cluster events in feature space.");
        d->add_button(G,"open_firing_rates","Firing Rates")->setToolTip("Open a view of firing rate as a function of time for the selected clusters.");
        d->add_horizontal_divider(layout);
	}

	layout->addStretch(0);

	this->setLayout(layout);
}

MVOverview2WidgetControlPanel::~MVOverview2WidgetControlPanel()
{
	delete d;
}

QVariant MVOverview2WidgetControlPanel::getParameterValue(QString name)
{
	if (d->m_lineedit_parameters.contains(name)) return d->m_lineedit_parameters[name]->text();
    if (d->m_checkbox_parameters.contains(name)) return d->m_checkbox_parameters[name]->isChecked();
    if (d->m_combobox_parameters.contains(name)) return d->m_combobox_parameters[name]->currentText();
	if (d->m_groupbox_parameters.contains(name)) {
		QGroupBox *G=d->m_groupbox_parameters[name];
		QList<QObject *> ch=G->children();
		foreach (QObject *obj,ch) {
			QRadioButton *R=dynamic_cast<QRadioButton *>(obj);
			if (R) {
				if (R->isChecked()) return R->text();
			}
		}
	}
    return "";
}

void MVOverview2WidgetControlPanel::setParameterValue(QString name, QVariant val)
{
    if (d->m_lineedit_parameters.contains(name)) return d->m_lineedit_parameters[name]->setText(val.toString());
    if (d->m_checkbox_parameters.contains(name)) return d->m_checkbox_parameters[name]->setChecked(val.toBool());
    if (d->m_combobox_parameters.contains(name)) return d->m_combobox_parameters[name]->setCurrentText(val.toString());
	if (d->m_groupbox_parameters.contains(name)) {
		QGroupBox *G=d->m_groupbox_parameters[name];
		QList<QObject *> ch=G->children();
		foreach (QObject *obj,ch) {
			QRadioButton *R=dynamic_cast<QRadioButton *>(obj);
			if (R) {
				if (R->text()==val) {
					R->setChecked(true);
				}
			}
		}
	}
}

void MVOverview2WidgetControlPanel::setParameterLabel(QString name, QString text)
{
    if (d->m_checkbox_parameters.contains(name)) return d->m_checkbox_parameters[name]->setText(text);
    if (d->m_buttons.contains(name)) return d->m_buttons[name]->setText(text);
}

void MVOverview2WidgetControlPanel::setParameterChoices(QString name, QStringList choices)
{
    if (d->m_combobox_parameters.contains(name)) {
        QComboBox *CB=d->m_combobox_parameters[name];
        QString txt=CB->currentText();
        CB->clear();
        foreach (QString choice,choices) {
            CB->addItem(choice);
        }
        CB->setCurrentText(txt);
    }
}

void MVOverview2WidgetControlPanel::slot_button_clicked()
{
    emit signalButtonClicked(sender()->property("name").toString());
}

void MVOverview2WidgetControlPanel::slot_checkbox_clicked(bool val)
{
    ((QCheckBox *)sender())->setChecked(val); //make sure this is set before we emit the signal
    emit signalButtonClicked(sender()->property("name").toString());
}

void MVOverview2WidgetControlPanel::slot_radio_button_clicked()
{
    emit signalButtonClicked(sender()->property("name").toString());
}

void MVOverview2WidgetControlPanel::slot_combobox_activated()
{
    emit signalComboBoxActivated(sender()->property("name").toString());
}

