 #include "graphscene.h"
#include "graphview.h"
#include <QDebug>
#include <QApplication>
#include <QTimer>
#include <QElapsedTimer>
#include <QGraphicsView>
#include <QVector>
#include "nodedata.h"
#include "edgedata.h"



GraphScene::GraphScene(QObject *parent)
    : QGraphicsScene(parent), selectedNode(nullptr), tempLine(nullptr) {}

// **新增：自动调整 sceneRect**


void GraphScene::removeNode(NodeItem *node) {
    if (!node) return;

    //qDebug() << "准备隐藏 NodeItem：" << static_cast<void*>(node);

    // **取消选中的 NodeItem，防止连线功能被触发**
    if (selectedNode == node) {
        selectedNode = nullptr;
    }

    // **删除虚线（临时连线）**
    if (tempLine) {
        removeItem(tempLine);
        delete tempLine;
        tempLine = nullptr;
    }

    // **取消 NodeItem 的所有交互**
    node->setEnabled(false);                 // 禁用交互
    node->setVisible(false);                 // 视觉上隐藏
    node->setAcceptHoverEvents(false);       // 禁用鼠标悬停
    node->setFlag(QGraphicsItem::ItemIsSelectable, false);
    node->setFlag(QGraphicsItem::ItemIsMovable, false);

    // **将 NodeItem 移动到底层，防止意外点击**
    node->setZValue(-1000);

    // **隐藏所有相关连线**
    QList<QPair<NodeItem *, NodeItem *>> edgesToHide;
    for (auto it = connections.begin(); it != connections.end(); ++it) {
        if (it.key().first == node || it.key().second == node) {
            edgesToHide.append(it.key());
        }
    }

    for (const auto &key : edgesToHide) {
        EdgeItem *edge = connections.value(key);
        if (edge) {
            edge->setVisible(false);
            edge->setEnabled(false);
        }
    }

    //qDebug() << "NodeItem 已隐藏：" << static_cast<void*>(node);
}







void GraphScene::mousePaint(QGraphicsSceneMouseEvent *event) {
    QPointF scenePos = event->scenePos();

    for (QGraphicsItem *item : items(scenePos)) {
        if (auto existingNode = dynamic_cast<NodeItem *>(item)) {
            //qDebug() << "位置重叠，取消创建";
            carriage = 0;
            QApplication::restoreOverrideCursor();
            return;  // **如果已有 NodeItem，则不创建新节点**
        }
    }

    // 处理图片选择
    QString folderPath = ":/qrc/";
    QString fileName;
    QString nodeName;
    if (carriage == 1){
        fileName = "storage.png";
        nodeName = "仓库"+QString::number(warehouseCount);
        warehouseCount++;
    }
    else if (carriage == 2){
        fileName = "client.png";
        nodeName = "客户"+QString::number(clientCount);
        clientCount++;
    }
    else
        return;

    QPixmap pixmap(folderPath + fileName);
    if (pixmap.isNull()) {
        //qDebug() << "无法加载图片：" << folderPath + fileName;
        return;
    }

    // 创建 NodeItem
    NodeItem *node = new NodeItem(pixmap, nodeName);
    node->setPos(scenePos - QPointF(pixmap.width() / 2, pixmap.height() / 2));

    // 添加到场景
    addItem(node);

    //4.14upload
    if(Data_V.isEmpty()){
        QVector<edgedata> newRow(1, edgedata());  // 用 0 初始化新行
        Data_V.append(newRow);
        nodedata newnd(node->x(),node->y(),true,carriage==2);
        node->live_ID=newnd.live_ID;
        node->live=newnd.live;
        //qDebug()<<node->live_ID<<newnd.live_ID;
        node->cs=newnd.cs;
        NodeList.append(newnd);
        NodeIList.append(node);
    }
    else{
        nodedata newnd(node->x(),node->y(),true,carriage==2);
        node->live_ID=newnd.live_ID;
        node->live=newnd.live;
        //qDebug()<<node->live_ID<<newnd.live_ID;
        node->cs=newnd.cs;
        NodeList.append(newnd);
        NodeIList.append(node);
        for(auto& row :Data_V){
            row.append(edgedata());
        }
        int newColSize = Data_V.isEmpty() ? 1 : Data_V[0].size();  // 已经列数 +1
        QVector<edgedata> newRow(newColSize, edgedata());  // 用 0 初始化新行
        Data_V.append(newRow);
    }




    //qDebug() << "绘制节点于:" << scenePos;

    // 复原光标状态
    carriage = 0;
    QApplication::restoreOverrideCursor();
}


void GraphScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {

    QPointF scenePos = event->scenePos();
    QGraphicsItem *clickedItem = nullptr;
    QList<QGraphicsItem *> itemsAtPos = items(scenePos);

    for (QGraphicsItem *item : itemsAtPos) {
        if (auto node = dynamic_cast<NodeItem *>(item)) {
            clickedItem = node;
            break;
        }
    }

    // **如果没有选中 NodeItem，才检查 EdgeItem**
    if (!clickedItem) {
        for (QGraphicsItem *item : itemsAtPos) {
            if (auto edge = dynamic_cast<EdgeItem *>(item)) {
                clickedItem = edge;
                break;
            }
        }
    }

    // 处理 NodeItem 交互（连线逻辑）
    if (auto node = dynamic_cast<NodeItem *>(clickedItem)) {
        if(node&&carriage!=0){
            carriage=0;
            QApplication::restoreOverrideCursor();
            return;
        }
        if (!selectedNode&&carriage==0) {
            selectedNode = node;
            tempLine = new QGraphicsLineItem(QLineF(node->sceneBoundingRect().center(), scenePos));
            tempLine->setPen(QPen(Qt::black, 1, Qt::DashLine));
            addItem(tempLine);
        } else if (selectedNode != node&&carriage==0) {
            if (!connections.contains(qMakePair(selectedNode, node)) &&
                !connections.contains(qMakePair(node, selectedNode))) {
                EdgeItem *edge = new EdgeItem(this, selectedNode, node);
                addItem(edge);
                connections.insert(qMakePair(selectedNode, node), edge);
            }
            removeItem(tempLine);
            delete tempLine;
            tempLine = nullptr;
            selectedNode = nullptr;
        }
    }
    // 处理空白区域点击，创建 NodeItem
    else {
        if (selectedNode) {
            removeItem(tempLine);
            delete tempLine;
            tempLine = nullptr;
            selectedNode = nullptr;
        } else {
            mousePaint(event);
        }
    }
}

void GraphScene::adjustSceneRect(GraphView *view) {
    if (view->isDrag()) {
           return;  // 在拖动时，不调整 sceneRect
       }
    // 获取所有图形项的边界矩形
    QRectF boundingBox = itemsBoundingRect();

    // 添加边距，避免物体的边缘被切掉
    setSceneRect(boundingBox.marginsAdded(QMarginsF(50, 50, 50, 50)));
}

void GraphScene::updateView(QGraphicsView *view) {
    if (view) {
        QRectF viewRect = view->mapToScene(view->viewport()->rect()).boundingRect();
        QGraphicsScene::update(viewRect);  // 只更新视图内的区域
    }
}

void GraphScene::update(const QRectF &rect) {
    // 确保每次更新时场景大小是有效的
    if (views().isEmpty()) {
        return;  // 如果没有视图，就不进行处理
    }
    GraphView *view = dynamic_cast<GraphView*>(views().first());  // 获取视图对象
    if (view) {
        adjustSceneRect(view);  // 调用 adjustSceneRect 来更新 sceneRect
    }

    // 更新场景
    QGraphicsScene::update(rect);
}

void GraphScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (tempLine) {
        tempLine->setLine(QLineF(selectedNode->sceneBoundingRect().center(), event->scenePos()));
    }
}

void GraphScene::removeEdge(NodeItem* sourceNode, NodeItem* targetNode) {
    connections.remove(qMakePair(sourceNode, targetNode));
}
