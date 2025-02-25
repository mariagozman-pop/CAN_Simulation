#include "CANSim.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QVBoxLayout>
#include <QPushButton>
#include <QGraphicsTextItem>
#include <QPen>
#include <QRandomGenerator>
#include <QTimer>
#include <QDockWidget>
#include <QTableWidget>
#include <QLabel>
#include <QDialog>
#include <QCheckBox>
#include <QMessageBox>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QFile>
#include <QUrl>
#include <random>

#include "NodeConfigWidget.h"
#include "Message.h"
#include "MessageDialog.h"
#include "Node.h"


CANSim::CANSim(QWidget* parent)
    : QMainWindow(parent), nextXPosition(50), simulationStarted(false)
{
    resize(600, 500);
    createWelcomeScreen();
}

void CANSim::createWelcomeScreen()
{
    QWidget* welcomeWidget = new QWidget(this);
    setCentralWidget(welcomeWidget);

    QVBoxLayout* welcomeLayout = new QVBoxLayout(welcomeWidget);

    QLabel* welcomeLabel = new QLabel("Welcome to CAN Simulator", this);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    QFont font = welcomeLabel->font();
    font.setPointSize(18);
    welcomeLabel->setFont(font);
    welcomeLayout->addWidget(welcomeLabel);

    QPushButton* predefinedButton = new QPushButton("Use Predefined Scenario", this);
    QPushButton* customButton = new QPushButton("Configure Your Own", this);

    welcomeLayout->addWidget(predefinedButton);
    welcomeLayout->addWidget(customButton);

    connect(predefinedButton, &QPushButton::clicked, this, &CANSim::selectPredefinedScenario);
    connect(customButton, &QPushButton::clicked, this, &CANSim::initializeCustomConfiguration);
}

void CANSim::selectPredefinedScenario()
{
    QDialog* predefinedDialog = new QDialog(this);
    predefinedDialog->setWindowTitle("Select Predefined Scenario");

    QVBoxLayout* layout = new QVBoxLayout(predefinedDialog);

    QLabel* label = new QLabel("Choose a predefined scenario:", predefinedDialog);
    layout->addWidget(label);

    QPushButton* scenario1Button = new QPushButton("Scenario 1", predefinedDialog);
    QPushButton* scenario2Button = new QPushButton("Scenario 2", predefinedDialog);
    QPushButton* scenario3Button = new QPushButton("Scenario 3", predefinedDialog);

    layout->addWidget(scenario1Button);
    layout->addWidget(scenario2Button);
    layout->addWidget(scenario3Button);

    connect(scenario1Button, &QPushButton::clicked, this, [this, predefinedDialog]() {
        setupPredefinedScenario(1);
        predefinedDialog->accept();
        });

    connect(scenario2Button, &QPushButton::clicked, this, [this, predefinedDialog]() {
        setupPredefinedScenario(2);
        predefinedDialog->accept();
        });

    connect(scenario3Button, &QPushButton::clicked, this, [this, predefinedDialog]() {
        setupPredefinedScenario(3);
        predefinedDialog->accept();
        });

    predefinedDialog->exec();
}

void CANSim::setupPredefinedScenario(int scenario)
{
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    layout = new QVBoxLayout(centralWidget);

    graphicsView = new QGraphicsView(this);
    scene = new QGraphicsScene(this);
    graphicsView->setScene(scene);
    layout->addWidget(graphicsView);

    QGraphicsLineItem* canHighLine = scene->addLine(0, 100, 500, 100);
    QGraphicsLineItem* canLowLine = scene->addLine(0, 200, 500, 200);

    changeLineColor(canHighLine, Qt::white);
    changeLineColor(canLowLine, Qt::white);

    canLines.append(canHighLine);
    canLines.append(canLowLine);

    QGraphicsTextItem* canHighLabel = scene->addText("CAN High");
    canHighLabel->setPos(10, 80);

    QGraphicsTextItem* canLowLabel = scene->addText("CAN Low");
    canLowLabel->setPos(10, 180);

    canBusLabel = scene->addText("CAN Bus");
    canBusLabel->setPos(250, 140);

    canBus = new CANBus(this);
    canBus->bitStuffingVisible = false;

    roundLabel = scene->addText("Round: -");
    roundLabel->setDefaultTextColor(Qt::white);
    roundLabel->setFont(QFont("Arial", 14));
    roundLabel->setPos(1, 10);

    bitPositionLabel = scene->addText("Bit Position: -");
    bitPositionLabel->setDefaultTextColor(Qt::white);
    bitPositionLabel->setFont(QFont("Arial", 14));
    bitPositionLabel->setPos(1, 40);

    switch (scenario) {
    case 1: 
        addPredefinedNode(1, { {2, 5}, {3, 10} }, false);
        addPredefinedNode(2, { {3, 1} }, false);
        addPredefinedNode(3, {}, true);
        break;

    case 2:
        addPredefinedNode(1, { {2, 5} }, false);
        addPredefinedNode(2, { {1, 5}, {3, 5} }, false);
        addPredefinedNode(3, { {1, 1} }, false);
        break;

    case 3:
        addPredefinedNode(1, { {2, 5}, {3, 1} }, false);
        addPredefinedNode(2, { {3, 1} }, true);
        addPredefinedNode(3, { {1, 5} }, false);
        addPredefinedNode(4, { {3, 1}, {1, 5} }, false);
        break;
    }

    startPredefinedSimulation();
}

void CANSim::addPredefinedNode(int nodeId, const std::map<int, int>& messageAndFreq, bool error) 
{
    NodeConfigWidget* newNode = new NodeConfigWidget(nodeId, 0, 0, true);
    nodeWidgets.append(newNode);

    Node* node = new Node(nodeId, canBus);
    addNode(node, error);

    scene->addItem(newNode);

    newNode->setPos(nextXPosition, 600);
    nextXPosition += 150;

    for (const auto& target : messageAndFreq) {
        int targetId = target.first;
        int frequency = target.second;

        int interval = 60 / frequency;
        for (int sec = 0; sec < 60; sec += interval) {
            node->addNodesAndRound(sec, targetId);
            qDebug() << "Node " << nodeId << " will send a message to Node " << targetId << " at second " << sec;
        }

        node->generate11BitID();
    }

    QGraphicsLineItem* line1 = scene->addLine(newNode->x() + 25, newNode->y(), newNode->x() + 25, 100);
    QGraphicsLineItem* line2 = scene->addLine(newNode->x() + 75, newNode->y(), newNode->x() + 75, 200);

    changeLineColor(line1, Qt::white);
    changeLineColor(line2, Qt::white);

    canLines.append(line1);
    canLines.append(line2);
    nodeLinesHigh.append(line1);
    nodeLinesLow.append(line2);

    updateCANBusLines();
}

void CANSim::startPredefinedSimulation() 
{
    canBus->pendingMessages = collectAllMessages();

    createMessagePanel();
    processPendingMessages();

    QPushButton* seeLogFileButton = new QPushButton("See Log File", this);
    layout->addWidget(seeLogFileButton);

    connect(seeLogFileButton, &QPushButton::clicked, this, [this]() {
        QString logFilePath = "C:/Users/Maria\ Gozman-Pop/OneDrive/Desktop/Semester1/SCS_p/CANSim/log.txt";

        if (QFile::exists(logFilePath)) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(logFilePath));
        }
        else {
            QMessageBox::warning(this, "File Not Found", "The log file 'log.txt' does not exist in the current directory.");
        }
        });

    bool messagesPending;
    do {
        messagesPending = true;

        std::cout << "Round: " << canBus->getRound() << "\n";

        bool success = canBus->arbitrate();

        if (!canBus->hasPendingMessages()) {
            messagesPending = false;
        }

        if (success)
            canBus->incrementRound();

        if (canBus->getRound() > 60) {
            messagesPending = false;
        }

    } while (messagesPending);

    processSimulation();
}

void CANSim::initializeCustomConfiguration()
{
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    layout = new QVBoxLayout(centralWidget);

    graphicsView = new QGraphicsView(this);
    scene = new QGraphicsScene(this);
    graphicsView->setScene(scene);
    layout->addWidget(graphicsView);

    QGraphicsLineItem* canHighLine = scene->addLine(0, 100, 500, 100);
    QGraphicsLineItem* canLowLine = scene->addLine(0, 200, 500, 200); 

    changeLineColor(canHighLine, Qt::white);
    changeLineColor(canLowLine, Qt::white);

    canLines.append(canHighLine);
    canLines.append(canLowLine);

    QGraphicsTextItem* canHighLabel = scene->addText("CAN High");
    canHighLabel->setPos(10, 80); 

    QGraphicsTextItem* canLowLabel = scene->addText("CAN Low");
    canLowLabel->setPos(10, 180);  

    canBusLabel = scene->addText("CAN Bus");
    canBusLabel->setPos(250, 140); 

    canBus = new CANBus(this);

    NodeConfigWidget* exampleNode1 = new NodeConfigWidget(1, 0, 0, true);
	nodeWidgets.append(exampleNode1);

    NodeConfigWidget* exampleNode2 = new NodeConfigWidget(2, 0, 0, true);
    nodeWidgets.append(exampleNode2);

    scene->addItem(exampleNode1);
    scene->addItem(exampleNode2);

    Node* node1 = new Node(1, canBus);
    bool error = getRandomBool();
    addNode(node1, false);

    roundLabel = scene->addText("Round: -");
    roundLabel->setDefaultTextColor(Qt::white);
    roundLabel->setFont(QFont("Arial", 14));
    roundLabel->setPos(1, 10);  

    bitPositionLabel = scene->addText("Bit Position: -");
    bitPositionLabel->setDefaultTextColor(Qt::white);
    bitPositionLabel->setFont(QFont("Arial", 14));
    bitPositionLabel->setPos(1, 40);

    connect(exampleNode1, &NodeConfigWidget::nodeClicked, this,
        [this, node1](int nodeId) {
            openSendMessageDialog(nodeId, node1);
        });

    Node* node2 = new Node(2, canBus);
	error = getRandomBool();
    addNode(node2, true);

    connect(exampleNode2, &NodeConfigWidget::nodeClicked, this,
        [this, node2](int nodeId) {
            openSendMessageDialog(nodeId, node2);
        });

    exampleNode1->setPos(nextXPosition, 600);  
    nextXPosition += 150; 
    exampleNode2->setPos(nextXPosition, 600); 
    nextXPosition += 150; 

    QGraphicsLineItem* line1 = scene->addLine(exampleNode1->x() + 25, exampleNode1->y(), exampleNode1->x() + 25, 100);
    QGraphicsLineItem* line2 = scene->addLine(exampleNode1->x() + 75, exampleNode1->y(), exampleNode1->x() + 75, 200);  

    QGraphicsLineItem* line3 = scene->addLine(exampleNode2->x() + 25, exampleNode2->y(), exampleNode2->x() + 25, 100);  
    QGraphicsLineItem* line4 = scene->addLine(exampleNode2->x() + 75, exampleNode2->y(), exampleNode2->x() + 75, 200);  

    changeLineColor(line1, Qt::white);
    changeLineColor(line2, Qt::white);
    changeLineColor(line3, Qt::white);
    changeLineColor(line4, Qt::white);

    canLines.append(line1);
    canLines.append(line2);
    canLines.append(line3);
    canLines.append(line4);

    nodeLinesHigh.append(line1); 
    nodeLinesLow.append(line2); 
    nodeLinesHigh.append(line3); 
    nodeLinesLow.append(line4);   

    QCheckBox* bitStuffingCheckBox = new QCheckBox("Show Bit Stuffing", this);
    layout->addWidget(bitStuffingCheckBox);

    addNodeButton = new QPushButton("Add Node", this);
    layout->addWidget(addNodeButton);

    startSimButton = new QPushButton("Start Simulation", this);
    layout->addWidget(startSimButton);

    connect(bitStuffingCheckBox, &QCheckBox::stateChanged, this, [this](int state) {
        canBus->bitStuffingVisible = (state == Qt::Checked); 
        if (state == Qt::Checked) {
            QMessageBox::information(this, "Bit Stuffing", "Bit stuffing is now visible.");
        }
        else {
            QMessageBox::information(this, "Bit Stuffing", "Bit stuffing is now hidden.");
        }
        });

    connect(startSimButton, &QPushButton::clicked, this, [this]() {
        simulationStarted = true;
		addMessage = false;
        startSimButton->setEnabled(false);
        addNodeButton->setEnabled(false);

        canBus->pendingMessages = collectAllMessages();

        createMessagePanel();
        processPendingMessages();

        QPushButton* seeLogFileButton = new QPushButton("See Log File", this);
        layout->addWidget(seeLogFileButton);

        connect(seeLogFileButton, &QPushButton::clicked, this, [this]() {
            QString logFilePath = "C:/Users/Maria\ Gozman-Pop/OneDrive/Desktop/Semester1/SCS_p/CANSim/log.txt";

            if (QFile::exists(logFilePath)) {
                QDesktopServices::openUrl(QUrl::fromLocalFile(logFilePath));
            }
            else {
                QMessageBox::warning(this, "File Not Found", "The log file 'log.txt' does not exist in the current directory.");
            }
            });

        bool messagesPending;
        do {
            messagesPending = true;

            std::cout << "Round: " << canBus->getRound() << "\n";

            bool success = canBus->arbitrate();

            if (!canBus->hasPendingMessages()) {
                messagesPending = false;
            }

            if (success)
                canBus->incrementRound();

			if (canBus->getRound() > 60) {
				messagesPending = false;
			}

        } while (messagesPending);

        processSimulation();

        });

    connect(addNodeButton, &QPushButton::clicked, this, [this]() {
        if (simulationStarted) {
            return; 
        }

        if (nodesInSim.size() >= 8) {
            QMessageBox::warning(this, "Max Nodes Reached", "You cannot add more than 8 nodes.");
            return;
        }

        static int nodeId = 3; 
        NodeConfigWidget* newNode = new NodeConfigWidget(nodeId, 0, 0, true);
        nodeWidgets.append(newNode);

        Node* node = new Node(nodeId, canBus);
		bool error = getRandomBool();
        addNode(node, error);

        scene->addItem(newNode);

        connect(newNode, &NodeConfigWidget::nodeClicked, this,
            [this, node](int nodeId) {
                openSendMessageDialog(nodeId, node);
            });

        newNode->setPos(nextXPosition, 600);  
        nextXPosition += 150;  

        QGraphicsLineItem* line1 = scene->addLine(newNode->x() + 25, newNode->y(), newNode->x() + 25, 100);  
        QGraphicsLineItem* line2 = scene->addLine(newNode->x() + 75, newNode->y(), newNode->x() + 75, 200); 

        changeLineColor(line1, Qt::white);
        changeLineColor(line2, Qt::white);

        canLines.append(line1);
        canLines.append(line2);

        nodeLinesHigh.append(line1);  
        nodeLinesLow.append(line2);  

        updateCANBusLines();

        nodeId++;
        });
}

CANSim::~CANSim()
{
    delete graphicsView;
    delete scene;
}

//void CANSim::processSimulation() {
//
//    int counter = 0;
//    for (const auto& step : canBus->arbitrationSteps) {
//        // Print the arbitration details for the current step
//        if (counter % 11 == 0)
//        {
//            qDebug() << "Arbitration step for round:" << step.round;
//        }
//
//        qDebug() << "-----";
//        qDebug() << "Bit Position:" << step.bitPosition;
//
//        // Print contenders and their bit values for the current step
//        qDebug() << "---";
//        qDebug() << "Contenders for round" << step.round << ":";
//        for (const auto& msg : step.contenders) {
//            uint16_t id = msg.getId();
//            qDebug() << "Message ID: " << id << " Bit: " << ((id >> (11 - 1)) & 1);  // Example: show the bit value
//        }
//
//        // Print non-contenders for the current step
//        qDebug() << "---";
//        qDebug() << "Non-Contenders for round" << step.round << ":";
//        for (const auto& msg : step.nonContenders) {
//            qDebug() << "Message ID: " << msg.getId();
//        }
//
//        qDebug() << "---";
//        if (step.bitPosition == 0) {
//            qDebug() << "Winner for round" << step.round << ":";
//            Message winner = canBus->winners.front();
//			markMessageAsSent(winner.getSenderId(), winner.getId(), winner.getRound());
//            canBus->winners.erase(canBus->winners.begin());
//            qDebug() << "Winner Message ID: " << winner.getId();
//            qDebug() << "---------------------------";
//        }
//
//        counter++;
//    }
//}

void CANSim::processSimulation()
{
    stepCounter = 0;
    roundCount = 0;

    simulationTimer = new QTimer(this);
    simulationTimer->start(500);

    connect(simulationTimer, &QTimer::timeout, this, [this]() {

        //qDebug() << "Step Counter: " << stepCounter;
        //qDebug() << "Round Count: " << roundCount;

        nodesInSim = canBus->nodes;

        bool errorSent = false;

        if (stepCounter < canBus->arbitrationSteps.size()) {
            const auto& step = canBus->arbitrationSteps[stepCounter];

            int i = 0;
            for (const auto& node : nodeWidgets) {
                Node nodeStep = step.nodes.at(step.round).at(i).front();
                int tecCount = nodeStep.getTEC();
                int recCount = nodeStep.getREC();

                node->setTECCount(tecCount);
                node->setRECCount(recCount);

                if (nodeStep.nodeActive == false) {
                    node->setNodeActive(false);
                    errorSent = true;
                }

                i++;
            }

            if (roundCount < step.round) {
                setAllLinesToWhite();
                roundLabel->setPlainText(QString("Round (event/second): %1").arg(roundCount));
                //qDebug() << "Round " << roundCount << " has no arbitration steps.";
                roundCount++;
                return;
            }

            setAllLinesToWhite();

            dom = false;
            rec = false;

            roundLabel->setPlainText(QString("Round (event/second): %1").arg(step.round));

            for (const auto& msg : step.contenders)
            {
                bitPositionLabel->setPlainText(QString("Bit Position: %1").arg(step.bitPosition));

                uint16_t bitValue = step.roundContenderBitValues.at(step.round).at(msg.getSenderId()).front();

                int senderId = msg.getSenderId() - 1;

                if (bitValue == 1) {
                    rec = true;
                    toggleLineColorRec(nodeLinesLow[senderId], nodeLinesHigh[senderId]);
                    //qDebug() << "Node " << senderId + 1 << " is recessive.";
                }
                else {
                    rec = false;
                    dom = true;
                    toggleLineColorDom(nodeLinesLow[senderId], nodeLinesHigh[senderId]);
                    //qDebug() << "Node " << senderId + 1 << " is dominant.";
                }
            }

            /*for (const auto& msg : step.nonContenders) {
                qDebug() << "Message ID: " << msg.getId();
            }*/

            if (step.bitPosition == 0)
            {
                Message winner = canBus->winners.front();
                if (winner.getId() != 0)
                {
                    markMessageAsSent(winner.getSenderId(), winner.getId(), winner.getRound());
                    
					uint16_t message_id = winner.getId();
                    int winner_round = winner.getRound();
                    for (int i = 0; i < 8; i++)
                    {
                        if ((message_id >> i) & 1)
                        {
                            std::vector<Message*> receivedMessages = canBus->nodes[i]->getReceivedMessages();
                            if (receivedMessages.empty())
                            {
                                qDebug() << "EMPTY FOR NODE " << canBus->nodes[i]->getNodeId();
                            }
                            for (const auto& msg : receivedMessages) {
								//qDebug() << "Message ID: " << msg->getId() << " Round: " << msg->getRound();
                                if (msg->getRound() == winner_round && msg->getId() == message_id) {
                                    if (nodeWidgets[i]->isActive()) {
                                        nodeWidgets[i]->receiveMessage(message_id);
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
                canBus->winners.erase(canBus->winners.begin());
            }

            stepCounter++;
            roundCount = step.round;
        }
        else
        {
            setAllLinesToWhite();
            roundLabel->setPlainText(QString("Round (event/second): %1").arg(roundCount));
            roundCount++;

            if (roundCount >= 60) {
                qDebug() << "Simulation complete.";
                QMessageBox::information(this, "Simulation Complete", "The simulation has successfully completed!");
                simulationTimer->stop();
            }
        }
        });
}

void CANSim::createMessagePanel()
{
    QDockWidget* messageDockWidget = new QDockWidget("Message Status", this);
    messageDockWidget->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, messageDockWidget);

    QWidget* messagePanelWidget = new QWidget(messageDockWidget);
    QVBoxLayout* panelLayout = new QVBoxLayout(messagePanelWidget);

    pendingMessagesTable = new QTableWidget(0, 3, this);
    pendingMessagesTable->setHorizontalHeaderLabels({ "Node ID", "Message ID", "Initial Round" });
    pendingMessagesTable->setObjectName("pendingMessagesTable");
    panelLayout->addWidget(new QLabel("Pending Messages"));
    panelLayout->addWidget(pendingMessagesTable);

    sentMessagesTable = new QTableWidget(0, 3, this);
    sentMessagesTable->setHorizontalHeaderLabels({ "Node ID", "Message ID", "Initial Round" });
    sentMessagesTable->setObjectName("sentMessagesTable");
    panelLayout->addWidget(new QLabel("Sent Messages"));
    panelLayout->addWidget(sentMessagesTable);

    messagePanelWidget->setLayout(panelLayout);
    messageDockWidget->setWidget(messagePanelWidget);
}

void CANSim::addPendingMessage(int nodeId, uint16_t identifier, int round)
{
    int row = pendingMessagesTable->rowCount();
    pendingMessagesTable->insertRow(row);

    pendingMessagesTable->setItem(row, 0, new QTableWidgetItem(QString::number(nodeId)));

    QString binaryIdentifier = QString::number(identifier, 2).rightJustified(11, '0');
    pendingMessagesTable->setItem(row, 1, new QTableWidgetItem(binaryIdentifier));

    pendingMessagesTable->setItem(row, 2, new QTableWidgetItem(QString::number(round)));
}

void CANSim::processPendingMessages()
{
    for (Node* node : nodesInSim) {
        const std::vector<Message*> messages = node->getMessagesToBeSent();

        for (const auto& message : messages) {
            int round = message->getRound();
            int senderNodeId = node->getNodeId();
            uint16_t identifier = message->getId();

            addPendingMessage(senderNodeId, identifier, round);
        }
    }
}

void CANSim::markMessageAsSent(int nodeId, uint16_t messageId, int round)
{
    for (int row = 0; row < pendingMessagesTable->rowCount(); ++row) {
        if (pendingMessagesTable->item(row, 0)->text().toInt() == nodeId &&
            pendingMessagesTable->item(row, 1)->text().toInt(nullptr, 2) == messageId &&
            pendingMessagesTable->item(row, 2)->text().toInt() == round) {

            QString binaryIdentifier = QString::number(messageId, 2).rightJustified(11, '0');

            int sentRow = sentMessagesTable->rowCount();
            sentMessagesTable->insertRow(sentRow);
            sentMessagesTable->setItem(sentRow, 0, new QTableWidgetItem(QString::number(nodeId)));
            sentMessagesTable->setItem(sentRow, 1, new QTableWidgetItem(binaryIdentifier));
            sentMessagesTable->setItem(sentRow, 2, new QTableWidgetItem(QString::number(round)));

            pendingMessagesTable->removeRow(row);

            break;
        }
    }

    pendingMessagesTable->viewport()->update();
    sentMessagesTable->viewport()->update();

}

void CANSim::changeLineColor(QGraphicsLineItem* line, const QColor& color)
{
    QPen pen = line->pen();
    pen.setColor(color);  
    line->setPen(pen);  
}

void CANSim::updateCANBusLines()
{
    canLines[0]->setLine(0, 100, nextXPosition + 50, 100);
    canLines[1]->setLine(0, 200, nextXPosition + 50, 200); 

    int centerY = (100 + 200) / 2; 
    canBusLabel->setPos(nextXPosition / 2 - 50, centerY - 10);
}

void CANSim::toggleLineColorDom(QGraphicsLineItem* canl, QGraphicsLineItem* canh)
{
    changeLineColor(canl, Qt::yellow);
    changeLineColor(canh, Qt::yellow);
    changeLineColor(canLines[0], Qt::yellow);
    changeLineColor(canLines[1], Qt::yellow);
}

void CANSim::toggleLineColorRec(QGraphicsLineItem* canl, QGraphicsLineItem* canh)
{
    changeLineColor(canl, Qt::red);
    changeLineColor(canh, Qt::green);
    if (rec == true && dom == false) {
        changeLineColor(canLines[1], Qt::red);
        changeLineColor(canLines[0], Qt::green);
    }
}

// added for dem
//void CANSim::toggleRandomLineColor()
//{
//    int random = QRandomGenerator::global()->bounded(5);
//
//    if (!nodeLinesHigh.isEmpty() && !nodeLinesLow.isEmpty()) {
//        if (random == 0)
//        {
//            dom = true;
//            int randomIndex = QRandomGenerator::global()->bounded(nodeLinesHigh.size()-1);
//            QGraphicsLineItem* lineHigh = nodeLinesHigh[randomIndex];
//            QGraphicsLineItem* lineLow = nodeLinesLow[randomIndex];
//            toggleLineColorDom(lineHigh, lineLow);
//            dom = false;
//        }
//        else if (random == 1)
//        {
//            rec = true;
//            dom = false;
//            int randomIndex = QRandomGenerator::global()->bounded(nodeLinesHigh.size()-1);
//            QGraphicsLineItem* lineHigh = nodeLinesHigh[randomIndex];
//            QGraphicsLineItem* lineLow = nodeLinesLow[randomIndex];
//            toggleLineColorRec(lineHigh, lineLow);
//            rec = false;
//        }
//        else if (random == 2)
//        {
//            dom = true;
//            int randomIndex = QRandomGenerator::global()->bounded(nodeLinesHigh.size()-1);
//            QGraphicsLineItem* lineHigh = nodeLinesHigh[randomIndex];
//            QGraphicsLineItem* lineLow = nodeLinesLow[randomIndex];
//            toggleLineColorDom(lineHigh, lineLow);
//
//            randomIndex = QRandomGenerator::global()->bounded(nodeLinesHigh.size()-1);
//            lineHigh = nodeLinesHigh[randomIndex];
//            lineLow = nodeLinesLow[randomIndex];
//            toggleLineColorRec(lineHigh, lineLow);
//        }
//        else
//        {
//            rec = true;
//            dom = false;
//            int randomIndex = QRandomGenerator::global()->bounded(nodeLinesHigh.size()-1);
//            QGraphicsLineItem* lineHigh = nodeLinesHigh[randomIndex];
//            QGraphicsLineItem* lineLow = nodeLinesLow[randomIndex];
//            toggleLineColorRec(lineHigh, lineLow);
//
//            randomIndex = QRandomGenerator::global()->bounded(nodeLinesHigh.size()-1);
//            lineHigh = nodeLinesHigh[randomIndex];
//            lineLow = nodeLinesLow[randomIndex];
//            toggleLineColorRec(lineHigh, lineLow);
//        }
//    }
//}

void CANSim::setAllLinesToWhite()
{
    dom = false;
    rec = false;
    for (QGraphicsLineItem* line : canLines) {
        changeLineColor(line, Qt::white);
    }

    for (QGraphicsLineItem* line : nodeLinesHigh) {
        changeLineColor(line, Qt::white);
    }
    for (QGraphicsLineItem* line : nodeLinesLow) {
        changeLineColor(line, Qt::white);
    }
	for (NodeConfigWidget* node : nodeWidgets) {
        node->messageReceived = false;
		node->update();
	}
}

//void CANSim::resetLinesAndToggleColor()
//{
//    //necessary
//    setAllLinesToWhite();
//
//    if (pendingMessagesTable->rowCount() == 0) {
//        simulationTimer->stop();  // Stop the simulation timer when no more messages are pending
//        return;  // Exit the function to prevent further toggling
//    }
//
//    //added for dem
//    toggleCount++;
//    if (toggleCount % 4 == 0) {
//        int rowCount = pendingMessagesTable->rowCount();
//
//        if (rowCount > 0) {
//            // Select a random pending message to mark as sent
//            int randomRow = QRandomGenerator::global()->bounded(rowCount);
//
//            // Retrieve the message details from the selected row
//            int nodeId = pendingMessagesTable->item(randomRow, 0)->text().toInt();
//            int messageId = pendingMessagesTable->item(randomRow, 1)->text().toInt();
//            bool errorDetected = QRandomGenerator::global()->bounded(2);
//
//            // Move the message to "Sent Messages" table
//            //markMessageAsSent(nodeId, messageId, errorDetected);
//        }
//    }
//
//    QTimer::singleShot(500, this, &CANSim::toggleRandomLineColor); 
//}

void CANSim::openSendMessageDialog(int senderNodeId, Node* node) {
    QVector<int> availableNodes;
    for (auto nodeItem : scene->items()) {
        NodeConfigWidget* node = dynamic_cast<NodeConfigWidget*>(nodeItem);
        if (node) {
            availableNodes.append(node->getNodeId());
        }
    }

    MessageDialog* dialog = new MessageDialog(senderNodeId, availableNodes, node, this);

    connect(dialog, &MessageDialog::messageReady, this, [this](int senderNodeId, int messageId, QVector<int> targetNodeIds) {
        messageList.append(qMakePair(senderNodeId, messageId));
    });

    dialog->exec();
}

void CANSim::addNode(Node* node, bool error) {
    if (error){
		node->setError(true);
    }
    else {
        node->setError(false);
    }
    node->setNodeActive(true);
    nodesInSim.push_back(node);
}

std::vector<Node*>&CANSim::getNodes() {
    return nodesInSim;
}

Node* CANSim::findNodeById(int id) {
    for (auto nodePtr : nodesInSim) { 
        if (nodePtr->getNodeId() == id) {  
            return nodePtr; 
        }
    }
    return nullptr;
}

std::vector<Message> CANSim::collectAllMessages() const {
    std::vector<Message> allMessages;

    for (Node* node : nodesInSim) {
        const auto& messagesAndRounds = node->getMessagesToBeSent();

        for (const auto& roundMessagePair : messagesAndRounds) {
            const Message* msg = roundMessagePair;
            if (msg) {
                allMessages.push_back(*msg);
            }
        }
    }

    return allMessages;
}

bool CANSim::getRandomBool() {
    std::random_device rd;  
    std::mt19937 gen(rd()); 
    std::uniform_int_distribution<> dist(0, 1); 
    return dist(gen);
}

//void CANSim::printNodesAndRounds() {
//    // Iterate through each node in the simulation
//    for (Node* node : nodesInSim) {
//        // Print the node ID in debug output
//        qDebug() << "Node ID:" << node->getNodeId();
//
//        // Get the nodesAndRounds map for this node
//        const std::map<int, std::vector<int>>& nodesAndRounds = node->getNodesAndRounds();
//
//        // Iterate through the map (rounds -> nodes)
//        for (auto it = nodesAndRounds.begin(); it != nodesAndRounds.end(); ++it) {
//            int second = it->first;  // The round (second)
//            const std::vector<int>& nodeIds = it->second;  // List of node IDs in this round
//
//            // Print the second (round) and node IDs in debug output
//            QString nodeIdsString = "Nodes -> ";
//            for (int id : nodeIds) {
//                nodeIdsString += QString::number(id) + " ";
//            }
//
//            qDebug() << "  Second" << second << ":" << nodeIdsString;
//        }
//    }
//}