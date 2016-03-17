#include "mvutils.h"

#include <QCoreApplication>
#include "get_pca_features.h"
#include <math.h>
#include "textfile.h"
#include <QDebug>

Mda compute_mean_waveform(DiskArrayModel *C) {
	Mda ret;
	if (!C->dim3()) return ret;
	int M=C->size(0);
	int T=C->size(1)/C->dim3();
	int NC=C->dim3();
	if (!NC) return ret;

	double sum[M*T];
	for (int ii=0; ii<M*T; ii++) sum[ii]=0;
	for (int c=0; c<NC; c++) {
		if ((c%100==0)||(c==NC-1)) {
			qApp->processEvents();
			//int pct=(int)(c*1.0/NC*100);
			//printf("Computing mean waveform...%d/%d (%d%%)\n",c,NC,pct);
		}
		int ii=0;
		Mda tmp=C->loadData(1,T*c,T*(c+1));
		for (int t=0; t<T; t++) {
			for (int m=0; m<M; m++) {
				sum[ii]+=tmp.value(m,t);
				ii++;
			}
		}
	}
	ret.allocate(M,T);
	{
		int ii=0;
		for (int t=0; t<T; t++) {
			for (int m=0; m<M; m++) {
				ret.setValue(sum[ii]/NC,m,t);
				ii++;
			}
		}
	}
	return ret;
}

Mda compute_mean_stdev_waveform(DiskArrayModel *C) {
	Mda ret;
	if (!C->dim3()) return ret;
	int M=C->size(0);
	int T=C->size(1)/C->dim3();
	int NC=C->dim3();
	if (!NC) return ret;

	double sum[M*T];
	double sumsqr[M*T];
	for (int ii=0; ii<M*T; ii++) {sum[ii]=0; sumsqr[ii]=0;}
	for (int c=0; c<NC; c++) {
		if ((c%100==0)||(c==NC-1)) {
			qApp->processEvents();
			//int pct=(int)(c*1.0/NC*100);
			//printf("Computing mean waveform...%d/%d (%d%%)\n",c,NC,pct);
		}
		int ii=0;
		Mda tmp=C->loadData(1,T*c,T*(c+1));
		for (int t=0; t<T; t++) {
			for (int m=0; m<M; m++) {
				float val=tmp.value(m,t);
				sum[ii]+=val;
				sumsqr[ii]+=val*val;
				ii++;
			}
		}
	}
	ret.allocate(M,T*2);
	{
		int ii=0;
		for (int t=0; t<T; t++) {
			for (int m=0; m<M; m++) {
				float mu=sum[ii]/NC;
				float sigma=sqrt(sumsqr[ii]/NC-mu*mu);
				float tmp=0;
				if (mu>0) {
					tmp=mu-sigma;
					if (tmp<0) tmp=0;
				}
				else {
					tmp=mu+sigma;
					if (tmp>0) tmp=0;
				}
				ret.setValue(mu,m,t);
				ret.setValue(tmp,m,T+t);
				ii++;
			}
		}
	}
	return ret;
}

Mda compute_features(DiskArrayModel *C) {
	Mda ret;
	if (!C->dim3()) return ret;
	int M=C->size(0);
	int T=C->size(1)/C->dim3();
	int NC=C->dim3();
	if (!NC) return ret;

	Mda X=C->loadData(1,0,T*NC);
	ret.allocate(3,NC);
	get_pca_features(M*T,NC,3,ret.dataPtr(),X.dataPtr());

	return ret;
}

Mda compute_features(const QList<DiskArrayModel *> &C) {
	Mda ret;
	if (C.isEmpty()) return ret;
	if (!C[0]->dim3()) return ret;
	int M=C[0]->size(0);
	int T=C[0]->size(1)/C[0]->dim3();
	int NC=0;
	for (int i=0; i<C.count(); i++) {
		NC+=C[i]->dim3();
	}
	if (!NC) return ret;

	Mda X; X.allocate(M,T,NC);
	int jj=0;
	for (int i=0; i<C.count(); i++) {
		Mda tmp=C[i]->loadData(1,0,T*C[i]->dim3());
		int ii=0;
		for (int aa=0; aa<T*C[i]->dim3(); aa++) {
			for (int m=0; m<M; m++) {
				X.set(tmp.get(ii),jj);
				ii++;
				jj++;
			}
		}
	}
	ret.allocate(3,NC);
	get_pca_features(M*T,NC,3,ret.dataPtr(),X.dataPtr());

	return ret;
}

QColor get_heat_map_color(double val)
{
    double r=0,g=0,b=0;
    if (val<0.2) {
        double tmp=(val-0)/0.2;
        r=200*(1-tmp)+150*tmp;
        b=200*(1-tmp)+255*tmp;
        g=0*(1-tmp)+0*tmp;
    }
    else if (val<0.4) {
        double tmp=(val-0.2)/0.2;
        r=150*(1-tmp)+0*tmp;
        b=255*(1-tmp)+255*tmp;
        g=0*(1-tmp)+100*tmp;
    }
    else if (val<0.6) {
        double tmp=(val-0.4)/0.2;
        r=0*(1-tmp)+255*tmp;
        b=255*(1-tmp)+0*tmp;
        g=100*(1-tmp)+20*tmp;
    }
    else if (val<0.8) {
        double tmp=(val-0.6)/0.2;
        r=255*(1-tmp)+255*tmp;
        b=0*(1-tmp)+0*tmp;
        g=20*(1-tmp)+255*tmp;
    }
    else if (val<=1.0) {
        double tmp=(val-0.8)/0.2;
        r=255*(1-tmp)+255*tmp;
        b=0*(1-tmp)+255*tmp;
        g=255*(1-tmp)+255*tmp;
    }

    return QColor((int)r,(int)g,(int)b);
}

QList<Epoch> read_epochs(const QString &path)
{
    QList<Epoch> ret;
    QString txt=read_text_file(path);
    QStringList lines=txt.split("\n");
    foreach (QString line,lines) {
        QList<QString> vals=line.split(QRegExp("\\s+"));
        if (vals.value(0)=="EPOCH") {
            if (vals.count()==4) {
                Epoch E;
                E.name=vals.value(1);
                E.t_begin=vals.value(2).toDouble()-1;
                E.t_end=vals.value(3).toDouble()-1;
                ret << E;
            }
            else {
                qWarning() << "Problem parsing epochs file:" << path;
            }
        }
    }
    return ret;
}
