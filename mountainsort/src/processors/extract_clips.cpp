#include "extract_clips.h"

Mda extract_clips(DiskReadMda &X, const QList<double> &times, int clip_size)
{
	int M=X.N1();
	int N=X.N2();
	int T=clip_size;
	int L=times.count();
	int Tmid=(int)((T+1)/2)-1;
	Mda clips(M,T,L);
	for (int i=0; i<L; i++) {
		int t1=(int)times[i]-Tmid;
		int t2=t1+T-1;
		if ((t1>=0)&&(t2<N)) {
			Mda tmp;
			X.readChunk(tmp,0,t1,M,T);
			for (int t=0; t<T; t++) {
				for (int m=0; m<M; m++) {
					clips.set(tmp.get(m,t),m,t,i);
				}
			}
		}
	}
	return clips;
}


Mda extract_clips(DiskReadMda &X, const QList<double> &times, const QList<int> &channels, int clip_size)
{
	int M=X.N1();
	int N=X.N2();
	int M0=channels.count();
	int T=clip_size;
	int L=times.count();
	int Tmid=(int)((T+1)/2)-1;
	Mda clips(M0,T,L);
	for (int i=0; i<L; i++) {
		int t1=(int)times[i]-Tmid;
		int t2=t1+T-1;
		if ((t1>=0)&&(t2<N)) {
			Mda tmp;
			X.readChunk(tmp,0,t1,M,T);
			for (int t=0; t<T; t++) {
				for (int m0=0; m0<M0; m0++) {
					clips.set(tmp.get(channels[m0],t),m0,t,i);
				}
			}
		}
	}
	return clips;
}
