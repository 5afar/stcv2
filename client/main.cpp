#include <QApplication>

#include "client/mainwindow.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("users-client"));
    QApplication::setApplicationVersion(QStringLiteral("1.0"));

    MainWindow w;
    w.show();
    return app.exec();
}