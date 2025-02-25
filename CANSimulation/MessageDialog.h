#ifndef MESSAGE_DIALOG_H
#define MESSAGE_DIALOG_H

#include <QDialog>
#include <QVector>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QPair>
#include <QComboBox>
#include <QMap>

#include "Node.h"

class MessageDialog : public QDialog {
    Q_OBJECT

public:
    explicit MessageDialog(int senderNodeId, const QVector<int>& availableNodes, Node* node, QWidget* parent = nullptr);

signals:
    void messageReady(int senderNodeId, int messageId, QVector<int> targetNodeIds);
    void frequenciesSet(int senderNodeId, const QMap<int, int>& nodeFrequencies);

private slots:
    void onSetButtonClicked();

private:
    int senderNodeId;
    QVector<int> availableNodes;
    QVector<QCheckBox*> checkboxes;
    QMap<int, QComboBox*> frequencySelectors;
    Node* node;
};

#endif