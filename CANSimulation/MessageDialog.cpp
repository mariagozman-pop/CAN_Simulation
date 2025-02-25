#include "MessageDialog.h"

#include <algorithm>

#include "Node.h"

MessageDialog::MessageDialog(int senderNodeId, const QVector<int>& availableNodes, Node* node, QWidget* parent)
    : QDialog(parent), senderNodeId(senderNodeId), availableNodes(availableNodes), node(node)
{
    setWindowTitle("Set Node Message Frequencies");

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Set message sending frequency (messages/min):"));

    for (int nodeId : availableNodes) {
        if (nodeId != senderNodeId) {
            QLabel* label = new QLabel(QString("Node %1").arg(nodeId), this);
            layout->addWidget(label);

            QComboBox* frequencyBox = new QComboBox(this);
            frequencyBox->addItems({
                "None",
                "1 message/min",
                "5 messages/min",
                "10 messages/min",
                "15 messages/min",
                "30 messages/min",
                "60 messages/min"
                });
            layout->addWidget(frequencyBox);

            frequencySelectors[nodeId] = frequencyBox;
        }
    }

    QPushButton* setButton = new QPushButton("Set Frequencies", this);
    layout->addWidget(setButton);

    connect(setButton, &QPushButton::clicked, this, &MessageDialog::onSetButtonClicked);
}

void MessageDialog::onSetButtonClicked() {
    QMap<int, int> nodeFrequencies;

    for (auto it = frequencySelectors.begin(); it != frequencySelectors.end(); ++it) {
        int nodeId = it.key();
        QString selectedFrequency = it.value()->currentText();

        int frequency = 0;  

        if (selectedFrequency == "None") {
            frequency = 0;  
        }
        else if (selectedFrequency == "1 message/min") {
            frequency = 1;  
        }
        else if (selectedFrequency == "5 messages/min") {
            frequency = 5;  
        }
        else if (selectedFrequency == "10 messages/min") {
            frequency = 10; 
        }
        else if (selectedFrequency == "15 messages/min") {
            frequency = 15; 
        }
        else if (selectedFrequency == "30 messages/min") {
            frequency = 30; 
        }
        else if (selectedFrequency == "60 messages/min") {
            frequency = 60; 
        }

        if (frequency > 0) {

            nodeFrequencies[nodeId] = frequency;

            int interval = 60 / frequency;
            for (int sec = 0; sec < 60; sec += interval) {
                node->addNodesAndRound(sec, nodeId);
            }

			node->generate11BitID();
        }
    }

    emit frequenciesSet(senderNodeId, nodeFrequencies);

    accept();
}
