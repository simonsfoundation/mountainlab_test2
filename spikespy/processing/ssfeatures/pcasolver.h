#ifndef pcasolver_H
#define pcasolver_H

#include "array2d.h"
#include <QList>

class PCASolverPrivate;
class PCASolver {
public:
	friend class PCASolverPrivate;
	PCASolver();
	virtual ~PCASolver();
	void setVectors(const Array2D &V);
	void setWeights(const QList<float> &W);
	void setComponentCount(int c);
	const Array2D &components() const;
	const Array2D &coefficients() const;
	void setNumIterations(int val=10);
	QList<double> energies() const;
	bool solve();
private:
	PCASolverPrivate *d;
};

float vector_norm(long N,float *v);
void define_random_vector(long N,float *v);
void normalize(long N,float *v);
float inner_product(long N,float *v,float *w);
void subtract_component(long N,float *v,float *comp);

#endif
