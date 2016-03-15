#include "mvoverview2widget.h"
#include "diskreadmda.h"
#include "sstimeseriesview.h"
#include "sstimeserieswidget.h"
#include "mvcrosscorrelogramswidget.h"
#include "mvoverview2widgetcontrolpanel.h"
#include "get_principal_components.h"
#include "get_sort_indices.h"
#include "mvclusterdetailwidget.h"
#include "mvclipsview.h"
#include "mvclusterwidget.h"
#include "mvfiringrateview.h"

#include <QHBoxLayout>
#include <QMessageBox>
#include <QSplitter>
#include <QTabWidget>
#include <QTime>
#include <QTimer>
#include <math.h>
#include <QProgressDialog>
#include "msutils.h"
#include "mvutils.h"
#include <QColor>
#include <QStringList>
#include <QSet>

class MVOverview2WidgetPrivate {
public:
	MVOverview2Widget *q;
    QMap<QString,QString> m_raw_data_paths;
    QString m_current_raw_data_name;
	DiskReadMda m_raw;
	DiskReadMda m_firings_original;
	Mda m_firings_split;
	Mda m_firings;
    QList<Epoch> m_epochs;
	QList<int> m_original_cluster_numbers;
    QList<int> m_original_cluster_offsets;
	int m_current_k;
	QSet<int> m_selected_ks;
	float m_sampling_frequency;
	MVEvent m_current_event;

	MVOverview2WidgetControlPanel *m_control_panel;

	QSplitter *m_splitter1,*m_splitter2;
	CustomTabWidget *m_tabs1,*m_tabs2;
	CustomTabWidget *m_current_tab_widget;
	QProgressDialog *m_progress_dialog;

	Mda m_cross_correlograms_data;
    //Mda m_templates_data;

	QList<QColor> m_channel_colors;
	QMap<QString,QColor> m_colors;

    bool m_cross_correlograms_data_update_needed;

	void create_cross_correlograms_data();
    //void create_templates_data();

	void update_sizes();
    //void update_templates();
	void update_all_widgets();
	void update_cluster_details();
    void update_clips();
	void update_cluster_views();
    void update_firing_rate_views();
	void do_shell_split();
	void do_event_filter();
	void add_tab(QWidget *W,QString label);

	void open_auto_correlograms();
	void open_cross_correlograms(int k);
    void open_matrix_of_cross_correlograms();
    //void open_templates();
	void open_cluster_details();
	void open_raw_data();
    void open_clips();
	void open_clusters();
    void open_firing_rates();

	void update_cross_correlograms();
	void update_raw_views();
    void move_to_timepoint(double tp);
	void update_widget(QWidget *W);

    void set_cross_correlograms_current_number(int kk);
	void set_cross_correlograms_selected_numbers(const QList<int> &kks);
    void set_templates_current_number(int kk);
	void set_templates_selected_numbers(const QList<int> &kks);

    void set_times_labels();

	QList<QWidget *> get_all_widgets();
	CustomTabWidget *current_tab_widget();
	CustomTabWidget *get_other_tab_widget(CustomTabWidget *W);
    CustomTabWidget *tab_widget_of(QWidget *W);

	void remove_widgets_of_type(QString widget_type);

	Mda compute_centroid(Mda &clips);
	Mda compute_geometric_median(Mda &clips,int num_iterations);
	void compute_geometric_median(int M,int N,double *output,double *input,int num_iterations);

	void set_progress(QString title,QString text,float frac);
	void set_current_event(MVEvent evt);
};

QColor brighten(QColor col,int amount) {
	int r=col.red()+amount;
	int g=col.green()+amount;
	int b=col.blue()+amount;
	if (r>255) r=255; if (r<0) r=0;
	if (g>255) g=255; if (g<0) g=0;
	if (b>255) b=255; if (b<0) b=0;
	return QColor(r,g,b,col.alpha());
}

MVOverview2Widget::MVOverview2Widget(QWidget *parent) : QWidget (parent)
{
	d=new MVOverview2WidgetPrivate;
	d->q=this;

	d->m_current_k=0;
	d->m_sampling_frequency=0;

	d->m_progress_dialog=0;
    d->m_cross_correlograms_data_update_needed=true;
	d->m_current_event.time=-1; d->m_current_event.label=-1;

	d->m_control_panel=new MVOverview2WidgetControlPanel;
	connect(d->m_control_panel,SIGNAL(signalButtonClicked(QString)),this,SLOT(slot_control_panel_button_clicked(QString)));
    connect(d->m_control_panel,SIGNAL(signalComboBoxActivated(QString)),this,SLOT(slot_control_panel_combobox_activated(QString)));

	QSplitter *splitter1=new QSplitter;
	splitter1->setOrientation(Qt::Horizontal);
	d->m_splitter1=splitter1;

	QSplitter *splitter2=new QSplitter;
	splitter2->setOrientation(Qt::Vertical);
	d->m_splitter2=splitter2;

	splitter1->addWidget(d->m_control_panel);
	splitter1->addWidget(splitter2);

	d->m_tabs1=new CustomTabWidget(this);
	d->m_tabs2=new CustomTabWidget(this);
	d->m_current_tab_widget=d->m_tabs1;

	splitter2->addWidget(d->m_tabs1);
	splitter2->addWidget(d->m_tabs2);

	QHBoxLayout *hlayout=new QHBoxLayout;
	hlayout->addWidget(splitter1);
	this->setLayout(hlayout);

	QStringList color_strings; color_strings
			<< "#282828"
			<< "#402020"
			<< "#204020"
			<< "#202070";
	for (int i=0; i<color_strings.count(); i++) d->m_channel_colors << QColor(brighten(color_strings[i],80));

	d->m_colors["background"]=QColor(240,240,240);
	d->m_colors["frame1"]=QColor(245,245,245);
	d->m_colors["info_text"]=QColor(80,80,80);
	d->m_colors["view_background"]=QColor(245,245,245);
	d->m_colors["view_background_highlighted"]=QColor(210,230,250);
	d->m_colors["view_background_selected"]=QColor(220,240,250);
	d->m_colors["view_background_hovered"]=QColor(240,245,240);

	d->m_colors["view_frame_selected"]=QColor(50,20,20);
    d->m_colors["divider_line"]=QColor(255,100,150);
}

MVOverview2Widget::~MVOverview2Widget()
{
    delete d;
}

void MVOverview2Widget::addRawPath(const QString &name, const QString &path)
{
    d->m_raw_data_paths[name]=path;
    QStringList choices=d->m_raw_data_paths.keys();
    qSort(choices);
    d->m_control_panel->setParameterChoices("raw_data_name",choices);
    if (d->m_raw_data_paths.count()==1) {
        this->setCurrentRawDataName(name);
    }

}

void MVOverview2Widget::setCurrentRawDataName(const QString &name)
{
    d->m_current_raw_data_name=name;
    d->m_raw.setPath(d->m_raw_data_paths[d->m_current_raw_data_name]);
    d->m_control_panel->setParameterValue("raw_data_name",name);
    d->update_raw_views();
    d->update_cluster_details();
	d->update_clips();
	d->update_cluster_views();
    d->update_firing_rate_views();
}

void MVOverview2Widget::setFiringsPath(const QString &firings)
{
	/*
    QList<long> times;
    QList<long> labels;
    for (int n=0; n<d->m_firings.N2(); n++) {
        times << (long)d->m_firings.value(1,n);
        labels << (long)d->m_firings.value(2,n);
    }
	*/
	d->m_firings_original.setPath(firings);
	d->do_shell_split();
	d->update_cross_correlograms();
	d->update_cluster_details();
	d->update_raw_views();
}

void MVOverview2Widget::setSamplingFrequency(float freq)
{
	d->m_sampling_frequency=freq;
}

void MVOverview2Widget::setDefaultInitialization()
{
	//d->open_templates();
	d->open_cluster_details();
	d->m_current_tab_widget=d->m_tabs2;
    d->open_auto_correlograms();
}

void MVOverview2Widget::setEpochs(const QList<Epoch> &epochs)
{
    d->m_epochs=epochs;
}

void MVOverview2Widget::resizeEvent(QResizeEvent *evt)
{
	Q_UNUSED(evt)
	d->update_sizes();
}

void MVOverview2Widget::slot_control_panel_button_clicked(QString str)
{
	if (str=="update_cross_correlograms") {
		d->m_cross_correlograms_data_update_needed=true;
		d->update_cross_correlograms();
	}
    else if (str=="update_templates") {
    //	d->update_templates();
        d->update_cluster_details();
        d->update_clips();
    }
	else if (str=="update_cluster_details") {
		d->update_cluster_details();
	}
	else if ((str=="update_shell_split")||(str=="use_shell_split")) {
		d->do_shell_split();
		d->remove_widgets_of_type("cross_correlograms");
        d->remove_widgets_of_type("matrix_of_cross_correlograms");
		d->remove_widgets_of_type("clips");
		d->remove_widgets_of_type("clusters");
        d->remove_widgets_of_type("firing_rates");
        d->update_cluster_details();
		d->update_cross_correlograms();
	}
	else if ((str=="update_event_filter")||(str=="use_event_filter")) {
		d->do_event_filter();
		d->create_cross_correlograms_data();
		d->update_all_widgets();
	}
	else if (str=="open_auto_correlograms") {
		d->open_auto_correlograms();
	}
    else if (str=="open_matrix_of_cross_correlograms") {
        d->open_matrix_of_cross_correlograms();
    }
    //else if (str=="open_templates") {
    //    d->open_templates();
    //}
	else if (str=="open_cluster_details") {
		d->open_cluster_details();
	}
	else if (str=="open_raw_data") {
		d->open_raw_data();
	}
    else if (str=="open_clips") {
        d->open_clips();
    }
	else if (str=="open_clusters") {
		d->open_clusters();
	}
    else if (str=="open_firing_rates") {
        d->open_firing_rates();
    }
	else if (str=="template_method") {
        d->update_cluster_details();
    }
}

void MVOverview2Widget::slot_control_panel_combobox_activated(QString str)
{
    if (str=="raw_data_name") {
        this->setCurrentRawDataName(d->m_control_panel->getParameterValue("raw_data_name").toString());
    }
}

void MVOverview2Widget::slot_auto_correlogram_activated(int k)
{
    d->m_current_tab_widget=d->get_other_tab_widget(d->tab_widget_of((QWidget *)sender()));
    d->open_cross_correlograms(k);
}

//void MVOverview2Widget::slot_templates_clicked()
//{
//    SSTimeSeriesView *X=(SSTimeSeriesView *)sender();
//    int clip_size=d->m_control_panel->getParameterValue("clip_size").toInt();
//    int x=X->currentX();
//    int clip_num=(x/clip_size)+1;
//	d->m_current_k=clip_num;
//	d->set_cross_correlograms_current_number(clip_num);
//}

void MVOverview2Widget::slot_details_current_k_changed()
{
	MVClusterDetailWidget *X=(MVClusterDetailWidget *)sender();
	int k=X->currentK();
	d->m_current_k=k;
	d->set_cross_correlograms_current_number(k);
}

void MVOverview2Widget::slot_details_selected_ks_changed()
{
	MVClusterDetailWidget *X=(MVClusterDetailWidget *)sender();
	QList<int> ks=X->selectedKs();
	d->m_selected_ks=ks.toSet();
    d->set_cross_correlograms_selected_numbers(ks);
}

void MVOverview2Widget::slot_details_template_activated()
{
    MVClusterDetailWidget *X=(MVClusterDetailWidget *)sender();
    int k=X->currentK();
    if (k<0) return;
    d->open_clips();
}

void MVOverview2Widget::slot_cross_correlogram_current_unit_changed()
{
    MVCrossCorrelogramsWidget *X=(MVCrossCorrelogramsWidget *)sender();
	d->m_current_k=X->currentUnit();
    d->set_cross_correlograms_current_number(X->currentUnit());
	d->set_templates_current_number(X->currentUnit());
}

void MVOverview2Widget::slot_cross_correlogram_selected_units_changed()
{
	MVCrossCorrelogramsWidget *X=(MVCrossCorrelogramsWidget *)sender();
	d->m_selected_ks=X->selectedUnits().toSet();
	d->set_cross_correlograms_selected_numbers(X->selectedUnits());
	d->set_templates_selected_numbers(X->selectedUnits());
}

void MVOverview2Widget::slot_clips_view_current_event_changed()
{
	MVClipsView *W=(MVClipsView *)sender();
	MVEvent evt=W->currentEvent();
	d->set_current_event(evt);
}

void MVOverview2Widget::slot_cluster_view_current_event_changed()
{
	MVClusterWidget *W=(MVClusterWidget *)sender();
	MVEvent evt=W->currentEvent();
	d->set_current_event(evt);
}

typedef QList<long> IntList;
void MVOverview2WidgetPrivate::create_cross_correlograms_data()
{
	set_progress("Computing","Creating cross correlograms data",0);
	QList<long> times,labels;
	long L=m_firings.N2();
	float samplefreq=30000;
	int max_dt=(int)(m_control_panel->getParameterValue("max_dt").toInt()*samplefreq/1000);

	printf("Setting up times and labels...\n");
	for (int n=0; n<L; n++) {
		times << (long)m_firings.value(1,n);
		labels << (long)m_firings.value(2,n);
	}
	int K=0;
	for (int n=0; n<labels.count(); n++) {
		if (labels[n]>K) K=labels[n];
	}

	printf("Sorting by times...\n");
	QList<long> indices=get_sort_indices(times);
	QList<long> times2,labels2;
	for (int i=0; i<indices.count(); i++) {
		times2 << times[indices[i]];
		labels2 << labels[indices[i]];
	}
	times=times2; labels=labels2;

	printf("Initializing output...\n");
	QList<IntList> out;
	IntList empty_list;
	for (int k1=1; k1<=K; k1++) {
		for (int k2=1; k2<=K; k2++) {
			out << empty_list;
		}
	}

	printf("Setting time differences...\n");
	int i1=0;
	for (int i2=0; i2<L; i2++) {
		if (i2%100==0) {
			set_progress("Computing","Creating cross correlograms data",i2*1.0/L);
		}
		while ((i1+1<L)&&(times[i1]<times[i2]-max_dt)) i1++;
		int k2=labels[i2];
		int t2=times[i2];
		if (k2>=1) {
			for (int jj=i1; jj<i2; jj++) {
				int k1=labels[jj];
				int t1=times[jj];
				if (k1>=1) {
					out[(k1-1)+K*(k2-1)] << t2-t1;
					out[(k2-1)+K*(k1-1)] << t1-t2;
				}
			}
		}
	}

	printf("Counting...\n");
	int ct=0;
	for (int k1=1; k1<=K; k1++) {
		for (int k2=1; k2<=K; k2++) {
			ct+=out[(k1-1)+K*(k2-1)].count();
		}
	}

	printf("Creating mda...\n");
	Mda ret; ret.allocate(3,ct);

	ct=0;
	for (int k1=1; k1<=K; k1++) {
		for (int k2=1; k2<=K; k2++) {
			IntList *tmp=&out[(k1-1)+K*(k2-1)];
			for (int jj=0; jj<tmp->count(); jj++) {
				ret.setValue(k1,0,ct);
				ret.setValue(k2,1,ct);
				ret.setValue((*tmp)[jj],2,ct);
				ct++;
			}
		}
	}
	printf(".\n");

	m_cross_correlograms_data=ret;

	set_progress("Computing","Creating cross correlograms data",1);
}

//void MVOverview2WidgetPrivate::create_templates_data()
//{
//	set_progress("Computing","Creating templates",0);
//	QList<long> times,labels;
//	long L=m_firings.N2();
//	int M=m_raw.N1();
//	int T=m_control_panel->getParameterValue("clip_size").toInt();

//	printf("Setting up times and labels...\n");
//	for (int n=0; n<L; n++) {
//		times << (long)m_firings.value(1,n)-1; //convert to 0-based indexing
//		labels << (long)m_firings.value(2,n);
//	}
//	int K=0;
//	for (int n=0; n<labels.count(); n++) {
//		if (labels[n]>K) K=labels[n];
//	}

//	printf("Creating mda...\n");
//	Mda ret; ret.allocate(M,T,K);
//	for (int k=1; k<=K; k++) {
//		set_progress("Computing","Creating templates",k*1.0/(K+1));
//		QList<long> times_k;
//		for (int ii=0; ii<times.count(); ii++) {
//			if (labels[ii]==k) times_k << times[ii];
//		}
//		Mda clips_k=extract_clips(m_raw,times_k,T);
//		Mda template_k;
//		if (m_control_panel->getParameterValue("template_method").toString()=="centroids") {
//			template_k=compute_centroid(clips_k);
//		}
//		else if (m_control_panel->getParameterValue("template_method").toString()=="geometric medians") {
//			int num_iterations=10;
//			template_k=compute_geometric_median(clips_k,num_iterations);
//		}
//		for (int t=0; t<T; t++) {
//			for (int m=0; m<M; m++) {
//				double val=template_k.value(m,t);
//				ret.setValue(val,m,t,k-1);
//			}
//		}
//	}
//	printf(".\n");

//	m_templates_data=ret;

//	set_progress("Computing","Creating templates closing",1);
//}

void MVOverview2WidgetPrivate::update_sizes()
{
	float W0=q->width();
	float H0=q->height();

	int W1=W0/3; if (W1<150) W1=150; if (W1>400) W1=400;
	int W2=W0-W1;

	int H1=H0/2;
	int H2=H0/2;
	//int H3=H0-H1-H2;

	{
		QList<int> sizes; sizes << W1 << W2;
		m_splitter1->setSizes(sizes);
	}
	{
		QList<int> sizes; sizes << H1 << H2;
		m_splitter2->setSizes(sizes);
	}

}

void MVOverview2WidgetPrivate::update_all_widgets()
{
	QList<QWidget *> list=get_all_widgets();
	foreach (QWidget *W,list) {
		update_widget(W);
	}
}

//void MVOverview2WidgetPrivate::update_templates()
//{
//	create_templates_data();
//	QList<QWidget *> list=get_all_widgets();
//	foreach (QWidget *W,list) {
//		if (W->property("widget_type")=="templates") {
//			update_widget(W);
//		}
//        if (W->property("widget_type")=="cluster_details") {
//            update_widget(W);
//        }
//		if (W->property("widget_type")=="clips") {
//			update_widget(W);
//		}
//	}
//}

void MVOverview2WidgetPrivate::update_cluster_details()
{
	QList<QWidget *> list=get_all_widgets();
	foreach (QWidget *W,list) {
		if (W->property("widget_type")=="cluster_details") {
			update_widget(W);
		}
    }
}

void MVOverview2WidgetPrivate::update_clips()
{
    QList<QWidget *> list=get_all_widgets();
    foreach (QWidget *W,list) {
        if (W->property("widget_type")=="clips") {
            update_widget(W);
        }
	}
}

void MVOverview2WidgetPrivate::update_cluster_views()
{
	QList<QWidget *> list=get_all_widgets();
	foreach (QWidget *W,list) {
		if (W->property("widget_type")=="clusters") {
			update_widget(W);
		}
	}
}

void MVOverview2WidgetPrivate::update_firing_rate_views()
{
    QList<QWidget *> list=get_all_widgets();
    foreach (QWidget *W,list) {
        if (W->property("widget_type")=="firing_rates") {
            update_widget(W);
        }
    }
}

double get_max(QList<double> &list) {
	double ret=list.value(0);
	for (int i=0; i<list.count(); i++) {
		if (list[i]>ret) ret=list[i];
	}
	return ret;
}

double get_min(QList<double> &list) {
	double ret=list.value(0);
	for (int i=0; i<list.count(); i++) {
		if (list[i]<ret) ret=list[i];
	}
	return ret;
}

void define_shells(QList<double> &shell_mins,QList<double> &shell_maxs,QList<double> &clip_peaks,double shell_increment,int min_shell_count) {
	//positives
	double max_clip_peaks=get_max(clip_peaks);
	QList<double> shell_mins_pos;
	QList<double> shell_maxs_pos;
	{
		int num_bins=1;
		while (shell_increment*num_bins<=max_clip_peaks) num_bins++;
		num_bins++;
		long counts[num_bins];
		for (int b=0; b<num_bins; b++) counts[b]=0;
		long tot=0;
		for (int i=0; i<clip_peaks.count(); i++) {
			if (clip_peaks[i]>0) {
				int b=(int)(clip_peaks[i]/shell_increment);
				if (b<num_bins) counts[b]++;
				else qWarning() << "Unexpected problem" << __FILE__ << __LINE__;
				tot++;
			}
		}
		int min_b=0;
		int max_b=0;
		int count_in=counts[0];
		while (min_b<num_bins) {
			if ((count_in>=min_shell_count)&&(tot-count_in>=min_shell_count)) {
				shell_mins_pos << min_b*shell_increment;
				shell_maxs_pos << (max_b+1)*shell_increment;
				min_b=max_b+1;
				max_b=max_b+1;
				tot-=count_in;
				if (min_b<num_bins) count_in=counts[min_b];
				else count_in=0;
			}
			else {
				max_b++;
				if (max_b<num_bins) {
					count_in+=counts[max_b];
				}
				else {
					if (count_in>0) {
						shell_mins_pos << min_b*shell_increment;
						shell_maxs_pos << (max_b+1)*shell_increment;
					}
					break;
				}
			}
		}
	}

	//negatives
	double min_clip_peaks=get_min(clip_peaks);
	QList<double> shell_mins_neg;
	QList<double> shell_maxs_neg;
	{
		int num_bins=1;
		while (shell_increment*num_bins<=-min_clip_peaks) num_bins++;
		num_bins++;
		long counts[num_bins];
		for (int b=0; b<num_bins; b++) counts[b]=0;
		long tot=0;
		for (int i=0; i<clip_peaks.count(); i++) {
			if (clip_peaks[i]<0) {
				int b=(int)(-clip_peaks[i]/shell_increment);
				if (b<num_bins) counts[b]++;
				else qWarning() << "Unexpected problem" << __FILE__ << __LINE__;
				tot++;
			}
		}
		int min_b=0;
		int max_b=0;
		int count_in=counts[0];
		while (min_b<num_bins) {
			if ((count_in>=min_shell_count)&&(tot-count_in>=min_shell_count)) {
				shell_mins_neg << min_b*shell_increment;
				shell_maxs_neg << (max_b+1)*shell_increment;
				min_b=max_b+1;
				max_b=max_b+1;
				tot-=count_in;
				if (min_b<num_bins) count_in=counts[min_b];
				else count_in=0;
			}
			else {
				max_b++;
				if (max_b<num_bins) {
					count_in+=counts[max_b];
				}
				else {
					if (count_in>0) {
						shell_mins_neg << min_b*shell_increment;
						shell_maxs_neg << (max_b+1)*shell_increment;
					}
					break;
				}
			}
		}
	}

	//combine
	for (int i=shell_mins_neg.count()-1; i>=0; i--) {
		shell_maxs << -shell_mins_neg[i];
		shell_mins << -shell_maxs_neg[i];
	}
	for (int i=0; i<shell_mins_pos.count(); i++) {
		shell_mins << shell_mins_pos[i];
		shell_maxs << shell_maxs_pos[i];
	}
}

void MVOverview2WidgetPrivate::do_shell_split() {
    m_cross_correlograms_data_update_needed=true;
	m_current_k=0;
	if (!m_control_panel->getParameterValue("use_shell_split").toBool()) {
		m_firings_split.allocate(m_firings_original.N1(),m_firings_original.N2());
		for (int i2=0; i2<m_firings_split.N2(); i2++) {
			for (int i1=0; i1<m_firings_split.N1(); i1++) {
				m_firings_split.setValue(m_firings_original.value(i1,i2),i1,i2);
			}
		}
		m_original_cluster_numbers.clear();
		m_original_cluster_offsets.clear();
		int K=0;
		for (int n=0; n<m_firings_split.N2(); n++) {
			if (m_firings_split.value(2,n)>K) K=(int)m_firings_split.value(2,n);
		}
		for (int k=0; k<=K; k++) {
			m_original_cluster_numbers << k;
			m_original_cluster_offsets << 0;
		}

		do_event_filter();
		return;
	}

	float shell_width=m_control_panel->getParameterValue("shell_width").toFloat();
	int min_per_shell=m_control_panel->getParameterValue("min_per_shell").toInt();

	QList<int> labels;
	QList<double> peaks;
	for (int n=0; n<m_firings_original.N2(); n++) {
		float peak=m_firings_original.value(3,n);
		labels << (int)m_firings_original.value(2,n);
		peaks << peak;
	}

	int K=0;
	for (int n=0; n<labels.count(); n++) {
		if (labels[n]>K) K=labels[n];
	}

	QList<int> nums;
	QList<float> mins;
	QList<float> maxs;
	for (int k=1; k<=K; k++) {
		QList<double> peaks_k;
		for (int n=0; n<labels.count(); n++) {
			if (labels[n]==k) {
				peaks_k << peaks[n];
			}
		}
		QList<double> shell_mins,shell_maxs; shell_mins.clear(); shell_maxs.clear();
		define_shells(shell_mins,shell_maxs,peaks_k,shell_width,min_per_shell);
		for (int ii=0; ii<shell_mins.count(); ii++) {
			nums << k;
			mins << shell_mins[ii];
			maxs << shell_maxs[ii];
		}
	}

	int KK=nums.count();
	m_firings_split.allocate(m_firings_original.N1(),m_firings_original.N2());
	for (int i2=0; i2<m_firings_split.N2(); i2++) {
		for (int i1=0; i1<m_firings_split.N1(); i1++) {
			if (i1!=2) { //don't set the labels!
				m_firings_split.setValue(m_firings_original.value(i1,i2),i1,i2);
			}
		}
	}

	m_original_cluster_numbers.clear();
	m_original_cluster_offsets.clear();
	m_original_cluster_numbers << 0;
	m_original_cluster_offsets << 0;
	int offset=0;
	for (int kk=0; kk<KK; kk++) {
		if ((kk==0)||(nums[kk]!=nums[kk-1])) offset=0;
		int k=nums[kk];
		float min0=mins[kk];
		float max0=maxs[kk];
		m_original_cluster_numbers << k;
		m_original_cluster_offsets << offset;
		offset++;
		for (int n=0; n<labels.count(); n++) {
			if (labels[n]==k) {
				if ((min0<=peaks[n])&&(peaks[n]<max0)) {
					m_firings_split.setValue(kk+1,2,n);
				}
			}
		}
	}

    this->set_templates_current_number(-1);
    this->set_templates_selected_numbers(QList<int>());

	do_event_filter();
}

void MVOverview2WidgetPrivate::do_event_filter()
{
	if (!m_control_panel->getParameterValue("use_event_filter").toBool()) {
		m_firings=m_firings_split;
		return;
	}
	float min_amplitude=m_control_panel->getParameterValue("min_amplitude").toFloat();
	float max_outlier_score=m_control_panel->getParameterValue("max_outlier_score").toFloat();

	QList<int> inds;
	for (int i=0; i<m_firings_split.N2(); i++) {
		if (fabs(m_firings_split.value(3,i))>=min_amplitude) {
			if (max_outlier_score) {
				if (m_firings_split.value(4,i)<=max_outlier_score) {
					inds << i;
				}
			}
			else inds << i;
		}
	}

	int N2=inds.count();
	m_firings.allocate(m_firings_split.N1(),N2);
	for (int i=0; i<N2; i++) {
		for (int j=0; j<m_firings_split.N1(); j++) {
			m_firings.setValue(m_firings_split.value(j,inds[i]),j,i); //speed this up?
		}
	}
}

/*
void MVOverview2WidgetPrivate::do_amplitude_split_old()
{
	m_current_k=0;
    if (!m_control_panel->getParameterValue("use_amplitude_split").toBool()) {
        m_firings.allocate(m_firings_original.N1(),m_firings_original.N2());
        for (int i2=0; i2<m_firings.N2(); i2++) {
            for (int i1=0; i1<m_firings.N1(); i1++) {
                m_firings.setValue(m_firings_original.value(i1,i2),i1,i2);
            }
        }
        m_original_cluster_numbers.clear();
        m_original_cluster_offsets.clear();
        int K=0;
        for (int n=0; n<m_firings.N2(); n++) {
            if (m_firings.value(2,n)>K) K=(int)m_firings.value(2,n);
        }
        for (int k=0; k<=K; k++) {
            m_original_cluster_numbers << k;
            m_original_cluster_offsets << 0;
        }

        return;
    }
	float shell_width=m_control_panel->getParameterValue("shell_width").toFloat();
	int min_per_shell=m_control_panel->getParameterValue("min_per_shell").toInt();
    float min_amplitude=m_control_panel->getParameterValue("min_amplitude").toInt();
	QList<long> times;
	QList<long> labels;
	QList<double> peaks;
	for (int n=0; n<m_firings_original.N2(); n++) {
        float peak=m_firings_original.value(3,n);
        times << (long)m_firings_original.value(1,n);
        labels << (long)m_firings_original.value(2,n);
        peaks << peak;
	}
	int K=0;
	for (int n=0; n<times.count(); n++) {
		if (labels[n]>K) K=labels[n];
	}
	QList<int> nums;
	QList<float> mins;
	QList<float> maxs;
	int MAXAMP=100;
	int BINS_SIZE=(int)(2*MAXAMP/shell_width);
	for (int k=1; k<=K; k++) {
		long bins[BINS_SIZE];
		for (int ii=0; ii<BINS_SIZE; ii++) bins[ii]=0;
		long tot_count=0;
		for (int n=0; n<times.count(); n++) {
			if (labels[n]==k) {
				int ind=(int)((peaks[n]-(-MAXAMP))/shell_width);
                if ((0<=ind)&&(ind<BINS_SIZE)&&(fabs(peaks[n])>=min_amplitude)) {
					bins[ind]++;
					tot_count++;
				}
			}
		}
		float tmp_min=-MAXAMP;
		int running_count=0;
		int tot_used=0;
		for (int ii=0; ii<BINS_SIZE; ii++) {
			running_count+=bins[ii];
			if ((running_count>=min_per_shell)&&(tot_count-tot_used-running_count>=min_per_shell)) {
				nums << k;
				mins << tmp_min;
				maxs << -MAXAMP+(ii+1)*shell_width;
				tmp_min=-MAXAMP+(ii+1)*shell_width;
				running_count=0;
			}
		}
		if (running_count>0) {
			nums << k;
			mins << tmp_min;
			maxs << MAXAMP;
		}
	}
	int KK=nums.count();
	m_firings.allocate(m_firings_original.N1(),m_firings_original.N2());
	for (int i2=0; i2<m_firings.N2(); i2++) {
		for (int i1=0; i1<m_firings.N1(); i1++) {
			if (i1!=2) { //don't set the labels!
				m_firings.setValue(m_firings_original.value(i1,i2),i1,i2);
			}
		}
	}

	m_original_cluster_numbers.clear();
    m_original_cluster_offsets.clear();
	m_original_cluster_numbers << 0;
    m_original_cluster_offsets << 0;
    int offset=0;
	for (int kk=0; kk<KK; kk++) {
        if ((kk==0)||(nums[kk]!=nums[kk-1])) offset=0;
		int k=nums[kk];
		float min0=mins[kk];
		float max0=maxs[kk];
		m_original_cluster_numbers << k;
        m_original_cluster_offsets << offset;
        offset++;
		for (int n=0; n<times.count(); n++) {
			if (labels[n]==k) {
                if ((min0<=peaks[n])&&(peaks[n]<max0)&&(fabs(peaks[n])>=min_amplitude)) {
					m_firings.setValue(kk+1,2,n);
				}
			}
		}
	}
}
*/

void MVOverview2WidgetPrivate::add_tab(QWidget *W,QString label)
{
	W->setFocusPolicy(Qt::StrongFocus);
	current_tab_widget()->addTab(W,label);
	current_tab_widget()->setCurrentIndex(current_tab_widget()->count()-1);
	W->setProperty("tab_label",label);
}

void MVOverview2WidgetPrivate::open_auto_correlograms()
{
    if (m_cross_correlograms_data_update_needed) {
        create_cross_correlograms_data();
        m_cross_correlograms_data_update_needed=false;
    }
	MVCrossCorrelogramsWidget *X=new MVCrossCorrelogramsWidget;
	X->setProperty("widget_type","auto_correlograms");
	add_tab(X,"Auto-Correlograms");
	QObject::connect(X,SIGNAL(unitActivated(int)),q,SLOT(slot_auto_correlogram_activated(int)));
    QObject::connect(X,SIGNAL(currentUnitChanged()),q,SLOT(slot_cross_correlogram_current_unit_changed()));
	QObject::connect(X,SIGNAL(selectedUnitsChanged()),q,SLOT(slot_cross_correlogram_selected_units_changed()));
	update_widget(X);
}

void MVOverview2WidgetPrivate::open_cross_correlograms(int k)
{
    if (m_cross_correlograms_data_update_needed) {
        create_cross_correlograms_data();
        m_cross_correlograms_data_update_needed=false;
    }
	MVCrossCorrelogramsWidget *X=new MVCrossCorrelogramsWidget;
	X->setProperty("widget_type","cross_correlograms");
	X->setProperty("kk",k);
    add_tab(X,QString("CC for %1(%2)").arg(m_original_cluster_numbers.value(k)).arg(m_original_cluster_offsets.value(k)+1));
    QObject::connect(X,SIGNAL(currentUnitChanged()),q,SLOT(slot_cross_correlogram_current_unit_changed()));
    update_widget(X);
}

QStringList int_list_to_string_list(const QList<int> &list) {
    QStringList list2;
    for (int i=0; i<list.count(); i++) list2 << QString("%1").arg(list[i]);
    return list2;
}

QList<int> string_list_to_int_list(const QList<QString> &list) {
    QList<int> list2;
    for (int i=0; i<list.count(); i++) list2 << list[i].toInt();
    return list2;
}

void MVOverview2WidgetPrivate::open_matrix_of_cross_correlograms()
{
    if (m_cross_correlograms_data_update_needed) {
        create_cross_correlograms_data();
        m_cross_correlograms_data_update_needed=false;
    }
    MVCrossCorrelogramsWidget *X=new MVCrossCorrelogramsWidget;
    X->setProperty("widget_type","matrix_of_cross_correlograms");
    QList<int> ks=m_selected_ks.toList();
    if (ks.isEmpty()) return;
    X->setProperty("ks",int_list_to_string_list(ks));
    add_tab(X,QString("CC Matrix"));
    update_widget(X);
}

//void MVOverview2WidgetPrivate::open_templates()
//{
//	SSTimeSeriesView *X=new SSTimeSeriesView;
//	X->initialize();
//	X->setProperty("widget_type","templates");
//	add_tab(X,QString("Templates"));
//    QObject::connect(X,SIGNAL(currentXChanged()),q,SLOT(slot_templates_clicked()));
//	update_widget(X);
//}

void MVOverview2WidgetPrivate::open_cluster_details()
{
	MVClusterDetailWidget *X=new MVClusterDetailWidget;
	X->setChannelColors(m_channel_colors);
	X->setRaw(m_raw);
    //X->setFirings(DiskReadMda(m_firings)); //done in update_widget
	X->setSamplingFrequency(m_sampling_frequency);
    QObject::connect(X,SIGNAL(signalTemplateActivated()),q,SLOT(slot_details_template_activated()));
	QObject::connect(X,SIGNAL(signalCurrentKChanged()),q,SLOT(slot_details_current_k_changed()));
	QObject::connect(X,SIGNAL(signalSelectedKsChanged()),q,SLOT(slot_details_selected_ks_changed()));
	X->setProperty("widget_type","cluster_details");
	add_tab(X,QString("Details"));
	update_widget(X);
}

void MVOverview2WidgetPrivate::open_raw_data()
{
    SSTimeSeriesWidget *X=new SSTimeSeriesWidget;
    X->hideMenu();
    SSTimeSeriesView *V=new SSTimeSeriesView;
    V->initialize();
    V->setSamplingFrequency(m_sampling_frequency);
    X->addView(V);
    X->setProperty("widget_type","raw_data");
	add_tab(X,QString("Raw"));
    update_widget(X);
}

void MVOverview2WidgetPrivate::open_clips()
{
	QList<int> ks=m_selected_ks.toList();
	qSort(ks);
	if (ks.count()==0) {
		QMessageBox::information(q,"Unable to open clips","You must select at least one cluster.");
		return;
	}
    MVClipsView *X=new MVClipsView;
    X->setProperty("widget_type","clips");
	X->setProperty("ks",int_list_to_string_list(ks));
	q->connect(X,SIGNAL(currentEventChanged()),q,SLOT(slot_clips_view_current_event_changed()));
	QString tab_title="Clips";
	if (ks.count()==1) {
		int kk=ks[0];
		tab_title=QString("Clips %1(%2)").arg(m_original_cluster_numbers.value(kk)).arg(m_original_cluster_offsets.value(kk)+1);
	}
	add_tab(X,tab_title);
    update_widget(X);
	X->setXRange(vec2(0,5000));
}

void MVOverview2WidgetPrivate::open_clusters()
{
	QList<int> ks=m_selected_ks.toList();
	qSort(ks);
	if (ks.count()==0) {
		QMessageBox::information(q,"Unable to open clusters","You must select at least one cluster.");
		return;
	}
	MVClusterWidget *X=new MVClusterWidget;
	X->setProperty("widget_type","clusters");
	X->setProperty("ks",int_list_to_string_list(ks));
	q->connect(X,SIGNAL(currentEventChanged()),q,SLOT(slot_cluster_view_current_event_changed()));
	add_tab(X,QString("Clusters"));
    update_widget(X);
}

void MVOverview2WidgetPrivate::open_firing_rates()
{
    QList<int> ks=m_selected_ks.toList();
    qSort(ks);
    if (ks.count()==0) {
        QMessageBox::information(q,"Unable to open firing rates","You must select at least one cluster.");
        return;
    }
    MVFiringRateView *X=new MVFiringRateView;
    X->setProperty("widget_type","firing_rates");
    X->setProperty("ks",int_list_to_string_list(ks));
    //q->connect(X,SIGNAL(currentEventChanged()),q,SLOT(slot_firing_rate_view_current_event_changed()));
    add_tab(X,QString("Firing Rates"));
    update_widget(X);
}

void MVOverview2WidgetPrivate::update_cross_correlograms()
{
    if (m_cross_correlograms_data_update_needed) {
        create_cross_correlograms_data();
        m_cross_correlograms_data_update_needed=false;
    }
	QList<QWidget *> widgets=get_all_widgets();
	foreach (QWidget *W,widgets) {
		QString widget_type=W->property("widget_type").toString();
		if ((widget_type=="auto_correlograms")||(widget_type=="cross_correlograms")) {
			update_widget(W);
		}
	}
}

void MVOverview2WidgetPrivate::update_raw_views()
{
	QList<QWidget *> widgets=get_all_widgets();
	foreach (QWidget *W,widgets) {
		QString widget_type=W->property("widget_type").toString();
		if (widget_type=="raw_data") {
			update_widget(W);
		}
    }
}

void MVOverview2WidgetPrivate::move_to_timepoint(double tp)
{
    QList<QWidget *> widgets=get_all_widgets();
    foreach (QWidget *W,widgets) {
        QString widget_type=W->property("widget_type").toString();
        if (widget_type=="raw_data") {
            SSTimeSeriesWidget *V=(SSTimeSeriesWidget *)W;
            V->view(0)->setCurrentTimepoint(tp);
        }
    }
}

void subtract_features_mean(Mda &F) {
	if (F.N2()==0) return;
	double mean[F.N1()];
	for (int i=0; i<F.N1(); i++) mean[i]=0;
	for (int i=0; i<F.N2(); i++) {
		for (int j=0; j<F.N1(); j++) {
			mean[j]+=F.value(j,i);
		}
	}
	for (int i=0; i<F.N1(); i++) mean[i]/=F.N2();
	for (int i=0; i<F.N2(); i++) {
		for (int j=0; j<F.N1(); j++) {
			F.setValue(F.value(j,i)-mean[j],j,i);
		}
	}
}

void normalize_features(Mda &F,bool individual_channels) {
	if (individual_channels) {
		for (int m=0; m<F.N1(); m++) {
			double sumsqr=0;
            double sum=0;
			int NN=F.N2();
            for (int i=0; i<NN; i++) {
                sumsqr+=F.value(m,i)*F.value(m,i);
                sum+=F.value(m,i);
            }
			double norm=1;
            if (NN>=2) norm=sqrt((sumsqr-sum*sum/NN)/(NN-1));
			for (int i=0; i<NN; i++) F.setValue(F.value(m,i)/norm,m,i);
		}
	}
	else {
		double sumsqr=0;
        double sum=0;
		int NN=F.totalSize();
        for (int i=0; i<NN; i++) {
            sumsqr+=F.value1(i)*F.value1(i);
            sum+=F.value1(i);
        }
		double norm=1;
        if (NN>=2) norm=sqrt((sumsqr-sum*sum/NN)/(NN-1));
		for (int i=0; i<NN; i++) F.setValue1(F.value1(i)/norm,i);
	}
}


void MVOverview2WidgetPrivate::update_widget(QWidget *W)
{
	QString widget_type=W->property("widget_type").toString();
	if (widget_type=="auto_correlograms") {
		MVCrossCorrelogramsWidget *WW=(MVCrossCorrelogramsWidget *)W;
		WW->setColors(m_colors);
		WW->setCrossCorrelogramsData(DiskReadMda(m_cross_correlograms_data));
        QStringList labels;
        for (int i=0; i<m_original_cluster_numbers.count(); i++) {
            if ((i==0)||(m_original_cluster_numbers[i]!=m_original_cluster_numbers[i-1])) {
                labels << QString("Auto %1").arg(m_original_cluster_numbers[i]);
            }
            else labels << "";
        }
        WW->setLabels(labels);
		WW->updateWidget();
	}
	else if (widget_type=="cross_correlograms") {
		MVCrossCorrelogramsWidget *WW=(MVCrossCorrelogramsWidget *)W;
        int k=W->property("kk").toInt();
		WW->setColors(m_colors);
		WW->setCrossCorrelogramsData(DiskReadMda(m_cross_correlograms_data));
		WW->setBaseUnit(k);
        QStringList labels;
        for (int i=0; i<m_original_cluster_numbers.count(); i++) {
            if ((i==0)||(m_original_cluster_numbers[i]!=m_original_cluster_numbers[i-1])) {
                labels << QString("Cross %1").arg(m_original_cluster_numbers[i]);
            }
            else labels << "";
        }
        WW->setLabels(labels);
		WW->updateWidget();
	}
    else if (widget_type=="matrix_of_cross_correlograms") {
        MVCrossCorrelogramsWidget *WW=(MVCrossCorrelogramsWidget *)W;
        QList<int> ks=string_list_to_int_list(W->property("ks").toStringList());
		WW->setColors(m_colors);
        WW->setCrossCorrelogramsData(DiskReadMda(m_cross_correlograms_data));
        WW->setUnitNumbers(ks);
        QStringList labels;
        for (int a1=0; a1<ks.count(); a1++) {
            QString str1=QString("%1(%2)").arg(m_original_cluster_numbers[ks[a1]]).arg(m_original_cluster_offsets[ks[a1]]+1);
            for (int a2=0; a2<ks.count(); a2++) {
                QString str2=QString("%1(%2)").arg(m_original_cluster_numbers[ks[a2]]).arg(m_original_cluster_offsets[ks[a2]]+1);
                labels << QString("%1/%2").arg(str1).arg(str2);
            }
        }
        WW->setLabels(labels);
        WW->updateWidget();
    }
    /*else if (widget_type=="templates") {
		printf("Setting templates data...\n");
		SSTimeSeriesView *WW=(SSTimeSeriesView *)W;
		Mda TD=m_templates_data;
		DiskArrayModel *MM=new DiskArrayModel;
		MM->setFromMda(TD);
		int KK=TD.N3();
		QList<long> times,labels;
		int last_k=-1;
		for (int kk=1; kk<=KK; kk++) {
			int k=m_original_cluster_numbers.value(kk);
			if (k!=last_k) {
				times << (long)(TD.N2()*(kk-1+0.5));
				labels << k;
			}
			last_k=k;
		}
		WW->setData(MM,true);
		WW->setTimesLabels(times,labels);
		WW->setMarkerLinesVisible(false);
		printf(".\n");
    }*/
	else if (widget_type=="cluster_details") {
		MVClusterDetailWidget *WW=(MVClusterDetailWidget *)W;
        int clip_size=m_control_panel->getParameterValue("clip_size").toInt();
		WW->setColors(m_colors);
        WW->setRaw(m_raw);
        WW->setClipSize(clip_size);
        WW->setFirings(DiskReadMda(m_firings));
        WW->setGroupNumbers(m_original_cluster_numbers);
	}
    else if (widget_type=="clips") {
        printf("Extracting clips...\n");
        MVClipsView *WW=(MVClipsView *)W;
		QList<int> ks=string_list_to_int_list(WW->property("ks").toStringList());

        QList<int> labels;
        QList<double> times;

		for (int n=0; n<m_firings.N2(); n++) {
			times << m_firings.value(1,n);
			labels << (int)m_firings.value(2,n);
		}

        QList<double> times_kk;
		QList<int> labels_kk;
		for (int ik=0; ik<ks.count(); ik++) {
			int kk=ks[ik];
			for (int n=0; n<labels.count(); n++) {
				if (labels[n]==kk) {
					times_kk << times[n];
					labels_kk << labels[n];
				}
			}
		}
		int clip_size=m_control_panel->getParameterValue("clip_size").toInt();
		Mda clips=extract_clips(m_raw,times_kk,clip_size);
		printf("Setting data...\n");
        //DiskArrayModel *DAM=new DiskArrayModel;
        //DAM->setFromMda(clips);
        //WW->setData(DAM,true);
        WW->setClips(clips);
        WW->setTimes(times_kk);
		WW->setLabels(labels_kk);
		printf(".\n");
    }
	else if (widget_type=="clusters") {
		set_progress("Extracting clips","Extracting clips",0);
		MVClusterWidget *WW=(MVClusterWidget *)W;
		QList<int> ks=string_list_to_int_list(WW->property("ks").toStringList());

		QList<int> labels;
		QList<double> times;
        QList<double> peaks;
		QList<double> outlier_scores;
		for (int n=0; n<m_firings.N2(); n++) {
			times << m_firings.value(1,n);
			labels << (int)m_firings.value(2,n);
            peaks << m_firings.value(3,n);
			outlier_scores << m_firings.value(4,n);
		}

		QList<double> times_kk;
		QList<int> labels_kk;
        QList<double> peaks_kk;
		QList<double> outlier_scores_kk;
		for (int ik=0; ik<ks.count(); ik++) {
			int kk=ks[ik];
			for (int n=0; n<labels.count(); n++) {
				if (labels[n]==kk) {
					times_kk << times[n];
					labels_kk << labels[n];
                    peaks_kk << peaks[n];
					outlier_scores_kk << outlier_scores[n];
				}
			}
		}
		int clip_size=m_control_panel->getParameterValue("clip_size").toInt();
		Mda clips=extract_clips(m_raw,times_kk,clip_size);
		int M=clips.N1(); int T=clips.N2(); int L=clips.N3();
		Mda features; features.allocate(3,L);
		set_progress("Computing features","Computing features",0);
		printf("Computing features...\n");
		get_pca_features(M*T,L,3,features.dataPtr(),clips.dataPtr());
        //subtract_features_mean(features);
		normalize_features(features,false);
		features.write("/tmp/tmp_features.mda");
		WW->setRaw(m_raw);
		WW->setData(features);
		WW->setTimes(times_kk);
		WW->setLabels(labels_kk);
        WW->setAmplitudes(peaks_kk);
		WW->setOutlierScores(outlier_scores_kk);
		set_progress("","",1);
		printf(".\n");
	}
    else if (widget_type=="firing_rates") {
        MVFiringRateView *WW=(MVFiringRateView *)W;
        QList<int> ks=string_list_to_int_list(WW->property("ks").toStringList());
        QSet<int> ks_set=ks.toSet();

        QList<double> times,amplitudes;
        QList<int> labels;
        for (int i=0; i<m_firings.N2(); i++) {
            int label=(int)m_firings.value(2,i);
            if (ks_set.contains(label)) {
                times << m_firings.value(1,i);
                amplitudes << m_firings.value(3,i);
                labels << label;
            }
        }
        Mda firings2; firings2.allocate(4,times.count());
        for (int i=0; i<times.count(); i++) {
            firings2.setValue(times[i],1,i);
            firings2.setValue(labels[i],2,i);
            firings2.setValue(amplitudes[i],3,i);
        }
        WW->setFirings(firings2);
        WW->setSamplingFreq(m_sampling_frequency);
        WW->setEpochs(m_epochs);
    }
	else if (widget_type=="raw_data") {
        SSTimeSeriesWidget *WW=(SSTimeSeriesWidget *)W;
		DiskArrayModel *X=new DiskArrayModel;
        X->setPath(m_raw_data_paths[m_current_raw_data_name]);
        ((SSTimeSeriesView *)(WW->view()))->setData(X,true);
        set_times_labels();
    }
}

void MVOverview2WidgetPrivate::set_cross_correlograms_current_number(int kk)
{
    QList<QWidget *> widgets=get_all_widgets();
    foreach (QWidget *W,widgets) {
        QString widget_type=W->property("widget_type").toString();
        if ((widget_type=="auto_correlograms")||(widget_type=="cross_correlograms")) {
            MVCrossCorrelogramsWidget *WW=(MVCrossCorrelogramsWidget *)W;
			WW->setCurrentUnit(kk);
        }
	}
}

void MVOverview2WidgetPrivate::set_cross_correlograms_selected_numbers(const QList<int> &kks)
{
	QList<QWidget *> widgets=get_all_widgets();
	foreach (QWidget *W,widgets) {
		QString widget_type=W->property("widget_type").toString();
		if ((widget_type=="auto_correlograms")||(widget_type=="cross_correlograms")) {
			MVCrossCorrelogramsWidget *WW=(MVCrossCorrelogramsWidget *)W;
			WW->setSelectedUnits(kks);
		}
	}
}

void MVOverview2WidgetPrivate::set_templates_current_number(int kk)
{
    QList<QWidget *> widgets=get_all_widgets();
    foreach (QWidget *W,widgets) {
        QString widget_type=W->property("widget_type").toString();
//        if (widget_type=="templates") {
//            int clip_size=m_control_panel->getParameterValue("clip_size").toInt();
//            SSTimeSeriesView *WW=(SSTimeSeriesView *)W;
//            WW->setCurrentX((int)(clip_size*(kk-1+0.5)));
//        }
        if (widget_type=="cluster_details") {
			MVClusterDetailWidget *WW=(MVClusterDetailWidget *)W;
			WW->setCurrentK(kk);
		}
	}
}

void MVOverview2WidgetPrivate::set_templates_selected_numbers(const QList<int> &kks)
{
	QList<QWidget *> widgets=get_all_widgets();
	foreach (QWidget *W,widgets) {
		QString widget_type=W->property("widget_type").toString();
//		if (widget_type=="templates") {
//			int clip_size=m_control_panel->getParameterValue("clip_size").toInt();
//			SSTimeSeriesView *WW=(SSTimeSeriesView *)W;
//			/////// NA
//			Q_UNUSED(clip_size)
//			Q_UNUSED(WW)
//		}
        if (widget_type=="cluster_details") {
			MVClusterDetailWidget *WW=(MVClusterDetailWidget *)W;
			WW->setSelectedKs(kks);
		}
	}
}

void MVOverview2WidgetPrivate::set_times_labels()
{
    QList<long> times,labels;
    for (int n=0; n<m_firings_original.N2(); n++) {
        times << (long)m_firings_original.value(1,n);
        labels << (long)m_firings_original.value(2,n);
    }
    QList<QWidget *> widgets=get_all_widgets();
    foreach (QWidget *W,widgets) {
        QString widget_type=W->property("widget_type").toString();
        if (widget_type=="raw_data") {
            SSTimeSeriesWidget *WW=(SSTimeSeriesWidget *)W;
            SSTimeSeriesView *V=(SSTimeSeriesView *)WW->view();
            V->setTimesLabels(times,labels);
        }
    }
}

QList<QWidget *> MVOverview2WidgetPrivate::get_all_widgets()
{
	QList<QWidget *> ret;
	for (int i=0; i<m_tabs1->count(); i++) {
		ret << m_tabs1->widget(i);
	}
	for (int i=0; i<m_tabs2->count(); i++) {
		ret << m_tabs2->widget(i);
	}
	return ret;
}

CustomTabWidget *MVOverview2WidgetPrivate::current_tab_widget()
{
	return m_current_tab_widget;
}

CustomTabWidget *MVOverview2WidgetPrivate::get_other_tab_widget(CustomTabWidget *W)
{
	if (W==m_tabs1) return m_tabs2;
    else return m_tabs1;
}

CustomTabWidget *MVOverview2WidgetPrivate::tab_widget_of(QWidget *W)
{
    for (int i=0; i<m_tabs1->count(); i++) {
        if (m_tabs1->widget(i)==W) return m_tabs1;
    }
    for (int i=0; i<m_tabs2->count(); i++) {
        if (m_tabs2->widget(i)==W) return m_tabs2;
    }
    return m_tabs1;
}

void MVOverview2WidgetPrivate::remove_widgets_of_type(QString widget_type)
{
	QList<QWidget *> list=get_all_widgets();
	foreach (QWidget *W,list) {
		if (W->property("widget_type")==widget_type) {
			delete W;
		}
	}
}

Mda MVOverview2WidgetPrivate::compute_centroid(Mda &clips)
{
	int M=clips.N1();
	int T=clips.N2();
	int NC=clips.N3();
	Mda ret; ret.allocate(M,T);
	double *retptr=ret.dataPtr();
	double *clipsptr=clips.dataPtr();
	for (int i=0; i<NC; i++) {
		int iii=i*M*T;
		int jjj=0;
		for (int t=0; t<T; t++) {
			for (int m=0; m<M; m++) {
				retptr[jjj]+=clipsptr[iii];
				iii++;
				jjj++;
			}
		}
	}
	if (NC) {
		for (int t=0; t<T; t++) {
			for (int m=0; m<M; m++) {
				double val=ret.value(m,t)/NC;
				ret.setValue(val,m,t);
			}
		}
	}
	return ret;
}

Mda MVOverview2WidgetPrivate::compute_geometric_median(Mda &clips, int num_iterations)
{
	int M=clips.N1();
	int T=clips.N2();
	int L=clips.N3();
	double *clipsptr=clips.dataPtr();
	Mda ret; ret.allocate(M,T);
	if (L==0) return ret;

	int num_features=6;
	Mda features; features.allocate(num_features,L);
	double *featuresptr=features.dataPtr();
	get_pca_features(M*T,L,num_features,featuresptr,clipsptr);

	Mda geomedian; geomedian.allocate(num_features,1);
	double *geomedianptr=geomedian.dataPtr();
	compute_geometric_median(num_features,L,geomedianptr,featuresptr,num_iterations);

	QList<double> dists;
	{
		int kkk=0;
		for (int j=0; j<L; j++) {
			double sumsqr=0;
			for (int a=0; a<num_features; a++) {
				double diff=featuresptr[kkk]-geomedianptr[a];
				sumsqr+=diff*diff;
				kkk++;
			}
			dists << sqrt(sumsqr);
		}
	}

	QList<double> dists_sorted=dists;
	qSort(dists_sorted);
	double cutoff=dists_sorted[(int)(L*0.3)];
	QList<int> inds_to_use;
	for (int j=0; j<L; j++) {
		if (dists[j]<=cutoff) inds_to_use << j;
	}

	Mda clips2;
	clips2.allocate(M,T,inds_to_use.count());
	for (int j=0; j<inds_to_use.count(); j++) {
		int jj=inds_to_use[j];
		for (int t=0; t<T; t++) {
			for (int m=0; m<M; m++) {
				clips2.setValue(clips.value(m,t,jj),m,t,j);
			}
		}
	}

	ret=compute_centroid(clips2);
	return ret;
}


void MVOverview2WidgetPrivate::compute_geometric_median(int M, int N, double *output, double *input, int num_iterations)
{
	double *weights=(double *)malloc(sizeof(double)*N);
	double *dists=(double *)malloc(sizeof(double)*N);
	for (int j=0; j<N; j++) weights[j]=1;
	for (int it=1; it<=num_iterations; it++) {
		float sumweights=0;
		for (int j=0; j<N; j++) sumweights+=weights[j];
		if (sumweights) for (int j=0; j<N; j++) weights[j]/=sumweights;
		for (int i=0; i<M; i++) output[i]=0;
		{
			//compute output
			int kkk=0;
			for (int j=0; j<N; j++) {
				int i=0;
				for (int m=0; m<M; m++) {
					output[i]+=weights[j]*input[kkk];
					i++;
					kkk++;
				}
			}
		}
		{
			//compute dists
			int kkk=0;
			for (int j=0; j<N; j++) {
				int i=0;
				double sumsqr=0;
				for (int m=0; m<M; m++) {
					double diff=output[i]-input[kkk];
					i++;
					kkk++;
					sumsqr+=diff*diff;
				}
				dists[j]=sqrt(sumsqr);
			}
		}
		{
			//compute weights
			for (int j=0; j<N; j++) {
				if (dists[j]) weights[j]=1/dists[j];
				else weights[j]=0;
			}
		}
	}
	free(dists);
	free(weights);
}

void MVOverview2WidgetPrivate::set_progress(QString title, QString text, float frac)
{
	if (!m_progress_dialog) {
		m_progress_dialog=new QProgressDialog;
		m_progress_dialog->setCancelButton(0);
	}
	static QTime *timer=0;
	if (!timer) {
		timer=new QTime;
		timer->start();
		m_progress_dialog->show();
		m_progress_dialog->repaint();
	}
	if (timer->elapsed()>500) {
		timer->restart();
		if (!m_progress_dialog->isVisible()) {
			m_progress_dialog->show();
		}
		m_progress_dialog->setLabelText(text);
		m_progress_dialog->setWindowTitle(title);
		m_progress_dialog->setValue((int)(frac*100));
		m_progress_dialog->repaint();
	}
	if (frac>=1) {
		delete m_progress_dialog;
		m_progress_dialog=0;
	}
}

void MVOverview2WidgetPrivate::set_current_event(MVEvent evt)
{
	if ((m_current_event.time==evt.time)&&(m_current_event.label==evt.label)) {
		return;
	}
	m_current_event=evt;
	QList<QWidget *> widgets=get_all_widgets();
	foreach (QWidget *W,widgets) {
		QString widget_type=W->property("widget_type").toString();
		if (widget_type=="clips") {
			MVClipsView *WW=(MVClipsView *)W;
			WW->setCurrentEvent(evt);
		}
		else if (widget_type=="clusters") {
			MVClusterWidget *WW=(MVClusterWidget *)W;
			WW->setCurrentEvent(evt);
		}
        else if (widget_type=="firing_rates") {
            MVFiringRateView *WW=(MVFiringRateView *)W;
            WW->setCurrentEvent(evt);
        }
		else if (widget_type=="cluster_details") {
			MVClusterDetailWidget *WW=(MVClusterDetailWidget *)W;
			if (evt.label>0) {
				WW->setCurrentK(evt.label);
			}
		}
		else if (widget_type=="raw_data") {
			SSTimeSeriesWidget *WW=(SSTimeSeriesWidget *)W;
			SSTimeSeriesView *VV=(SSTimeSeriesView *)WW->view(0);
			if (evt.time>=0) {
				VV->setCurrentX(evt.time);
			}
		}
	}
}

CustomTabWidget::CustomTabWidget(MVOverview2Widget *q) {
	this->setTabsClosable(true);
	this->setMovable(true);
	this->q=q;

	this->setFocusPolicy(Qt::StrongFocus);

	QObject::connect(this,SIGNAL(tabCloseRequested(int)),this,SLOT(slot_tab_close_requested(int)));
	QObject::connect(this,SIGNAL(tabBarClicked(int)),this,SLOT(slot_tab_bar_clicked()));
	QObject::connect(this,SIGNAL(tabBarDoubleClicked(int)),this,SLOT(slot_tab_bar_double_clicked()));
}

void CustomTabWidget::mousePressEvent(QMouseEvent *evt)
{
	q->d->m_current_tab_widget=this;
	QTabWidget::mousePressEvent(evt);
}

void CustomTabWidget::slot_tab_close_requested(int num)
{
	delete this->widget(num);
}

void CustomTabWidget::slot_tab_bar_clicked()
{
	q->d->m_current_tab_widget=this;
}

void CustomTabWidget::slot_tab_bar_double_clicked()
{
	q->d->m_current_tab_widget=q->d->get_other_tab_widget(this);
	q->d->add_tab(this->currentWidget(),this->currentWidget()->property("tab_label").toString());
	//the following is necessary because the slot_tab_bar_clicked is always called after this function!
	QTimer::singleShot(100,this,SLOT(slot_switch_to_other_tab_widget()));
}

void CustomTabWidget::slot_switch_to_other_tab_widget()
{
	q->d->m_current_tab_widget=q->d->get_other_tab_widget(this);
}
