#ifndef MDAOBJECT_H
#define MDAOBJECT_H

#include <QObject>
#include "mda.h"

class MdaObject : public QObject, public Mda
{
	Q_OBJECT
public:
	explicit MdaObject(QObject *parent = 0);
	~MdaObject();

signals:

public slots:
};

#endif // MDAOBJECT_H
