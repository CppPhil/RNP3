#include "functions.h"
#include <array>
#include <utility>
#include <QByteArray>
#include "Types.h"
#include <thread>
#include <chrono>
#include "Logger.h"

namespace {
    struct CommonHeader final {
        using this_type = CommonHeader;
        utils::Word version;
        utils::MessageType type;
        utils::Word length;
    }; // END of struct CommonHeader

    struct SendMsgStruct final {
        using this_type = SendMsgStruct;
        utils::Word messageId;
        utils::Word sourceIp;
        utils::Word targetIp;
        utils::HalfWord sourcePort;
        utils::HalfWord targetPort;
    }; // END of struct SendMsgStruct

    QByteArray readFromSocket(int bytesToRead, QTcpSocket &socket) {
        LOG_SCOPE;
        LOG_DEBUG << "readFromSocket: " << bytesToRead << " bytesToRead\n";
        if (bytesToRead < 1) {
            throw std::logic_error{ "readFromSocket: bytesToRead was < 1\n" };
        }
        QByteArray ret{ bytesToRead, '\0' };
        auto c = '\0';
        if (socket.bytesAvailable() < bytesToRead) {
            std::this_thread::sleep_for(std::chrono::seconds{ 3 });
        }

        if (socket.bytesAvailable() < bytesToRead) {
            throw std::logic_error{ "client didn't send enough data, after 3 seconds still no data - too bad.\n" };
        }

        for (auto i = 0; i < bytesToRead; ++i) {
            if (!socket.getChar(&c)) {
                LOG_WARNING << "failed to read a byte in readFromSocket\n";
                continue;
            }

            ret[i] = c;
        }
        return ret;
    }

    std::string readString(char const *begin, utils::Word length) {
        LOG_SCOPE;
        std::string str(length + 1, '\0');
        auto end = begin;
        utils::advancePtr(end, length);
        std::copy(begin, end, std::begin(str));
        return str;
    }

    std::unique_ptr<utils::Message> makeReqFindServerMessage(CommonHeader commonHeader, QTcpSocket &/*socket*/) {
        LOG_SCOPE;
        return std::make_unique<utils::ReqFindServerMessage>(commonHeader.version, commonHeader.type, commonHeader.length);
    }

    std::unique_ptr<utils::Message> makeResFindServerMessage(CommonHeader commonHeader, QTcpSocket &/*socket*/) {
        LOG_SCOPE;
        return std::make_unique<utils::ResFindServerMessage>(commonHeader.version, commonHeader.type, commonHeader.length);
    }

    std::unique_ptr<utils::Message> makeReqLoginMessage(CommonHeader commonHeader, QTcpSocket &socket) {
        LOG_SCOPE;
        static auto constexpr maxiMumUserNameLength = 255;
        if (commonHeader.length > maxiMumUserNameLength) {
            throw std::logic_error{ "length was too large in makeReqLoginMessage" };
        }
        
        auto bytes = readFromSocket(commonHeader.length, socket);
        void const *pData = bytes.constData();

        auto username = readString(static_cast<char const *>(pData), commonHeader.length);

        return std::make_unique<utils::ReqLoginMessage>(commonHeader.version, commonHeader.type, commonHeader.length, std::move(username));
    }

    utils::UsernameRecord readUsernameRecord(QTcpSocket &socket) {
        LOG_SCOPE;
        static auto constexpr cbStaticComponent = sizeof(utils::Word) + sizeof(utils::HalfWord) + sizeof(utils::Byte) * 2;

        auto bytes = readFromSocket(cbStaticComponent, socket);
        void const *pData = bytes.constData();

        auto ip = utils::readFromAddress<utils::Word>(pData);
        utils::advancePtr(pData, sizeof(utils::Word));

        auto port = utils::readFromAddress<utils::HalfWord>(pData);
        utils::advancePtr(pData, sizeof(utils::HalfWord));

        auto lengthUserName = utils::readFromAddress<utils::Byte>(pData);
        utils::advancePtr(pData, sizeof(utils::Byte));

        static auto constexpr maxiMumUserNameLength = 255;
        if (lengthUserName > maxiMumUserNameLength) {
            throw std::logic_error{ "lengthUserName was too large in readUsernameRecord" };
        }

        utils::advancePtr(pData, sizeof(utils::Byte)); // skip the reserved part

        auto const additionalBytesToRead = utils::bitAlignment - lengthUserName % utils::bitAlignment;
        auto const totalBytesToRead = lengthUserName + additionalBytesToRead;
        if (totalBytesToRead % utils::bitAlignment != 0) {
            throw std::logic_error{ "totalBytesToRead was not a multiple of 32 in readUsernameRecord" };
        }

        bytes = readFromSocket(totalBytesToRead, socket);
        pData = bytes.constData();

        auto username = readString(static_cast<char const *>(pData), lengthUserName);
        utils::advancePtr(pData, lengthUserName);

        for (; reinterpret_cast<std::ptrdiff_t>(pData) % utils::bitAlignment; utils::advancePtr(pData, sizeof(utils::Byte))) { }

        return utils::UsernameRecord{ ip, port, lengthUserName, std::move(username) };
    }

    std::unique_ptr<utils::Message> makeUpdateClientListMessage(CommonHeader commonHeader, QTcpSocket &socket) {
        LOG_SCOPE;
        std::vector<utils::UsernameRecord> v{ };
        for (auto i = static_cast<utils::Word>(0U); i < commonHeader.length; ++i) {
            v.push_back(readUsernameRecord(socket));
        }
        return std::make_unique<utils::UpdateClientListMessage>(commonHeader.version, commonHeader.type, commonHeader.length, std::move(v));
    }

    SendMsgStruct makeSendMsgStruct(QTcpSocket &socket) {
        LOG_SCOPE;
        auto bytes = readFromSocket(utils::sendMsgStructByteSize, socket);
        void const *pData = bytes.constData();
        auto messageId = utils::readFromAddress<utils::Word>(pData);
        utils::advancePtr(pData, sizeof(utils::Word));
        auto sourceIp = utils::readFromAddress<utils::Word>(pData);
        utils::advancePtr(pData, sizeof(utils::Word));
        auto targetIp = utils::readFromAddress<utils::Word>(pData);
        utils::advancePtr(pData, sizeof(utils::Word));
        auto sourePort = utils::readFromAddress<utils::HalfWord>(pData);
        utils::advancePtr(pData, sizeof(utils::HalfWord));
        auto targetPort = utils::readFromAddress<utils::HalfWord>(pData);
        utils::advancePtr(pData, sizeof(utils::HalfWord));
        return SendMsgStruct{ messageId, sourceIp, targetIp, sourePort, targetPort };
    }

    template <class RunTimeType>
    std::unique_ptr<utils::Message> makeSendMessage(CommonHeader commonHeader, QTcpSocket &socket) {
        LOG_SCOPE;
        auto sendMsgStruct = makeSendMsgStruct(socket);
        auto messageTextStringLength = commonHeader.length - utils::sendMsgStructByteSize;
        auto bytes = readFromSocket(messageTextStringLength, socket);
        void const *pData = bytes.constData();
        auto messageTextString = readString(static_cast<char const *>(pData), messageTextStringLength);
        return std::make_unique<RunTimeType>(commonHeader.version, commonHeader.type, commonHeader.length, sendMsgStruct.messageId, sendMsgStruct.sourceIp, sendMsgStruct.targetIp, sendMsgStruct.sourcePort, sendMsgStruct.targetPort, std::move(messageTextString));
    }

    std::unique_ptr<utils::Message> makeSendMsgGrpMessage(CommonHeader commonHeader, QTcpSocket &socket) {
        LOG_SCOPE;
        return makeSendMessage<utils::SendMsgGrpMessage>(commonHeader, socket);
    }

    std::unique_ptr<utils::Message> makeSendMsgUsrMessage(CommonHeader commonHeader, QTcpSocket &socket) {
        LOG_SCOPE;
        return makeSendMessage<utils::SendMsgUsrMessage>(commonHeader, socket);
    }

    std::unique_ptr<utils::Message> makeReqHeartbeatMessage(CommonHeader commonHeader, QTcpSocket &/*socket*/) {
        LOG_SCOPE;
        return std::make_unique<utils::ReqHeartbeatMessage>(commonHeader.version, commonHeader.type, commonHeader.length);
    }

    std::unique_ptr<utils::Message> makeResHeartbeatMessage(CommonHeader commonHeader, QTcpSocket &/*socket*/) {
        LOG_SCOPE;
        return std::make_unique<utils::ResHeartbeatMessage>(commonHeader.version, commonHeader.type, commonHeader.length);
    }

    std::unique_ptr<utils::Message> makeErrorMsgNotDeliveredMessage(CommonHeader commonHeader, QTcpSocket &socket) {
        LOG_SCOPE;
        auto sendMsgStruct = makeSendMsgStruct(socket);
        return std::make_unique<utils::ErrorMsgNotDeliveredMessage>(commonHeader.version, commonHeader.type, commonHeader.length, sendMsgStruct.messageId, sendMsgStruct.sourceIp, sendMsgStruct.targetIp, sendMsgStruct.sourcePort, sendMsgStruct.targetPort);
    }

} // END of anonymous namespace  

namespace func {
    std::unique_ptr<utils::Message> makeMessage(QTcpSocket &socket) {
        LOG_SCOPE;
        static std::array<std::unique_ptr<utils::Message>(*)(CommonHeader, QTcpSocket &), utils::amtMessageTypes> functions{ &makeReqFindServerMessage, &makeResFindServerMessage,
            &makeReqLoginMessage, &makeUpdateClientListMessage, &makeSendMsgGrpMessage,
            &makeSendMsgUsrMessage, &makeReqHeartbeatMessage, &makeResHeartbeatMessage,
            &makeErrorMsgNotDeliveredMessage
        };

        auto bytes = readFromSocket(utils::commonHeaderByteSize, socket);
        void const *pData = bytes.constData();

        auto version = utils::readFromAddress<utils::Word>(pData);
        utils::advancePtr(pData, sizeof(utils::Word));
        using MessageTypeType = std::underlying_type_t<utils::MessageType>;
        auto type = static_cast<utils::MessageType>(utils::readFromAddress<MessageTypeType>(pData));
        utils::advancePtr(pData, sizeof(MessageTypeType));
        auto length = utils::readFromAddress<utils::Word>(pData);
        utils::advancePtr(pData, sizeof(utils::Word));
        // read common header

        return functions.at(static_cast<std::size_t>(static_cast<MessageTypeType>(type) - 1))(CommonHeader{ version, type, length }, socket);
    }
} // END of namespace func
