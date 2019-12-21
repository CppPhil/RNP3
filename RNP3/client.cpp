#include "client.hpp"
#include <QTextEdit>
#include "Types.h"
#include "Logger.h"
#include <functional>
#include <utility>
#include "functions.h"
#include "Types.h"
#include "Other.h"

namespace app {
    Client::Client(QString hostToConnectTo, qint16 port, QObject *parent)
        : Base{ parent },
          port_{ port },
          hostToConnectTo_{ std::move(hostToConnectTo) } {

        LOG_SCOPE;
        connect(this, SIGNAL(gotResponseSignal(utils::Message *)),
                this, SLOT(responseSlot(utils::Message *)), Qt::QueuedConnection);

        auto f = std::bind(&this_type::clientThreadFunction, this);
        workerThread_ = std::async(std::launch::async, std::move(f)); {
            Lock lock{ initMutex_ };
            initDoneCv_.wait(lock, [this]() -> std::atomic_bool& {
                                 return isInitialized_;
                             });
        }

    } // END Client::Client

    Client::~Client() {
        LOG_SCOPE;
        isThreadRunning_ = false;
        workerThread_.get();
    }

    void Client::writeToSocket(QByteArray data) {
        qDataStream_.writeBytes(data.constData(), data.size());
    }

    void Client::responseSlot(utils::Message *msg) {
        auto clientManager = qobject_cast<Client *>(sender());
        if (clientManager == nullptr) {
            LOG_ERROR << "downcast in Client::responseSlot failed\n";
            return;
        }

        auto finalAction = utils::finally([msg] {
                                              delete msg;
                                          });

        std::type_index typeIndex{ typeid(*msg) };
        auto it = utils::rttiTable.find(typeIndex);
        if (it == std::end(utils::rttiTable)) {
            throw std::logic_error{ "typeIndex not found in rttiTable in IncomingMessageHandler::readyReadSlot" };

        } // TODO: make it so that you can wirte to this socket
        auto messageType = it->second;
        switch (messageType) { // TODO: respond to messages, emit signal with new string to display chat msgs
            case utils::MessageType::reqFindServer : {
                auto p = static_cast<utils::ReqFindServerMessage *>(msg);
                break;
            }
            case utils::MessageType::resFindServer : {
                auto p = static_cast<utils::ResFindServerMessage *>(msg);
                break;
            }
            case utils::MessageType::reqLogin : {
                auto p = static_cast<utils::ReqLoginMessage *>(msg);
                break;
            }
            case utils::MessageType::updateClientList : {
                auto p = static_cast<utils::UpdateClientListMessage *>(msg);
                break;
            }
            case utils::MessageType::sendMsgGrp : {
                auto p = static_cast<utils::SendMsgGrpMessage *>(msg);
                break;
            }
            case utils::MessageType::sendMsgUsr : {
                auto p = static_cast<utils::SendMsgUsrMessage *>(msg);
                emit printMsgSignal(QString::fromStdString(p->getMessageText())); //TODO: add username
                break;
            }
            case utils::MessageType::reqHeartbeat : {
                auto p = static_cast<utils::ReqHeartbeatMessage *>(msg);
                break;
            }
            case utils::MessageType::resHeartbeat : {
                auto p = static_cast<utils::ResHeartbeatMessage *>(msg);
                break;
            }
            case utils::MessageType::errorMsgNotDelivered : {
                auto p = static_cast<utils::ErrorMsgNotDeliveredMessage *>(msg);
                break;
            }
            default : throw std::logic_error{ "unrecognized MessageType in Client::responseSlot" };
        } // END switch (messageType)
    } // END Client::responseSlot

    void Client::clientThreadFunction() {
        LOG_SCOPE;
        QTcpSocket socket{ };
        socket.connectToHost(hostToConnectTo_, port_);
        {
            Lock lock{ initMutex_ };
            qDataStream_.setDevice(&socket);
            isInitialized_ = true;  
            initDoneCv_.notify_all();
        }

        auto r = socket.waitForConnected(3000);
        if (!r) {
            LOG_WARNING << "Client.cpp clientThreadFunction, connecting to socket took too long\n";
            return;
        }

        forever {
            if (!isThreadRunning_) {
                return;
            }
            socket.waitForReadyRead(3000);
            if (socket.bytesAvailable() < 1) {
                continue;
            }
            try {
                auto msg = func::makeMessage(socket);
                emit gotResponseSignal(msg.release());
                // TODO: emit message 
            } catch (std::logic_error const &ex) {
                LOG_DEBUG << "Caught std::logic_error in Client::clientThreadFunction() :\n" << ex.what() << '\n';
            } catch (...) {
                LOG_ERROR << "Caught unknown exception in Client::clientThreadFunction()\n";
            }
        } // END forever
    } // END Client::clientThreadFunction()     
} // END of namespace app
