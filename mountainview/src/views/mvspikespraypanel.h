#ifndef MVSPIKESPRAYPANEL_H
#define MVSPIKESPRAYPANEL_H

#include <QList>
#include <QWidget>
#include <mvcontext.h>

class MVSpikeSprayPanelPrivate;
class MVSpikeSprayPanel : public QWidget {
public:
    friend class MVSpikeSprayPanelPrivate;
    MVSpikeSprayPanel(MVContext* context);
    virtual ~MVSpikeSprayPanel();
    void setLabelsToUse(const QSet<int>& labels);
    void setClipsToRender(Mda* X);
    void setLabelsToRender(const QVector<int>& X);
    void setAmplitudeFactor(double X);
    void setLegendVisible(bool val);

protected:
    void paintEvent(QPaintEvent* evt);

private:
    MVSpikeSprayPanelPrivate* d;
};

#endif // MVSPIKESPRAYPANEL_H
