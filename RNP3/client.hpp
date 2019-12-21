#pragma once
#include <QObject>
#include <QString>
#include <QTcpSocket>
#include <future>
#include <mutex>
#include <condition_variable>
#include <QDataStream>

namespace utils {
    class Message;
}

namespace app {
    class Client : public QObject {
        Q_OBJECT

    public:
        using this_type = Client;
        using Base = QObject;
        using Mutex = std::mutex;
        using Lock = std::unique_lock<Mutex>;

        explicit Client(QString hostToConnectTo, qint16 port, QObject *parent = nullptr);
        ~Client();

        void writeToSocket(QByteArray data);

    private slots:
        void responseSlot(utils::Message *);

    signals:
        void gotResponseSignal(utils::Message *);
        void printMsgSignal(QString);

    private:
        void clientThreadFunction();

        std::future<void> workerThread_;
        QDataStream qDataStream_;
        Mutex initMutex_;
        std::atomic_bool isInitialized_;
        std::condition_variable initDoneCv_;
        std::atomic_bool isThreadRunning_;
        qint16 port_;
        QString hostToConnectTo_;
    }; // END of class Client
} // END of namespace app
