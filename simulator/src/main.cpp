#include "simulator.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    qDebug("started");
    QApplication a(argc, argv);
    Simulator w;
    w.show();
    return a.exec();
}
