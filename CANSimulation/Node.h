#ifndef NODE_H
#define NODE_H

#include <queue>
#include <vector>
#include <map>

#include "Message.h"
#include "ErrorCheck.h"

class CANBus;

class Node {

public:
    Node(int id, CANBus* canBus);

    bool receiveMessage(Message& msg);
    std::string sendNextMessage();
    void removeMessage();
    void addNodesAndRound(int round, int nodeId);
    bool isQueueEmpty() const { return messagesToBeSent.empty(); };
    int getNodeId() const;
    std::vector<Message*>& getReceivedMessages();
    std::map<int, std::vector<int>> getNodesAndRounds() const { return nodesAndRounds; }
    std::vector <Message*> getMessagesToBeSent() const { return messagesToBeSent; }
    void generate11BitID();
	void incrREC() { REC++; }
	void incrTEC() { TEC++; }
	int getREC() const { return REC; }
	int getTEC() const { return TEC; }
	void decrementTEC() { if (TEC > 0) TEC--; }
	void decrementREC() { if (REC > 0) REC--; }
	void setError(bool val) { nodeError = val; }
	void setNodeActive(bool val) { nodeActive = val; }

    std::vector<Message*> receivedMessages;
    int REC = 0;
    int TEC = 0;
    bool nodeActive;
    bool nodeError;

private:
    int nodeId;
    std::vector <Message*> messagesToBeSent;
    std::map<int, std::vector<int>> nodesAndRounds;
    CANBus* canBus;
    ErrorCheck* errorCheck = new ErrorCheck();
    std::string polynomial = "1100000000000010";
};

#endif