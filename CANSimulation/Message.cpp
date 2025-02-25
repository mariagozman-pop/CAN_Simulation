#include "Message.h"

#include <sstream>
#include <iomanip>
#include <stdexcept>

Message::Message(uint16_t id, const std::vector<uint8_t>& data, int round, bool ACK)
    : id(id), data(data), round(round), crc(0), ACK(false) {

    if (data.size() > 8) {
        throw std::invalid_argument("Data length exceeds CAN limit of 8 bytes.");
    }
}

int Message::getRound() const {
    return round;
}

void Message::setRound(int newRound) {
    round = newRound;
}

uint16_t Message::getId() const {
    return id;
}

std::vector<uint8_t> Message::getData() const {
    return data;
}

uint8_t Message::getDataLength() const {
    return static_cast<uint8_t>(data.size());
}

bool Message::getACK() const {
    return ACK;
}

uint16_t Message::getCRC() const {
    return crc;
}

int Message::getSenderId() const {
	return senderId;
}

uint16_t Message::getStuffedId() const {
	return stuffedId;
}

void Message::setId(uint16_t newId) {
    id = newId;
}

void Message::setStuffedId(uint16_t newId) {
    stuffedId = newId;
}

void Message::setData(const std::vector<uint8_t>& newData) {
    if (newData.size() > 8) {
        throw std::invalid_argument("Data length exceeds CAN limit of 8 bytes.");
    }
    data = newData;
}

void Message::setACK(bool ack) {
    ACK = ack;
}

void Message::setCRC(uint16_t newCRC) {
    crc = newCRC;
}

void Message::setSenderId(int newSenderId) {
	senderId = newSenderId;
}