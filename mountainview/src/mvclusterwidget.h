/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef MVCLUSTERWIDGET_H
#define MVCLUSTERWIDGET_H

#include <QWidget>
#include "mvutils.h"
#include "affinetransformation.h"
#include "mvclusterview.h" //for FilterInfo
#include "mvabstractview.h"
#include "mvabstractviewfactory.h"
#include "mvmainwindow.h"

/** \class MVClusterWidget
 *  \brief Presents one or more cluster views and a synchronized clip view
 *
 */

class MVClusterWidgetPrivate;
class MVClusterWidget : public MVAbstractView {
    Q_OBJECT
public:
    friend class MVClusterWidgetPrivate;
    MVClusterWidget(MVViewAgent* view_agent);
    virtual ~MVClusterWidget();

    void prepareCalculation() Q_DECL_OVERRIDE;
    void runCalculation() Q_DECL_OVERRIDE;
    void onCalculationFinished() Q_DECL_OVERRIDE;

    void setLabelsToUse(const QList<int>& labels);
    void setFeatureMode(QString mode); //"pca", "channels"
    void setChannels(QList<int> channels); //relevant for "channels" mode

    void setTransformation(const AffineTransformation& T);

private:
    void setData(const Mda& X);
    void setTimes(const QList<double>& times);
    void setLabels(const QList<int>& labels);
    void setAmplitudes(const QList<double>& amps);
    void setScores(const QList<double>& detectability_scores, const QList<double>& outlier_scores);

signals:
private slots:
    void slot_current_event_changed();
    void slot_view_current_event_changed();
    void slot_view_transformation_changed();
    void slot_view_active_cluster_number_toggled();
    void slot_show_clip_view_toggled(bool val);
    void slot_show_view_toggled(bool val);

private:
    MVClusterWidgetPrivate* d;
};

class MVPCAFeaturesFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    MVPCAFeaturesFactory(QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVViewAgent* agent, QWidget* parent) Q_DECL_OVERRIDE;
public slots:
    void updateEnabled();
};

class MVChannelFeaturesFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    MVChannelFeaturesFactory(QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVViewAgent* agent, QWidget* parent) Q_DECL_OVERRIDE;
public slots:
    void updateEnabled();
};

#endif // MVCLUSTERWIDGET_H
