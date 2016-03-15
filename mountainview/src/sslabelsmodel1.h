#ifndef SSLABELSMODEL1_H
#define SSLABELSMODEL1_H

#include "diskreadmda.h"
#include "sslabelsmodel.h"

class SSLabelsModel1Private;
class SSLabelsModel1 : public SSLabelsModel
{
public:
	friend class SSLabelsModel1Private;
	SSLabelsModel1();
	~SSLabelsModel1();
	void setTimepointsLabels(DiskReadMda *TL,bool is_owner=false);

	Mda getTimepointsLabels(int t1,int t2);
	qint32 getMaxTimepoint();
	qint32 getMaxLabel();
private:
	SSLabelsModel1Private *d;
};


#endif // SSLABELSMODEL1_H
