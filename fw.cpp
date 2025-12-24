#include "fw.h"
#include <QDir>
#include <QFileDialog>
#include <QListView>
#include <QComboBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include "nodedata.h"
#include "nodeitem.h"
#include "edgedata.h"
#include "edgeitem.h"
#include "graphscene.h"
#include "graphview.h"
#include "order.h"
#include "result.h"
#include "chartwidget.h"

FW::FW(QWidget *parent)
    : QWidget(parent)
{
    // 设置布局
    QVBoxLayout *layout = new QVBoxLayout(this);

    QPushButton *saveButton = new QPushButton("保存文件", this);
    layout->addWidget(saveButton);
    connect(saveButton, &QPushButton::clicked, this, &FW::saveFile);

    QPushButton *importButton = new QPushButton("导入文件", this);
    layout->addWidget(importButton);
    connect(importButton, &QPushButton::clicked, this, &FW::loadFile);
}

void FW::loadFile() {


    // 清空 control_guy 和未操作的全局变量
    // control_guy 重置，模拟 result::result()
    control_guy.elite.clear();
    control_guy.prev_elite.clear();
    control_guy.elite_save.clear();
    control_guy.metrics_history.clear();
    control_guy.priceedge.clear();
    control_guy.timeedge.clear();
    for (int i = 0; i < control_guy.max_elite; ++i) {
        control_guy.priceedge.append(i);
        control_guy.timeedge.append(i);
    }
    control_guy.priceedge_save.clear();
    control_guy.timeedge_save.clear();
    control_guy.p_best.clear();
    control_guy.priceedge_pbest.clear();
    control_guy.timeedge_pbest.clear();

    order_num_for_result_pbest = 0;

    control_guy.poweredge.clear();
    control_guy.for_power = 0.0;
    control_guy.last_p_max = 0.0;
    control_guy.last_p_min = 0.0;
    control_guy.last_t_max = 0.0;
    control_guy.last_t_min = 0.0;
    control_guy.prev_weighted_metric = 0.0;
    control_guy.iteration_count = 0;
    control_guy.be_dominate_flag = false;
    control_guy.be_dominate_flag_save = false;
    control_guy.be_dominate_flag_pbest = false;
    control_guy.a = 0.1; // 可选：恢复默认值
    control_guy.b = 0.3;
    control_guy.c = 0.6;

    // 重置未在 loadFile 中操作的全局变量
    choose = 0.7;
    survive = nullptr;
    debug_cut = false;
    message_num = 0;
    brother_check.clear();
    go_fuck_no_brother.clear();
    popu_num = 0;
    need_structure = false;
    massacall = false;



    QString fileName = QFileDialog::getOpenFileName(this, "打开保存文件", "", "JSON 文件 (*.json)");
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("无法打开文件读取！");
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject rootObj = doc.object();

    // 清空当前场景和数据
    NodeIList.clear();
    NodeList.clear();
    Data_V.clear();
    if (S_Catch) {
        S_Catch->clear();
    }

    // ---------- 还原 NodeData ----------
    QJsonArray nodeDataArray = rootObj["NodeData"].toArray();
    for (const QJsonValue& v : nodeDataArray) {
        QJsonObject obj = v.toObject();
        nodedata d(obj["x"].toDouble(), obj["y"].toDouble(), obj["live"].toBool(), obj["cs"].toBool());
        d.live_ID = obj["live_ID"].toInt();
        d.can_num = obj["can_num"].toInt();

        // goods_wants
        QJsonArray goodsArray = obj["goods_wants"].toArray();
        d.goods_wants.clear();
        for (const QJsonValue& rowVal : goodsArray) {
            QVector<int> row;
            for (const QJsonValue& val : rowVal.toArray()) row.append(val.toInt());
            d.goods_wants.append(row);
        }

        // live_goods (二维容器数组)
        QJsonArray liveGoodsArray = obj["live_goods"].toArray();
        for (int i = 0; i < 5 && i < liveGoodsArray.size(); ++i) {
            d.live_goods[i].clear();
            for (const QJsonValue& rowVal : liveGoodsArray[i].toArray()) {
                QVector<int> row;
                for (const QJsonValue& val : rowVal.toArray()) row.append(val.toInt());
                d.live_goods[i].append(row);
            }
        }

        // hope_times
        QJsonArray hopeArray = obj["hope_times"].toArray();
        d.hope_times.clear();
        for (const QJsonValue& rowVal : hopeArray) {
            QVector<double> row;
            for (const QJsonValue& val : rowVal.toArray()) row.append(val.toDouble());
            d.hope_times.append(row);
        }

        // path_can
        QJsonObject hashObj = obj["path_can"].toObject();
        for (const QString& key : hashObj.keys()) {
            d.path_can[key.toInt()] = hashObj[key].toDouble();
        }

        NodeList.append(d);
        qDebug() << d.goods_wants;
    }

    // ---------- 还原 NodeItem ----------
    QJsonArray nodeItemsArray = rootObj["NodeItems"].toArray();
    for (const QJsonValue& v : nodeItemsArray) {
        QJsonObject obj = v.toObject();
        QString label = obj["label"].toString();
        int id = obj["live_ID"].toInt();

        QPixmap pixmap("null");
        if (obj["cs"].toBool())
            pixmap.load(":/qrc/client.png");
        else
            pixmap.load(":/qrc/storage.png");
        NodeItem* item = new NodeItem(pixmap, label);
        item->live_ID = id;
        item->live = obj["live"].toBool();
        item->cs = obj["cs"].toBool();
        item->setPos(obj["x"].toDouble(), obj["y"].toDouble());

        NodeIList.append(item);
        if (S_Catch) S_Catch->addItem(item);
    }

    // ---------- 初始化 Data_V（edgedata 矩阵） ----------
    int N = NodeList.size();
    Data_V.resize(N);
    for (int i = 0; i < N; ++i)
        Data_V[i].resize(N);

    // ---------- 还原 edgedata ----------
    QJsonArray edgeDataArray = rootObj["EdgeData"].toArray();
    for (const QJsonValue& v : edgeDataArray) {
        QJsonObject obj = v.toObject();
        int from = obj["from"].toInt();
        int to = obj["to"].toInt();
        edgedata e;
        e.edgeLength = obj["edgeLength"].toDouble();
        e.highway = obj["highway"].toBool();
        e.edgePrice = obj["edgePrice"].toDouble();
        e.edgeHighwayPrice = obj["edgeHighwayPrice"].toDouble();
        e.live = obj["live"].toBool();

        Data_V[from][to] = e;
    }

    // ---------- 还原 EdgeItem ----------
    QJsonArray edgeItemsArray = rootObj["EdgeItems"].toArray();
    for (const QJsonValue& v : edgeItemsArray) {
        QJsonObject obj = v.toObject();
        int AID = obj["AID"].toInt();
        int BID = obj["BID"].toInt();

        // 找到对应节点
        NodeItem* start = nullptr;
        NodeItem* end = nullptr;
        for (NodeItem* n : NodeIList) {
            if (n->live_ID == AID) start = n;
            if (n->live_ID == BID) end = n;
        }
        if (start && end) {
            EdgeItem* edge = new EdgeItem(S_Catch, start, end, 1);
            edge->updatePosition();
            if (S_Catch) S_Catch->addItem(edge);
        }
    }

    // ------------------- 读取参数部分 -------------------
    if (rootObj.contains("Params")) {
        QJsonObject params = rootObj["Params"].toObject();

        if (params.contains("ID")) ID = params["ID"].toInt();
        if (params.contains("calledge")) calledge = params["calledge"].toBool();
        if (params.contains("error_ack")) error_ack = params["error_ack"].toBool();
        if (params.contains("carriage")) carriage = params["carriage"].toInt();
        if (params.contains("warehouseCount")) warehouseCount = params["warehouseCount"].toInt();
        if (params.contains("clientCount")) clientCount = params["clientCount"].toInt();

        qDebug() << "[参数加载完毕]"
                 << "ID:" << ID
                 << "calledge:" << calledge
                 << "error_ack:" << error_ack
                 << "carriage:" << carriage
                 << "warehouseCount:" << warehouseCount
                 << "clientCount:" << clientCount;
    }



       qDebug() << "图数据已成功加载：" << fileName << "，control_guy.elite 及相关数据已清空";

    qDebug() << "图数据已成功加载：" << fileName;
}

void FW::saveFile() {
    QString fileName = QFileDialog::getSaveFileName(this, "保存文件", "", "JSON 文件 (*.json)");
    if (fileName.isEmpty())
        return;

    QJsonObject rootObj;

    // ------------------- NodeItem 部分 -------------------
    QJsonArray nodeItemsArray;
    for (NodeItem* node : NodeIList) {
        if (!node) continue;
        QJsonObject obj;
        obj["live_ID"] = node->live_ID;
        obj["label"] = node->getLabel()->toPlainText();
        obj["x"] = node->pos().x();
        obj["y"] = node->pos().y();
        obj["live"] = node->live;
        obj["cs"] = node->cs;
        nodeItemsArray.append(obj);
    }
    rootObj["NodeItems"] = nodeItemsArray;

    // ------------------- nodedata 部分 -------------------
    QJsonArray nodeDataArray;
    for (const nodedata& data : NodeList) {
        QJsonObject obj;
        obj["x"] = data.x;
        obj["y"] = data.y;
        obj["live"] = data.live;
        obj["cs"] = data.cs;
        obj["live_ID"] = data.live_ID;
        obj["can_num"] = data.can_num;

        // 保存 QVector<QVector<int>> goods_wants
        QJsonArray goodsArray;
        for (const auto& row : data.goods_wants) {
            QJsonArray inner;
            for (int val : row) inner.append(val);
            goodsArray.append(inner);
        }
        obj["goods_wants"] = goodsArray;

        // 保存 QVector<QVector<int>> live_goods[5]
        QJsonArray liveGoodsArray;
        for (int i = 0; i < 5; ++i) {
            QJsonArray array;
            for (const auto& row : data.live_goods[i]) {
                QJsonArray inner;
                for (int val : row) inner.append(val); // 修复 bug：inner.append(inner) -> inner.append(val)
                array.append(inner);
            }
            liveGoodsArray.append(array);
        }
        obj["live_goods"] = liveGoodsArray;

        // 保存 QVector<QVector<double>> hope_times
        QJsonArray hopeArray;
        for (const auto& row : data.hope_times) {
            QJsonArray inner;
            for (double val : row) inner.append(val);
            hopeArray.append(inner);
        }
        obj["hope_times"] = hopeArray;

        // 保存 QHash<int, double> path_can
        QJsonObject hashObj;
        for (auto it = data.path_can.begin(); it != data.path_can.end(); ++it) {
            hashObj[QString::number(it.key())] = it.value();
        }
        obj["path_can"] = hashObj;

        nodeDataArray.append(obj);
    }
    rootObj["NodeData"] = nodeDataArray;

    // ------------------- edgedata (Data_V) -------------------
    QJsonArray edgeDataArray;
    for (int i = 0; i < Data_V.size(); ++i) {
        for (int j = 0; j < Data_V[i].size(); ++j) {
            const edgedata& ed = Data_V[i][j];
            if (!ed.live) continue; // 如果边不存在就跳过

            QJsonObject obj;
            obj["from"] = i;
            obj["to"] = j;
            obj["edgeLength"] = ed.edgeLength;
            obj["highway"] = ed.highway;
            obj["edgePrice"] = ed.edgePrice;
            obj["edgeHighwayPrice"] = ed.edgeHighwayPrice;
            obj["live"] = ed.live;
            edgeDataArray.append(obj);
        }
    }
    rootObj["EdgeData"] = edgeDataArray;

    // ------------------- EdgeItem 部分 -------------------
    QJsonArray edgeItemsArray;
    for (QGraphicsItem* item : S_Catch->items()) {
        EdgeItem* edge = dynamic_cast<EdgeItem*>(item);
        if (edge) {
            QJsonObject obj;
            obj["AID"] = edge->AID;
            obj["BID"] = edge->BID;
            obj["safelock"] = edge->safelock;
            edgeItemsArray.append(obj);
        }
    }
    rootObj["EdgeItems"] = edgeItemsArray;

    // 参数部分
    QJsonObject params;
    params["ID"] = ID;
    params["calledge"] = calledge;
    params["error_ack"] = error_ack;
    params["carriage"] = carriage;
    params["warehouseCount"] = warehouseCount;
    params["clientCount"] = clientCount;
    rootObj["Params"] = params;

    // ------------------- 保存到文件 -------------------
    QJsonDocument doc(rootObj);
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("无法打开文件保存");
        return;
    }
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    qDebug() << "图数据已保存到:" << fileName;
}
