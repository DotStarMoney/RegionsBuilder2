#include "mainwin.h"
#include <QApplication>
#include <QPixmap>
#include <QSplashScreen>
#include <QFile>
#include <QDebug>
#include <QTime>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QPixmap splashLogo(":/gfx/gfx/regionsLogo.png");
    QSplashScreen splash(splashLogo);

    splash.show();
    //splash.showMessage("Loading...");

    QFile file(":/qdarkstyle/style.qss");

    mainwin w;
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());

    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        a.setStyleSheet(file.readAll());
        file.close();
    }
    else{
        qDebug() << file.errorString();
        return 1;
    }

    w.show();
    splash.finish(&w);
    return a.exec();
}
