#include "nodeitem.h"
#include <QMenu>
#include <QAction>
#include <QInputDialog>

#include "nodeitem.h"
#include "graphscene.h"
#include "nodepropertydialog.h"
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QDebug>





NodeItem::NodeItem(const QPixmap &pixmap, QString &nodename) : QGraphicsPixmapItem(pixmap) {
    setFlags(ItemIsMovable | ItemIsSelectable);

    setZValue(10);

    label = new QGraphicsTextItem(nodename, this);
    label->setFlags(ItemIsSelectable | ItemIsMovable);
    label->setDefaultTextColor(Qt::black);
    label->setPos(pixmap.width() / 2 - label->boundingRect().width() / 2,
                  pixmap.height() + 5);

}

NodeItem::~NodeItem() {
//    //qDebug() << "NodeItem 被安全销毁：" << static_cast<void*>(this);
    this->setEnabled(false);
    this->setVisible(false);
    this->setAcceptHoverEvents(false);
}

QRectF NodeItem::boundingRect() const {
//    //qDebug() << "boundingRect() 被调用：" << static_cast<const void*>(this);

    return QGraphicsPixmapItem::boundingRect();
}

void NodeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
//    //qDebug() << "paint() 被调用：" << static_cast<void*>(this);
    QGraphicsPixmapItem::paint(painter, option, widget);
}

void NodeItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
//    //qDebug() << "mousePressEvent() 被调用：" << static_cast<void*>(this);
    QGraphicsPixmapItem::mousePressEvent(event);
}

void NodeItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    Q_UNUSED(event);

    int colcount=NodeList[live_ID].colcount;

    NodePropertyDialog dialog(colcount, NodeList[live_ID].cs);

    dialog.setLabel(this->getLabel()->toPlainText());
    dialog.setGoodsWants(NodeList[live_ID].goods_wants[0]);

    if (NodeList[live_ID].cs) {
        //qDebug()<<"只有是 client 时才设置 hope_times";
        dialog.setHopeTimes(NodeList[live_ID].hope_times[0]);
    }

    if (dialog.exec() == QDialog::Accepted) {
        this->getLabel()->setPlainText(dialog.label());
        NodeList[live_ID].goods_wants[0] = dialog.goodsWants();
        if (NodeList[live_ID].cs) {
            NodeList[live_ID].hope_times[0] = dialog.hopeTimes();
        }
    }
}

void NodeItem::DCE(){

    int colcount=NodeList[live_ID].colcount;

    NodePropertyDialog dialog(colcount, NodeList[live_ID].cs);

    dialog.setLabel(this->getLabel()->toPlainText());
    dialog.setGoodsWants(NodeList[live_ID].goods_wants[0]);

    if (NodeList[live_ID].cs) {
        //qDebug()<<"只有是 client 时才设置 hope_times";
        dialog.setHopeTimes(NodeList[live_ID].hope_times[0]);
    }

    if (dialog.exec() == QDialog::Accepted) {
        this->getLabel()->setPlainText(dialog.label());
        if (NodeList[live_ID].cs) {
            NodeList[live_ID].hope_times[0] = dialog.hopeTimes();
        }
        NodeList[live_ID].goods_wants[0] = dialog.goodsWants();

    }
}

void NodeItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    QMenu menu;
    QAction *deleteAction = menu.addAction("删除节点");
    QAction *selectedAction = menu.exec(event->screenPos());

    if (selectedAction == deleteAction) {
        if (auto scene = dynamic_cast<GraphScene *>(this->scene())) {
            scene->removeNode(this);
            live=false;
            for(auto &xDV : Data_V[live_ID]){
                xDV.takedown();
            }
            for(int i=0;i<ID;i++){
                //qDebug()<<ID;
                Data_V[i][live_ID].takedown();
            }
            NodeList[live_ID].live=false;
        }
    }
    //qDebug()<<"12";
}

