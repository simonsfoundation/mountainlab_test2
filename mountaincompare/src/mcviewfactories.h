#ifndef MCVIEWFACTORIES_H
#define MCVIEWFACTORIES_H

#include "mccontext.h"

#include <mvabstractviewfactory.h>

class ClusterDetail2Factory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    ClusterDetail2Factory(QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVContext* context) Q_DECL_OVERRIDE;
private slots:
    //void openClipsForTemplate();
};

class Synchronizer1 : public QObject {
    Q_OBJECT
public:
    Synchronizer1(MCContext* C, MVContext* C_new);
private slots:
    void sync_new_to_old();
    void sync_old_to_new();

private:
    MCContext* m_C;
    MVContext* m_C_new;
};

class CompareClustersFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    CompareClustersFactory(QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVContext* context) Q_DECL_OVERRIDE;
private slots:
    //void openClipsForTemplate();
};

#endif // MCVIEWFACTORIES_H
