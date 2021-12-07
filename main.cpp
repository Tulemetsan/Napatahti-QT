#include <QApplication>
#include <QScreen>
#include "static.h"

int main(int argc, char* argv[])
{
    QApplication app (argc, argv);

    napatahti::staticInit(app);

    auto win (new napatahti::MainWindow);
    auto mgn (napatahti::SharedGui::getMargins());
    auto geo (app.screens().at(0)->geometry());

    win->setGeometry(
        mgn.left(), mgn.top(),
        geo.width() - mgn.left() - mgn.right(),
        geo.height() - mgn.top() - mgn.bottom() - 2
    );
    win->show();

    return app.exec();
}
