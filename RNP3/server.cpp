#include "server.h"
#include "Other.h"
#include "Logger.h"

namespace app {
    Server::Server(qint16 port, QObject *parent)
        : QTcpServer{ parent }, port_{ port } {
        LOG_SCOPE;
    }

    qint16 Server::getPort() const {
        LOG_SCOPE;
        return port_;
    }

    void Server::activateServer() {
        listen(QHostAddress::Any, port_);
    }

    void Server::incomingConnection(qintptr socketDescriptor) {
        LOG_SCOPE;
        clientManagers_.push_back(std::make_unique<ClientManager>(socketDescriptor));
        connect(clientManagers_.back().get(), SIGNAL(gotDataSignal(utils::Message *)),
                this, SLOT(receiveData(utils::Message *)), Qt::QueuedConnection);
    }

    void Server::receiveData(utils::Message *message) const {
        LOG_SCOPE;
        auto clientManager = qobject_cast<ClientManager *>(sender());
        if (clientManager == nullptr) {
            LOG_ERROR << "downcast in Server::receiveData failed\n";
            return;
        }

        auto finalAction = utils::finally([message] {
                                              delete message;
                                          });

        std::type_index typeIndex{ typeid(*message) };
        auto it = utils::rttiTable.find(typeIndex);
        if (it == std::end(utils::rttiTable)) {
            throw std::logic_error{ "typeIndex not found in rttiTable in IncomingMessageHandler::readyReadSlot" };

        }
        auto messageType = it->second;
        switch (messageType) { // TODO: respond to messages
            case utils::MessageType::reqFindServer : {
                auto p = static_cast<utils::ReqFindServerMessage *>(message);
                break;
            }
            case utils::MessageType::resFindServer : {
                auto p = static_cast<utils::ResFindServerMessage *>(message);
                break;
            }
            case utils::MessageType::reqLogin : {
                auto p = static_cast<utils::ReqLoginMessage *>(message);
                break;
            }
            case utils::MessageType::updateClientList : {
                auto p = static_cast<utils::UpdateClientListMessage *>(message);
                break;
            }
            case utils::MessageType::sendMsgGrp : {
                auto p = static_cast<utils::SendMsgGrpMessage *>(message);
                break;
            }
            case utils::MessageType::sendMsgUsr : {
                auto p = static_cast<utils::SendMsgUsrMessage *>(message);
                clientManager->writeToSocket(p->toByteArray());
                break;
            }
            case utils::MessageType::reqHeartbeat : {
                auto p = static_cast<utils::ReqHeartbeatMessage *>(message);
                break;
            }
            case utils::MessageType::resHeartbeat : {
                auto p = static_cast<utils::ResHeartbeatMessage *>(message);
                break;
            }
            case utils::MessageType::errorMsgNotDelivered : {
                auto p = static_cast<utils::ErrorMsgNotDeliveredMessage *>(message);
                break;
            }
            default : throw std::logic_error{ "unrecognized MessageType in IncomingMessageHandler::readyReadSlot" };
        } // END switch (messageType)
        
    } // END void Server::receiveData(utils::Message *message)

} // END of namespace app
