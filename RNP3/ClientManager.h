#pragma once
#include <QObject>
#include <future>
#include <QHostAddress>
#include <QDataStream>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <QByteArray>
#include "Types.h"

namespace app {
    class ClientManager final : public QObject {
        Q_OBJECT
    public:
        using this_type = ClientManager;
        using Base = QObject;
        using Mutex = std::mutex;
        using Lock = std::unique_lock<Mutex>;
        struct ClientInfo final {
            QHostAddress clientAddress;
            qint16 clientPort;
            QHostAddress localAddress;
            qint16 localPort;
        }; // END of struct ClientInfo

        explicit ClientManager(qintptr socketDescriptor, QObject *parent = nullptr);
        ~ClientManager();
        ClientInfo getClientInfo() const;
        void writeToSocket(QByteArray);

    signals:
        void gotDataSignal(utils::Message *);

    private:
        void clientManagerThreadFunction(Mutex &initMutex, std::condition_variable &initCv,
            std::atomic_bool &isInitlialized,
            ClientInfo &ci, QDataStream &dataStream,
            qintptr socketDescriptor);

        std::future<void> workerThread_;
        ClientInfo clientInfo_;
        QDataStream qDataStream_;
        Mutex initMutex_;
        std::atomic_bool isInitialized_;
        std::condition_variable initDoneCv_;   
        std::atomic_bool isThreadRunning_;
    }; // END of class ClientManager
} // END of namespace app
