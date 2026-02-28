#include <QApplication>
#include "MainWindow.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("VantaPM");
    app.setOrganizationName("VantaPM");

    MainWindow window;
    window.show();

    return app.exec();
}