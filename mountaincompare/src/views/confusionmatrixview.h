/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 7/29/2016
*******************************************************/

#ifndef CONFUSIONMATRIXVIEW_H
#define CONFUSIONMATRIXVIEW_H

#include "mvabstractview.h"

#include <mccontext.h>
#include <mvabstractviewfactory.h>

class ConfusionMatrixViewPrivate;
class ConfusionMatrixView : public MVAbstractView {
    Q_OBJECT
public:
    friend class ConfusionMatrixViewPrivate;
    ConfusionMatrixView(MVContext* mvcontext);
    virtual ~ConfusionMatrixView();

    void prepareCalculation() Q_DECL_OVERRIDE;
    void runCalculation() Q_DECL_OVERRIDE;
    void onCalculationFinished() Q_DECL_OVERRIDE;

    MCContext *mcContext();

protected:
    void keyPressEvent(QKeyEvent* evt) Q_DECL_OVERRIDE;
    void prepareMimeData(QMimeData& mimeData, const QPoint& pos) Q_DECL_OVERRIDE;

private slots:

private:
    ConfusionMatrixViewPrivate* d;
};

class ConfusionMatrixViewFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    ConfusionMatrixViewFactory(MVContext* context, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(QWidget* parent) Q_DECL_OVERRIDE;
private slots:
    //void openClipsForTemplate();
};

#endif // CONFUSIONMATRIXVIEW_H
