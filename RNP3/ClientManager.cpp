#include "ClientManager.h"
#include <functional>
#include <memory>
#include <QTcpSocket>
#include "functions.h"
#include "Logger.h"
#include <utility>

namespace app {
    void ClientManager::clientManagerThreadFunction(Mutex &initMutex, std::condition_variable &initCv,
                                     std::atomic_bool &isInitlialized,
                                     ClientInfo &ci, QDataStream &dataStream,
                                     qintptr socketDescriptor) {
        
        LOG_SCOPE;
        std::unique_ptr<QTcpSocket> pSocket{ nullptr };
        {
            Lock lock{ initMutex };
            pSocket = std::make_unique<QTcpSocket>();
            pSocket->setSocketDescriptor(socketDescriptor);
            ci.clientAddress = pSocket->peerAddress();
            ci.clientPort = pSocket->peerPort();
            ci.localAddress = pSocket->localAddress();
            ci.localPort = pSocket->localPort();
            dataStream.setDevice(pSocket.get());
            isInitlialized = true;
            initCv.notify_all();
        }
        
        forever {
            // read messages from socket
            try {
                if (!isThreadRunning_) {
                    return;
                }
                pSocket->waitForReadyRead(3000);
                if (pSocket->bytesAvailable() == 0) {
                    continue;
                }
                auto msg = func::makeMessage(*pSocket);
                emit gotDataSignal(msg.release());
            } catch (std::logic_error const &ex) {
                LOG_DEBUG << "Caught logic_error in clientManagerThreadFunction:\n" << ex.what() << '\n';
            } catch (...) {
                LOG_ERROR << "Unknown exception caught in clientManagerThreadFunction\n";
            }
        }
    }

    ClientManager::ClientManager(qintptr socketDescriptor, QObject *parent) {
        LOG_SCOPE;
        auto f = std::bind(&this_type::clientManagerThreadFunction, this,
                           std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
                           std::placeholders::_4, std::placeholders::_5, std::placeholders::_6);

        workerThread_ = std::async(std::launch::async, std::move(f),
                                   std::ref(initMutex_), std::ref(initDoneCv_),
                                   std::ref(isInitialized_),
                                   std::ref(clientInfo_), std::ref(qDataStream_),
                                   socketDescriptor);
        {
            Lock lock{ initMutex_ };
            initDoneCv_.wait(lock, [this]() -> std::atomic_bool & {
                return isInitialized_;
            });
        }
        isThreadRunning_ = true;
    }

    ClientManager::~ClientManager() {
        LOG_SCOPE;
        isThreadRunning_ = false;
        workerThread_.get();
    }

    ClientManager::ClientInfo ClientManager::getClientInfo() const {
        LOG_SCOPE;
        return clientInfo_;
    }

    void ClientManager::writeToSocket(QByteArray data) {
        LOG_SCOPE;
        qDataStream_.writeBytes(data.constData(), data.size());
    }

} // END of namespace app
