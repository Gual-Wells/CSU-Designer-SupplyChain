#include "population_and_order.h"
#include "nodedata.h"
#include <QDebug>
#include <QRandomGenerator>

#include "edgedata.h"
#include "result.h"

#include <QMessageBox>
#include <QSet>

population::population()
{
    arrived=false;
    crowded=0;
    pricefit=-1;
    timefit=-1;
    linedex=5;
    P_ID=-1;
    sleeping=false;
    fitness=-1;
    order_num=0;
    p_c=-1;
    t_c=-1;
    power=0;
    p_c_=-1;
    t_c_=-1;

    ID_crad.resize(5);
    for(int i=0;i<5;++i){
    ID_crad[i].resize(NodeList.size());
    }

    for(auto&q:ID_crad){
        for(auto&p:q){
            p=-1;
        }
    }


    //qDebug()<<P_ID;
    for(auto &client : NodeList){
        if(client.live&&client.cs){
            auto &list0 = client.goods_wants[0];
            auto &list1 = client.goods_wants[1];

            size_t size = std::min(list0.size(), list1.size());
            //qDebug()<<size;
            //qDebug()<<"{";
            for (size_t i = 0; i < size; ++i) {
                if(client.hope_times[0][i]==0)continue;
                //qDebug()<<"|";
                if(list0[i]<=0)continue;
                auto &w0 = list0[i];
                auto &w1 = list1[i];
                order new_order = order(w1,w0,client.live_ID,P_ID);
                //qDebug()<<"}";

                //qDebug()<<new_order.my_ID;


                ID_crad[w1-1][new_order.my_ID]=order_num;
                particles.append(std::move(new_order));

                //qDebug()<<particles[0].reach[0][0][0];
                //qDebug()<<"}";
                order_num++;
//                qDebug()<<order_num;
                //qDebug()<<"}";
            }

        }
    }
    //qDebug()<<P_ID;





}

population::population(int id)
{
    arrived=false;
    crowded=0;
    pricefit=-1;
    timefit=-1;
    linedex=5;
    P_ID=id;
    sleeping=false;
    fitness=-1;
    order_num=0;
    p_c=-1;
    t_c=-1;
    power=0;
    p_c_=-1;
    t_c_=-1;

    ID_crad.resize(population_goods_num);
    for(int i=0;i<population_goods_num;++i){
    ID_crad[i].resize(NodeList.size());
    }

    for(auto&q:ID_crad){
        for(auto&p:q){
            p=-1;
        }
    }


    //qDebug()<<P_ID;
    for(auto &client : NodeList){
        if(client.live&&client.cs){
            auto &list0 = client.goods_wants[0];
            auto &list1 = client.goods_wants[1];

            size_t size = std::min(list0.size(), list1.size());
            //qDebug()<<size;
            //qDebug()<<"{";
            for (size_t i = 0; i < size; ++i) {
                if(client.hope_times[0][i]==0)continue;
                //qDebug()<<"|";
                if(list0[i]<=0)continue;
                auto &w0 = list0[i];
                auto &w1 = list1[i];
                order new_order = order(w1,w0,client.live_ID,P_ID);
                //qDebug()<<"}";

                //qDebug()<<new_order.my_ID;
                new_order.my_index=order_num;




                ID_crad[w1-1][new_order.my_ID]=order_num;
                particles.append(std::move(new_order));

                //qDebug()<<particles[0].reach[0][0][0];
                //qDebug()<<"}";
                order_num++;
//                qDebug()<<order_num;
                //qDebug()<<"}";
            }

        }
    }
    //qDebug()<<P_ID;

    order_num_for_result_pbest=order_num;




}

bool population::C_or_S(){
    int ordex=QRandomGenerator::global()->bounded(10);
    c_o_s=true;
    if(ordex>linedex){
        c_o_s=false;
        return c_o_s;
    }
    return c_o_s;
}

bool population::OK_Check(){
    arrived=true;
    for(auto&orderit:particles){
        if(!orderit.arriving){
            arrived=false;
            return false;
        }
    }

    return true;
}

bool population::load_info(){
    pricefit=0;
    timefit=0;
    for(auto &guys:particles){
        if(!guys.have_brother||guys.brother_order==&guys||guys.brother_order==nullptr||guys.false_are_not_brother_guard/*||go_fuck_no_brother[guys.Pop_ID][guys.my_ID]*/){
            pricefit+=guys.cost;
//            qDebug()<<"pricefit+=guys.cost;";
        }
        else{
            pricefit+=0;
//            qDebug()<<"pricefit+=0;";
        }
        timefit+=guys.time;
        if(guys.time>1)power++;
    }
    qDebug()<<pricefit<<"and"<<timefit;

    if(timefit<=0||pricefit<=0)
        return false;


    return true;
}

population::order::order()
{

}

population::order::order(int goods_ID,int order_num,int my_ID,int Pop_ID){
    massage_order=false;
    arriving=false;
    this->Pop_ID=Pop_ID;
    last_ID=-1;
    server_num=0;
    this->goods_ID=goods_ID;
    this->order_need=order_num;
    this->my_ID=my_ID;
    end_ID=-1;
    time=-1;
    cost=-1;
    cost_with=-1;
    hope_time=-1;
    time_hold=-1;
    time_per=-1;
    wait_merge=0;
    fitness=-1;
    rank=-1;
    follow=0;
    point_ID=-1;
    now_ID=my_ID;
    QVector<int> server_table;
    for(auto &ends : NodeList){
        if(allocation.isEmpty()){
            if(ends.live&&!ends.cs){
                server_table.append(ends.live_ID);
                server_num++;
            }
        }
    }
    hope_time=NodeList[my_ID].hope_times[0][goods_ID-1];
    allocation.append(server_table);
    QVector<int> old_table(server_table.size(),0);
    QVector<int> believe_table(server_table.size(),1);
    QVector<int> fitness_table(server_table.size(),0);
    allocation.append(old_table);
    allocation.append(believe_table);
    allocation.append(fitness_table);
    QVector<int> zero;
    for(int i=0;i<max_path;i++){
        zero.append(-1);
    }
    exp_path.append(zero);
    exp_path.append(zero);
//    last_path.append(zero);
//    last_path.append(zero);

    index=0;


    reach = QVector<QVector<QVector<bool>>>(server_num, QVector<QVector<bool>>(ID, QVector<bool>(ID, true)));
    //qDebug()<<reach.size()<<reach[0].size()<<reach[0][0].size();

}
void population::order::al_random(){


    //qDebug()<<"?";
//    if(end_ID>=0){
//    NodeList[end_ID].live_goods[0][goods_ID-1]+=order_need;//假设正常取货，下一轮凭借正常仓库ID归还上一轮取货

//    qDebug()<<NodeList[end_ID].live_goods[0][goods_ID-1];
//}

    end_ID=-1;


    index=QRandomGenerator::global()->bounded(server_num);//仓库随机


    //qDebug()<<"?";
    int none_index=0;//仓库不可达标记
    while(allocation[2][index]==0&&none_index!=100){//一百次以内，allocation置信度行被不可达0否决，循环
        index=QRandomGenerator::global()->bounded(server_num);
        none_index++;
    }


    //qDebug()<<"?";
    if(none_index==100){
    //不可达次数100次定性为物理不可达
        QMessageBox::critical(nullptr, "错误", QString("存在订单物理不可达或剪枝激进！"));
        error_ack=true;
        return;
    }

    //qDebug()<<NodeList[0].live_goods[0];
    end_ID=allocation[0][index];
    //allocation仓库ID行对应分配ID


    //qDebug()<<"?1";


    bool id_if=false;
    for(auto canif = NodeList[now_ID].path_can.constBegin(); canif != NodeList[now_ID].path_can.constEnd(); ++canif){
        if(reach[index][my_ID][canif.key()]){
         id_if=true;
         break;
        }
    }
    //当前仓库ID下的从出发点下的可达节点有未被reach否决目标点，可以探询

    //qDebug()<<"?2";

    if(!id_if){
    allocation[2][index]=0;
    }

    QSet<int> done_index;

    int wrong=0;
    while((NodeList[end_ID].live_goods[this->Pop_ID][0][goods_ID-1]<order_need||!id_if)&&wrong<=100){//100次以内，寻找可达的且够货的仓库
        done_index.insert(index);//这是上一次尝试的仓库ID，证明已经不可行，即将被换掉
        if(done_index.size()==server_num){
            end_ID=-1;//-1需要被定义？
            return;
        }

        index=QRandomGenerator::global()->bounded(server_num);

        if(done_index.contains(index))continue;


        if(allocation[2][index]==0)continue;

        end_ID=allocation[0][index];
        wrong++;


        //qDebug()<<NodeList[end_ID].live_goods[0][goods_ID-1]<<":"<<order_num<<":"<<goods_ID<<":"<<end_ID<<NodeList[end_ID].live_goods[0];


        id_if=false;
        for(auto canif = NodeList[now_ID].path_can.constBegin(); canif != NodeList[now_ID].path_can.constEnd(); ++canif){
            if(reach[index][my_ID][canif.key()]){
             id_if=true;
             break;
            }
        }

        if(!id_if)
        allocation[2][index]=0;


    }

    bool check_if=false;
    for(auto &check:allocation[2]){
        if(check==1)check_if=true;
    }
    if(!check_if){
        QMessageBox::critical(nullptr, "错误", QString("存在订单物理不可达！"));
        error_ack=true;
        return;
    }

    if(wrong==100){
        end_ID=-1;
        QMessageBox::critical(nullptr, "错误", QString("存在订单缺货！"));
        error_ack=true;
        return;
    }
    //qDebug()<<"ini_al_random";
    NodeList[end_ID].live_goods[this->Pop_ID][0][goods_ID-1]-=order_need;
}

void population::order::exp(population*boss){
    false_are_not_brother_guard=false;
    message_num=0;
//    qDebug()<<"call";
    cost=0;
    time=0;
    time_hold=0;
    cost_with=0;
    arriving=false;
    exp_path.clear();
    QVector<int> zero;
    for(int i=0;i<max_path;i++){
        zero.append(-1);
    }
//    qDebug()<<"a";
    exp_path.append(zero);
    exp_path.append(zero);
    //qDebug()<<"ini_exp";
    follow=0;
    point_ID=my_ID;
    last_ID=my_ID;
    now_ID=my_ID;




    if(have_brother&&brother_order!=this&&brother_order!=nullptr&&QRandomGenerator::global()->generateDouble()<=brother_point&&brother_order->arriving&&!false_are_not_brother_guard&&!go_fuck_no_brother[Pop_ID][my_ID]){
        exp_path=brother_order->exp_path;
        time_hold=brother_order->time_hold;
        time=time_hold/hope_time;
        cost=brother_order->cost;
        cost_with=brother_order->cost_with;
        wait_merge=brother_order->wait_merge;
        merge_order=brother_order->merge_order;
        arriving=brother_order->arriving;



        control_guy.catchorder(*this,my_index);

        return;
    }



    do{
        last_ID=now_ID;
        now_ID=point_ID;

//        qDebug()<<"b";
        point_ID=randomChoice();

//        qDebug()<<"c";
        //qDebug()<<"ID:"<<point_ID;
        if(point_ID==-2)return;


//        qDebug()<<"d";
        if(point_ID==-1){
            if(now_ID!=end_ID){
                //qDebug()<<"!=";

                message_num++;

                if(!control_guy.disable_cut){
                    reach[index][last_ID][now_ID]=false;
                    reach[index][now_ID][last_ID]=false;
                }

                if(!merge_ing)
                {
                    population::order guard=control_guy.false_return(my_index);
                    if(!guard.massage_order){
                        int pid=Pop_ID;
                        *this=guard;
                        Pop_ID=pid;
                        false_are_not_brother_guard=true;

                        go_fuck_no_brother[Pop_ID][my_ID]=true;
                    }
                }
                return;

            }
            //qDebug()<<reach[index];
            return;
        }



//        qDebug()<<"e";
        exp_path[0][follow]=point_ID;
        exp_path[1][follow]=randomHightway();
        if(exp_path[1][follow]==1){
            cost+=Data_V[now_ID][point_ID].edgeHighwayPrice;
            cost_with=cost;
            time_hold+=Data_V[now_ID][point_ID].edgeLength/100;
        }
        else {
            cost+=Data_V[now_ID][point_ID].edgePrice;
            cost_with=cost;
            time_hold+=Data_V[now_ID][point_ID].edgeLength/80;
        }
        ++follow;

        time=time_hold/hope_time;

//        qDebug()<<time<<"()"<<cost;



//        qDebug()<<"f";


        if(NodeList[point_ID].cs&&QRandomGenerator::global()->generateDouble()<=wait){





                QSet<int> pass_way;
                QSet<int> same_way;

                bool same=false;


//                qDebug()<<"g";

                for(int i=0;i<order_goods_num;++i){



//                    qDebug()<<"h";
                    if(end_ID>=0&&
                            boss->ID_crad[i][point_ID]!=-1&&
                            boss->particles[boss->ID_crad[i][point_ID]].end_ID!=-1&&
                            boss->particles[boss->ID_crad[i][point_ID]].end_ID!=this->end_ID&&
                            NodeList[boss->particles[boss->ID_crad[i][point_ID]].end_ID].live_goods[Pop_ID][0][goods_ID-1]>=order_need&&
                            !boss->particles[boss->ID_crad[i][point_ID]].merge_ing){

//                        qDebug()<<"i";

                        pass_way.insert(boss->ID_crad[i][point_ID]);
                    }

                    else if(boss->ID_crad[i][point_ID]!=-1&&boss->particles[boss->ID_crad[i][point_ID]].end_ID!=this->end_ID&&

                            boss->particles[boss->ID_crad[i][point_ID]].end_ID!=-1&&

                            !boss->particles[boss->ID_crad[i][point_ID]].merge_ing){


//                        qDebug()<<"j";

                        same_way.insert(boss->ID_crad[i][point_ID]);
                        same=true;
                    }
                }


//                qDebug()<<"k";

                if(!pass_way.isEmpty()&&!same){


                auto it = pass_way.constBegin();
                std::advance(it, QRandomGenerator::global()->bounded(pass_way.size()));


//                qDebug()<<"l";

                if(boss->particles[*it].arriving){


                    time_hold+=boss->particles[*it].time_hold;
                    time=time_hold/hope_time;
                    boss->particles[*it].time_hold=time_hold;
                    boss->particles[*it].time=boss->particles[*it].time_hold/boss->particles[*it].hope_time;
                    cost_with=cost+boss->particles[*it].cost_with;
                    arriving=true;
                    wait_merge=true;
                    boss->particles[*it].wait_merge=true;


                    NodeList[end_ID].live_goods[this->Pop_ID][0][goods_ID-1]+=order_need;


                    end_ID=boss->particles[*it].end_ID;


                    NodeList[end_ID].live_goods[this->Pop_ID][0][goods_ID-1]-=order_need;

                    merge_order=&boss->particles[*it];

                    boss->child_sisters.insert(my_index);
                    boss->child_sisters.insert(boss->particles[*it].my_index);


                    control_guy.catchorder(*this,my_index);


//                    qDebug()<<"m";


                    return;
                }
                else{


//                    qDebug()<<"n";

                    merge_ing=true;
                    boss->particles[*it].merge_ing=true;
                    boss->particles[*it].exp(boss);
                    boss->particles[*it].merge_ing=false;
                    merge_ing=false;



//                    qDebug()<<"o";

                    if(boss->particles[*it].arriving){


//                        qDebug()<<"p";

                        time_hold+=boss->particles[*it].time_hold;
                        time=time_hold/hope_time;
                        boss->particles[*it].time_hold=time_hold;
                        boss->particles[*it].time=boss->particles[*it].time_hold/boss->particles[*it].hope_time;
                        cost_with=cost+boss->particles[*it].cost_with;
                        arriving=true;
                        wait_merge=true;
                        boss->particles[*it].wait_merge=true;

                        NodeList[end_ID].live_goods[this->Pop_ID][0][goods_ID-1]+=order_need;


                        end_ID=boss->particles[*it].end_ID;


                        NodeList[end_ID].live_goods[this->Pop_ID][0][goods_ID-1]-=order_need;


                        merge_order=&boss->particles[*it];


                        boss->child_sisters.insert(my_index);
                        boss->child_sisters.insert(boss->particles[*it].my_index);


                        control_guy.catchorder(*this,my_index);


//                        qDebug()<<"q";


                        return;
                    }
                    else
                    {


                        if(!merge_ing)
                        {
                            population::order guard=control_guy.false_return(my_index);
                            if(!guard.massage_order){
                                int pid=Pop_ID;
                                *this=guard;
                                Pop_ID=pid;
                                false_are_not_brother_guard=true;
                                go_fuck_no_brother[Pop_ID][my_ID]=true;
                            }
                        }
                        return;


//                        if(follow<max_path)
//                            continue;
//                        else
                            return;
                    }

                }

        }

                else if(!same_way.isEmpty()&&same){



//                    qDebug()<<"r";

                    wait_merge=true;

                    auto it = same_way.constBegin();
                    std::advance(it, QRandomGenerator::global()->bounded(same_way.size()));


                    if(boss->particles[*it].arriving){


//                        qDebug()<<"s";

                        time_hold+=boss->particles[*it].time_hold;
                        time=time_hold/hope_time;
                        boss->particles[*it].time_hold=time_hold;
                        boss->particles[*it].time=boss->particles[*it].time_hold/boss->particles[*it].hope_time;
                        cost_with=cost+boss->particles[*it].cost_with;
                        arriving=true;
                        wait_merge=true;
                        boss->particles[*it].wait_merge=true;
                        merge_order=&boss->particles[*it];


                        boss->child_sisters.insert(my_index);
                        boss->child_sisters.insert(boss->particles[*it].my_index);

                        control_guy.catchorder(*this,my_index);


//                        qDebug()<<"t";


                        return;
                    }
                    else{


//                        qDebug()<<"u";
                        merge_ing=true;
                        boss->particles[*it].merge_ing=true;
                        boss->particles[*it].exp(boss);
                        boss->particles[*it].merge_ing=false;
                        merge_ing=false;


                        if(boss->particles[*it].arriving){

//                            qDebug()<<"v";

                            time_hold+=boss->particles[*it].time_hold;
                            time=time_hold/hope_time;
                            boss->particles[*it].time_hold=time_hold;
                            boss->particles[*it].time=boss->particles[*it].time_hold/boss->particles[*it].hope_time;
                            cost_with=cost+boss->particles[*it].cost_with;
                            arriving=true;
                            wait_merge=true;
                            boss->particles[*it].wait_merge=true;
                            merge_order=&boss->particles[*it];


                            boss->child_sisters.insert(my_index);
                            boss->child_sisters.insert(boss->particles[*it].my_index);

                            control_guy.catchorder(*this,my_index);


//                            qDebug()<<"w";

                            return;
                        }
                        else
                        {



                            if(!merge_ing)
                            {
                                population::order guard=control_guy.false_return(my_index);
                                if(!guard.massage_order){
                                    int pid=Pop_ID;
                                    *this=guard;
                                    Pop_ID=pid;
                                    false_are_not_brother_guard=true;
                                    go_fuck_no_brother[Pop_ID][my_ID]=true;
                                }
                            }
                            return;

//                            if(follow<max_path)
//                                continue;
//                            else
                                return;
                        }
                    }
                }



     }


    }while(point_ID!=end_ID&&follow<max_path);

    if(point_ID==end_ID){
        arriving=true;

        control_guy.catchorder(*this,my_index);
    }


    if(!merge_ing)

    {
        population::order guard=control_guy.false_return(my_index);
        if(!guard.massage_order){
            int pid=Pop_ID;
            *this=guard;
            Pop_ID=pid;
            false_are_not_brother_guard=true;
            go_fuck_no_brother[Pop_ID][my_ID]=true;
        }
    }
    return;

//    qDebug()<<"end";
    //qDebug()<<reach[index];

}

void population::order::simply_exp(){
    false_are_not_brother_guard=false;
    message_num=0;
//    qDebug()<<"call";
    cost=0;
    time=0;
    time_hold=0;
    cost_with=0;
    arriving=false;
    exp_path.clear();
    QVector<int> zero;
    for(int i=0;i<max_path;i++){
        zero.append(-1);
    }
//    qDebug()<<"a";
    exp_path.append(zero);
    exp_path.append(zero);
    //qDebug()<<"ini_exp";
    follow=0;
    point_ID=my_ID;
    last_ID=my_ID;
    now_ID=my_ID;




//    if(brother_order!=this&&brother_order!=nullptr&&QRandomGenerator::global()->generateDouble()<=brother_point&&brother_order->arriving&&!false_are_not_brother_guard&&!go_fuck_no_brother[Pop_ID][my_ID]){
//        exp_path=brother_order->exp_path;
//        time_hold=brother_order->time_hold;
//        time=time_hold/hope_time;
//        cost=brother_order->cost;
//        cost_with=brother_order->cost_with;
//        wait_merge=brother_order->wait_merge;
//        merge_order=brother_order->merge_order;
//        arriving=brother_order->arriving;

//        control_guy.catchorder(*this,my_index);

//        return;
//    }



    do{
        last_ID=now_ID;
        now_ID=point_ID;

//        qDebug()<<"b";
        point_ID=randomChoice();

//        qDebug()<<"c";
        //qDebug()<<"ID:"<<point_ID;
        if(point_ID==-2)return;


//        qDebug()<<"d";
        if(point_ID==-1){
            if(now_ID!=end_ID){
                //qDebug()<<"!=";

                message_num++;
                if(!control_guy.disable_cut){
                    reach[index][last_ID][now_ID]=false;
                    reach[index][now_ID][last_ID]=false;
                }

//                if(!merge_ing)
//                {
//                    population::order guard=control_guy.false_return(my_index);
//                    if(!guard.massage_order){
//                        int pid=Pop_ID;
//                        *this=guard;
//                        Pop_ID=pid;
//                        false_are_not_brother_guard=true;

//                        go_fuck_no_brother[Pop_ID][my_ID]=true;
//                    }
//                }
                return;

            }
            //qDebug()<<reach[index];
            return;
        }



//        qDebug()<<"e";
        exp_path[0][follow]=point_ID;
        exp_path[1][follow]=randomHightway();
        if(exp_path[1][follow]==1){
            cost+=Data_V[now_ID][point_ID].edgeHighwayPrice;
            cost_with=cost;
            time_hold+=Data_V[now_ID][point_ID].edgeLength/100;
        }
        else {
            cost+=Data_V[now_ID][point_ID].edgePrice;
            cost_with=cost;
            time_hold+=Data_V[now_ID][point_ID].edgeLength/80;
        }
        ++follow;

        time=time_hold/hope_time;

//        qDebug()<<time<<"()"<<cost;



//        qDebug()<<"f";


//        if(NodeList[point_ID].cs&&QRandomGenerator::global()->generateDouble()<=wait){





//                QSet<int> pass_way;
//                QSet<int> same_way;

//                bool same=false;


////                qDebug()<<"g";

//                for(int i=0;i<order_goods_num;++i){



////                    qDebug()<<"h";
//                    if(end_ID>=0&&
//                            boss->ID_crad[i][point_ID]!=-1&&
//                            boss->particles[boss->ID_crad[i][point_ID]].end_ID!=-1&&
//                            boss->particles[boss->ID_crad[i][point_ID]].end_ID!=this->end_ID&&
//                            NodeList[boss->particles[boss->ID_crad[i][point_ID]].end_ID].live_goods[0][goods_ID-1]>=order_need&&
//                            !boss->particles[boss->ID_crad[i][point_ID]].merge_ing){

////                        qDebug()<<"i";

//                        pass_way.insert(boss->ID_crad[i][point_ID]);
//                    }

//                    else if(boss->ID_crad[i][point_ID]!=-1&&boss->particles[boss->ID_crad[i][point_ID]].end_ID!=this->end_ID&&

//                            boss->particles[boss->ID_crad[i][point_ID]].end_ID!=-1&&

//                            !boss->particles[boss->ID_crad[i][point_ID]].merge_ing){


////                        qDebug()<<"j";

//                        same_way.insert(boss->ID_crad[i][point_ID]);
//                        same=true;
//                    }
//                }


////                qDebug()<<"k";

//                if(!pass_way.isEmpty()&&!same){


//                auto it = pass_way.constBegin();
//                std::advance(it, QRandomGenerator::global()->bounded(pass_way.size()));


////                qDebug()<<"l";

//                if(boss->particles[*it].arriving){


//                    time_hold+=boss->particles[*it].time_hold;
//                    time=time_hold/hope_time;
//                    boss->particles[*it].time_hold=time_hold;
//                    boss->particles[*it].time=boss->particles[*it].time_hold/boss->particles[*it].hope_time;
//                    cost_with=cost+boss->particles[*it].cost_with;
//                    arriving=true;
//                    wait_merge=true;
//                    boss->particles[*it].wait_merge=true;


//                    NodeList[end_ID].live_goods[0][goods_ID-1]+=order_need;


//                    end_ID=boss->particles[*it].end_ID;


//                    NodeList[end_ID].live_goods[0][goods_ID-1]-=order_need;

//                    merge_order=&boss->particles[*it];

//                    boss->child_sisters.insert(my_index);
//                    boss->child_sisters.insert(boss->particles[*it].my_index);


//                    control_guy.catchorder(*this,my_index);


////                    qDebug()<<"m";


//                    return;
//                }
//                else{


////                    qDebug()<<"n";

//                    merge_ing=true;
//                    boss->particles[*it].merge_ing=true;
//                    boss->particles[*it].exp(boss);
//                    boss->particles[*it].merge_ing=false;
//                    merge_ing=false;



////                    qDebug()<<"o";

//                    if(boss->particles[*it].arriving){


////                        qDebug()<<"p";

//                        time_hold+=boss->particles[*it].time_hold;
//                        time=time_hold/hope_time;
//                        boss->particles[*it].time_hold=time_hold;
//                        boss->particles[*it].time=boss->particles[*it].time_hold/boss->particles[*it].hope_time;
//                        cost_with=cost+boss->particles[*it].cost_with;
//                        arriving=true;
//                        wait_merge=true;
//                        boss->particles[*it].wait_merge=true;

//                        NodeList[end_ID].live_goods[0][goods_ID-1]+=order_need;


//                        end_ID=boss->particles[*it].end_ID;


//                        NodeList[end_ID].live_goods[0][goods_ID-1]-=order_need;


//                        merge_order=&boss->particles[*it];


//                        boss->child_sisters.insert(my_index);
//                        boss->child_sisters.insert(boss->particles[*it].my_index);


//                        control_guy.catchorder(*this,my_index);


////                        qDebug()<<"q";


//                        return;
//                    }
//                    else
//                    {


//                        if(!merge_ing)
//                        {
//                            population::order guard=control_guy.false_return(my_index);
//                            if(!guard.massage_order){
//                                int pid=Pop_ID;
//                                *this=guard;
//                                Pop_ID=pid;
//                                false_are_not_brother_guard=true;
//                                go_fuck_no_brother[Pop_ID][my_ID]=true;
//                            }
//                        }
//                        return;


////                        if(follow<max_path)
////                            continue;
////                        else
//                            return;
//                    }

//                }

//        }

//                else if(!same_way.isEmpty()&&same){



////                    qDebug()<<"r";

//                    wait_merge=true;

//                    auto it = same_way.constBegin();
//                    std::advance(it, QRandomGenerator::global()->bounded(same_way.size()));


//                    if(boss->particles[*it].arriving){


////                        qDebug()<<"s";

//                        time_hold+=boss->particles[*it].time_hold;
//                        time=time_hold/hope_time;
//                        boss->particles[*it].time_hold=time_hold;
//                        boss->particles[*it].time=boss->particles[*it].time_hold/boss->particles[*it].hope_time;
//                        cost_with=cost+boss->particles[*it].cost_with;
//                        arriving=true;
//                        wait_merge=true;
//                        boss->particles[*it].wait_merge=true;
//                        merge_order=&boss->particles[*it];


//                        boss->child_sisters.insert(my_index);
//                        boss->child_sisters.insert(boss->particles[*it].my_index);

//                        control_guy.catchorder(*this,my_index);


////                        qDebug()<<"t";


//                        return;
//                    }
//                    else{


////                        qDebug()<<"u";
//                        merge_ing=true;
//                        boss->particles[*it].merge_ing=true;
//                        boss->particles[*it].exp(boss);
//                        boss->particles[*it].merge_ing=false;
//                        merge_ing=false;


//                        if(boss->particles[*it].arriving){

////                            qDebug()<<"v";

//                            time_hold+=boss->particles[*it].time_hold;
//                            time=time_hold/hope_time;
//                            boss->particles[*it].time_hold=time_hold;
//                            boss->particles[*it].time=boss->particles[*it].time_hold/boss->particles[*it].hope_time;
//                            cost_with=cost+boss->particles[*it].cost_with;
//                            arriving=true;
//                            wait_merge=true;
//                            boss->particles[*it].wait_merge=true;
//                            merge_order=&boss->particles[*it];


//                            boss->child_sisters.insert(my_index);
//                            boss->child_sisters.insert(boss->particles[*it].my_index);

//                            control_guy.catchorder(*this,my_index);


////                            qDebug()<<"w";

//                            return;
//                        }
//                        else
//                        {



//                            if(!merge_ing)
//                            {
//                                population::order guard=control_guy.false_return(my_index);
//                                if(!guard.massage_order){
//                                    int pid=Pop_ID;
//                                    *this=guard;
//                                    Pop_ID=pid;
//                                    false_are_not_brother_guard=true;
//                                    go_fuck_no_brother[Pop_ID][my_ID]=true;
//                                }
//                            }
//                            return;

////                            if(follow<max_path)
////                                continue;
////                            else
//                                return;
//                        }
//                    }
//                }



//     }


    }while(point_ID!=end_ID&&follow<max_path);

    if(point_ID==end_ID){
        arriving=true;

        control_guy.catchorder(*this,my_index);
    }


//    if(!merge_ing)

//    {
//        population::order guard=control_guy.false_return(my_index);
//        if(!guard.massage_order){
//            int pid=Pop_ID;
//            *this=guard;
//            Pop_ID=pid;
//            false_are_not_brother_guard=true;
//            go_fuck_no_brother[Pop_ID][my_ID]=true;
//        }
//    }
    return;

//    qDebug()<<"end";
    //qDebug()<<reach[index];

}

void population::order::hello_help(int please_ID){

}

int population::order::randomChoice(){
    //qDebug()<<"randomChoice1";
    double total = 0.0;
    int reach_num=0;
    //qDebug()<<"case1";


    for (auto w = NodeList[now_ID].path_can.constBegin(); w != NodeList[now_ID].path_can.constEnd(); ++w) {
        //qDebug()<<end_ID<<"//"<<now_ID<<"//"<<w.key();
        //qDebug()<<reach.size()<<reach[0].size()<<reach[0][0].size();
        //qDebug()<<reach.size()<<reach[0].size()<<reach[0][0].size()<<reach[index][now_ID][w.key()];
        if(reach[index][now_ID][w.key()]){
        total += w.value();
        reach_num++;
        }
    }

    //qDebug()<<"reach_num:"<<reach_num;

    if(reach_num==0){
        if(!massacall)
            QMessageBox::critical(nullptr, "错误", QString("存在订单物理不可达或剪枝激进！"));
        massacall=true;
        error_ack=true;
        return -2;
    }

    //qDebug()<<"case2";

    if (total <= 0.0) {
        //qDebug()<<"randomChoice3";
        return -1;
    }

    double rnd = QRandomGenerator::global()->generateDouble() * total;
    double acc = 0.0;

    if (NodeList[now_ID].path_can.isEmpty()) {
        //qDebug()<<"randomChoice2";
        return -1;
    }
    for (auto it = NodeList[now_ID].path_can.constBegin(); it != NodeList[now_ID].path_can.constEnd(); ++it) {
    if(reach[index][now_ID][it.key()])
    acc += it.value();
    if (rnd <= acc){
        if(it.key()==last_ID&&(NodeList[now_ID].path_can.size()==1||reach_num==1)){
            //qDebug()<<"randomChoice4";
            return -1;
        }

        if(it.key()==last_ID)return randomChoice();

        //qDebug()<<"randomChoice5";
        return it.key();
    }
    }


    //qDebug()<<"randomChoice6";
    return NodeList[now_ID].path_can.keys().last(); // 理论上不会执行
}
int population::order::randomHightway(){
    if(Data_V[my_ID][point_ID].highway){
        double rule=QRandomGenerator::global()->generateDouble()*2;
        if(follow==0){
            return rule<1 ? 0 : 1;
        }
        else{
            return rule<1 ? 0 : 1;//下次添加评价功能
        }
    }
    return 0;
}
