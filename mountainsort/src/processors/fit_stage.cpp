/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/25/2016
*******************************************************/

#include "fit_stage.h"
#include <QList>
#include "msmisc.h"
#include "diskreadmda.h"
#include <math.h>

double compute_norm(long N,double *X);
double compute_score(long N,double *X,double *template0);
QList<int> find_events_to_use(const QList<long> &times,const QList<double> &scores,const fit_stage_opts &opts);
void subtract_template(long N,double *X,double *template0);

bool fit_stage(const QString &timeseries_path, const QString &firings_path, const QString &firings_out_path, const fit_stage_opts &opts)
{
    Mda X(timeseries_path);
    Mda firings; firings.read(firings_path);
    int M=X.N1();
    int T=opts.clip_size;
    int Tmid=(int)((T+1)/2)-1;
    long L=firings.N2();
    QList<long> times;
    QList<int> labels;
    for (long i=0; i<L; i++) {
        times << (long)(firings.value(1,i)+0.5);
        labels << (int)firings.value(2,i);
    }
    int K=compute_max(labels);
    Mda templates(M,T,K+1);
    QList<long> counts; for (int k=0; k<=K; k++) counts << k;
    for (long i=0; i<L; i++) {
        int k=labels[i];
        long t0=times[i];
        if (k>0) {
            for (int t=0; t<T; t++) {
                for (int m=0; m<M; m++) {
                    templates.set(templates.get(m,t,k)+X.value(m,t0+t-Tmid),m,t,k);
                }
            }
            counts[k]++;
        }
    }
    for (int k=0; k<=K; k++) {
        for (int t=0; t<T; t++) {
            for (int m=0; m<M; m++) {
                if (counts[k]) {
                    templates.set(templates.get(m,t,k)/counts[k],m,t,k);
                }
            }
        }
    }

    QList<double> template_norms;
    for (int k=0; k<=K; k++) {
        template_norms << compute_norm(M*T,templates.dataPtr(0,0,k));
    }

    bool something_changed=true;
    QList<int> all_to_use; for (long i=0; i<L; i++) all_to_use << 0;
    int num_passes=0;
    while (something_changed) {
        num_passes++;
        printf("pass %d... ",num_passes);
        QList<double> scores_to_try;
        QList<long> times_to_try;
        QList<int> labels_to_try;
        QList<long> inds_to_try;
        //QList<double> template_norms_to_try;
        for (long i=0; i<L; i++) {
            if (all_to_use[i]==0) {
                long t0=times[i];
                int k0=labels[i];
                if (k0>0) {
                    double score0=compute_score(M*T,X.dataPtr(0,t0-Tmid),templates.dataPtr(0,0,k0));
                    if (score0<template_norms[k0]*template_norms[k0]*0.5) score0=0; //the norm of the improvement needs to be at least 0.5 times the norm of the template
                    if (score0>0) {
                        scores_to_try << score0;
                        times_to_try << t0;
                        labels_to_try << k0;
                        inds_to_try << i;
                    }
                    else {
                        all_to_use[i]=-1;
                    }
                }
            }
        }
        QList<int> to_use=find_events_to_use(times_to_try,scores_to_try,opts);
        something_changed=false;
        long num_added=0;
        for (long i=0; i<to_use.count(); i++) {
            if (to_use[i]==1) {
                something_changed=true;
                num_added++;
                subtract_template(M*T,X.dataPtr(0,times_to_try[i]-Tmid),templates.dataPtr(0,0,labels_to_try[i]));
                all_to_use[inds_to_try[i]]=1;
            }
        }
        printf("added %ld events\n",num_added);
    }

    long num_to_use=0;
    for (long i=0; i<L; i++) {
        if (all_to_use[i]==1) num_to_use++;
    }
    if (times.count()) {
        printf("using %ld/%ld events (%g%%)\n",num_to_use,(long)times.count(),num_to_use*100.0/times.count());
    }
    Mda firings_out(firings.N1(),num_to_use);
    long aa=0;
    for (long i=0; i<L; i++) {
        if (all_to_use[i]==1) {
            for (int j=0; j<firings.N1(); j++) {
                firings_out.set(firings.get(j,i),j,aa);
            }
            aa++;
        }
    }

    firings_out.write64(firings_out_path);

    return true;
}

double compute_norm(long N,double *X) {
    double sumsqr=0;
    for (long i=0; i<N; i++) sumsqr+=X[i]*X[i];
    return sqrt(sumsqr);
}
double compute_score(long N,double *X,double *template0) {
    Mda resid(1,N);
    double *resid_ptr=resid.dataPtr();
    for (long i=0; i<N; i++) resid_ptr[i]=X[i]-template0[i];
    double norm1=compute_norm(N,X);
    double norm2=compute_norm(N,resid_ptr);
    return norm1*norm1-norm2*norm2;
}

QList<int> find_events_to_use(const QList<long> &times,const QList<double> &scores,const fit_stage_opts &opts) {
    QList<int> to_use;
    long L=times.count();
    for (long i=0; i<L; i++) to_use << 0;
    double last_best_score=0;
    long last_best_ind=0;
    for (long i=0; i<L; i++) {
        if (scores[i]>0) {
            if (times[last_best_ind]<times[i]-opts.clip_size) {
                for (int ii=last_best_ind+1; ii<i; ii++) {
                    last_best_score=0;
                    if (times[ii]>=times[i]-opts.clip_size) {
                        if (scores[ii]>=last_best_score) {
                            last_best_score=scores[ii];
                            last_best_ind=ii;
                        }
                    }
                }
            }
            if (scores[i]>last_best_score) {
                to_use[last_best_score]=0;
                to_use[i]=1;
            }
        }
    }
    return to_use;
}

void subtract_template(long N,double *X,double *template0) {
    for (long i=0; i<N; i++) {
        X[i]-=template0[i];
    }
}
