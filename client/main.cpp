#include <QApplication>
#include <QLabel>

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QLabel label("users_client");
    label.resize(400, 80);
    label.show();
    return app.exec();
}