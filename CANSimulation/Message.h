#ifndef MESSAGE_H
#define MESSAGE_H

#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdint>
#include <string>
#include <bitset>

class Message {
public:
    Message(uint16_t id, const std::vector<uint8_t>& data, int round, bool ack = false);

    uint16_t getId() const;
    std::vector<uint8_t> getData() const;
    uint8_t getDataLength() const;
    bool getACK() const;
    uint16_t getCRC() const;
    int getRound() const; 
	int getSenderId() const;
	uint16_t getStuffedId() const;

    void setId(uint16_t id);
    void setData(const std::vector<uint8_t>& data);
    void setACK(bool rtr);
    void setCRC(uint16_t crc);
    void setRound(int round);
    void setSenderId(int id);
	void setStuffedId(uint16_t newId);

    bool operator==(const Message& other) const {
        return this->getId() == other.getId() &&
            this->getSenderId() == other.getSenderId() &&
            this->getRound() == other.getRound(); 
    }

    std::string toString() const {
        std::string result;

        result += std::bitset<16>(id).to_string();

        for (const auto& byte : data) {
            result += std::bitset<8>(byte).to_string();
        }

        result += std::bitset<1>(ACK).to_string();

        result += std::bitset<16>(crc).to_string();

        result += std::bitset<32>(round).to_string();

        result += std::bitset<32>(senderId).to_string();

        return result;
    }

private:
    uint16_t id;                      
    std::vector<uint8_t> data;        
    uint16_t crc;                   
    bool ACK;                         
    int round;                       
	int senderId;					  
	uint16_t stuffedId;              
};

#endif