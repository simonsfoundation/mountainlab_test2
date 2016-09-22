#include <QApplication>
#include <QDebug>
#include <QWidget>
#include <mlcommon.h>
#include "momainwindow.h"
#include <QDesktopWidget>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    CLParams CLP(argc, argv);

    MOMainWindow W;
    W.read(CLP.unnamed_parameters.value(0, "/tmp/example.mo"));
    W.setMinimumSize(800, 600);
    W.show();
    //QRect G=a.desktop()->geometry();
    //W.setGeometry(G.right()-900,G.bottom()-700,800,500);

    int ret = a.exec();

    return ret;
}
