#include "rnp3.h"
#include <QtWidgets/QApplication>
#include "server.h"
#include "Logger.h"

int main(int argc, char *argv[]) {
    SET_LOG_LEVEL_DEBUG;
    LOG_SCOPE;
    static auto constexpr port = static_cast<qint16>(31337);
    QApplication application{ argc, argv };
    app::Server server{ port };
    server.activateServer();
    gui::RNP3 mainWindow{ port };
    mainWindow.show();    
    return application.exec();
} // END of main
