#ifndef NODEITEM_H
#define NODEITEM_H

#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <QGraphicsSceneContextMenuEvent>
#include <QObject>  // 让 NodeItem 支持 deleteLater()
#include "edgepropertydialog.h"
#include "nodedata.h"

class NodeItem : public QObject, public QGraphicsPixmapItem {
        Q_OBJECT
public:
    NodeItem(const QPixmap &pixmap, QString &nodename);
    ~NodeItem();
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    QGraphicsTextItem* getLabel() const { return label; }
    int live_ID;
    bool live;
    bool cs;
    void DCE();

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QGraphicsTextItem *label;
};

extern QVector<NodeItem*>NodeIList;
#endif // NODEITEM_H

