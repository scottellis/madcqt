/*
 * Copyright (C) 2012, Pansenti, LLC
 *
 */

#include <QtGui/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    
#ifdef Q_WS_QWS
    w.showFullScreen();
#endif

    return a.exec();
}
