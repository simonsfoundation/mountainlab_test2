#ifndef CLUSTERANNOTATIONGUIDE_H
#define CLUSTERANNOTATIONGUIDE_H

#include <QWizard>
#include "mvcontext.h"
#include "mvmainwindow.h"

class ClusterAnnotationGuidePrivate;
class ClusterAnnotationGuide : public QWizard {
    Q_OBJECT
public:
    friend class ClusterAnnotationGuidePrivate;
    ClusterAnnotationGuide(MVContext* mvcontext, MVMainWindow* mw);
    virtual ~ClusterAnnotationGuide();
signals:
private slots:
    void slot_button_clicked();

private:
    ClusterAnnotationGuidePrivate* d;
};

#endif // CLUSTERANNOTATIONGUIDE_H
