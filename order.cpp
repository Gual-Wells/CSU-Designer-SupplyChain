//#include "order.h"
//#include "nodedata.h"
//#include <QRandomGenerator>
//#include "edgedata.h"
//#include <QDebug>
//#include <QMessageBox>
//#include "population_and_order.h"

//order::order()
//{

//}

//order::order(int goods_ID,int order_num,int my_ID,int Pop_ID){
//    arriving=false;
//    this->Pop_ID=Pop_ID;
//    last_ID=-1;
//    server_num=0;
//    this->goods_ID=goods_ID;
//    this->order_num=order_num;
//    this->my_ID=my_ID;
//    end_ID=-1;
//    time=-1;
//    cost=-1;
//    hope_time=-1;
//    time_per=-1;
//    wait_merge=0;
//    fitness=-1;
//    rank=-1;
//    follow=0;
//    point_ID=-1;
//    now_ID=my_ID;
//    QVector<int> server_table;
//    for(auto &ends : NodeList){
//        if(allocation.isEmpty()){
//            if(ends.live&&!ends.cs){
//                server_table.append(ends.live_ID);
//                server_num++;
//            }
//        }
//    }
//    hope_time=NodeList[my_ID].hope_times[0][goods_ID-1];
//    allocation.append(server_table);
//    QVector<int> old_table(server_table.size(),0);
//    QVector<int> believe_table(server_table.size(),1);
//    QVector<int> fitness_table(server_table.size(),0);
//    allocation.append(old_table);
//    allocation.append(believe_table);
//    allocation.append(fitness_table);
//    QVector<int> zero;
//    for(int i=0;i<max_path;i++){
//        zero.append(-1);
//    }
//    exp_path.append(zero);
//    exp_path.append(zero);
////    last_path.append(zero);
////    last_path.append(zero);

//    index=0;


//    reach = QVector<QVector<QVector<bool>>>(server_num, QVector<QVector<bool>>(ID, QVector<bool>(ID, true)));
//    //qDebug()<<reach.size()<<reach[0].size()<<reach[0][0].size();

//}
//void order::al_random(){

//    //qDebug()<<"?";
//    if(end_ID!=-1)
//    NodeList[end_ID].live_goods[0][goods_ID-1]+=order_num;

//    index=QRandomGenerator::global()->bounded(server_num);


//    //qDebug()<<"?";
//    int none_index=0;
//    while(allocation[2][index]==0&&none_index!=100){
//        index=QRandomGenerator::global()->bounded(server_num);
//        none_index++;
//    }


//    //qDebug()<<"?";
//    if(none_index==100){
//        QMessageBox::critical(nullptr, "错误", QString("存在订单物理不可达！"));
//        error_ack=true;
//        return;
//    }

//    //qDebug()<<NodeList[0].live_goods[0];
//    end_ID=allocation[0][index];


//    //qDebug()<<"?1";


//    bool id_if=false;
//    for(auto canif = NodeList[now_ID].path_can.constBegin(); canif != NodeList[now_ID].path_can.constEnd(); ++canif){
//        if(reach[index][my_ID][canif.key()]){
//         id_if=true;
//        }
//    }

//    //qDebug()<<"?2";

//    if(!id_if)
//    allocation[2][index]=0;

//    int wrong=0;
//    while((NodeList[end_ID].live_goods[0][goods_ID-1]<order_num||!id_if)&&wrong!=100){
//        index=QRandomGenerator::global()->bounded(server_num);
//        end_ID=allocation[0][index];
//        wrong++;

//        if(allocation[2][index]==0)continue;

//        //qDebug()<<NodeList[end_ID].live_goods[0][goods_ID-1]<<":"<<order_num<<":"<<goods_ID<<":"<<end_ID<<NodeList[end_ID].live_goods[0];

//        id_if=false;
//        for(auto canif = NodeList[now_ID].path_can.constBegin(); canif != NodeList[now_ID].path_can.constEnd(); ++canif){
//            if(reach[index][my_ID][canif.key()]){
//             id_if=true;
//            }
//        }

//        if(!id_if)
//        allocation[2][index]=0;


//    }

//    bool check_if=false;
//    for(auto &check:allocation[2]){
//        if(check==1)check_if=true;
//    }
//    if(!check_if){
//        QMessageBox::critical(nullptr, "错误", QString("存在订单物理不可达！"));
//        error_ack=true;
//        return;
//    }

//    if(wrong==100){
//        end_ID=-1;
//        QMessageBox::critical(nullptr, "错误", QString("存在订单缺货！"));
//        error_ack=true;
//        return;
//    }
//    //qDebug()<<"ini_al_random";
//    NodeList[end_ID].live_goods[0][goods_ID-1]-=order_num;
//}
//void order::exp(){
//    cost=0;
//    time=0;
//    arriving=false;
//    exp_path.clear();
//    QVector<int> zero;
//    for(int i=0;i<max_path;i++){
//        zero.append(-1);
//    }
//    exp_path.append(zero);
//    exp_path.append(zero);
//    //qDebug()<<"ini_exp";
//    follow=0;
//    point_ID=my_ID;
//    last_ID=my_ID;
//    now_ID=my_ID;
//    do{
//        last_ID=now_ID;
//        now_ID=point_ID;
//        point_ID=randomChoice();
//        //qDebug()<<"ID:"<<point_ID;
//        if(point_ID==-2)return;
//        if(point_ID==-1){
//            if(now_ID!=end_ID){
//                //qDebug()<<"!=";
//                reach[index][last_ID][now_ID]=false;
//                reach[index][now_ID][last_ID]=false;
//            }
//            //qDebug()<<reach[index];
//            return;
//        }
//        exp_path[0][follow]=point_ID;
//        exp_path[1][follow]=randomHightway();
//        if(exp_path[1][follow]==1){
//            cost+=Data_V[now_ID][point_ID].edgeHighwayPrice;
//            time_hold+=Data_V[now_ID][point_ID].edgeLength/100;
//        }
//        else {
//            cost+=Data_V[now_ID][point_ID].edgePrice;
//            time_hold+=Data_V[now_ID][point_ID].edgeLength/80;
//        }
//        ++follow;

//        time=time_hold/hope_time;



//    }while(point_ID!=end_ID&&follow<max_path);

//    if(point_ID==end_ID)arriving=true;
//    //qDebug()<<reach[index];

//}


//void order::hello_help(int please_ID){

//}

//int order::randomChoice(){
//    //qDebug()<<"randomChoice1";
//    double total = 0.0;
//    int reach_num=0;
//    //qDebug()<<"case1";


//    for (auto w = NodeList[now_ID].path_can.constBegin(); w != NodeList[now_ID].path_can.constEnd(); ++w) {
//        //qDebug()<<end_ID<<"//"<<now_ID<<"//"<<w.key();
//        //qDebug()<<reach.size()<<reach[0].size()<<reach[0][0].size();
//        //qDebug()<<reach.size()<<reach[0].size()<<reach[0][0].size()<<reach[index][now_ID][w.key()];
//        if(reach[index][now_ID][w.key()]){
//        total += w.value();
//        reach_num++;
//        }
//    }

//    //qDebug()<<"reach_num:"<<reach_num;

//    if(reach_num==0){
////        QMessageBox::critical(nullptr, "错误", QString("存在订单物理不可达！"));
////        error_ack=true;
//        return -2;
//    }

//    //qDebug()<<"case2";

//    if (total <= 0.0) {
//        //qDebug()<<"randomChoice3";
//        return -1;
//    }

//    double rnd = QRandomGenerator::global()->generateDouble() * total;
//    double acc = 0.0;

//    if (NodeList[now_ID].path_can.isEmpty()) {
//        //qDebug()<<"randomChoice2";
//        return -1;
//    }
//    for (auto it = NodeList[now_ID].path_can.constBegin(); it != NodeList[now_ID].path_can.constEnd(); ++it) {
//    if(reach[index][now_ID][it.key()])
//    acc += it.value();
//    if (rnd <= acc){
//        if(it.key()==last_ID&&(NodeList[now_ID].path_can.size()==1||reach_num==1)){
//            //qDebug()<<"randomChoice4";
//            return -1;
//        }

//        if(it.key()==last_ID)return randomChoice();

//        //qDebug()<<"randomChoice5";
//        return it.key();
//    }
//    }


//    //qDebug()<<"randomChoice6";
//    return NodeList[now_ID].path_can.keys().last(); // 理论上不会执行
//}
//int order::randomHightway(){
//    if(Data_V[my_ID][point_ID].highway){
//        double rule=QRandomGenerator::global()->generateDouble()*2;
//        if(follow==0){
//            return rule<1 ? 0 : 1;
//        }
//        else{
//            return rule<1 ? 0 : 1;//下次添加评价功能
//        }
//    }
//    return 0;
//}
