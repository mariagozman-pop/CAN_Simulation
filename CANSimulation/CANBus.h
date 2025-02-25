#ifndef CANBUS_H
#define CANBUS_H

#include <iostream>
#include <iomanip>
#include <queue>
#include <vector>
#include <memory>
#include <fstream> 
#include <map>     
#include <ctime>   

#include <QObject>

#include "Message.h"
#include "Node.h"
#include "CANSim.h"
#include "ErrorCheck.h"

class Node; 
class CANSim;

class CANBus : public QObject {
    Q_OBJECT

public:
    explicit CANBus(CANSim* simulation, QObject* parent = nullptr);

    bool arbitrate();
    void addNode(Node* node);
    bool hasPendingMessages() const { return !pendingMessages.empty(); }
    int getRound() const { return round; }
    void incrementRound();
    void logMessage(const std::string& message); 

    std::vector<Node*> nodes;
    std::vector<Message> pendingMessages;
    int round;
    CANSim* sim;
    struct ArbitrationStep {
        int round;
        int bitPosition;
        std::vector<Message> contenders;
        std::vector<Message> nonContenders;
        std::map<int, std::map<int, std::vector<int>>> roundContenderBitValues;
        std::map<int, std::map<int, std::vector<Node>>> nodes;

        ArbitrationStep(int r, int bp, const std::vector<Message>& c, const std::vector<Message>& nc)
            : round(r), bitPosition(bp), contenders(c), nonContenders(nc) {}
    };
    std::vector<ArbitrationStep> arbitrationSteps;
    std::vector<Message> winners;
    bool bitStuffingVisible;
    ErrorCheck* errorCheck = new ErrorCheck();
};

#endif