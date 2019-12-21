#include <QTcpServer>
#include <vector>
#include <memory>
#include "ClientManager.h"

namespace app {
    class Server final : public QTcpServer {
        Q_OBJECT

    public:
        using this_type = Server;
        using Base = QObject;
        using container_type = std::vector<std::unique_ptr<ClientManager>>;
        
        explicit Server(qint16 port, QObject *parent = nullptr);
        qint16 getPort() const;
        void activateServer();

    public slots:
        void receiveData(utils::Message *) const;

    protected:
        virtual void incomingConnection(qintptr socketDescriptor) override;

    private:
        container_type clientManagers_;
        qint16 port_;
    }; // END of class Server    
} // END of namespace app
