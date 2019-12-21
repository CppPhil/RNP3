#include "rnp3.h"
#include "Logger.h"
#include "functions.h"

namespace gui {
    RNP3::RNP3(qint16 port, QWidget *parent)
        : QMainWindow{ parent }, client_{ "localhost", port } { // TODO: make the host changeable.
    
        LOG_SCOPE;
        ui.setupUi(this);
        connect(&client_, SIGNAL(printMsgSignal(QString)),
                this, SLOT(drawResponse(QString)));

        utils::SendMsgUsrMessage msg{ 5, utils::MessageType::sendMsgUsr, 22, 1, 0, 0, 0, 0, "Hallo" };
        auto bA = msg.toByteArray(); // TODO: this shit is big endian for no reason; fuck you!!!!
        client_.writeToSocket(bA); // TODO: move this to gui
    }

    void RNP3::drawResponse(QString dataToDisplay) const {
        LOG_SCOPE;
        
        ui.textBrowser->append(dataToDisplay);
    }

} // END of namespace gui
