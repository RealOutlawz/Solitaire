#include "SolitaireWindow.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    SolitaireWindow window;
    window.show();
    return app.exec();
}
