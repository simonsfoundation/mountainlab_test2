/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
*******************************************************/

#ifndef MVCLIPSVIEW_H
#define MVCLIPSVIEW_H

#include "mvutils.h"
#include "mvviewagent.h"

#include <QWidget>

/** \class MVClipsView
 *  \brief View a set of clips. Usually each clip contains a single spike.
 */

class MVClipsViewPrivate;
class MVClipsView : public QWidget {
    Q_OBJECT
public:
    friend class MVClipsViewPrivate;
    MVClipsView(MVViewAgent* view_agent);
    virtual ~MVClipsView();

    void setClips(const DiskReadMda& clips);
/// TODO: (MEDIUM) in mvclipsview implement times/labels for purpose of current event and labeling
//void setTimes(const QList<double>& times);
//void setLabels(const QList<int>& labels);
signals:
private
slots:

private:
    MVClipsViewPrivate* d;
};

#endif // MVCLIPSVIEW_H
