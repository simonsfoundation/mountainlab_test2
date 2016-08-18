#ifndef MCVIEWFACTORIES_H
#define MCVIEWFACTORIES_H

#include <mvabstractviewfactory.h>

class MVClusterDetails2Factory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    MVClusterDetails2Factory(MVContext* context, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(QWidget* parent) Q_DECL_OVERRIDE;
private slots:
    //void openClipsForTemplate();
};

#endif // MCVIEWFACTORIES_H

