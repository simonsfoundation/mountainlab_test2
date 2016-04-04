#include "msmisc.h"
#include <math.h>
#include <QDateTime>
#include <QDir>
#include <QCryptographicHash>
#include "textfile.h"

double compute_min(const QList<double> &X) {
	double ret=X.value(0);
	for (int i=0; i<X.count(); i++) if (X[i]<ret) ret=X[i];
	return ret;
}

double compute_max(const QList<double> &X) {
	double ret=X.value(0);
	for (int i=0; i<X.count(); i++) if (X[i]>ret) ret=X[i];
	return ret;
}

int compute_max(const QList<int> &X) {
	int ret=X.value(0);
	for (int i=0; i<X.count(); i++) if (X[i]>ret) ret=X[i];
	return ret;
}

double compute_norm(long N,double *X) {
    double sumsqr=0;
    for (long i=0; i<N; i++) sumsqr+=X[i]*X[i];
    return sqrt(sumsqr);
}

Mda compute_mean_clip(Mda &clips) {
	int M=clips.N1();
	int T=clips.N2();
	int L=clips.N3();
	Mda ret; ret.allocate(M,T);
	int aaa=0;
	for (int i=0; i<L; i++) {
		int bbb=0;
		for (int t=0; t<T; t++) {
			for (int m=0; m<M; m++) {
				ret.set(ret.get(bbb)+clips.get(aaa),bbb);
				aaa++;
				bbb++;
			}
		}
	}
	if (L) {
		for (int t=0; t<T; t++) {
			for (int m=0; m<M; m++) {
				ret.set(ret.get(m,t)/L,m,t);
			}
		}
	}
	return ret;
}

double compute_mean(const QList<double> &X)
{
	double sum=0;
	for (int i=0; i<X.count(); i++) sum+=X[i];
	if (X.count()) sum/=X.count();
	return sum;
}

double compute_stdev(const QList<double> &X)
{
	double sumsqr=0;
	for (int i=0; i<X.count(); i++) sumsqr+=X[i]*X[i];
	double sum=0;
	for (int i=0; i<X.count(); i++) sum+=X[i];
	int ct=X.count();
	if (ct>=2) {
		return sqrt((sumsqr-sum*sum/ct)/(ct-1));
	}
	else return 0;
}
Mda grab_clips_subset(Mda &clips,const QList<int> &inds) {
	int M=clips.N1();
	int T=clips.N2();
	int LLL=inds.count();
	Mda ret; ret.allocate(M,T,LLL);
	for (int i=0; i<LLL; i++) {
		long aaa=i*M*T;
		long bbb=inds[i]*M*T;
		for (int k=0; k<M*T; k++) {
			ret.set(clips.get(bbb),aaa);
			aaa++; bbb++;
		}
	}
	return ret;
}

double compute_max(long N, double *X)
{
	if (N==0) return 0;
	double ret=X[0];
	for (long i=0; i<N; i++) {
		if (X[i]>ret) ret=X[i];
	}
	return ret;
}

QString get_temp_fname()
{
    long rand_num = qrand() + QDateTime::currentDateTime().toMSecsSinceEpoch();
    return QString("%1/MdaClient_%2.tmp").arg(QDir::tempPath()).arg(rand_num);
}

QString http_get_text(QString url)
{
    QString tmp_fname = get_temp_fname();
    QString cmd = QString("curl \"%1\" > %2").arg(url).arg(tmp_fname);
    qDebug()  << cmd;
    int exit_code = system(cmd.toLatin1().data());
    if (exit_code != 0) {
        qWarning() << "Problem with system call: " + cmd;
        QFile::remove(tmp_fname);
        return "";
    }
    QString ret = read_text_file(tmp_fname);
    QFile::remove(tmp_fname);
    qDebug()  << "RESPONSE: " << ret;
    return ret;
}

QString http_get_binary_mda_file(QString url)
{
    QString tmp_fname = get_temp_fname() + ".mda";
    QString cmd = QString("curl \"%1\" > %2").arg(url).arg(tmp_fname);
    qDebug()  << cmd;
    int exit_code = system(cmd.toLatin1().data());
    if (exit_code != 0) {
        qWarning() << "Problem with system call: " + cmd;
        QFile::remove(tmp_fname);
        return "";
    }
    return tmp_fname;
}

QString compute_hash(const QString &str) {
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(str.toLatin1());
    return QString(hash.result().toHex());
}

QString remote_name_of_path(const QString &path)
{
    if (!path.startsWith("remote://")) return "";
    int len1=QString("remote://").count();
    int ind1=path.indexOf("/",len1);
    if (ind1<0) return path.mid(len1);
    else return path.mid(len1,ind1-len1);
}

QString mscmd_url_for_remote(const QString &remote_name)
{
    QString url;
    if (remote_name=="localhost") url="http://localhost:8001";
    if (remote_name=="magland") url="http://magland.org:8001";
    return url;
}

QString file_url_for_remote_path(const QString &path)
{
    QString remote_name=remote_name_of_path(path);
    QString url="";

    if (remote_name=="localhost") url="http://localhost:8000";
    if (remote_name=="magland") url="http://magland.org:8000";

    if (!path.startsWith("remote://")) return ";";
    int len1=QString("remote://").count();
    int ind1=path.indexOf("/",len1);
    if (ind1<0) return url;
    else return url+"/"+path.mid(ind1+1);
}
