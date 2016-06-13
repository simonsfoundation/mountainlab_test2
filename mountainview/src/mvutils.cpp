#include "mvutils.h"

#include <QCoreApplication>
#include "get_pca_features.h"
#include <math.h>
#include "textfile.h"
#include <QDebug>
#include <QSettings>
#include <QDir>
#include <QFileDialog>
#include <QString>
#include <QImageWriter>
#include <QMessageBox>
#include "imagesavedialog.h"
#include "taskprogress.h"

Mda compute_mean_waveform(DiskArrayModel_New* C)
{
    Q_UNUSED(C)
    Mda ret;
    return ret; //disabled for now.
    /*
	if (!C->dim3()) return ret;
	int M=C->size(0);
	int T=C->size(1)/C->dim3();
	int NC=C->dim3();
	if (!NC) return ret;

	double sum[M*T];
	for (int ii=0; ii<M*T; ii++) sum[ii]=0;
	for (int c=0; c<NC; c++) {
		if ((c%100==0)||(c==NC-1)) {
			qApp->processEvents();
			//int pct=(int)(c*1.0/NC*100);
			//printf("Computing mean waveform...%d/%d (%d%%)\n",c,NC,pct);
		}
		int ii=0;
		Mda tmp=C->loadData(1,T*c,T*(c+1));
		for (int t=0; t<T; t++) {
			for (int m=0; m<M; m++) {
				sum[ii]+=tmp.value(m,t);
				ii++;
			}
		}
	}
	ret.allocate(M,T);
	{
		int ii=0;
		for (int t=0; t<T; t++) {
			for (int m=0; m<M; m++) {
				ret.setValue(sum[ii]/NC,m,t);
				ii++;
			}
		}
	}
	return ret;
    */
}

Mda compute_mean_stdev_waveform(DiskArrayModel_New* C)
{
    Q_UNUSED(C)
    Mda ret;
    return ret; //disabled for now
    /*
	if (!C->dim3()) return ret;
	int M=C->size(0);
	int T=C->size(1)/C->dim3();
	int NC=C->dim3();
	if (!NC) return ret;

	double sum[M*T];
	double sumsqr[M*T];
	for (int ii=0; ii<M*T; ii++) {sum[ii]=0; sumsqr[ii]=0;}
	for (int c=0; c<NC; c++) {
		if ((c%100==0)||(c==NC-1)) {
			qApp->processEvents();
			//int pct=(int)(c*1.0/NC*100);
			//printf("Computing mean waveform...%d/%d (%d%%)\n",c,NC,pct);
		}
		int ii=0;
		Mda tmp=C->loadData(1,T*c,T*(c+1));
		for (int t=0; t<T; t++) {
			for (int m=0; m<M; m++) {
				float val=tmp.value(m,t);
				sum[ii]+=val;
				sumsqr[ii]+=val*val;
				ii++;
			}
		}
	}
	ret.allocate(M,T*2);
	{
		int ii=0;
		for (int t=0; t<T; t++) {
			for (int m=0; m<M; m++) {
				float mu=sum[ii]/NC;
				float sigma=sqrt(sumsqr[ii]/NC-mu*mu);
				float tmp=0;
				if (mu>0) {
					tmp=mu-sigma;
					if (tmp<0) tmp=0;
				}
				else {
					tmp=mu+sigma;
					if (tmp>0) tmp=0;
				}
				ret.setValue(mu,m,t);
				ret.setValue(tmp,m,T+t);
				ii++;
			}
		}
	}
	return ret;
    */
}

Mda compute_features(DiskArrayModel_New* C)
{
    Q_UNUSED(C)
    Mda ret;
    return ret; //disabled for now
    /*
	if (!C->dim3()) return ret;
	int M=C->size(0);
	int T=C->size(1)/C->dim3();
	int NC=C->dim3();
	if (!NC) return ret;

	Mda X=C->loadData(1,0,T*NC);
	ret.allocate(3,NC);
	get_pca_features(M*T,NC,3,ret.dataPtr(),X.dataPtr());

	return ret;
    */
}

Mda compute_features(const QList<DiskArrayModel_New*>& C)
{
    Q_UNUSED(C)
    Mda ret;
    return ret; //disabled for now
    /*
	if (C.isEmpty()) return ret;
	if (!C[0]->dim3()) return ret;
	int M=C[0]->size(0);
	int T=C[0]->size(1)/C[0]->dim3();
	int NC=0;
	for (int i=0; i<C.count(); i++) {
		NC+=C[i]->dim3();
	}
	if (!NC) return ret;

	Mda X; X.allocate(M,T,NC);
	int jj=0;
	for (int i=0; i<C.count(); i++) {
		Mda tmp=C[i]->loadData(1,0,T*C[i]->dim3());
		int ii=0;
		for (int aa=0; aa<T*C[i]->dim3(); aa++) {
			for (int m=0; m<M; m++) {
				X.set(tmp.get(ii),jj);
				ii++;
				jj++;
			}
		}
	}
	ret.allocate(3,NC);
	get_pca_features(M*T,NC,3,ret.dataPtr(),X.dataPtr());

	return ret;
    */
}

QColor get_heat_map_color(double val)
{
    double r = 0, g = 0, b = 0;
    if (val < 0.2) {
        double tmp = (val - 0) / 0.2;
        r = 200 * (1 - tmp) + 150 * tmp;
        b = 200 * (1 - tmp) + 255 * tmp;
        g = 0 * (1 - tmp) + 0 * tmp;
    }
    else if (val < 0.4) {
        double tmp = (val - 0.2) / 0.2;
        r = 150 * (1 - tmp) + 0 * tmp;
        b = 255 * (1 - tmp) + 255 * tmp;
        g = 0 * (1 - tmp) + 100 * tmp;
    }
    else if (val < 0.6) {
        double tmp = (val - 0.4) / 0.2;
        r = 0 * (1 - tmp) + 255 * tmp;
        b = 255 * (1 - tmp) + 0 * tmp;
        g = 100 * (1 - tmp) + 20 * tmp;
    }
    else if (val < 0.8) {
        double tmp = (val - 0.6) / 0.2;
        r = 255 * (1 - tmp) + 255 * tmp;
        b = 0 * (1 - tmp) + 0 * tmp;
        g = 20 * (1 - tmp) + 255 * tmp;
    }
    else if (val <= 1.0) {
        double tmp = (val - 0.8) / 0.2;
        r = 255 * (1 - tmp) + 255 * tmp;
        b = 0 * (1 - tmp) + 255 * tmp;
        g = 255 * (1 - tmp) + 255 * tmp;
    }

    return QColor((int)r, (int)g, (int)b);
}

QList<Epoch> read_epochs(const QString& path)
{
    QList<Epoch> ret;
    QString txt = read_text_file(path);
    QStringList lines = txt.split("\n");
    foreach (QString line, lines) {
        QList<QString> vals = line.split(QRegExp("\\s+"));
        if (vals.value(0) == "EPOCH") {
            if (vals.count() == 4) {
                Epoch E;
                E.name = vals.value(1);
                E.t_begin = vals.value(2).toDouble() - 1;
                E.t_end = vals.value(3).toDouble() - 1;
                ret << E;
            }
            else {
                qWarning() << "Problem parsing epochs file:" << path;
            }
        }
    }
    return ret;
}

void user_save_image(const QImage& img)
{
    ImageSaveDialog::presentImage(img);
}

void draw_axis(QPainter* painter, draw_axis_opts opts)
{
    if (opts.orientation == Qt::Vertical) {
        //ensure that pt2.y>=pt1.y
        if (opts.pt2.y() < opts.pt1.y()) {
            QPointF tmp = opts.pt2;
            opts.pt2 = opts.pt1;
            opts.pt1 = tmp;
        }
    }
    painter->drawLine(opts.pt1, opts.pt2);
    double range = opts.maxval - opts.minval;
    if (opts.maxval <= opts.minval)
        return;
    QList<double> possible_tick_intervals;
    for (double x = 0.00001; x <= 10000; x *= 10) {
        possible_tick_intervals << x << x * 2 << x * 5;
    }

    long best_count = 0;
    double best_interval = 0;
    for (int i = 0; i < possible_tick_intervals.count(); i++) {
        long count = (long)range / possible_tick_intervals[i];
        if (count >= 4) {
            if ((best_interval == 0) || (count < best_count)) {
                best_count = count;
                best_interval = possible_tick_intervals[i];
            }
        }
    }
    double tick_length = opts.tick_length;
    if (best_interval) {
        long ind1 = (long)(opts.minval / best_interval) - 1;
        long ind2 = (long)(opts.maxval / best_interval) + 1;
        for (long ind = ind1; ind <= ind2; ind++) {
            if ((opts.minval <= ind * best_interval) && (ind * best_interval <= opts.maxval)) {
                double pct = (ind * best_interval - opts.minval) / (opts.maxval - opts.minval);
                if (opts.orientation == Qt::Vertical)
                    pct = 1 - pct;
                QPointF ptA = opts.pt1 + pct * (opts.pt2 - opts.pt1);
                QPointF ptB;
                QRectF text_rect;
                int align;
                if (opts.orientation == Qt::Horizontal) {
                    ptB = ptA + QPointF(0, tick_length);
                    text_rect = QRectF(ptB + QPointF(20, 0), QSize(40, 50 - 3));
                    align = Qt::AlignTop | Qt::AlignCenter;
                }
                else { //vertical
                    ptB = ptA + QPointF(-tick_length, 0);
                    text_rect = QRectF(ptB + QPointF(-50, -20), QSize(50 - 3, 40));
                    align = Qt::AlignRight | Qt::AlignVCenter;
                }
                painter->drawLine(ptA, ptB);
                QString text = QString("%1").arg(ind * best_interval);
                if (opts.draw_tick_labels) {
                    painter->drawText(text_rect, align, text);
                }
            }
        }
    }
    if (opts.draw_range) {
        if (opts.orientation == Qt::Horizontal) {
            /// TODO handle horizontal case
        }
        else {
            painter->save();
            painter->rotate(-90);
            QRectF rect(opts.pt1.x() - 50, opts.pt1.y() - 50, 50 - 3, opts.pt2.y() - opts.pt1.y() + 100);
            QTransform transform;
            transform.rotate(90);
            rect = transform.mapRect(rect);
            QString text = QString("%1").arg(range);
            int align = Qt::AlignBottom | Qt::AlignCenter;
            painter->drawText(rect, align, text);
            painter->restore();
        }
    }
}
