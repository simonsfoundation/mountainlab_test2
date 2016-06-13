/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef MVUTILS_H
#define MVUTILS_H

#include "diskarraymodel_new.h"
#include "diskreadmda.h"
#include <QList>
#include <QColor>
#include <math.h>
#include <QPainter>

///A firing event defined by a time and a label
struct MVEvent {
    MVEvent()
    {
        time = -1;
        label = -1;
    }
    bool operator==(const MVEvent& other) const
    {
        return ((time == other.time) && (label == other.label));
    }

    double time;
    int label;
};

///An epoch or time interval within a timeseries. t_begin and t_end are in timepoint units
struct Epoch {
    QString name;
    double t_begin;
    double t_end;
};
///Read a set of epochs from a text file (special format)
QList<Epoch> read_epochs(const QString& path);

///Utility
Mda compute_mean_waveform(DiskArrayModel_New* C);
///Utility
Mda compute_mean_stdev_waveform(DiskArrayModel_New* C);
///Utility
Mda compute_features(DiskArrayModel_New* C);
///Utility
Mda compute_features(const QList<DiskArrayModel_New*>& C);

QColor get_heat_map_color(double val);

void user_save_image(const QImage& img);

struct draw_axis_opts {
    draw_axis_opts()
    {
        orientation = Qt::Vertical;
        minval = 0;
        maxval = 1;
        tick_length = 5;
        draw_tick_labels = true;
        draw_range = false;
    }

    QPointF pt1, pt2;
    Qt::Orientation orientation;
    double minval, maxval;
    double tick_length;
    bool draw_tick_labels;
    bool draw_range;
};

void draw_axis(QPainter* painter, draw_axis_opts opts);

#endif // MVUTILS_H
