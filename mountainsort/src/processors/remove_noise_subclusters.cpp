#include "remove_noise_subclusters.h"
#include "diskreadmda.h"
#include <math.h>
#include "get_pca_features.h"
#include <QTime>
#include <stdio.h>
#include <QDebug>
#include "msmisc.h"
#include "extract_clips.h"

bool remove_noise_subclusters(const QString &timeseries_path,const QString &firings_path,const QString &firings_out_path,const Remove_noise_subclusters_opts &opts) {

	DiskReadMda raw; raw.setPath(timeseries_path);
	Mda firings; firings.read(firings_path);

	int T=opts.clip_size;
	int L=firings.N2();

    QList<long> channels,labels;
	QList<double> times;
	for (int i=0; i<L; i++) {
		channels << (int)firings.get(0,i)-1; //convert to zero-based indexing
		times << firings.get(1,i) -1; //convert to zero-based indexing
		labels << (int)firings.get(2,i);
	}

	Mda clips=extract_clips(raw,times,T);

	int K=compute_max(labels);

	QList<Subcluster> subclusters;

    #pragma omp parallel for
	for (int k=1; k<=K; k++) {
		int count1=0;
		int count2=0;

        QList<long> inds_k;
        #pragma omp critical
        inds_k=find_label_inds(labels,k);

        if (inds_k.count()>0) {
            int channel;
            Mda clips_k;
            #pragma omp critical
            {
                printf("k=%d/%d\n",k,K);
                channel=channels[inds_k[0]];
                clips_k=get_subclips(clips,inds_k);
            }
            Define_Shells_Opts opts2; opts2.min_shell_size=opts.min_shell_size; opts2.shell_increment=opts.shell_increment;
            QList<Subcluster> subclusters0=compute_subcluster_detectability_scores(raw,clips_k,channel,opts2);
            #pragma omp critical
            {
                for (int ii=0; ii<subclusters0.count(); ii++) {
                    Subcluster SC=subclusters0[ii];
                    QList<long> new_inds;
                    for (int j=0; j<SC.inds.count(); j++) {
                        new_inds << inds_k[SC.inds[j]];
                    }
                    SC.inds=new_inds;
                    SC.label=k;
                    subclusters << SC;
                    count1+=SC.inds.count();
                    if (SC.inds.count()>0) count2++;
                }
            }
		}
	}

	int num_subclusters=subclusters.count();
	QList<double> subcluster_abs_peaks,subcluster_scores;
	for (int ii=0; ii<num_subclusters; ii++) {
		subcluster_abs_peaks << fabs(subclusters[ii].peak);
		subcluster_scores << subclusters[ii].detectability_score;
	}
	double slope=compute_slope(subcluster_abs_peaks,subcluster_scores);
	if (slope) {
		for (int ii=0; ii<num_subclusters; ii++) {
			subcluster_scores[ii]/=slope;
			subclusters[ii].detectability_score/=slope;
		}
	}
    QList<long> labels_to_use; for (int k=0; k<=K; k++) labels_to_use << 0;
    QList<long> events_to_use; for (int i=0; i<L; i++) events_to_use << 0;
	int num_events_to_use=0;
	for (int ii=0; ii<num_subclusters; ii++) {
		if (subcluster_scores[ii]>=opts.detectability_threshold) {
			labels_to_use[subclusters[ii].label]=1;
			for (int jj=0; jj<subclusters[ii].inds.count(); jj++) {
				events_to_use[subclusters[ii].inds[jj]]=1;
				num_events_to_use++;
			}
		}
	}
    QList<long> labels_map; for (int k=0; k<=K; k++) labels_map << 0;
	int kk=1;
	for (int k=1; k<=K; k++) {
		if (labels_to_use[k]) {
			labels_map[k]=kk;
			kk++;
		}
	}
	Mda firings_out;
	firings_out.allocate(firings.N1(),num_events_to_use);
	int ee=0;
	for (int i=0; i<L; i++) {
		if (events_to_use[i]) {
			for (int jj=0; jj<firings.N1(); jj++) {
				firings_out.set(firings.get(jj,i),jj,ee);
			}
			int label=(int)firings_out.get(2,ee);
			if (label) label=labels_map[label];
			firings_out.set(label,2,ee);
			ee++;
		}
	}

    printf("Using %d/%d events in %ld/%d clusters\n",num_events_to_use,L,compute_max(labels_map),K);
    firings_out.write64(firings_out_path);

	return true;
}
