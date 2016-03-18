#ifndef REMOVE_NOISE_SUBCLUSTERS_H
#define REMOVE_NOISE_SUBCLUSTERS_H

#include <QList>
#include <QString>

struct Remove_noise_subclusters_opts {
	int clip_size;
	double detectability_threshold;
	double shell_increment;
    int min_shell_size;
};

bool remove_noise_subclusters(const QString &signal_path,const QString &firings_path,const QString &firings_out_path,const Remove_noise_subclusters_opts &opts);

struct Shell {
    QList<int> inds;
};
struct Define_Shells_Opts {
    double shell_increment;
    int min_shell_size;
};

QList<Shell> define_shells(const QList<double> &peaks,const Define_Shells_Opts &opts);

#endif // REMOVE_NOISE_SUBCLUSTERS_H
