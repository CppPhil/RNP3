#pragma once
#include "Utility.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <typeinfo>
#include <cstddef>
#include <typeindex>
#include <QByteArray>

namespace utils {
    static auto constexpr bitAlignment = 32;
    static auto constexpr commonHeaderByteSize = 12;
    static auto constexpr sendMsgStructByteSize = 16;

    enum class MessageType : Word {
        reqFindServer = 1U,
        resFindServer,
        reqLogin,
        updateClientList,
        sendMsgGrp,
        sendMsgUsr,
        reqHeartbeat,
        resHeartbeat,
        errorMsgNotDelivered,
    }; // END of enum class MessageType

    static auto constexpr amtMessageTypes = 9;
   
    class Message {
    public:
        using this_type = Message;
        Message(Word version, MessageType type, Word length);
        virtual ~Message();
        virtual QByteArray toByteArray() const;
        Word getVersion() const;
        MessageType getType() const;
        Word getLength() const;

    private:
        Word version_;
        MessageType type_;
        Word length_;
    }; // END of class Message

    class ReqFindServerMessage final : public Message {
    public:
        using this_type = ReqFindServerMessage;
        using Base = Message;

        using Message::Message;
    }; // END of class ReqFindServerMessage

    class ReqLoginMessage final : public Message {
    public:
        using this_type = ReqLoginMessage;
        using Base = Message;

        virtual QByteArray toByteArray() const override;

        ReqLoginMessage(Word version, MessageType type, Word length, std::string username);
        std::string getUsername() const;
    private:
        std::string username_;
    }; // END of class ReqLoginMessage

    class ReqHeartbeatMessage final : public Message {
    public:
        using this_type = ReqHeartbeatMessage;
        using Base = Message;

        using Message::Message;
    }; // END of class ReqHeartbeatMessage

    class ResFindServerMessage final : public Message {
    public:
        using this_type = ResFindServerMessage;
        using Base = Message;

        using Message::Message;
    }; // END of class ResFindServerMessage

    class ResHeartbeatMessage final : public Message {
    public:
        using this_type = ResHeartbeatMessage;
        using Base = Message;

        using Message::Message;
    }; // END of class ResHeartbeatMessage

    class UsernameRecord final {
    public:
        using this_type = UsernameRecord;
        UsernameRecord(Word ip, HalfWord port, Byte lengthUsername, std::string username);
        Word getIp() const;
        HalfWord getPort() const;
        Byte getLengthUsername() const;
        std::string getUsername() const;
        QByteArray toByteArray() const;
    private:
        Word ip_;
        HalfWord port_;
        Byte lengthUsername_;
        std::string username_;
    }; // END of class UsernameRecord

    class UpdateClientListMessage final : public Message {
    public:
        using this_type = UpdateClientListMessage;
        using Base = Message;
        using value_type = UsernameRecord;
        using container_type = std::vector<UsernameRecord>;
        using iterator = container_type::iterator;
        using const_iterator = container_type::const_iterator;
        using reverse_iterator = container_type::reverse_iterator;
        using const_reverse_iterator = container_type::const_reverse_iterator;

        UpdateClientListMessage(Word version, MessageType type, Word length, container_type cont);
        void addElement(value_type const &val);
        void addElement(value_type &&val);
        iterator begin();
        const_iterator begin() const;
        const_iterator cbegin() const;
        iterator end();
        const_iterator end() const;
        const_iterator cend() const;
        reverse_iterator rbegin();
        const_reverse_iterator rbegin() const;
        const_reverse_iterator crbegin() const;
        reverse_iterator rend();
        const_reverse_iterator rend() const;
        const_reverse_iterator crend() const;
        virtual QByteArray toByteArray() const override;

    private:
        container_type records_;
    }; // END of class UpdateClientListMessage

    class SendMessageBase : public Message {
    public:
        using this_type = SendMessageBase;
        using Base = Message;

        SendMessageBase(Word version, MessageType type, Word length, Word messageId,
                        Word sourceIp, Word targetIp, HalfWord sourcePort, HalfWord targetPort);
        virtual ~SendMessageBase();
        Word getMessageId() const;
        Word getSourceIp() const;
        Word getTargetIp() const;
        HalfWord getSourcePort() const;
        HalfWord getTargetPort() const;
        virtual QByteArray toByteArray() const override;

    protected:
        Word messageId_;
        Word sourceIp_;
        Word targetIp_;
        HalfWord sourcePort_;
        HalfWord targetPort_;
    }; // END of class SendMessageBase

    class SendMsgGrpMessage final : public SendMessageBase {
    public:
        using this_type = SendMsgGrpMessage;
        using Base = SendMessageBase;

        SendMsgGrpMessage(Word version, MessageType type, Word length, Word messageId,
                          Word sourceIp, Word targetIp, HalfWord sourcePort, HalfWord targetPort,
                          std::string messageText);

        std::string getMessageText() const;

        virtual QByteArray toByteArray() const override;
    private:
        std::string messageText_;
        Word const messageTextStringLength_;
    }; // END of class SendMsgGrpMessage

    class SendMsgUsrMessage final : public SendMessageBase {
    public:
        using this_type = SendMsgUsrMessage;
        using Base = SendMessageBase;

        SendMsgUsrMessage(Word version, MessageType type, Word length, Word messageId,
                          Word sourceIp, Word targetIp, HalfWord sourcePort, HalfWord targetPort,
                          std::string messageText);

        std::string getMessageText() const;
        virtual QByteArray toByteArray() const override;

    private:
        std::string messageText_;
        Word const messageTextStringLength_;
    }; // END of class SendMsgUsrMessage

    class ErrorMsgNotDeliveredMessage final : public SendMessageBase {
    public:
        using this_type = ErrorMsgNotDeliveredMessage;
        using Base = SendMessageBase;

        ErrorMsgNotDeliveredMessage(Word version, MessageType type, Word length, Word messageId,
            Word sourceIp, Word targetIp, HalfWord sourcePort, HalfWord targetPort);
    }; // END of class ErrorMsgNotDeliveredMessage

    extern std::unordered_map<std::type_index, MessageType> const rttiTable;
} // END of namespace utils
