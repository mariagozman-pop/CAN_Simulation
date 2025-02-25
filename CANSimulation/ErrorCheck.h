#ifndef ERRORCHECK_H
#define ERRORCHECK_H

#include "Message.h"

#include <string>

class ErrorCheck {
public:
    uint16_t calculateCRC(Message& message, const std::string& generatorPolynomial, bool simulateError);

    std::string applyBitStuffing(const Message& message, bool simulateError);

    std::string applyBitStuffingToId(uint16_t messaegId);

    Message* removeBitStuffing(const std::string& stuffedMessage, bool simulateError);

    uint16_t extractStuffedId(const std::string& stuffedString);

    uint16_t binaryStringToUint16(const std::string& binaryString);
};

#endif