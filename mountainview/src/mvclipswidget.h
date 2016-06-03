/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/4/2016
*******************************************************/

#ifndef MVCLIPSWIDGET_H
#define MVCLIPSWIDGET_H

#include "diskreadmda.h"
#include <QWidget>
#include "mvutils.h"
#include "mvviewagent.h"

class MVClipsWidgetPrivate;
class MVClipsWidget : public QWidget {
    Q_OBJECT
public:
    friend class MVClipsWidgetPrivate;
    MVClipsWidget();
    virtual ~MVClipsWidget();
    void setMLProxyUrl(const QString& url);
    void setTimeseries(DiskReadMda& X);
    void setFirings(DiskReadMda& F);
    void setLabelsToUse(const QList<int>& labels);
    void setClipSize(int clip_size);
    void setViewAgent(MVViewAgent* agent);

    int currentClipIndex();
signals:
    void currentEventChanged();
private
slots:
    void slot_computation_finished();

private:
    MVClipsWidgetPrivate* d;
};

#endif // MVCLIPSWIDGET_H
