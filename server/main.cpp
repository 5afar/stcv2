#include <QCoreApplication>
#include <QDebug>

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    qInfo() << "users_server";
    return 0;
}