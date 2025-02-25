#include "Node.h"

#include <iostream>
#include <bitset>

#include "CANBus.h"
#include "ErrorCheck.h"

Node::Node(int id, CANBus* bus) : nodeId(id), canBus(bus) {}

bool Node::receiveMessage(Message& msg) 
{
	uint16_t crc = msg.getCRC();
    int round = msg.getRound();
	uint16_t id = msg.getId();
    //qDebug() << "Initial CRC: " << crc;

    Message* message = errorCheck->removeBitStuffing(msg.toString(), nodeError);
	message->setCRC(crc);
    message->setRound(round);
	message->setId(id);

    bool check = false;

	//qDebug() << "NODE ERROR:" << nodeError;
	uint16_t validCRC = errorCheck->calculateCRC(msg, polynomial, nodeError);
    if (validCRC == crc)
    {
        check = true;
		receivedMessages.push_back(message);
        //qDebug() << "Node " << nodeId << "GOT HERE AND PUSHED!" << message->getId() << " " << message->getRound();
    }
	//qDebug() << "Valid CRC: " << check;

    return check;
}

std::string Node::sendNextMessage()
{
    if (!messagesToBeSent.empty()) {
        Message* message = messagesToBeSent.front();

        std::string msgString = errorCheck->applyBitStuffing(*message, nodeError);

        return msgString;
    }
}

void Node::removeMessage()
{
	if (!messagesToBeSent.empty()) {
		messagesToBeSent.erase(messagesToBeSent.begin());
	}
}

void Node::addNodesAndRound(int round, int nodeId) {
    nodesAndRounds[round].push_back(nodeId);
}

std::vector<uint8_t> generateRandomData(size_t size) {
    std::vector<uint8_t> data(size);
    std::srand(std::time(nullptr)); 

    for (size_t i = 0; i < size; ++i) {
        data[i] = std::rand() % 256; 
    }

    return data;
}

void Node::generate11BitID() {

    int senderBits = nodeId - 1;

    for (auto& roundEntry : nodesAndRounds) {
        int round = roundEntry.first;
        const std::vector<int>& receiverNodeIds = roundEntry.second;

        int receiverBits = 0;
        for (int receiverId : receiverNodeIds) {
            receiverBits |= (1 << (receiverId - 1));
        }

        int identifier = (senderBits << 8) | receiverBits;

        std::vector<uint8_t> randomData = generateRandomData(8);

        Message* message = new Message(identifier, randomData, round, false);
        message->setSenderId(nodeId);

        uint16_t crc;
        if (nodeError) {
            //qDebug() << "GOT HERE!!";
            crc = 0b0000000000000000;
        }
        else {
            crc = errorCheck->calculateCRC(*message, polynomial, nodeError);
        }

        message->setCRC(crc);
   
		//qDebug() << "Node " << nodeId << " generated message with ID: " << identifier << " and CRC: " << message->getCRC();

        auto it = std::remove_if(messagesToBeSent.begin(), messagesToBeSent.end(), [&](Message* existingMessage) {
            int bitDifference = existingMessage->getId() ^ identifier;
            if (((std::bitset<16>(bitDifference).count() == 1) || (std::bitset<16>(bitDifference).count() == 0)) && (existingMessage->getRound() == round)) {
                qDebug() << "Node " << nodeId << " removed message with ID: " << existingMessage->getId();
                delete existingMessage;
                return true;
            }
            return false;
            });
        messagesToBeSent.erase(it, messagesToBeSent.end());

        messagesToBeSent.push_back(message); 
    }
}

int Node::getNodeId() const { return nodeId; }
std::vector<Message*>& Node::getReceivedMessages() { return receivedMessages; }