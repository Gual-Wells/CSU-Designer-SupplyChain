#include "edgeitem.h"
#include "graphscene.h"
#include "edgepropertydialog.h"
#include "nodedata.h"
#include <QMenu>
#include <QPainter>
#include <QDebug>

EdgeItem::EdgeItem(GraphScene* scene, NodeItem* start, NodeItem* end)
    : scene(scene), startNode(start), endNode(end) {
    safelock=false;
    setPen(QPen(Qt::black, 2));
    setFlags(ItemIsSelectable);
    updatePosition();

    EdgePropertyDialog dialog;
    if (dialog.exec() == QDialog::Accepted) {
        //qDebug()<<start->live_ID<<end->live_ID;
        //qDebug()<<"ED定位于"<<start->live_ID<<"与"<<end->live_ID;
        Data_V[start->live_ID][end->live_ID].setedgedata(dialog.length(),dialog.isHighway(),dialog.price(),dialog.highwayPrice());
        //qDebug()<<"更新边表";
        Data_V[end->live_ID][start->live_ID].setedgedata(dialog.length(),dialog.isHighway(),dialog.price(),dialog.highwayPrice());
        AID=start->live_ID;
        BID=end->live_ID;
        safelock=true;
        Data_V[AID][BID].live=true;
        Data_V[BID][AID].live=true;
    }
    else{
        AID=start->live_ID;
        BID=end->live_ID;
        safelock=true;
        Data_V[AID][BID].live=true;
        Data_V[BID][AID].live=true;
    }

}

EdgeItem::EdgeItem(GraphScene* scene, NodeItem* start, NodeItem* end,int nothingjust1)
    : scene(scene), startNode(start), endNode(end) {
    safelock=false;
    setPen(QPen(Qt::black, 2));
    setFlags(ItemIsSelectable);
    updatePosition();

    AID=start->live_ID;
    BID=end->live_ID;
    safelock=true;


}

EdgeItem::~EdgeItem() {
    if (scene) {
        scene->removeEdge(startNode, endNode);
    }
}

void EdgeItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    Q_UNUSED(event);

    EdgePropertyDialog dialog;

    if (dialog.exec() == QDialog::Accepted) {
        if (NodeList[AID].live&&NodeList[BID].live) {
            Data_V[AID][BID].setedgedata(dialog.length(), dialog.isHighway(), dialog.price(), dialog.highwayPrice());
            update();
        }
    }
}

void EdgeItem::updatePosition() {
    if (startNode && endNode) {
        setLine(QLineF(startNode->sceneBoundingRect().center(), endNode->sceneBoundingRect().center()));
    }
}

void EdgeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    if (!startNode || !endNode) return;

    // **获取 `NodeItem` 的边界**
    QRectF startRect = startNode->sceneBoundingRect();
    QRectF endRect = endNode->sceneBoundingRect();

    // **计算 `NodeItem` 的半径**
    qreal startRadius = qMin(startRect.width(), startRect.height()) / 2;
    qreal endRadius = qMin(endRect.width(), endRect.height()) / 2;

    // **获取起点和终点的中心坐标**
    QPointF startCenter = startRect.center();
    QPointF endCenter = endRect.center();

    // **计算修正后的起点和终点**
    QLineF fullLine(startCenter, endCenter);
    QPointF startPoint = fullLine.pointAt(startRadius / fullLine.length());
    QPointF endPoint = fullLine.pointAt(1.0 - (endRadius / fullLine.length()));

    // **获取 `label` 位置**
    QGraphicsTextItem* startLabel = startNode->getLabel();
    QGraphicsTextItem* endLabel = endNode->getLabel();

    QRectF startLabelRect, endLabelRect;
    if (startLabel) startLabelRect = startLabel->sceneBoundingRect();
    if (endLabel) endLabelRect = endLabel->sceneBoundingRect();

    // **调整线段，避免覆盖 `label`**
    if (startLabelRect.contains(startPoint)) {
        startPoint = fullLine.pointAt((startRadius + startLabelRect.height()) / fullLine.length());
    }
    if (endLabelRect.contains(endPoint)) {
        endPoint = fullLine.pointAt(1.0 - (endRadius + endLabelRect.height()) / fullLine.length());
    }

    painter->setPen(QPen(Qt::black, 2, Qt::SolidLine));
    painter->drawLine(startPoint, endPoint);

    while(!safelock){
        //qDebug()<<"被锁咯";
    }

     QString text = QString("长度:%1\n高速:%2\n普通价格:%3\n高速价格:%4\n")
         .arg(Data_V[AID][BID].edgeLength)
         .arg(Data_V[AID][BID].highway ? "有" : "无")
         .arg(Data_V[AID][BID].edgePrice)
         .arg(Data_V[AID][BID].highway ?  QString::number(Data_V[AID][BID].edgeHighwayPrice) : "NULL");


     safelock=true;

     QFont font = painter->font();
     font.setPointSize(8); // 小字
     painter->setFont(font);
     QPointF textPos = (startPoint + endPoint) / 2 + QPointF(5, -15);
     QRectF textRect(textPos, QSizeF(150, 100));  // 设置一个足够大的文字区域
     painter->drawText(textRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, text);

}


QRectF EdgeItem::boundingRect() const {
    qreal extra = 6;
    return QGraphicsLineItem::boundingRect().adjusted(-extra, -extra, extra, extra);
}

QPainterPath EdgeItem::shape() const {
    QPainterPath path;
    path.moveTo(line().p1());
    path.lineTo(line().p2());

    QPainterPathStroker stroker;
    stroker.setWidth(100);
    return stroker.createStroke(path);
}

void EdgeItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    Data_V[AID][BID].takedown();
    Data_V[BID][AID].takedown();
    QMenu menu;
    QAction *deleteAction = menu.addAction("删除路径");
    QAction *selectedAction = menu.exec(event->screenPos());

    if (selectedAction == deleteAction) {
        if (scene) {
            scene->removeEdge(startNode, endNode);
        }
        delete this;
    }
}
