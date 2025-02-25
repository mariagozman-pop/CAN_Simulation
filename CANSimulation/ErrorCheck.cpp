#include "ErrorCheck.h"

#include <bitset>
#include <stdexcept>
#include <QDebug>

uint16_t ErrorCheck::calculateCRC(Message& message, const std::string& generatorPolynomial, bool simulateError) 
{
    std::string binaryData = std::bitset<11>(message.getId()).to_string();
    for (const auto& byte : message.getData()) {
        binaryData += std::bitset<8>(byte).to_string(); 
    }

    std::string paddedMessage = binaryData + std::string(generatorPolynomial.size() - 1, '0');
    std::string remainder = paddedMessage;

    for (size_t i = 0; i <= paddedMessage.size() - generatorPolynomial.size(); ++i) {
        if (remainder[i] == '1') {
            for (size_t j = 0; j < generatorPolynomial.size(); ++j) {
                remainder[i + j] = (remainder[i + j] == generatorPolynomial[j]) ? '0' : '1';
            }
        }
    }

    std::string crcBinary = remainder.substr(paddedMessage.size() - generatorPolynomial.size() + 1);
    uint16_t crc = static_cast<uint16_t>(std::bitset<16>(crcBinary).to_ulong());

    if (simulateError)
    {
        crc = 0;
    }

	//qDebug() << "CRC in calcCRC: " << crc;

    return crc;
}

std::string ErrorCheck::applyBitStuffing(const Message& message, bool simulateError) {
    std::string binaryMessage;

    binaryMessage += std::bitset<11>(message.getId()).to_string();

    for (const auto& byte : message.getData()) {
        binaryMessage += std::bitset<8>(byte).to_string(); 
    }

    binaryMessage += std::bitset<1>(message.getACK()).to_string();

    binaryMessage += std::bitset<16>(message.getCRC()).to_string();

    binaryMessage += std::bitset<32>(message.getRound()).to_string();

    binaryMessage += std::bitset<32>(message.getSenderId()).to_string();

    std::string stuffedMessage;
    int consecutiveBits = 0;
    char lastBit = ' ';

    for (char bit : binaryMessage) {
        stuffedMessage += bit;
        if (bit == lastBit) {
            ++consecutiveBits;
            if (consecutiveBits == 5) {
                stuffedMessage += (bit == '0' ? '1' : '0');
                consecutiveBits = 0;
            }
        }
        else {
            consecutiveBits = 1;
        }
        lastBit = bit;
    }

    if (simulateError) {
        if (!stuffedMessage.empty()) {
            stuffedMessage[5] = (stuffedMessage[5] == '0') ? '1' : '0';
        }
    }

    return stuffedMessage;
}

Message* ErrorCheck::removeBitStuffing(const std::string& stuffedMessage, bool simulateError) {
    std::string binaryMessage;
    int consecutiveBits = 0;
    char lastBit = ' ';

    for (size_t i = 0; i < stuffedMessage.size(); ++i) {
        char bit = stuffedMessage[i];
        binaryMessage += bit;

        if (bit == lastBit) {
            ++consecutiveBits;

            if (consecutiveBits == 5) {
                ++i;
                consecutiveBits = 0;
            }
        }
        else {
            consecutiveBits = 1;
        }

        lastBit = bit;
    }

    size_t pos = 0;

    uint16_t id = std::bitset<16>(binaryMessage.substr(pos, 16)).to_ulong();
    pos += 16;

    std::vector<uint8_t> data;
    for (int i = 0; i < 8; ++i) {
        data.push_back(static_cast<uint8_t>(std::bitset<8>(binaryMessage.substr(pos, 8)).to_ulong()));
        pos += 8;
    }

    bool ACK = std::bitset<1>(binaryMessage.substr(pos, 1)).to_ulong();
    pos += 1;

    uint16_t crc = std::bitset<16>(binaryMessage.substr(pos, 16)).to_ulong();
    pos += 16;

    uint32_t round = std::bitset<32>(binaryMessage.substr(pos, 32)).to_ulong();
    pos += 32;

    uint32_t senderId = std::bitset<32>(binaryMessage.substr(pos, 32)).to_ulong();
    pos += 32;

    if (simulateError) {
        id = -1;
    }

    Message* message = new Message(id, data, ACK, round);
    message->setSenderId(senderId);

    return message;
}

uint16_t ErrorCheck::extractStuffedId(const std::string& stuffedMessage) 
{
    uint16_t id = 0;              
    int extractedBits = 0;       
    int consecutiveBits = 0;      
    char lastBit = ' ';           
    int stuffedBitCount = 11; 

    for (size_t i = 0; i < stuffedMessage.size(); ++i) {
        char bit = stuffedMessage[i];

        if (bit == lastBit) {
            ++consecutiveBits;

            if (consecutiveBits == 5) {
                ++stuffedBitCount; 
                consecutiveBits = 0;            
            }
        }
        else {
            consecutiveBits = 1; 
        }

        lastBit = bit; 

        id = (id << 1) | (bit - '0'); 
        ++extractedBits;
    }

    return id;
}

std::string ErrorCheck::applyBitStuffingToId(uint16_t messageId) {
	std::string binaryMessage = std::bitset<11>(messageId).to_string();
	std::string stuffedMessage;
	int consecutiveBits = 0;
	char lastBit = ' ';

	for (char bit : binaryMessage) {
		stuffedMessage += bit;
		if (bit == lastBit) {
			++consecutiveBits;
			if (consecutiveBits == 5) {
				stuffedMessage += (bit == '0' ? '1' : '0');
				consecutiveBits = 0;
			}
		}
		else {
			consecutiveBits = 1;
		}
		lastBit = bit;
	}

	return stuffedMessage;
}

uint16_t ErrorCheck::binaryStringToUint16(const std::string& binaryString) {
    if (binaryString.size() > 16) {
        throw std::invalid_argument("Input string exceeds 16 bits.");
    }

    uint16_t result = 0;

    for (char bit : binaryString) {
        if (bit != '0' && bit != '1') {
            throw std::invalid_argument("Input string contains invalid characters.");
        }

        result = (result << 1) | (bit - '0');
    }

    return result;
}

