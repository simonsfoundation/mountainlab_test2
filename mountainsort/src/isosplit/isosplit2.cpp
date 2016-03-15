#include "isosplit2.h"
#include <QSet>
#include <math.h>
#include "isocut.h"
#include <stdio.h>
#include <QDebug>
#include "lapacke.h"

QList<int> do_kmeans(Mda &X,int K);
bool eigenvalue_decomposition_sym_isosplit(Mda &U, Mda &S,Mda &X);

struct AttemptedComparisons {
    QList<double> centers1,centers2;
    QList<int> counts1,counts2;
};


QList<int> find_inds(const QList<int> &labels,int k) {
    QList<int> ret;
    for (int i=0; i<labels.count(); i++) {
        if (labels[i]==k) ret << i;
    }
    return ret;
}

void geometric_median(int M,int N,double *ret,double *X) {
    int num_iterations=10;
    if (N==0) return;
    if (N==1) {
        for (int m=0; m<M; m++) ret[m]=X[m];
        return;
    }
    double *weights=(double *)malloc(sizeof(double)*N);
    for (int i=0; i<N; i++) weights[i]=1;
    for (int it=1; it<=num_iterations; it++) {
        double sum_weights=0;
        for (int i=0; i<N; i++) sum_weights+=weights[i];
        if (sum_weights) {
            for (int i=0; i<N; i++) weights[i]/=sum_weights;
        }
        for (int m=0; m<M; m++) ret[m]=0;
        int aa=0;
        for (int n=0; n<N; n++) {
            for (int m=0; m<M; m++) {
                ret[m]+=X[aa]*weights[n];
                aa++;
            }
        }
        aa=0;
        for (int n=0; n<N; n++) {
            double sumsqr_diff=0;
            for (int m=0; m<M; m++) {
                double val=X[aa]-ret[m];
                sumsqr_diff=val*val;
                aa++;
            }
            if (sumsqr_diff!=0) {
                weights[n]=1/sqrt(sumsqr_diff);
            }
            else weights[n]=0;
        }
    }
}

QList<double> compute_centroid(Mda &X) {
	int M=X.N1();
	int N=X.N2();
	QList<double> ret;
	for (int i=0; i<M; i++) ret << 0;
	for (int n=0; n<N; n++) {
		for (int m=0; m<M; m++) {
			ret[m]+=X.value(m,n);
		}
	}
	for (int i=0; i<M; i++) ret[i]/=N;
	return ret;
}


QList<double> compute_center(Mda &X,const QList<int> &inds) {
    int M=X.N1();
    int NN=inds.count();
    if (NN==0) {
        QList<double> ret; for (int i=0; i<M; i++) ret << 0;
        return ret;
    }
	double *XX=(double *)malloc(sizeof(double)*M*NN);
    int aa=0;
    for (int n=0; n<NN; n++) {
        for (int m=0; m<M; m++) {
            XX[aa]=X.value(m,inds[n]);
            aa++;
        }
    }
    double *result=(double *)malloc(sizeof(double)*M);
    geometric_median(M,NN,result,XX);
    QList<double> ret; for (int m=0; m<M; m++) ret << result[m];
    free(result);
    free(XX);
    return ret;
}

Mda compute_centers(Mda &X,const QList<int> &labels,int K) {
    int M=X.N1();
    //int N=X.N2();
    Mda ret;
    ret.allocate(M,K);
    for (int k=0; k<K; k++) {
        QList<int> inds=find_inds(labels,k);
        QList<double> ctr=compute_center(X,inds);
        for (int m=0; m<M; m++) ret.setValue(ctr[m],m,k);
    }
    return ret;
}

double distance_between_vectors(int M,double *v1,double *v2) {
    double sumsqr=0;
    for (int i=0; i<M; i++) {
        double val=v1[i]-v2[i];
        sumsqr+=val*val;
    }
    return sqrt(sumsqr);
}


bool was_already_attempted(int M,AttemptedComparisons &attempted_comparisons,double *center1,double *center2,int count1,int count2,double repeat_tolerance) {
    double tol=repeat_tolerance;
    for (int i=0; i<attempted_comparisons.counts1.count(); i++) {
        double diff_count1=fabs(attempted_comparisons.counts1[i]-count1);
        double avg_count1=(attempted_comparisons.counts1[i]+count1)/2;
        if (diff_count1<=tol*avg_count1) {
            double diff_count2=fabs(attempted_comparisons.counts2[i]-count2);
            double avg_count2=(attempted_comparisons.counts2[i]+count2)/2;
            if (diff_count2<=tol*avg_count2) {
                double C1[M]; for (int m=0; m<M; m++) C1[m]=attempted_comparisons.centers1[i*M+m];
                double C2[M]; for (int m=0; m<M; m++) C2[m]=attempted_comparisons.centers2[i*M+m];
                double dist0=distance_between_vectors(M,C1,C2);
                double dist1=distance_between_vectors(M,C1,center1);
                double dist2=distance_between_vectors(M,C2,center2);
                if (dist0>0) {
                    double frac1=dist1/dist0;
                    double frac2=dist2/dist0;
                    if ((frac1<=tol*1/sqrt(count1))&&(frac2<=tol*1/sqrt(count2))) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void find_next_comparison(int M,int K,int &k1,int &k2,bool *active_labels,double *Cptr,int *counts,AttemptedComparisons &attempted_comparisons,double repeat_tolerance) {
    QList<int> active_inds;
    for (int k=0; k<K; k++) if (active_labels[k]) active_inds << k;
    if (active_inds.count()==0) {
        k1=-1; k2=-1;
        return;
    }
    int Nactive=active_inds.count();
    double dists[Nactive][Nactive];
    for (int a=0; a<Nactive; a++) {
        for (int b=0; b<Nactive; b++) {
            dists[a][b]=distance_between_vectors(M,&Cptr[active_inds[a]*M],&Cptr[active_inds[b]*M]);
        }
        dists[a][a]=-1; //don't use it
    }
    while (true) {
        int best_a=-1,best_b=-1;
        double best_dist=-1;
        for (int a=0; a<Nactive; a++) {
            for (int b=0; b<Nactive; b++) {
                double val=dists[a][b];
                if (val>=0) {
                    if ((best_dist<0)||(val<best_dist)) {
                        best_a=a;
                        best_b=b;
                        best_dist=val;
                    }
                }
            }
        }
		if (best_a<0) {
			k1=-1; k2=-1;
			return;
		}
        k1=active_inds[best_a];
        k2=active_inds[best_b];
        if ((counts[k1]>0)&&(counts[k2]>0)) { //just to make sure (zero was actually happening some times, but not sure why)
            if (!was_already_attempted(M,attempted_comparisons,&Cptr[k1*M],&Cptr[k2*M],counts[k1],counts[k2],repeat_tolerance)) {
                //hurray!
                return;
            }
        }
        dists[best_a][best_b]=-1;
        dists[best_b][best_a]=-1;
    }
    k1=-1; k2=-1;
}

Mda matrix_transpose_isosplit(const Mda &A) {
	Mda ret(A.N2(),A.N1());
	for (int i=0; i<A.N1(); i++) {
		for (int j=0; j<A.N2(); j++) {
			ret.set(A.get(i,j),j,i);
		}
	}
	return ret;
}

Mda matrix_multiply_isosplit(const Mda &A,const Mda &B) {
	int N1=A.N1();
	int N2=A.N2();
	int N2B=B.N1();
	int N3=B.N2();
	if (N2!=N2B) {
		qWarning() << "Unexpected problem in matrix_multiply" << N1 << N2 << N2B << N3;
		exit(-1);
	}
	Mda ret(N1,N3);
	for (int i=0; i<N1; i++) {
		for (int j=0; j<N3; j++) {
			double val=0;
			for (int k=0; k<N2; k++) {
				val+=A.get(i,k)*B.get(k,j);
			}
			ret.set(val,i,j);
		}
	}
	return ret;
}

Mda get_whitening_matrix_isosplit(Mda &COV) {
	int M=COV.N1();
	Mda U(M,M),S(1,M);
	eigenvalue_decomposition_sym_isosplit(U,S,COV);
	Mda S2(M,M);
	for (int m=0; m<M; m++) {
		if (S.get(m)) {
			S2.set(1/sqrt(S.get(m)),m,m);
		}
	}
	Mda W=matrix_multiply_isosplit(matrix_multiply_isosplit(U,S2),matrix_transpose_isosplit(U));
	return W;
}

void whiten_two_clusters(double *V,Mda &X1,Mda &X2) {
	int M=X1.N1();
	int N1=X1.N2();
	int N2=X2.N2();
	int N=N1+N2;

	QList<double> center1=compute_centroid(X1);
	QList<double> center2=compute_centroid(X2);

	if (M<N) { //otherwise there are too few points to whiten

		Mda XX(M,N);
		for (int i=0; i<N1; i++) {
			for (int j=0; j<M; j++) {
				XX.set(X1.get(j,i)-center1[j],j,i);
			}
		}
		for (int i=0; i<N2; i++) {
			for (int j=0; j<M; j++) {
				XX.set(X2.get(j,i)-center2[j],j,i+N1);
			}
		}

		Mda COV=matrix_multiply_isosplit(XX,matrix_transpose_isosplit(XX));
		Mda W=get_whitening_matrix_isosplit(COV);
		X1=matrix_multiply_isosplit(W,X1);
		X2=matrix_multiply_isosplit(W,X2);
	}


	//compute the vector
	center1=compute_centroid(X1);
	center2=compute_centroid(X2);
	for (int m=0; m<M; m++) {
		V[m]=center2[m]-center1[m];
	}
}

QList<int> test_redistribute(bool &do_merge,Mda &Y1,Mda &Y2,double isocut_threshold) {
	Mda X1; X1=Y1;
	Mda X2; X2=Y2;
	int M=X1.N1();
	int N1=X1.N2();
	int N2=X2.N2();
	QList<int> ret; for (int i=0; i<N1+N2; i++) ret << 1;
	do_merge=true;
	double V[M];
	whiten_two_clusters(V,X1,X2);
	double normv=0;
	{
		double sumsqr=0; for (int m=0; m<M; m++) sumsqr+=V[m]*V[m];
		normv=sqrt(sumsqr);
	}
	if (!normv) {
		printf("Warning: isosplit2: vector V is null.\n");
		return ret;
	}
	if (N1+N2<=5) {
		//avoid a crash?
		return ret;
	}

	//project onto line connecting the centers
	QList<double> XX;
	for (int i=0; i<N1; i++) {
		double val=0;
		for (int m=0; m<M; m++) val+=X1.value(m,i)*V[m];
		XX << val;
	}
    for (int i=0; i<N2; i++) {
		double val=0;
		for (int m=0; m<M; m++) val+=X2.value(m,i)*V[m];
		XX << val;
	}

	QList<double> XXs=XX;
	qSort(XXs);
	double *XXX=(double *)malloc(sizeof(double)*(N1+N2));
	for (int i=0; i<N1+N2; i++) XXX[i]=XXs[i];

	double cutpoint;
	bool do_cut=isocut(N1+N2,&cutpoint,XXX,isocut_threshold,5);
	free(XXX);

	if (do_cut) {
		do_merge=0;
		for (int i=0; i<N1+N2; i++) {
			if (XX[i]<=cutpoint) ret[i]=1;
			else ret[i]=2;
		}
	}
	return ret;
}

QList<int> test_redistribute(bool &do_merge,Mda &X,const QList<int> &inds1,const QList<int> &inds2,double isocut_threshold) {
    int M=X.N1();
    Mda X1;
    X1.allocate(M,inds1.count());
    for (int i=0; i<inds1.count(); i++) {
        for (int m=0; m<M; m++) {
            X1.setValue(X.value(m,inds1[i]),m,i);
        }
    }
    Mda X2;
    X2.allocate(M,inds2.count());
    for (int i=0; i<inds2.count(); i++) {
        for (int m=0; m<M; m++) {
            X2.setValue(X.value(m,inds2[i]),m,i);
        }
    }
    return test_redistribute(do_merge,X1,X2,isocut_threshold);
}

int compute_max_00(const QList<int> &X) {
    int ret=X.value(0);
    for (int i=0; i<X.count(); i++) if (X[i]>ret) ret=X[i];
    return ret;
}


QList<int> isosplit2(Mda &X, float isocut_threshold, int K_init,bool verbose)
{
    double repeat_tolerance=0.2;

    int M=X.N1();
    int N=X.N2();
	QList<int> labels=do_kmeans(X,K_init);

    bool active_labels[K_init];
    for (int ii=0; ii<K_init; ii++) active_labels[ii]=true;
    Mda centers=compute_centers(X,labels,K_init); //M x K_init
    int counts[K_init]; for (int ii=0; ii<K_init; ii++) counts[ii]=0;
    for (int i=0; i<N; i++) counts[labels[i]]++;
    double *Cptr=centers.dataPtr();

    AttemptedComparisons attempted_comparisons;

    int num_iterations=0;
    int max_iterations=1000;
    while ((true)&&(num_iterations<max_iterations)) {
        num_iterations++;
		if (verbose) printf("isosplit2: iteration %d\n",num_iterations);
        QList<int> old_labels=labels;
        int k1,k2;
        find_next_comparison(M,K_init,k1,k2,active_labels,Cptr,counts,attempted_comparisons,repeat_tolerance);
        if (k1<0) break;
        if (verbose) printf("compare %d(%d),%d(%d) --- ",k1,counts[k1],k2,counts[k2]);

        QList<int> inds1=find_inds(labels,k1);
        QList<int> inds2=find_inds(labels,k2);
        QList<int> inds12=inds1; inds12.append(inds2);
        for (int m=0; m<M; m++) {
            attempted_comparisons.centers1 << Cptr[m+k1*M];
            attempted_comparisons.centers2 << Cptr[m+k2*M];
        }
        attempted_comparisons.counts1 << inds1.count();
        attempted_comparisons.counts2 << inds2.count();
        for (int m=0; m<M; m++) {
            attempted_comparisons.centers2 << Cptr[m+k1*M];
            attempted_comparisons.centers1 << Cptr[m+k2*M];
        }
        attempted_comparisons.counts2 << inds1.count();
        attempted_comparisons.counts1 << inds2.count();

        bool do_merge;

        QList<int> labels0=test_redistribute(do_merge,X,inds1,inds2,isocut_threshold);
        int max_label=compute_max_00(labels0);
        if ((do_merge)||(max_label==1)) {
            if (verbose) printf("merging size=%d.\n",inds12.count());
            for (int i=0; i<N; i++) {
                if (labels[i]==k2) labels[i]=k1;
            }
            QList<double> ctr=compute_center(X,inds12);
            for (int m=0; m<M; m++) {
                centers.setValue(ctr[m],m,k1);
            }
            counts[k1]=inds12.count();
            counts[k2]=0;
            active_labels[k2]=false;

        }
        else {

            QList<int> indsA0=find_inds(labels0,1);
            QList<int> indsB0=find_inds(labels0,2);
            QList<int> indsA,indsB;
            for (int i=0; i<indsA0.count(); i++) indsA << inds12[indsA0[i]];
            for (int i=0; i<indsB0.count(); i++) indsB << inds12[indsB0[i]];
            for (int i=0; i<indsA.count(); i++) {
                labels[indsA[i]]=k1;
            }
            for (int i=0; i<indsB.count(); i++) {
                labels[indsB[i]]=k2;
            }
            if (verbose) printf("redistributing sizes=(%d,%d).\n",indsA.count(),indsB.count());
            {
                QList<double> ctr=compute_center(X,indsA);
                for (int m=0; m<M; m++) {
                    centers.setValue(ctr[m],m,k1);
                }
            }
            {
                QList<double> ctr=compute_center(X,indsB);
                for (int m=0; m<M; m++) {
                    centers.setValue(ctr[m],m,k2);
                }
            }
            counts[k1]=indsA.count();
            counts[k2]=indsB.count();
        }

    }

    int labels_map[K_init]; for (int k=0; k<K_init; k++) labels_map[k]=0;
    int kk=1;
    for (int j=0; j<K_init; j++) {
        if ((active_labels[j])&&(counts[j]>0)) {
            labels_map[j]=kk; kk++;
        }
    }
    QList<int> ret;
    for (int i=0; i<N; i++) {
        ret << labels_map[labels[i]];
    }
    return ret;
}

//choose K distinct (sorted) integers between 0 and N-1. If K>N then it will repeat the last integer a suitable number of times
QList<int> choose_random_indices(int N,int K) {;
    QList<int> ret;
    if (K>=N) {
        for (int i=0; i<N; i++) ret << i;
        while (ret.count()<K) ret << N-1;
        return ret;
    }
    QSet<int> theset;
    while (theset.count()<K) {
        int ind=(qrand()%N);
        theset.insert(ind);
    }
    ret=theset.toList();
    qSort(ret);
    return ret;
}

//do k-means with K clusters -- X is MxN representing N points in M-dimensional space. Returns a labels vector of size N.
QList<int> do_kmeans(Mda &X,int K) {
    int M=X.N1();
    int N=X.N2();
    double *Xptr=X.dataPtr();
    Mda centroids_mda; centroids_mda.allocate(M,K); double *centroids=centroids_mda.dataPtr();
    QList<int> labels; for (int i=0; i<N; i++) labels << -1;
    int *counts=(int *)malloc(sizeof(int)*K);

    //initialize the centroids
    QList<int> initial=choose_random_indices(N,K);
    for (int j=0; j<K; j++) {
        int ind=initial[j];
        int jj=ind*M;
        int ii=j*M;
        for (int m=0; m<M; m++) {
            centroids[m+ii]=Xptr[m+jj];
        }
    }

    bool something_changed=true;
    while (something_changed) {
        something_changed=false;
        //Assign the labels
        for (int n=0; n<N; n++) {
            int jj=n*M;
            double best_distsqr=0;
            int best_k=0;
            for (int k=0; k<K; k++) {
                int ii=k*M;
                double tmp=0;
                for (int m=0; m<M; m++) {
                    tmp+=(centroids[m+ii]-Xptr[m+jj])*(centroids[m+ii]-Xptr[m+jj]);
                }
                if ((k==0)||(tmp<best_distsqr)) {
                    best_distsqr=tmp;
                    best_k=k;
                }
            }
            if (labels[n]!=best_k) {
                labels[n]=best_k;
                something_changed=true;
            }
        }

        if (something_changed) {
            //Compute the centroids
            for (int k=0; k<K; k++) {
                int ii=k*M;
                for (int m=0; m<M; m++) {
                    centroids[m+ii]=0;
                }
                counts[k]=0;
            }
            for (int n=0; n<N; n++) {
                int jj=n*M;
                int k=labels[n];
                int ii=k*M;
                for (int m=0; m<M; m++) {
                    centroids[m+ii]+=Xptr[m+jj];
                }
                counts[k]++;
            }
            for (int k=0; k<K; k++) {
                int ii=k*M;
                if (counts[k]) {
                    for (int m=0; m<M; m++)
                        centroids[m+ii]/=counts[k];
                }
            }
        }
    }

    free(counts);

    return labels;
}


void test_isosplit2_routines()
{
	{ //whiten two clusters
		//compare this with the test in matlab isosplit2('test')
		Mda X1,X2;
		int M=4;
		X1.allocate(M,M);
		X2.allocate(M,M);
		for (int m1=0; m1<M; m1++) {
			for (int m2=0; m2<M; m2++) {
				X1.setValue(sin(m1+m2)+sin(m1*m2),m1,m2);
				X2.setValue(cos(m1+m2)-cos(m1*m2),m1,m2);
			}
		}
		printf("X1:\n");
		for (int m2=0; m2<M; m2++) {
			for (int m1=0; m1<M; m1++) {
				printf("%g ",X1.value(m2,m1));
			}
			printf("\n");
		}
		double V[M];
		whiten_two_clusters(V,X1,X2);
		printf("V: ");
		for (int m=0; m<M; m++) {
			printf("%g ",V[m]);
		}
		printf("\n");
	}

	{
		int M=2;
        int N=120;
		Mda X; X.allocate(M,N);
		for (int i=0; i<N; i++) {
			double r1=(qrand()%100000)*1.0/100000;
			double r2=(qrand()%100000)*1.0/100000;
            if (i<N/3) {
				double val1=r1;
				double val2=r2;
				X.setValue(val1,0,i);
				X.setValue(val2,1,i);
			}
            else if (i<2*N/3) {
                double val1=1.5+r1;
                double val2=r2;
				X.setValue(val1,0,i);
				X.setValue(val2,1,i);
			}
            else {
                double val1=r1;
                double val2=1.5+r2;
                X.setValue(val1,0,i);
                X.setValue(val2,1,i);
            }
		}

        QList<int> labels=isosplit2(X,1.5,30,false);
		printf("Labels:\n");
		for (int i=0; i<labels.count(); i++) {
			printf("%d ",labels[i]);
		}
		printf("\n");
	}
}

bool eigenvalue_decomposition_sym_isosplit(Mda &U, Mda &S,Mda &X)
{
	//X=U*diag(S)*U'
	//X is MxM, U is MxM, S is 1xM
	//X must be real and symmetric

	int M=X.N1();
	if (M!=X.N2()) {
		qWarning() << "Unexpected problem in eigenvalue_decomposition_sym" << X.N1() << X.N2();
		exit(-1);
	}

	U.allocate(M,M);
	S.allocate(1,M);
	double *Uptr=U.dataPtr();
	double *Sptr=S.dataPtr();
	double *Xptr=X.dataPtr();

	for (int ii=0; ii<M*M; ii++) {
		Uptr[ii]=Xptr[ii];
	}

	//'V' means compute eigenvalues and eigenvectors (use 'N' for eigenvalues only)
	//'U' means upper triangle of A is stored.
	//QTime timer; timer.start();
	int info=LAPACKE_dsyev(LAPACK_COL_MAJOR,'V','U',M,Uptr,M,Sptr);
	//printf("Time for call to LAPACKE_dsyev: %g sec\n",timer.elapsed()*1.0/1000);
	if (info!=0) {
		qWarning() << "Error in LAPACKE_dsyev" << info;
		return false;
	}
	return true;
}
