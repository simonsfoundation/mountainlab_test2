#ifndef AFFINETRANSFORMATION_H
#define AFFINETRANSFORMATION_H

#include "cvcommon.h" //for CVPoint -- need to change for future

class AffineTransformationPrivate;
class AffineTransformation
{
public:
	friend class AffineTransformationPrivate;
	AffineTransformation();
	AffineTransformation(const AffineTransformation &other);
	~AffineTransformation();
	void operator=(const AffineTransformation &other);

	CVPoint map(const CVPoint &p);

	void setIdentity();
	void rotateX(double theta,bool left=true);
	void rotateY(double theta,bool left=true);
	void rotateZ(double theta,bool left=true);

	void scale(double sx,double sy,double sz,bool left=true);
	void getMatrixData(double *data);

	bool equals(const AffineTransformation &other);

	AffineTransformation inverse() const;

private:
	AffineTransformationPrivate *d;

};

#endif // AFFINETRANSFORMATION_H
