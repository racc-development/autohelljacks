#include "mainwindow.h"

#include <QApplication>
#include <QMainWindow>
#include<iostream>
#include<thread>
#include<chrono>
#include<Windows.h>
#include <stdio.h>
#include <tchar.h>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    w.show();
    return a.exec();
}

