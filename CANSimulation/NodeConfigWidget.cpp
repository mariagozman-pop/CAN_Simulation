#include "NodeConfigWidget.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <bitset>
#include <string>

NodeConfigWidget::NodeConfigWidget(int nodeId, int tecCount, int recCount, bool active, QGraphicsObject* parent)
    : QGraphicsObject(parent), nodeId(nodeId), tecCount(tecCount), recCount(recCount), nodeActive(active)
{
    setFlag(QGraphicsObject::ItemIsSelectable);
    setFlag(QGraphicsObject::ItemIsFocusable);
    setFlag(QGraphicsObject::ItemIsMovable);
}

NodeConfigWidget::~NodeConfigWidget() {}

QRectF NodeConfigWidget::boundingRect() const
{
    return QRectF(0, 0, 110, 70);
}

void NodeConfigWidget::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (messageReceived && nodeActive) 
    {

        painter->setBrush(Qt::blue);
        painter->drawRect(boundingRect());

        painter->setPen(Qt::white);
        painter->setFont(QFont("Arial", 14, QFont::Bold));
        painter->drawText(QRectF(0, 10, 110, 20), Qt::AlignCenter, QString("ID: %1").arg(nodeId));

        painter->setPen(Qt::white);
        painter->setFont(QFont("Arial", 14));
        QString bitString = QString::fromStdString(std::bitset<11>(receivedMessageId).to_string());
        painter->drawText(QRectF(0, 30, 110, 15), Qt::AlignCenter, QString("Message:"));
        painter->setFont(QFont("Arial", 12));
        painter->drawText(QRectF(0, 45, 110, 15), Qt::AlignCenter, bitString);
    }
    else
    {
        painter->setBrush(Qt::lightGray);
        painter->drawRect(boundingRect());

        painter->setPen(Qt::white);
        painter->setFont(QFont("Arial", 14, QFont::Bold));
        painter->drawText(QRectF(0, 10, 110, 20), Qt::AlignCenter, QString("ID: %1").arg(nodeId));

        painter->setPen(Qt::black);
        painter->setFont(QFont("Arial", 10));
        painter->setBrush(Qt::white);
        painter->drawRect(5, 40, 50, 20);
        painter->drawText(QRectF(5, 40, 50, 20), Qt::AlignCenter, QString("TEC: %1").arg(tecCount));

        painter->drawRect(55, 40, 50, 20);
        painter->drawText(QRectF(55, 40, 50, 20), Qt::AlignCenter, QString("REC: %1").arg(recCount));

        if (!nodeActive) {

            painter->setPen(QPen(Qt::red, 3));
            painter->setBrush(Qt::NoBrush);
            painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
            painter->drawLine(0, 0, 110, 70);
            painter->drawLine(0, 70, 110, 0);
        }
    }
}

void NodeConfigWidget::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit nodeClicked(nodeId);
        event->accept();
    }
}

int NodeConfigWidget::getNodeId() const
{
    return nodeId;
}

void NodeConfigWidget::setTECCount(int count)
{
    tecCount = count;
    update();
}

void NodeConfigWidget::setRECCount(int count)
{
    recCount = count;
    update();
}

void NodeConfigWidget::setNodeActive(bool active)
{
    nodeActive = active;
    update();
}

void NodeConfigWidget::receiveMessage(uint16_t id)
{
    messageReceived = true;
    receivedMessageId = id; 
    update();
}