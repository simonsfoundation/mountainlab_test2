#ifndef MVABSTRACTVIEWFACTORY_H
#define MVABSTRACTVIEWFACTORY_H

#include <QWidget>

class MVViewAgent;
class MVAbstractView;

class MVAbstractViewFactory : public QObject
{
    Q_OBJECT
public:
    explicit MVAbstractViewFactory(QObject *parent = 0);
    bool isEnabled() const;

    virtual int order() const { return 0; }

    virtual QString name() const = 0;
    virtual QString group() const;
    virtual QString toolTip() const;

    virtual MVAbstractView* createView(MVViewAgent *agent, QWidget *parent = 0) = 0;
signals:
    void enabledChanged(bool);
protected:
    void setEnabled(bool e);
private:
    bool m_enabled;
};

#endif // MVABSTRACTVIEWFACTORY_H
