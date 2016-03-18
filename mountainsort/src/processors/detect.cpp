#include "detect.h"

#include <QTime>
#include <math.h>
#include "diskreadmda.h"
#include "msprefs.h"
#include "msmisc.h"

QList<double> do_detect(const QList<double> &vals,int detect_interval,double detect_threshold,int sign);

bool detect(const QString &signal_path,const QString &detect_path,const Detect_Opts &opts) {
	DiskReadMda X(signal_path);
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
				if ((timer.elapsed()>1000)||(num_timepoints_handled==N)) {
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

QList<double> do_detect(const QList<double> &vals,int detect_interval,double detect_threshold,int sign) {
	int N=vals.count();
	QList<int> to_use;
	for (int n=0; n<N; n++) to_use << 0;
	int last_best_ind=0;
	double last_best_val=0;
	for (int n=0; n<N; n++) {
		double val=vals[n];
		if (sign==0) val=fabs(val);
		else if (sign<0) val=-val;
		if (n-last_best_ind>detect_interval) last_best_val=0;
		if (val>=detect_threshold) {
			if (last_best_val>0) {
				if (val>last_best_val) {
					to_use[n]=1;
					to_use[last_best_ind]=0;
					last_best_ind=n;
					last_best_val=val;
				}
			}
			else {
				to_use[n]=1;
				last_best_ind=n;
				last_best_val=val;
			}
		}
	}
	QList<double> times;
	for (int n=0; n<N; n++) {
		if (to_use[n]) {
			times << n;
		}
	}
	return times;
}
