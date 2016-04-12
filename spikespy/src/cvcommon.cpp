#include "cvcommon.h"

#include <QTime>
#include <QList>
#include <QFile>
#include <QFileInfo>
#include <QDir>

CVPoint cvpoint(float x, float y, float z)
{
	CVPoint p; p.x=x; p.y=y; p.z=z;
	return p;
}

QChar make_random_alphanumeric() {
		static int val=0;
		val++;
		QTime time=QTime::currentTime();
		int num=qHash(time.toString("hh:mm:ss:zzz")+QString::number(qrand()+val));
		num=num%36;
		if (num<26) return QChar('A'+num);
		else return QChar('0'+num-26);
}
QString make_random_id(int numchars) {
		QString ret;
		for (int i=0; i<numchars; i++) {
				ret.append(make_random_alphanumeric());
		}
		return ret;
}

CVLine cvline(float x1,float y1,float z1,float x2,float y2,float z2) {
	CVLine L; L.p1=cvpoint(x1,y1,z1); L.p2=cvpoint(x2,y2,z2);
	return L;
}
CVLine cvline(CVPoint p1,CVPoint p2) {
	CVLine L; L.p1=p1; L.p2=p2;
	return L;
}
bool is_zero(CVPoint P) {
	return ((P.x==0)&&(P.y==0)&&(P.z==0));
}

bool is_zero(CVLine L) {
	return ((is_zero(L.p1))&&(is_zero(L.p2)));
}

bool compare(CVPoint P1, CVPoint P2)
{
	return ((P1.x==P2.x)&&(P1.y==P2.y)&&(P1.z==P2.z));
}

bool compare(CVLine L1, CVLine L2)
{
	return ((compare(L1.p1,L2.p1))&&(compare(L1.p2,L2.p2)));
}

static QList<QString> s_paths_to_remove;


void CleanupObject::closing()
{
	for (int i=0; i<s_paths_to_remove.count(); i++) {
		QString path=s_paths_to_remove[i];

		QDateTime time=QFileInfo(path).lastModified();
		QString timestamp=time.toString("yyyy-mm-dd-hh-mm-ss");
		QString tmp=QFileInfo(path).path()+"/spikespy."+QFileInfo(path).completeBaseName()+"."+timestamp;
		if (QDir(tmp).exists()) {
			qDebug()  << "Removing directory: " << tmp;
			QStringList list=QDir(tmp).entryList(QStringList("*.mda"),QDir::Files|QDir::NoDotAndDotDot);
			foreach (QString A,list) {
				QFile::remove(tmp+"/"+A);
			}
			QDir(QFileInfo(tmp).path()).rmdir(QFileInfo(tmp).fileName());
		}

		qDebug()  << "Removing file: " << path;
		QFile(path).remove();
	}
}

void removeOnClose(const QString &path)
{
	s_paths_to_remove << path;
}
