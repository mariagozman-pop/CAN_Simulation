#include "CANSim.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CANSim w;
    w.show();
    return a.exec();
}
