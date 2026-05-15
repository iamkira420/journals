/**
 * @file main.cpp
 * @brief Entry point for the Journal Article Database application.
 * @author BK201
 *
 * Initialises the Qt application, creates and shows the main window.
 */

#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set a clean application-wide style
    app.setStyle("Fusion");

    MainWindow w;
    w.setWindowTitle("Journal Article Database");
    w.resize(1200, 650);
    w.show();

    return app.exec();
}