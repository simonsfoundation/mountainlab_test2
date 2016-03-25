/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/25/2016
*******************************************************/

#include "adjust_times.h"
#include "diskreadmda.h"
#include "extract_clips.h"
#include <QDebug>
#include <math.h>
#include "get_pca_features.h"

QList<double> adjust_times_2(Mda &clips,const QList<double> &times,const adjust_times_opts &opts);
double get_best_offset(const QList<double> &vals,const adjust_times_opts &opts,Mda &upsampling_kernel);
Mda compute_upsampling_kernel(const adjust_times_opts &opts);
QList<double> upsample(const QList<double> &vals,int upsampling_factor,Mda &kernel);
Mda pca_denoise(Mda &clips,int num_features,int jiggle);

bool adjust_times(const QString &timeseries_path, const QString &detect_path, const QString &detect_out_path, const adjust_times_opts &opts)
{
    int clip_size=20;
    DiskReadMda TS(timeseries_path);
    Mda Detect; Detect.read(detect_path);
    QList<double> times_out;
    for (long i=0; i<Detect.N2(); i++) times_out << 0;
    int max_channel=0;
    for (long i=0; i<Detect.N2(); i++) {
        int ch=(int)Detect.value(0,i);
        if (ch>max_channel) max_channel=ch;
    }
    int M=max_channel;
    for (int m=0; m<=M; m++) {
        QList<long> inds;
        QList<double> times;
        for (long i=0; i<Detect.N2(); i++) {
            int ch=(int)Detect.value(0,i);
            if (ch==m) {
                inds << i;
                times << Detect.value(1,i)-1; //convert to 0-based indexing
            }
        }
        if (inds.count()>0) {
            Mda clips;
            if (m==0) {
                clips=extract_clips(TS,times,clip_size);
            }
            else {
                QList<int> channels; channels << m-1; //convert to 1-based indexing
                clips=extract_clips(TS,times,channels,clip_size);
            }
            clips.write32("/tmp/clips_before.mda");
            if (opts.num_pca_denoise_components) {
                clips=pca_denoise(clips,opts.num_pca_denoise_components,opts.pca_denoise_jiggle);
            }
            clips.write32("/tmp/clips_after.mda");
            QList<double> times2=adjust_times_2(clips,times,opts);
            for (long i=0; i<inds.count(); i++) {
                times_out[inds[i]]=times2[i];
            }
        }
    }
    Mda Detect_out=Detect;
    for (long i=0; i<Detect.N2(); i++) {
        Detect_out.setValue(times_out[i]+1,1,i); //convert back to 1-based indexing
    }
    Detect_out.write64(detect_out_path);
    return true;
}

Mda pca_denoise(Mda &clips,int num_features,int jiggle) {
    Mda ret_jiggled(clips.N1(),clips.N2(),clips.N3());
    Mda ret(clips.N1(),clips.N2(),clips.N3());
    Mda clips_jiggled(clips.N1(),clips.N2(),clips.N3());
    for (int i=0; i<clips.N3(); i++) {
        int jiggle_offset=(i%(2*jiggle+1))-jiggle;
        for (int t=0; t<clips.N2(); t++) {
            for (int m=0; m<clips.N1(); m++) {
                clips_jiggled.setValue(clips.value(m,t+jiggle_offset,i),m,t,i);
            }
        }
    }
    pca_denoise(clips.N1()*clips.N2(),clips.N3(),num_features,ret_jiggled.dataPtr(),clips_jiggled.dataPtr());
    for (int i=0; i<clips.N3(); i++) {
        int jiggle_offset=(i%(2*jiggle+1))-jiggle;
        for (int t=0; t<clips.N2(); t++) {
            for (int m=0; m<clips.N1(); m++) {
                ret.setValue(ret_jiggled.value(m,t-jiggle_offset,i),m,t,i);
            }
        }
    }
    return ret;
}

QList<double> adjust_times_2(Mda &clips,const QList<double> &times,const adjust_times_opts &opts) {
    int M=clips.N1();
    int T=clips.N2();
    long L=clips.N3();
    int Tmid=(int)((T+1)/2)-1;
    QList<double> times_out=times;
    if (times.count()!=L) {
        qWarning() << "Unexpected problem in" << __FUNCTION__;
        return times_out;
    }
    Mda upsampling_kernel=compute_upsampling_kernel(opts);
    for (long i=0; i<L; i++) {
        int best_chan=0;
        double best_chan_val=0;
        for (int m=0; m<M; m++) {
            double val=clips.value(m,Tmid,i);
            if (opts.sign==0) val=fabs(val);
            else val*=opts.sign;
            if (val>best_chan_val) {
                best_chan_val=val;
                best_chan=m;
            }
        }
        QList<double> vals;
        for (int t=0; t<T; t++) {
            vals << clips.value(best_chan,t,i);
        }
        double t_offset=get_best_offset(vals,opts,upsampling_kernel);
        times_out[i]+=t_offset;
    }
    return times_out;
}

double get_best_offset(const QList<double> &vals,const adjust_times_opts &opts,Mda &upsampling_kernel) {
    int T=vals.count();
    int Tmid=(int)((T+1)/2)-1;
    QList<double> vals2=upsample(vals,opts.upsampling_factor,upsampling_kernel);
    int Tmid2=Tmid*opts.upsampling_factor;
    double best_val=0;
    int best_dt=0;
    for (int dt=-opts.upsampling_factor*5; dt<=opts.upsampling_factor*5; dt++) {
        double val=vals2[Tmid2+dt];
        if (opts.sign==0) val=fabs(val);
        else val*=opts.sign;
        if (val>best_val) {
            best_val=val;
            best_dt=dt;
        }
    }
    return best_dt*1.0/opts.upsampling_factor;
}

double evaluate_kernel(double t,int Tf) {
    //from ahb
    //sin(pi*t)./(pi*t) .* cos((pi/2/pars.Tf)*t).^2
    if (t==0) return 1;
    if (t>=Tf) return 0;
    if (t<=-Tf) return 0;
    double cos_term=cos((M_PI/2)/Tf*t);
    return sin(M_PI*t)/(M_PI*t)*cos_term*cos_term;
}

Mda compute_upsampling_kernel(const adjust_times_opts &opts) {
    int Tf=5;
    Mda ret(opts.upsampling_factor,2*Tf+1);
    for (int i=0; i<opts.upsampling_factor; i++) {
        for (int j=-Tf; j<=Tf; j++) {
            double val=evaluate_kernel(-j+i*1.0/opts.upsampling_factor,Tf);
            ret.setValue(val,i,j+Tf);
        }
    }
    return ret;
}

QList<double> upsample(const QList<double> &vals,int upsampling_factor,Mda &kernel) {
    int Tf=(kernel.N2()-1)/2;
    QList<double> ret;
    for (int j=0; j<vals.count(); j++) {
        for (int i=0; i<upsampling_factor; i++) {
            double tmp=0;
            for (int k=-Tf; k<=Tf; k++) {
                tmp+=vals.value(j+k)*kernel.value(i,k+Tf);
            }
            ret << tmp;
        }
    }
    return ret;
}
