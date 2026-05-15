/**
 * @brief Entry point for the Journal Article Database application.
 * @author BK201
 * @date 15 May 2026 
 * Initialises the Qt application, creates and shows the main window.
 */

#include <QApplication>
#include "mainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set a clean application-wide style
    app.setStyle("Fusion");

    MainWindow window;
    window.setWindowTitle("Journal Article Database");
    window.resize(1200, 650);
    window.show();

    return app.exec();
}
