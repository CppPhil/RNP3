#include "Types.h"
#include <utility>
#include "Logger.h"

#ifdef RESERVED_BYTE
	static_assert(false, "RESERVED_BYTE was already defined in Types.cpp");
#endif // RESERVED_BYTE

#define RESERVED_BYTE 

namespace utils {
    Message::Message(Word version, MessageType type, Word length)
        : version_{ version }, type_{ type }, length_{ length } {
        LOG_SCOPE;
    }

    Message::~Message() = default;

    QByteArray Message::toByteArray() const {
        LOG_SCOPE;
        QByteArray ret{ commonHeaderByteSize, '\0' };
        auto ptr = ret.data();
        writeToAddress(ptr, version_);
        advancePtr(ptr, sizeof(Word));
        writeToAddress(ptr, type_);
        advancePtr(ptr, sizeof(MessageType));
        writeToAddress(ptr, length_);
        return ret;
    }
    
    Word Message::getVersion() const {
        LOG_SCOPE;
        return version_;
    }

    MessageType Message::getType() const {
        LOG_SCOPE;
        return type_;
    }

    Word Message::getLength() const {
        LOG_SCOPE;
        return length_;
    }

    ReqLoginMessage::ReqLoginMessage(Word version, MessageType type, Word length, std::string userName)
        : Message{ version, type, length }, username_{ std::move(userName) } {
        LOG_SCOPE;
    }

    QByteArray ReqLoginMessage::toByteArray() const {
        LOG_SCOPE;
        auto bytes = Base::toByteArray();
        bytes.append(username_.data(), getLength());
        return bytes;
    }

    std::string ReqLoginMessage::getUsername() const {
        LOG_SCOPE;
        return username_;
    }

    UsernameRecord::UsernameRecord(Word ip, HalfWord port, Byte lengthUsername, std::string username)
        : ip_{ ip }, port_{ port },
          lengthUsername_{ lengthUsername }, username_{ std::move(username) } {
        LOG_SCOPE;
    }

    Word UsernameRecord::getIp() const {
        LOG_SCOPE;
        return ip_;
    }

    HalfWord UsernameRecord::getPort() const {
        LOG_SCOPE;
        return port_;
    }

    Byte UsernameRecord::getLengthUsername() const {
        LOG_SCOPE;
        return lengthUsername_;
    }

    std::string UsernameRecord::getUsername() const {
        LOG_SCOPE;
        return username_;
    }

    QByteArray UsernameRecord::toByteArray() const {
        LOG_SCOPE;
        auto const extraPadding = bitAlignment - bitAlignment % username_.size();
        auto const totalStringBytes = username_.size() + extraPadding;
        if (totalStringBytes % bitAlignment != 0) {
            throw std::logic_error{ "totalStringBytes in UsernameRecord::toByteArray was not a multiple of 32" };
        }

        QByteArray bytes{ sizeof(Word) + sizeof(HalfWord) +
                          sizeof(Byte) + RESERVED_BYTE sizeof(Byte), '\0' };

        auto ptr = bytes.data();
        writeToAddress(ptr, ip_);
        advancePtr(ptr, sizeof(Word));
        writeToAddress(ptr, port_);
        advancePtr(ptr, sizeof(HalfWord));
        writeToAddress(ptr, lengthUsername_);
        bytes.append(username_.data(), lengthUsername_);
        for (auto i = 0; i < extraPadding; ++i) {
            bytes.append('\0');
        }

        return bytes;
    }
   
    UpdateClientListMessage::UpdateClientListMessage(Word version, MessageType type, Word length,
                                                     container_type cont)
        : Message{ version, type, length }, 
          records_{ std::move(cont) } {
        LOG_SCOPE;
    }

    void UpdateClientListMessage::addElement(value_type const &val) {
        LOG_SCOPE;
        records_.push_back(val);
    }

    void UpdateClientListMessage::addElement(value_type &&val) {
        LOG_SCOPE;
        records_.push_back(std::move(val));
    }

    UpdateClientListMessage::iterator UpdateClientListMessage::begin() {
        LOG_SCOPE;
        return records_.begin();
    }

    UpdateClientListMessage::const_iterator UpdateClientListMessage::begin() const {
        LOG_SCOPE;
        return cbegin();
    }

    UpdateClientListMessage::const_iterator UpdateClientListMessage::cbegin() const {
        LOG_SCOPE;
        return records_.cbegin();
    }

    UpdateClientListMessage::iterator UpdateClientListMessage::end() {
        LOG_SCOPE;
        return records_.end();
    }

    UpdateClientListMessage::const_iterator UpdateClientListMessage::end() const {
        LOG_SCOPE;
        return cend();
    }

    UpdateClientListMessage::const_iterator UpdateClientListMessage::cend() const {
        LOG_SCOPE;
        return records_.cend();
    }

    UpdateClientListMessage::reverse_iterator UpdateClientListMessage::rbegin() {
        LOG_SCOPE;
        return records_.rbegin();
    }

    UpdateClientListMessage::const_reverse_iterator UpdateClientListMessage::rbegin() const {
        LOG_SCOPE;
        return crbegin();
    }

    UpdateClientListMessage::const_reverse_iterator UpdateClientListMessage::crbegin() const {
        LOG_SCOPE;
        return records_.crbegin();
    }

    UpdateClientListMessage::reverse_iterator UpdateClientListMessage::rend() {
        LOG_SCOPE;
        return records_.rend();
    }

    UpdateClientListMessage::const_reverse_iterator UpdateClientListMessage::rend() const {
        LOG_SCOPE;
        return crend();
    }

    UpdateClientListMessage::const_reverse_iterator UpdateClientListMessage::crend() const {
        LOG_SCOPE;
        return records_.crend();
    }

    QByteArray UpdateClientListMessage::toByteArray() const {
        LOG_SCOPE;
        auto bytes = Base::toByteArray();
        for (auto const &e : records_) {
            bytes.append(e.toByteArray());
        }
        return bytes;
    }

    SendMessageBase::~SendMessageBase() = default;

    SendMessageBase::SendMessageBase(Word version, MessageType type, Word length, Word messageId, Word sourceIp, Word targetIp, HalfWord sourcePort, HalfWord targetPort) 
        : Message{ version, type, length },
        messageId_{ messageId }, sourceIp_{ sourceIp }, targetIp_{ targetIp },
        sourcePort_{ sourcePort }, targetPort_{ targetPort } {
        LOG_SCOPE;
    }

    Word SendMessageBase::getMessageId() const {
        LOG_SCOPE;
        return messageId_;
    }

    Word SendMessageBase::getSourceIp() const {
        LOG_SCOPE;
        return sourceIp_;
    }

    Word SendMessageBase::getTargetIp() const {
        LOG_SCOPE;
        return targetIp_;
    }

    HalfWord SendMessageBase::getSourcePort() const {
        LOG_SCOPE;
        return sourcePort_;
    }

    HalfWord SendMessageBase::getTargetPort() const {
        LOG_SCOPE;
        return targetPort_;
    }

    QByteArray SendMessageBase::toByteArray() const {
        LOG_SCOPE;
        auto result = Base::toByteArray();
        QByteArray sendMessageByteArray{ 3 * sizeof(Word) + 2 * sizeof(HalfWord), '\0' };
        auto ptr = sendMessageByteArray.data();
        writeToAddress(ptr, messageId_);
        advancePtr(ptr, sizeof(Word));
        writeToAddress(ptr, sourceIp_);
        advancePtr(ptr, sizeof(Word));
        writeToAddress(ptr, targetIp_);
        advancePtr(ptr, sizeof(Word));
        writeToAddress(ptr, sourcePort_);
        advancePtr(ptr, sizeof(HalfWord));
        writeToAddress(ptr, targetPort_);
        result.append(sendMessageByteArray);
        return result;
    }

    SendMsgGrpMessage::SendMsgGrpMessage(Word version, MessageType type, Word length, Word messageId, Word sourceIp, Word targetIp, HalfWord sourcePort, HalfWord targetPort, std::string messageText)
        : SendMessageBase{ version, type, length, messageId, sourceIp, targetIp, sourcePort, targetPort },
        messageText_{ std::move(messageText) }, messageTextStringLength_{ length - sendMsgStructByteSize } {
        LOG_SCOPE;
    }

    std::string SendMsgGrpMessage::getMessageText() const {
        LOG_SCOPE;
        return messageText_;
    }

    QByteArray SendMsgGrpMessage::toByteArray() const {
        LOG_SCOPE;
        auto bytes = Base::toByteArray();
        bytes.append(messageText_.data(), messageTextStringLength_);
        return bytes;
    }

    SendMsgUsrMessage::SendMsgUsrMessage(Word version, MessageType type, Word length, Word messageId, Word sourceIp, Word targetIp, HalfWord sourcePort, HalfWord targetPort, std::string messageText)
        : SendMessageBase{ version, type, length, messageId, sourceIp, targetIp, sourcePort, targetPort },
        messageText_{ std::move(messageText) }, messageTextStringLength_{ length - sendMsgStructByteSize } {
        LOG_SCOPE;
    }

    std::string SendMsgUsrMessage::getMessageText() const {
        LOG_SCOPE;
        return messageText_;
    }

    QByteArray SendMsgUsrMessage::toByteArray() const {
        LOG_SCOPE;
        auto bytes = Base::toByteArray();
        bytes.append(messageText_.data(), messageTextStringLength_);
        return bytes;
    }
    
    ErrorMsgNotDeliveredMessage::ErrorMsgNotDeliveredMessage(Word version, MessageType type, Word length, Word messageId, Word sourceIp, Word targetIp, HalfWord sourcePort, HalfWord targetPort)
        : SendMessageBase{ version, type, length, messageId, sourceIp, targetIp, sourcePort, targetPort } {
        LOG_SCOPE;
    }

    extern std::unordered_map<std::type_index, MessageType> const rttiTable{
        { std::type_index{ typeid(ReqFindServerMessage) }, MessageType::reqFindServer },
        { std::type_index{ typeid(ReqLoginMessage) }, MessageType::reqLogin },
        { std::type_index{ typeid(ReqHeartbeatMessage) }, MessageType::reqHeartbeat },
        { std::type_index{ typeid(ResFindServerMessage) }, MessageType::resFindServer },
        { std::type_index{ typeid(ResHeartbeatMessage) }, MessageType::resHeartbeat },
        { std::type_index{ typeid(UpdateClientListMessage) }, MessageType::updateClientList },
        { std::type_index{ typeid(SendMsgGrpMessage) }, MessageType::sendMsgGrp },
        { std::type_index{ typeid(SendMsgUsrMessage) }, MessageType::sendMsgUsr },
        { std::type_index{ typeid(ErrorMsgNotDeliveredMessage) }, MessageType::errorMsgNotDelivered }
    };

} // END of namespace utils
