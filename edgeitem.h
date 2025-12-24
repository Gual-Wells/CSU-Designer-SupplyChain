#ifndef EDGEITEM_H
#define EDGEITEM_H

#include <QGraphicsLineItem>
#include <QPainterPathStroker>
#include "nodeitem.h"
#include "edgedata.h"

class GraphScene;

class EdgeItem : public QGraphicsLineItem {
public:
    EdgeItem(GraphScene* scene, NodeItem* start, NodeItem* end);
    EdgeItem(GraphScene* scene, NodeItem* start, NodeItem* end,int nothingjust1);
    ~EdgeItem();

    void updatePosition();
    bool safelock=false;
    int AID;
    int BID;


protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private:
    GraphScene* scene;
    NodeItem* startNode;
    NodeItem* endNode;
};
extern bool calledge;

#endif // EDGEITEM_H
