#ifndef CLUSTERANNOTATIONGUIDE_H
#define CLUSTERANNOTATIONGUIDE_H

#include <QWizard>
#include "mvmainwindow.h"
#include "mvviewagent.h"

class ClusterAnnotationGuidePrivate;
class ClusterAnnotationGuide : public QWizard {
    Q_OBJECT
public:
    friend class ClusterAnnotationGuidePrivate;
    ClusterAnnotationGuide(MVContext* mvcontext, MVMainWindow* X);
    virtual ~ClusterAnnotationGuide();
private
slots:
    void slot_user_action_button_clicked();
    void slot_instructions_button_clicked();

private:
    ClusterAnnotationGuidePrivate* d;
};

#endif // CLUSTERANNOTATIONGUIDE_H
