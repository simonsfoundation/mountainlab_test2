/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/24/2016
*******************************************************/

#include "detect3.h"
#include "detect.h"

#include <QTime>
#include <math.h>
#include "diskreadmda.h"
#include "msprefs.h"
#include "msmisc.h"
#include <math.h>

QList<double> do_detect3(const QList<double> &vals,int detect_interval,double detect_threshold,int sign);
void adjust_detect_times(const QList<double> &vals,QList<double> &times,int beta);

bool detect3(const QString &timeseries_path,const QString &detect_path,const Detect3_Opts &opts) {
	DiskReadMda X(timeseries_path);
	long M=X.N1();
	long N=X.N2();

	long chunk_size=PROCESSING_CHUNK_SIZE;
	long overlap_size=PROCESSING_CHUNK_OVERLAP_SIZE;
	if (N<PROCESSING_CHUNK_SIZE) {chunk_size=N; overlap_size=0;}

	QList<int> channels;
	QList<double> times;
	int Tmid=(int)((opts.clip_size+1)/2)-1;
	{

		QTime timer; timer.start();
		long num_timepoints_handled=0;
		#pragma omp parallel for
		for (long timepoint=0; timepoint<N; timepoint+=chunk_size) {
			Mda chunk;
			#pragma omp critical (lock1)
			{
				X.readChunk(chunk,0,timepoint-overlap_size,M,chunk_size+2*overlap_size);
			}

			QList<double> times1;
			QList<int> channels1;
			for (int m=0; m<M; m++) {
				QList<double> vals;
				for (int j=0; j<chunk.N2(); j++) {
					vals << chunk.value(m,j);
				}
				QList<double> times0=do_detect(vals,opts.detect_interval,opts.detect_threshold,opts.sign);
				adjust_detect_times(vals,times0,opts.beta);

				for (int i=0; i<times0.count(); i++) {
					double time0=times0[i]+timepoint-overlap_size;
					if ((time0>=timepoint)&&(time0<timepoint+chunk_size)) {
						if ((time0>=Tmid)&&(time0+Tmid<N)) {
							times1 << time0+1;
							channels1 << m+1;
						}
					}
				}
			}
			#pragma omp critical (lock2)
			{
				times.append(times1);
				channels.append(channels1);
				num_timepoints_handled+=qMin(chunk_size,N-timepoint);
                                if ((timer.elapsed()>5000)||(num_timepoints_handled==N)) {
					printf("%ld/%ld (%d%%)\n",num_timepoints_handled,N,(int)(num_timepoints_handled*1.0/N*100));
					timer.restart();
				}
			}
		}
	}

	Mda output(2,times.count());
	for (int i=0; i<times.count(); i++) {
		output.set(channels[i],0,i);
		output.set(times[i],1,i);
	}
	output.write64(detect_path);

	return true;
}

double eval_kernel(double t,int Tf) {
	//from ahb
	//sin(pi*t)./(pi*t) .* cos((pi/2/pars.Tf)*t).^2
	if (t==0) return 1;
	if (t>=Tf) return 0;
	if (t<=-Tf) return 0;
	double cos_term=cos((M_PI/2)/Tf*t);
	return sin(M_PI*t)/(M_PI*t)*cos_term*cos_term;
}

void adjust_detect_times(const QList<double> &vals,QList<double> &times,int beta) {
	int Tf=5;
	Mda kernel(beta,Tf*2+1);
	for (int t=-Tf; t<=Tf; t++) {
		for (int b=0; b<beta; b++) {
			double diff=t-b*1.0/beta;
			kernel.setValue(eval_kernel(diff,Tf),b,t+Tf);
		}
	}
	for (long i=0; i<times.count(); i++) {
		long t0=(long)times[i];
		if ((t0-Tf-5>=0)&&(t0+Tf+5<vals.count())) {
			double bestval=0;
			double best_offset=0;
			for (int dt=0; dt<=0; dt++) {
				for (int db=0; db<beta; db++) {
					double offset0=dt+db*1.0/beta;
					double val0=0;
					for (int j=-Tf; j<=Tf; j++) {
						val0+=vals[t0+dt+j]*kernel.value(db,j+Tf);
					}
					if (val0>bestval) {
						bestval=val0;
						best_offset=offset0;
					}
				}
			}
			printf("best offset is %g, t0=%ld\n",best_offset,t0);
			times[i]+=best_offset;
		}
		else {
			printf("No offset\n");
		}
	}
}
