#ifndef MVABSTRACTVIEWFACTORY_H
#define MVABSTRACTVIEWFACTORY_H

#include <QWidget>
#include "mvabstractview.h"

class MVAbstractViewFactory : public QObject {
    Q_OBJECT
public:
    explicit MVAbstractViewFactory(MVContext* context, QObject* parent = 0);
    bool isEnabled() const;

    virtual QString id() const = 0;
    virtual QString name() const = 0;
    virtual QString group() const;
    virtual QString toolTip() const;
    virtual QString title() const; /// TODO: move title to the view itself
    virtual int order() const { return 0; }

    virtual MVAbstractView* createView(QWidget* parent = 0) = 0;
signals:
    void enabledChanged(bool);

protected:
    void setEnabled(bool e);
    MVContext* mvContext();

private:
    bool m_enabled;
    MVContext* m_context;
};

#endif // MVABSTRACTVIEWFACTORY_H
