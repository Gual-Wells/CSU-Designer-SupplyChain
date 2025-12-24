//#include "population_and_order.h"
//#include "nodedata.h"
//#include <QDebug>
//#include <QRandomGenerator>


//#include <QMessageBox>

//population::population()
//{
//    arrived=false;
//    crowded=0;
//    pricefit=-1;
//    timefit=-1;
//    linedex=5;
//    P_ID=-1;
//    sleeping=false;
//    fitness=-1;
//    order_num=0;
//    p_c=-1;
//    t_c=-1;
//    power=0;
//    p_c_=-1;
//    t_c_=-1;
//    //qDebug()<<P_ID;
//    for(auto &client : NodeList){
//        if(client.live&&client.cs){
//            auto &list0 = client.goods_wants[0];
//            auto &list1 = client.goods_wants[1];

//            size_t size = std::min(list0.size(), list1.size());
//            //qDebug()<<size;
//            //qDebug()<<"{";
//            for (size_t i = 0; i < size; ++i) {
//                if(client.hope_times[0][i]==0)continue;
//                //qDebug()<<"|";
//                if(list0[i]<=0)continue;
//                auto &w0 = list0[i];
//                auto &w1 = list1[i];
//                order new_order = order(w1,w0,client.live_ID,P_ID);
//                //qDebug()<<"}";

//                //qDebug()<<new_order.my_ID;

//                particles.append(std::move(new_order));
//                //qDebug()<<particles[0].reach[0][0][0];
//                //qDebug()<<"}";
//                order_num++;
//                //qDebug()<<order_num;
//            }


//        }
//    }

//    if(order_num==0){
//        QMessageBox::critical(nullptr, "错误", QString("不存在合法订单！"));
//        error_ack=true;
//        return;
//    }
//    //qDebug()<<P_ID;
//}



//population::population(int id)
//{
//    arrived=false;
//    crowded=0;
//    pricefit=-1;
//    timefit=-1;
//    linedex=5;
//    P_ID=id;
//    sleeping=false;
//    fitness=-1;
//    order_num=0;
//    p_c=-1;
//    t_c=-1;
//    power=0;
//    p_c_=-1;
//    t_c_=-1;
//    //qDebug()<<P_ID;
//    for(auto &client : NodeList){
//        if(client.live&&client.cs){
//            auto &list0 = client.goods_wants[0];
//            auto &list1 = client.goods_wants[1];

//            size_t size = std::min(list0.size(), list1.size());
//            //qDebug()<<size;
//            //qDebug()<<"{";
//            for (size_t i = 0; i < size; ++i) {
//                if(client.hope_times[0][i]==0)continue;
//                //qDebug()<<"|";
//                if(list0[i]<=0)continue;
//                auto &w0 = list0[i];
//                auto &w1 = list1[i];
//                order new_order = order(w1,w0,client.live_ID,P_ID);
//                //qDebug()<<"}";

//                //qDebug()<<new_order.my_ID;

//                particles.append(std::move(new_order));
//                //qDebug()<<particles[0].reach[0][0][0];
//                //qDebug()<<"}";
//                order_num++;
//                //qDebug()<<"}";
//            }

//        }
//    }
//    //qDebug()<<P_ID;
//}

//bool population::C_or_S(){
//    int ordex=QRandomGenerator::global()->bounded(10);
//    c_o_s=true;
//    if(ordex>linedex){
//        c_o_s=false;
//        return c_o_s;
//    }
//    return c_o_s;
//}

//bool population::OK_Check(){
//    arrived=true;
//    for(auto&orderit:particles){
//        if(!orderit.arriving){
//            arrived=false;
//            return false;
//        }
//    }

//    return true;
//}

//bool population::load_info(){
//    pricefit=0;
//    timefit=0;
//    for(auto &guys:particles){
//        pricefit+=guys.cost;
//        timefit+=guys.time;
//        if(guys.time>1)power++;
//    }
//    return true;
//}
