#ifndef GRAPHSCENE_H
#define GRAPHSCENE_H

#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QMap>
#include "nodeitem.h"
#include "edgeitem.h"
#include "graphview.h"

class GraphScene : public QGraphicsScene {
    Q_OBJECT

public:
    GraphScene(QObject *parent = nullptr);
    void removeEdge(NodeItem* sourceNode, NodeItem* targetNode);
    void removeNode(NodeItem *node);
    void adjustSceneRect() ;
    void updateView(QGraphicsView *view);
    void update(const QRectF &rect) ;
    void adjustSceneRect(GraphView *view);
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;


private:
    NodeItem *selectedNode;
    QGraphicsLineItem *tempLine;
    QMap<QPair<NodeItem *, NodeItem *>, EdgeItem *> connections;
    void mousePaint(QGraphicsSceneMouseEvent *event);

};

extern GraphScene* S_Catch;

extern int carriage;

extern int warehouseCount;
extern int clientCount;

#endif // GRAPHSCENE_H


