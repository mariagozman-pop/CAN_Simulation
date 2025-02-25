#include "CANBus.h"

#include <iostream>
#include <vector>

#include "Message.h"
#include "CANSim.h"
#include "ErrorCheck.h"

CANBus::CANBus(CANSim* simulation, QObject* parent)
    : QObject(nullptr), round(0), sim(simulation) 
{
    std::ofstream logFile("log.txt", std::ios::app);

    if (logFile.is_open()) {
        std::time_t now = std::time(nullptr);
        struct tm localTime;

        if (localtime_s(&localTime, &now) == 0) {
            char timeBuffer[80];
            std::strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", &localTime);

            logMessage("-----------------------");
            logFile << "CANBus initialized at: " << timeBuffer << "\n";
            logMessage("-----------------------");
        }
        else {
            logFile << "Error: Failed to retrieve local time.\n";
        }

        logFile.close();
    }
}

void CANBus::logMessage(const std::string& message) 
{
    std::ofstream logFile("log.txt", std::ios::app);

    if (logFile.is_open()) {
        logFile << message << "\n";
        logFile.close();
    }
}

void CANBus::incrementRound() {
    ++round;
}

void CANBus::addNode(Node* node) {
    nodes.push_back(node);
}

bool CANBus::arbitrate()
{
    bool successfullArbitration = true;

    if(round == 0)
        nodes = sim->nodesInSim;

    if (pendingMessages.empty()) {
        logMessage("No messages to transmit.");
        return false;
    }

    // Filter messages based on the current round and active senders
    std::vector<Message> filteredMessages;
    for (const auto& msg : pendingMessages) {
        int senderId = msg.getSenderId();
        if (msg.getRound() <= round && nodes[senderId - 1]->nodeActive) {
            filteredMessages.push_back(msg);
        }
    }

    // If no messages for this round, return false
    if (filteredMessages.empty()) {
        logMessage("No messages for the current round: " + std::to_string(round));
        return true;
    }

    // Bitwise arbitration
    std::vector<Message> contenders = filteredMessages;
    std::vector<Message> nonContenders;

    int ID_BITS = 11;
    if (bitStuffingVisible) {
        int maxStuffedBits = 0; 

        for (auto& msg : contenders) {
            std::string stuffedId = errorCheck->applyBitStuffingToId(msg.getId());
            uint16_t id = errorCheck->binaryStringToUint16(stuffedId);
            msg.setStuffedId(id);

            if (stuffedId.size() > maxStuffedBits) {
                maxStuffedBits = stuffedId.size();
            }
        }

        ID_BITS = maxStuffedBits;
    }

    std::string logEntry = "ROUND: " + std::to_string(round);
    logMessage(logEntry);

    for (int bit = ID_BITS - 1; bit >= 0; --bit) {

        ArbitrationStep step = ArbitrationStep(round, bit, contenders, nonContenders); 

        std::string logEntry = "  Arbitrating at bit position: " + std::to_string(bit+1);
        logMessage(logEntry);

        // Print the message IDs still in the race at this bit position
        logMessage("      Messages still in the race at the beginning: ");
        for (const auto& msg : contenders) {
			int senderId = msg.getSenderId();
            uint16_t id = msg.getId();

			if (nodes[senderId - 1]->nodeActive == false)
            {
				logMessage("Node " + std::to_string(senderId) + " is disabled. It will not participate in the arbitration.");
				continue;
			}

            if (bitStuffingVisible)
            {
                std::string stuffedId = errorCheck->applyBitStuffingToId(msg.getId());
                logEntry = "         - message: " + stuffedId;
                logEntry += ", sender ID: " + std::to_string(senderId);
                logEntry += ", initial round: " + std::to_string(msg.getRound());

                logMessage(logEntry);
            }
            else
            {
                std::string idBinary;
                for (int i = 10; i >= 0; --i) {
                    idBinary += ((id >> i) & 1) ? '1' : '0';
                }
                logEntry = "         - message: " + idBinary;
                logEntry += ", sender ID: " + std::to_string(senderId);
                logEntry += ", initial round: " + std::to_string(msg.getRound());

                logMessage(logEntry);
            }
        }

        logMessage(" ");
        std::vector<Message> newContenders;

        for (auto& msg : contenders) {
            uint16_t idBit;
            if (bitStuffingVisible) {
				idBit = (msg.getStuffedId() >> bit) & 1;
            }
            else
            {
                idBit = (msg.getId() >> bit) & 1;
            }

			step.roundContenderBitValues[round][msg.getSenderId()].push_back(idBit);
			for (int i = 0; i < nodes.size(); i++)
            {
                Node node = Node(nodes[i]->getNodeId(), this);
				node.setNodeActive(nodes[i]->nodeActive);
				node.TEC = nodes[i]->getTEC();
				node.REC = nodes[i]->getREC();
				step.nodes[round][i].push_back(node);
			}   

            if (newContenders.empty()) {
                newContenders.push_back(msg);
            }
            else {
                uint16_t prevBit = (newContenders[0].getId() >> bit) & 1;

                if (idBit < prevBit) {
                    nonContenders.push_back(msg);
                    newContenders.clear();
                    newContenders.push_back(msg);
                }
                else if (idBit == prevBit) {
                    newContenders.push_back(msg);
                }
                else {
                    nonContenders.push_back(msg);
                }
            }
        }

        contenders = newContenders;

        arbitrationSteps.push_back(step);

        if (contenders.empty()) {
            logMessage("No more contenders at bit position " + std::to_string(bit));
            break;
        }
    }

    // If we have a winner, process the winning message
    if (!contenders.empty()) {
        Message winningMsg = contenders.front();

        uint16_t id = winningMsg.getId();
        int senderId = winningMsg.getSenderId();
        std::string idBinary;
        for (int i = 10; i >= 0; --i) {
            idBinary += ((id >> i) & 1) ? '1' : '0';
        }
        logMessage("!!!");
        logEntry = "Winner " + idBinary;
        logEntry += ", sender ID: " + std::to_string(senderId);
        logEntry += ", initial round: " + std::to_string(winningMsg.getRound());
        logMessage(logEntry);

        if (nodes[senderId - 1]->nodeActive) {
            std::string stuffedMessage = nodes[senderId - 1]->sendNextMessage();
            logMessage("Stuffed Message: " + stuffedMessage);
        }

        uint16_t receiverBits = id & 0xFF;
        Message winningMsgCopy = winningMsg;

        uint16_t crcValue = winningMsgCopy.getCRC();
        std::string crcBinary;
        for (int i = 15; i >= 0; --i) {
            crcBinary += (crcValue & (1 << i)) ? '1' : '0';
        }
        logMessage("CRC: " + crcBinary);

        // Print the nodes that received the message and when they set the acknowledgement bit to 1
        bool activeReceiver = false;
        for (int i = 0; i < 8; ++i)
        {
            if (receiverBits & (1 << i)) {
                int receiverId = i + 1;

                if (nodes[receiverId - 1]->nodeActive == true)
                {
                    activeReceiver = true;
                    bool received = nodes[receiverId - 1]->receiveMessage(winningMsgCopy);

                    if (nodes[receiverId - 1]) {
                        if (received) {
                            if (!nodes[receiverId - 1]->receivedMessages.empty()) {
                                logMessage("Node " + std::to_string(receiverId) + " received the message, CRC verification was valid.");
                                if (!winningMsg.getACK())
                                {
                                    winningMsg.setACK(true);
                                    logMessage(" - ack bit was set to valid by node: " + std::to_string(receiverId) + "");
                                }
                                nodes[receiverId - 1]->decrementREC();
                            }
                            else {
                                logMessage("CRC check failed for Node " + std::to_string(receiverId) + ". Message was not received.");
                                nodes[receiverId - 1]->incrREC();
                            }
                        }
                        else {
                            logMessage("Message was not received.");
                            nodes[receiverId - 1]->incrREC();
                        }
                    }
                }
            }
        }

        int sender_id = winningMsg.getSenderId();
        // Check if the acknowledgement bit was set to 1
        if ((!winningMsg.getACK()) && (activeReceiver)) {
            logMessage("No nodes received the winning message. Adding it back to the pending messages.");
            nodes[sender_id - 1]->incrTEC();
            Message falseWinner = Message(0, std::vector<uint8_t>{0}, 0, false);
            winners.push_back(falseWinner);
        }
        else if (!activeReceiver) {
            logMessage("No nodes received the winning message. Adding it back to the pending messages.");
            Message falseWinner = Message(0, std::vector<uint8_t>{0}, 0, false);
            winners.push_back(falseWinner);
        }
        else {
			logMessage("Winning message was received by at least one node. Removing it from the pending messages.");
            winners.push_back(winningMsg);

            auto it = std::find(pendingMessages.begin(), pendingMessages.end(), winningMsg);

            if (it != pendingMessages.end()) {
                pendingMessages.erase(it);
            }
            nodes[sender_id - 1]->decrementTEC();
            nodes[sender_id - 1]->removeMessage();

			successfullArbitration = true;
        }

        if (activeReceiver == false)
        {
			for (const auto& msg : nodes[senderId-1]->getMessagesToBeSent())
            {
                if (msg->getId() == winningMsg.getId())
                {
					nodes[senderId - 1]->removeMessage();
                    pendingMessages.erase(
                        std::remove_if(
                            pendingMessages.begin(),
                            pendingMessages.end(),
                            [&msg](const Message& m) {
                                return (m.getId() == msg->getId()) && (m.getRound() == msg->getRound());
                            }
                        ),
                        pendingMessages.end()
                    );
				}
			}
        }

        logMessage("NODE ERROR COUNTERS :");
        for (size_t i = 0; i < nodes.size(); ++i) 
        {
            if (!nodes[i]->nodeActive) continue;

            logMessage("- node " + std::to_string(i + 1) + "  TEC: " + std::to_string(nodes[i]->getTEC()) + "  REC: " + std::to_string(nodes[i]->getREC()));
			if (nodes[i]->getTEC() >= 9)
            {
				logMessage("Node " + std::to_string(i + 1) + " has reached the maximum TEC value. It will be disabled.");
                nodes[i]->setNodeActive(false);
			}  

			if (nodes[i]->getREC() >= 4)
			{
				logMessage("Node " + std::to_string(i + 1) + " has reached the maximum REC value. It will be disabled.");
                nodes[i]->setNodeActive(false);
			}   
        }

        logMessage(" ");

        if (nodes[winningMsg.getSenderId() - 1]->nodeActive == false)
        {
            successfullArbitration = true;
        }
    }

    for (const auto& msg : nonContenders) {
        if (msg.getId() != contenders.front().getId() &&
            std::find_if(pendingMessages.begin(), pendingMessages.end(),
                [&msg](const Message& m) { return m.getId() == msg.getId(); }) == pendingMessages.end()) {
            pendingMessages.push_back(msg);
        }
    }

    return successfullArbitration;
}