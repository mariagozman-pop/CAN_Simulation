#ifndef NODEWIDGETCONFIG_H
#define NODEWIDGETCONFIG_H

#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

class NodeConfigWidget : public QGraphicsObject
{
    Q_OBJECT

public:
    NodeConfigWidget(int nodeId, int tecCount = 0, int recCount = 0, bool active = true, QGraphicsObject* parent = nullptr);
    ~NodeConfigWidget();

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    int getNodeId() const;
    void setTECCount(int count);
    void setRECCount(int count);
    void setNodeActive(bool active);
    void receiveMessage(uint16_t id);

    bool messageReceived = false;

signals:
    void nodeClicked(int nodeId);

private:
    int nodeId;
    int tecCount;
    int recCount;
    bool nodeActive;
	uint16_t receivedMessageId;
};

#endif 