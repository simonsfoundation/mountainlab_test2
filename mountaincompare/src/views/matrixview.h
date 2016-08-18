#ifndef MATRIXVIEW_H
#define MATRIXVIEW_H

#include <QWidget>
#include <mda.h>

class MatrixViewPrivate;
class MatrixView : public QWidget
{
public:

    enum Mode {
        PercentMode, CountsMode
    };

    friend class MatrixViewPrivate;
    MatrixView();
    virtual ~MatrixView();
    void setMode(Mode mode);
    void setMatrix(const Mda &A);
    void setValueRange(double minval,double maxval);

protected:
    void paintEvent(QPaintEvent *evt);
private:
    MatrixViewPrivate *d;
};

#endif // MATRIXVIEW_H

