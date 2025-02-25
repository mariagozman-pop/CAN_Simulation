#ifndef CAN_SIM_H
#define CAN_SIM_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPushButton>
#include <QGraphicsLineItem>
#include <QTimer>
#include <QVector>
#include <QTableWidget>
#include <QLabel>
#include <QVBoxLayout>

#include "NodeConfigWidget.h"
#include "Node.h"
#include "CANBus.h"

class CANSim : public QMainWindow
{
    Q_OBJECT

public:
    explicit CANSim(QWidget* parent = nullptr);
    ~CANSim();

    QList<QPair<int, int>> messageList;
    std::vector<Node*> nodesInSim;
    CANBus* canBus;

    void markMessageAsSent(int nodeId, uint16_t messageId, int round);
    void setAllLinesToWhite();
    void toggleLineColorDom(QGraphicsLineItem* canl, QGraphicsLineItem* canh); 
    void toggleLineColorRec(QGraphicsLineItem* canl, QGraphicsLineItem* canh); 

public slots:
    void openSendMessageDialog(int nodeId, Node* node);

private:
    void changeLineColor(QGraphicsLineItem* line, const QColor& color);
    void updateCANBusLines();
    void createMessagePanel();
    void addPendingMessage(int nodeId, uint16_t messageId, int round);
	void processPendingMessages(); 
    void addNode(Node* node, bool error);
    std::vector<Node*>& getNodes();
    Node* findNodeById(int id);
    std::vector<Message> collectAllMessages() const;
    void initializeCustomConfiguration();
    void processSimulation();
    void createWelcomeScreen();
    void selectPredefinedScenario();
    void setupPredefinedScenario(int scenario);
	void addPredefinedNode(int nodeId, const std::map<int, int>& messageAndFreq, bool error);
    void startPredefinedSimulation();
    bool getRandomBool();

    QGraphicsView* graphicsView;                  
    QGraphicsScene* scene;                        
    QPushButton* startSimButton;                  
    QPushButton* addNodeButton;                   
    QGraphicsTextItem* canBusLabel; 
	QVector<NodeConfigWidget*> nodeWidgets;
    QVector<QGraphicsLineItem*> canLines;         
    QVector<QGraphicsLineItem*> nodeLinesHigh;   
    QVector<QGraphicsLineItem*> nodeLinesLow;     
    QTimer* simulationTimer;                     
    QTableWidget* pendingMessagesTable;   
    QTableWidget* sentMessagesTable;
    QGraphicsTextItem* roundLabel;
    QGraphicsTextItem* bitPositionLabel;
    QVBoxLayout* layout;
    int nextXPosition;                           
    bool simulationStarted;                   
    bool dom = false;
    bool rec = false;
    bool addMessage = true;
    int stepCounter;
    int roundCount;
};

#endif