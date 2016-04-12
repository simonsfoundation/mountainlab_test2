#include "pcasolver.h"
#include <math.h>
#include <QCoreApplication>
#include <QDebug>

class PCASolverPrivate {
public:
	Array2D m_vectors;
	Array2D m_components;
	Array2D m_coefficients;
	Array2D m_weights;
	int m_component_count;
	int m_num_iterations;
	QList<double> m_energies;
};

PCASolver::PCASolver() 
{
	d=new PCASolverPrivate;
	d->m_component_count=1;
	d->m_num_iterations=10;
}
PCASolver::~PCASolver()
{
	delete d;
}
void PCASolver::setVectors(const Array2D &V) {
	d->m_vectors=V;
}
void PCASolver::setComponentCount(int c) {
	d->m_component_count=c;
}


float vector_norm(long N,float *v) {
	float norm=0;
	for (long j=0; j<N; j++)
		norm+=v[j]*v[j];
	norm=sqrt(norm);
	return norm;
}
void define_random_vector(long N,float *v) {
	for (long j=0; j<N; j++) {
		v[j]=(qrand()*1.0/RAND_MAX)*2-1;
	}
}
void normalize(long N,float *v) {
	float norm=vector_norm(N,v);
	if (norm==0) return;
	for (long j=0; j<N; j++)
		v[j]/=norm;
}
float inner_product(long N,float *v,float *w) {
	float ret=0;
	for (long j=0; j<N; j++)
		ret+=v[j]*w[j];
	return ret;
}
void subtract_component(long N,float *v,float *comp) {
	float ip=inner_product(N,v,comp);
	for (long j=0; j<N; j++) {
		v[j]-=ip*comp[j];
	}
}
float inner_product_with_weights(long N,float *v,float *w,float *weights) {
	float ret=0;
	for (long j=0; j<N; j++)
		ret+=v[j]*w[j]*weights[j];
	return ret;
}
void subtract_component_with_weights(long N,float *v,float *comp,float *weights) {
	float ip=inner_product_with_weights(N,v,comp,weights);
	for (long j=0; j<N; j++) {
		v[j]-=ip*comp[j];
	}
}




bool PCASolver::solve() {
	
	//Define the working vectors
	int N=d->m_vectors.width();
	int num_vectors=d->m_vectors.height();
	float *working_vectors=(float *)malloc(sizeof(float)*N*num_vectors);
	for (int vnum=0; vnum<num_vectors; vnum++) {
		for (long j=0; j<N; j++)
			working_vectors[vnum*N+j]=d->m_vectors.value(j,vnum);
	}
	
	//Define the working components
	int num_components=d->m_component_count;
	float *working_components=(float *)malloc(sizeof(float)*N*num_components);
	for (int cnum=0; cnum<num_components; cnum++) {
		for (long j=0; j<N; j++)
			working_components[cnum*N+j]=0;
	}
	
	//allocate the coefficients and energies
	float *coeffs=(float *)malloc(sizeof(float)*num_vectors*num_components);
	double *energies=(double *)malloc(sizeof(double)*num_components);
	
	for (int current_component=0; current_component<num_components; current_component++) {
		float *component_vector=&working_components[current_component*N];
		float component_norm=vector_norm(N,component_vector);
		if (component_norm<0.1) {
			define_random_vector(N,component_vector);
			normalize(N,component_vector);
		}
		for (long it=0; it<d->m_num_iterations; it++) {	
			float *hold=(float *)malloc(sizeof(float)*N);
			if (!hold) {
				qWarning() << "Unable to allocate hold of size:"<< N;
			}
			for (int j=0; j<N; j++) hold[j]=0;
			for (int vnum=0; vnum<num_vectors; vnum++) {
				float ip=0;
				if (d->m_weights.width()<=1) ip=inner_product(N,&working_vectors[vnum*N],component_vector);
				else ip=inner_product_with_weights(N,&working_vectors[vnum*N],component_vector,d->m_weights.ptrX(0));
				for (int j=0; j<N; j++) {
					hold[j]+=ip*working_vectors[vnum*N+j];
				}
			}
			normalize(N,hold);
			for (int j=0; j<N; j++) component_vector[j]=hold[j];
			free(hold);
		}
		//Compute coefficients
		for (long vnum=0; vnum<num_vectors; vnum++) {
			float ip0=0;
			if (d->m_weights.width()<=1) ip0=inner_product(N,&working_vectors[vnum*N],&working_components[current_component*N]);
			else ip0=inner_product_with_weights(N,&working_vectors[vnum*N],&working_components[current_component*N],d->m_weights.ptrX(0));
			coeffs[vnum+num_vectors*current_component]=ip0;
		}
		//Compute energy (lambda)
		double val=0;
		for (int vnum=0; vnum<num_vectors; vnum++) {
			val+=coeffs[vnum+num_vectors*current_component]*coeffs[vnum+num_vectors*current_component];
		}
		energies[current_component]=val;
		//Subtract this component from the working vectors
		for (int vnum=0; vnum<num_vectors; vnum++) {
			if (d->m_weights.width()<=1) subtract_component(N,&working_vectors[N*vnum],&working_components[N*current_component]);
			else subtract_component_with_weights(N,&working_vectors[N*vnum],&working_components[N*current_component],d->m_weights.ptrX(0));
		}
	}
	
	//set output
	d->m_coefficients.allocate(num_components,num_vectors);
	for (int vnum=0; vnum<num_vectors; vnum++)
	for (int cnum=0; cnum<num_components; cnum++) {
		d->m_coefficients.setValue(coeffs[vnum+num_vectors*cnum],cnum,vnum);
	}
	d->m_components.allocate(N,num_components);
	for (int cnum=0; cnum<num_components; cnum++) 
	for (int j=0; j<N; j++) {
		d->m_components.setValue(working_components[j+N*cnum],j,cnum);
	}
	d->m_energies.clear();
	for (int cnum=0; cnum<num_components; cnum++) {
		d->m_energies << energies[cnum];
	}
	
	//free working components, working vectors, coefficients, and energies
	free(working_components);
	free(working_vectors);
	free(coeffs);
	free(energies);
	
	return true;
}
const Array2D &PCASolver::components() const {
	return d->m_components;
}
const Array2D &PCASolver::coefficients() const {
	return d->m_coefficients;
}
void PCASolver::setNumIterations(int val) {
	d->m_num_iterations=val;
}
QList<double> PCASolver::energies() const {
	return d->m_energies;
}
void PCASolver::setWeights(const QList<float> &W) {
	d->m_weights.allocate(W.count(),1);
	d->m_weights.setDataX(W,0);
}
