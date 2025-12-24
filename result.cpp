#include "result.h"
#include <algorithm>
#include <QtMath>
#include <QMessageBox>
#include <QDebug>
#include <QRandomGenerator>
#include "nodedata.h"


result::result()
{
    for(int i=0;i<max_elite;++i){
        priceedge.append(i);
        timeedge.append(i);
    }
    error_ack = false;
    last_p_max = last_p_min = last_t_max = last_t_min = 0.0;
    prev_weighted_metric = 0.0;
}

void result::update_metrics(int iteration) {
    if (iteration % 20 != 0 || elite.isEmpty() || elite.size()==1) return;

    Metrics metrics;

    priceedge.clear();
    timeedge.clear();
    priceedge.resize(elite.size());
    timeedge.resize(elite.size());
    for (int i = 0; i < elite.size(); ++i) {
        priceedge[i] = i;
        timeedge[i] = i;
    }
    // 边界更新步长
    p_t_inline();
    double p_max = elite[priceedge[0]].pricefit;
    double p_min = elite[priceedge[elite.size() - 1]].pricefit;
    double t_max = elite[timeedge[0]].timefit;
    double t_min = elite[timeedge[elite.size() - 1]].timefit;

    double p_range = p_max - p_min + 1e-6;
    double t_range = t_max - t_min + 1e-6;

    if (metrics_history.isEmpty() && last_p_max == 0.0) {
        metrics.boundary_step = 0.0; // 首次计算
    } else {
        double dp_max = (p_max - last_p_max) / p_range;
        double dp_min = (p_min - last_p_min) / p_range;
        double dt_max = (t_max - last_t_max) / t_range;
        double dt_min = (t_min - last_t_min) / t_range;
        metrics.boundary_step = std::sqrt(dp_max * dp_max + dp_min * dp_min +
                                         dt_max * dt_max + dt_min * dt_min);
    }

    last_p_max = p_max;
    last_p_min = p_min;
    last_t_max = t_max;
    last_t_min = t_min;

    // 替换率
    int deleted_count = 0, new_count = 0;
    QSet<QPair<double, double>> prev_set, curr_set;
    for (const auto& man : prev_elite)
        prev_set.insert({man.pricefit, man.timefit});
    for (const auto& man : elite)
        curr_set.insert({man.pricefit, man.timefit});

    for (const auto& pair : prev_set)
        if (!curr_set.contains(pair)) deleted_count++;
    for (const auto& pair : curr_set)
        if (!prev_set.contains(pair)) new_count++;

    metrics.replacement_rate = (double)(deleted_count + new_count) / max_elite;
    prev_elite = elite;

    // 平均适应度
    double sum_pricefit = 0.0, sum_timefit = 0.0;
    for (const auto& man : elite) {
        sum_pricefit += man.pricefit;
        sum_timefit += man.timefit;
    }
    metrics.avg_pricefit = sum_pricefit / elite.size();
    metrics.avg_timefit = sum_timefit / elite.size();

    metrics_history.append(metrics);

    qDebug() << "Iteration" << iteration << "Boundary Step:" << metrics.boundary_step
             << "Replacement Rate:" << metrics.replacement_rate
             << "Avg Pricefit:" << metrics.avg_pricefit
             << "Avg Timefit:" << metrics.avg_timefit;
}

void result::dynamic_tune_parameters(int iteration) {
    if (iteration % 20 != 0 || iteration < 60 || metrics_history.size() < 2) return;

    const double w_boundary = 0.4, w_replace = 0.3, w_avg = 0.3;
    auto& curr_metrics = metrics_history.last();
    auto& prev_metrics = metrics_history[metrics_history.size() - 2];

    double curr_avg = (curr_metrics.avg_pricefit / (curr_metrics.avg_pricefit + 1e-6) +
                       curr_metrics.avg_timefit / (curr_metrics.avg_timefit + 1e-6)) / 2.0;
    double prev_avg = (prev_metrics.avg_pricefit / (prev_metrics.avg_pricefit + 1e-6) +
                       prev_metrics.avg_timefit / (prev_metrics.avg_timefit + 1e-6)) / 2.0;

    double curr_weighted = w_boundary * curr_metrics.boundary_step +
                          w_replace * curr_metrics.replacement_rate +
                          w_avg * curr_avg;
    double prev_weighted = w_boundary * prev_metrics.boundary_step +
                          w_replace * prev_metrics.replacement_rate +
                          w_avg * prev_avg;

    double slope = (curr_weighted - prev_weighted) / 100.0;
    double step_a = slope >= 0 ? qAbs(slope) * 0.5 : 0.05;

    a += step_a;
    if (a > 0.9) a = 0.9;
    if (a < 0.1) a = 0.1;

    double total_reduce = step_a;
    double reduce_b = QRandomGenerator::global()->generateDouble() * total_reduce;
    double reduce_c = total_reduce - reduce_b;
    b -= reduce_b;
    c -= reduce_c;
    if (b < 0.1) b = 0.1;
    if (c < 0.1) c = 0.1;

    double sum = a + b + c;
    if (qAbs(sum - 1.0) > 1e-6) {
        double adjust = (1.0 - sum) / 2.0;
        b += adjust;
        c += adjust;
        if (b < 0.1) b = 0.1;
        if (c < 0.1) c = 0.1;
        if (b > 0.9) b = 0.9;
        if (c > 0.9) c = 0.9;
    }

    qDebug() << "Iteration" << iteration << "Weights: a =" << a << "b =" << b << "c =" << c;
}

void result::global_renew(){
    p_best.resize(order_num_for_result_pbest);
    priceedge_pbest.resize(order_num_for_result_pbest);
    timeedge_pbest.resize(order_num_for_result_pbest);
}

void result::get_poweredge(int size){

    if(size==poweredge.size())return;

    for_power=0;

    poweredge.clear();
    poweredge.resize(size);

    const double pi = 3.141592653589793;
    for (int i = 0; i < size; ++i) {
        double x = 2.0 * i / (size - 1) - 1.0; // 归一化到 [-1, 1]
        poweredge[i] = 0.2 + 0.8 * (cos(pi * x) + 1.0) / 2.0; // 中间1.0，两端0.2
        for_power+=poweredge[i];
    }

}

bool result::elite_PP(int id){

    for (auto &path_find : NodeList) {
        if(!path_find.cs&&path_find.live){
            for(int i=0;i<path_find.live_goods[id][0].size();i++){
                path_find.live_goods[id][0][i]=path_find.goods_wants[0][i];
            }
        }
    }

    population PP(id);
    for(int i=0;i<order_num_for_result_pbest;i++){
        PP.particles[i]=random_pbest(i);
        if(!PP.particles[i].massage_order){
            NodeList[PP.particles[i].end_ID].live_goods[id][0][PP.particles[i].goods_ID-1]-=PP.particles[i].order_need;
            continue;
        }
        else {
            if(p_best[i].isEmpty())return false;
            else PP.particles[i]=p_best[i][0];
            PP.particles[i].end_ID=-1;
            PP.particles[i].al_random();
            if(error_ack){
                return false;
            }
            if(PP.particles[i].end_ID>=0){
                int allow_you_can_try=0;
                do{
                PP.particles[i].simply_exp();
                if (error_ack) {
                    return false;
                }
                allow_you_can_try++;
                }while(!PP.particles[i].arriving&&allow_you_can_try<=order_num_for_result_pbest);
                if(PP.particles[i].arriving){
                    continue;
                }
                else return false;
            }

            else return false;
        }
    }



    QMap<QPair<int, int>, QVector<int>> groups;
    for (int i = 0; i < order_num_for_result_pbest; i++) {
        if (PP.particles[i].arriving && PP.particles[i].end_ID >= 0) {
            QPair<int, int> key(PP.particles[i].my_ID, PP.particles[i].end_ID);
            groups[key].append(i);
        }
    }

    // 处理每组订单，设置 brother_order 和标志
    for (auto it = groups.constBegin(); it != groups.constEnd(); ++it) {
        const QVector<int> &group = it.value();
        if (group.size() > 1) { // 只有多于一个订单的组需要耦合
            for (int j = 0; j < group.size(); ++j) {
                int index = group[j];
                if (j == 0) {
                    // 第一个订单的 brother_order 指向自己
                    PP.particles[index].brother_order = &PP.particles[index];
                } else {
                    // 其他订单的 brother_order 指向第一个订单
                    PP.particles[index].brother_order = &PP.particles[group[0]];
                }
                PP.particles[index].have_brother = true;
                PP.particles[index].false_are_not_brother_guard = false;
            }
        }
    }
    // 耦合化逻辑结束

    if(PP.OK_Check()){
        if(PP.load_info()){
            popu_num++;
//            qDebug()<<"guy";
            control_guy.catchelite(PP);
//            if(popu_num==7)goto label;
            if(error_ack){
                return false;
            }
            qDebug()<<"从PP抓住了"<<PP.P_ID;
        }
    }

    return true;






}

bool result::elite_GA(int id){
    need_structure=false;
    int max_first_try=0;

    population GA(id);

    for (auto &path_find : NodeList) {
        if(!path_find.cs&&path_find.live){
            for(int i=0;i<path_find.live_goods[id][0].size();i++){
                path_find.live_goods[id][0][i]=path_find.goods_wants[0][i];
            }
        }
    }



    int index_fa=QRandomGenerator::global()->generateDouble()*(elite.size()+elite_save.size()+1);

    if(index_fa<=0)return false;
    int find=0;

    qDebug()<<order_num_for_result_pbest;

    QVector<bool>full(order_num_for_result_pbest,false);

    if(elite.size()>=index_fa){
        index_fa-=1;
        for(int i=0;i<order_num_for_result_pbest;){
            do{
            find=(int)(QRandomGenerator::global()->generateDouble()*(order_num_for_result_pbest));
            }while(full[find]);
            if(!need_structure&&elite[index_fa].child_sisters.isEmpty()&&elite[index_fa].particles[find].my_brothers.isEmpty()){
                need_structure=true;
            }
            if(!need_structure&&(elite[index_fa].child_sisters.contains(elite[index_fa].particles[find].my_index)||elite[index_fa].particles[find].my_brothers.contains(elite[index_fa].particles[find].my_index))){
                if(!elite[index_fa].child_sisters.isEmpty()){
                    for(auto&sis:elite[index_fa].child_sisters){
                        if(!full[sis]){
                            GA.particles[sis]=elite[index_fa].particles[sis];
                            GA.particles[sis].Pop_ID=id;
                            if(GA.particles[sis].brother_order==&elite[index_fa].particles[sis]){
                                GA.particles[sis].brother_order=&GA.particles[sis];
                            }
                            NodeList[GA.particles[sis].end_ID].live_goods[id][0][GA.particles[sis].goods_ID-1]-=GA.particles[sis].order_need;
                            full[sis]=true;
                            i++;
                            qDebug()<<i;
                        }
                    }
                }
                if(!elite[index_fa].particles[find].my_brothers.isEmpty()){
                    for(auto&bro:elite[index_fa].particles[find].my_brothers){
                        if(!full[bro]){
                            GA.particles[bro]=elite[index_fa].particles[bro];
                            GA.particles[bro].Pop_ID=id;
                            if(GA.particles[bro].brother_order==&elite[index_fa].particles[bro]){
                                GA.particles[bro].brother_order=&GA.particles[bro];
                            }
                            NodeList[GA.particles[bro].end_ID].live_goods[id][0][GA.particles[bro].goods_ID-1]-=GA.particles[bro].order_need;
                            full[bro]=true;
                            i++;
                            qDebug()<<i;
                        }
                    }
                }
                need_structure=true;
            }
            else if(need_structure){
                if(NodeList[elite[index_fa].particles[find].end_ID].live_goods[id][0][elite[index_fa].particles[find].goods_ID-1]>=elite[index_fa].particles[find].order_need){
                    GA.particles[find]=elite[index_fa].particles[find];
                    GA.particles[find].Pop_ID=id;
                    full[find]=true;
                    i++;
                    NodeList[elite[index_fa].particles[find].end_ID].live_goods[id][0][elite[index_fa].particles[find].goods_ID-1]-=elite[index_fa].particles[find].order_need;
                    qDebug()<<i;
                }
                else{
                    GA.particles[find].end_ID=-1;
                    GA.particles[find].al_random();
                    if(error_ack){
                        return false;
                    }
                    if(GA.particles[find].end_ID!=-1){
                        int allow_you_can_try=0;
                        do{
                        GA.particles[find].simply_exp();
                        if (error_ack) {
                            return false;
                        }
                        allow_you_can_try++;
                        }while(!GA.particles[find].arriving&&allow_you_can_try<=order_num_for_result_pbest);
                        if(GA.particles[find].arriving){
                            full[find]=true;
                            i++;
                            qDebug()<<i;
                        }
                        else return false;
                    }
                    else return false;
                }
            }
            else if(!need_structure&&max_first_try<=order_num_for_result_pbest){
                max_first_try++;
                continue;
            }
            index_fa=QRandomGenerator::global()->generateDouble()*(elite.size()-1);
        }
    }
    else if((elite.size()+elite_save.size())>=index_fa){
        index_fa-=elite.size();
        index_fa-=1;
        for(int i=0;i<order_num_for_result_pbest;){
            do{
            find=(int)(QRandomGenerator::global()->generateDouble()*(order_num_for_result_pbest));
            }while(full[find]);
            if(!need_structure&&elite_save[index_fa].child_sisters.isEmpty()&&elite_save[index_fa].particles[find].my_brothers.isEmpty()){
                need_structure=true;
            }
            if(!need_structure&&(elite_save[index_fa].child_sisters.contains(elite_save[index_fa].particles[find].my_index)||elite_save[index_fa].particles[find].my_brothers.contains(elite_save[index_fa].particles[find].my_index))){
                if(!elite_save[index_fa].child_sisters.isEmpty()){
                    for(auto&sis:elite_save[index_fa].child_sisters){
                        if(!full[sis]){
                            GA.particles[sis]=elite_save[index_fa].particles[sis];
                            GA.particles[sis].Pop_ID=id;
                            if(GA.particles[sis].brother_order==&elite[index_fa].particles[sis]){
                                GA.particles[sis].brother_order=&GA.particles[sis];
                            }
                            NodeList[GA.particles[sis].end_ID].live_goods[id][0][GA.particles[sis].goods_ID-1]-=GA.particles[sis].order_need;
                            full[sis]=true;
                            i++;
                            qDebug()<<i;
                        }
                    }
                }
                if(!elite_save[index_fa].particles[find].my_brothers.isEmpty()){
                    for(auto&bro:elite_save[index_fa].particles[find].my_brothers){
                        if(!full[bro]){
                            GA.particles[bro]=elite_save[index_fa].particles[bro];
                            GA.particles[bro].Pop_ID=id;
                            if(GA.particles[bro].brother_order==&elite[index_fa].particles[bro]){
                                GA.particles[bro].brother_order=&GA.particles[bro];
                            }
                            NodeList[GA.particles[bro].end_ID].live_goods[id][0][GA.particles[bro].goods_ID-1]-=GA.particles[bro].order_need;
                            full[bro]=true;
                            i++;
                            qDebug()<<i;
                        }
                    }
                }
                need_structure=true;
            }
            else if(need_structure){
                if(NodeList[elite_save[index_fa].particles[find].end_ID].live_goods[id][0][elite_save[index_fa].particles[find].goods_ID-1]>=elite_save[index_fa].particles[find].order_need){
                    GA.particles[find]=elite_save[index_fa].particles[find];
                    GA.particles[find].Pop_ID=id;
                    full[find]=true;
                    i++;
                    NodeList[elite_save[index_fa].particles[find].end_ID].live_goods[id][0][elite_save[index_fa].particles[find].goods_ID-1]-=elite_save[index_fa].particles[find].order_need;
                    qDebug()<<i;
                }
                else{
                    GA.particles[find].end_ID=-1;
                    GA.particles[find].al_random();
                    if(error_ack){
                        return false;
                    }
                    if(GA.particles[find].end_ID!=-1){
                        int allow_you_can_try=0;
                        do{
                        GA.particles[find].simply_exp();
                        if (error_ack) {
                            return false;
                        }
                        allow_you_can_try++;
                        }while(!GA.particles[find].arriving&&allow_you_can_try<=order_num_for_result_pbest);
                        if(GA.particles[find].arriving){
                            full[find]=true;
                            i++;
                            qDebug()<<i;
                        }
                        else return false;
                    }
                    else return false;
                }
            }
            else if(!need_structure&&max_first_try<=order_num_for_result_pbest){
                max_first_try++;
                continue;
            }
            index_fa=QRandomGenerator::global()->generateDouble()*(elite_save.size()-1);
        }

    }


    if(GA.OK_Check()){
        if(GA.load_info()){
            popu_num++;
//            qDebug()<<"guy";
            control_guy.catchelite(GA);
//            if(popu_num==7)goto label;
            if(error_ack){
                return false;
            }
            qDebug()<<"从GA抓住了"<<GA.P_ID;
        }
    }


    return true;
}

population::order result::false_return(int my_in){



    int index_fa=QRandomGenerator::global()->generateDouble()*(elite.size()+elite_save.size()+p_best[my_in].size());

    if(elite.size()>=index_fa)return random_elite(my_in);
    else if((elite.size()+elite_save.size())>=index_fa)return random_elite_save(my_in);
    else if((elite.size()+elite_save.size()+p_best[my_in].size())>=index_fa)return random_pbest(my_in);
    else{

        population::order new_order=population::order();
        new_order.massage_order=true;
        return new_order;
    }


}

population::order result::random_elite(int my_in){


    priceedge.clear();
    timeedge.clear();
    priceedge.resize(elite.size());
    timeedge.resize(elite.size());
    for (int i = 0; i < elite.size(); ++i) {
        priceedge[i] = i;
        timeedge[i] = i;
    }
    // 边界更新步长
    p_t_inline();


    int index_re=QRandomGenerator::global()->generateDouble()*for_power;
    int index_power=0;

    population::order new_order=population::order();

    get_poweredge(elite.size());

    for(int i=0;i<poweredge.size();++i){
        index_power+=poweredge[i];
        if(index_power>=index_re){
            new_order=elite[priceedge[i]].particles[my_in];
            new_order.cost=new_order.cost_with;
            return new_order;
        }
    }

    new_order.massage_order=true;

    return new_order;

}

population::order result::random_elite_save(int my_in){


    priceedge_save.clear();
    timeedge_save.clear();
    priceedge_save.resize(elite_save.size());
    timeedge_save.resize(elite_save.size());
    for (int i = 0; i < elite_save.size(); ++i) {
        priceedge_save[i] = i;
        timeedge_save[i] = i;
    }
    // 边界更新步长
    p_t_inline_save();


    int index_re=QRandomGenerator::global()->generateDouble()*elite_save.size();
    int index_power=0;

    population::order new_order=population::order();


    for(int i=0;i<elite_save.size();++i){
        index_power++;
        if(index_power>=index_re){
            new_order=elite_save[i].particles[my_in];
            new_order.cost=new_order.cost_with;
            return new_order;
        }
    }

    new_order.massage_order=true;

    return new_order;


}

population::order result::random_pbest(int which_order_index){

    priceedge_pbest[which_order_index].clear();
    timeedge_pbest[which_order_index].clear();
    priceedge_pbest[which_order_index].resize(p_best[which_order_index].size());
    timeedge_pbest[which_order_index].resize(p_best[which_order_index].size());
    for (int i = 0; i < p_best[which_order_index].size(); ++i) {
        priceedge_pbest[which_order_index][i] = i;
        timeedge_pbest[which_order_index][i] = i;
    }
    // 边界更新步长
    p_t_inline_pbest(which_order_index);




    int index_re=QRandomGenerator::global()->generateDouble()*p_best[which_order_index].size();
    int index_power=0;

    population::order new_order=population::order();


    for(int i=0;i<p_best[which_order_index].size();++i){
        index_power++;
        if(index_power>=index_re){
            new_order=p_best[which_order_index][i];
            new_order.cost=new_order.cost_with;
            return new_order;
        }
    }

    new_order.massage_order=true;

    return new_order;

}

void result::catchelite(population man){
    be_dominate_flag=false;
    if(elite.size()==0){
        elite.append(man);
        return;
    }
    if(dominate(man)){
        return;
    }
    else{
        if(be_dominate_flag)return;
        if(elite.size()<max_elite){
            elite.append(man);
            return;
        }
        else{
            p_t_inline();
            bool save_elite=false;
            population save_man;
            if(!crowded(man,save_elite,save_man)){

                QMessageBox::critical(nullptr, "错误", QString("精英拥挤错误！"));
                error_ack=true;
                return;
            }



            if(save_elite){
                be_dominate_flag_save=false;
                if(elite_save.size()==0){
                    elite_save.append(save_man);
                    return;
                }
                if(dominate_save(save_man)){
                    return;
                }
                else{
                    if(be_dominate_flag_save)return;
                    if(elite_save.size()<max_elite){
                        elite_save.append(save_man);
                        return;
                    }
                    else{
                        p_t_inline_save();
                        if(!crowded_save(save_man)){
                            QMessageBox::critical(nullptr, "错误", QString("精英拥挤错误！"));
                            error_ack=true;
                            return;
                        }
                    }
                }


            }


        }
    }
}

void result::catchorder(population::order kid,int index_ca){
    be_dominate_flag_pbest=false;
    if(p_best[index_ca].size()==0){
        p_best[index_ca].append(kid);
        return;
    }
    if(dominate_pbest(kid,index_ca)){
        return;
    }
    else{
        if(be_dominate_flag_pbest)return;
        if(p_best[index_ca].size()<max_elite_pbest){
            p_best[index_ca].append(kid);
            return;
        }
        else{
            priceedge_pbest[index_ca].clear();
            timeedge_pbest[index_ca].clear();
            priceedge_pbest[index_ca].resize(p_best[index_ca].size());
            timeedge_pbest[index_ca].resize(p_best[index_ca].size());
            for (int i = 0; i < p_best[index_ca].size(); ++i) {
                priceedge_pbest[index_ca][i] = i;
                timeedge_pbest[index_ca][i] = i;
            }
            p_t_inline_pbest(index_ca);
            if(!crowded_pbest(kid,index_ca)){
                QMessageBox::critical(nullptr, "错误", QString("pbest拥挤错误！"));
                error_ack=true;
                return;
            }


        }
    }
}

void result::last_change() {
    QVector<population> temp_elite = elite;
    for (const auto& save_man : elite_save) {
        be_dominate_flag=false;
        if(dominate(save_man)){
            continue;
        }
        else{
            if(be_dominate_flag)continue;
        }
    }

    elite = temp_elite;


//    elite_save.clear();


    std::sort(elite.begin(), elite.end(),
        [](const population& a, const population& b) {
            return a.pricefit < b.pricefit;
        });
}

bool result::crowded(population candidate, bool& is_in_top_30_percent, population& deleted_member) {



    is_in_top_30_percent = false;
    deleted_member = population();
    elite.append(candidate);

    priceedge.clear();
    timeedge.clear();
    priceedge.resize(elite.size());
    timeedge.resize(elite.size());
    for (int i = 0; i < elite.size(); ++i) {
        priceedge[i] = i;
        timeedge[i] = i;
    }

    p_t_inline();

    int p_max = elite[priceedge[0]].pricefit;
    int p_min = elite[priceedge[elite.size() - 1]].pricefit;
    int t_max = elite[timeedge[0]].timefit;
    int t_min = elite[timeedge[elite.size() - 1]].timefit;

    double price_range = p_max - p_min;
    double time_range = t_max - t_min;

    double std_per = 0.0001;
    double p_per_range = std_per;
    double t_per_range = std_per;
    double std_range = 10000;

    bool not_p = false;
    bool not_t = false;

    if (price_range <= std_per) {
        not_p = true;
    } else if (price_range <= 1) {
        price_range = std::floor(price_range / std_per);
        p_per_range = std_per;
    } else {
        p_per_range = price_range / std_range;
        price_range = std_range;
    }

    if (time_range <= std_per) {
        not_t = true;
    } else if (time_range <= 1) {
        time_range = std::floor(time_range / std_per);
        t_per_range = std_per;
    } else {
        t_per_range = time_range / std_range;
        time_range = std_range;
    }

    for (auto& manin : elite) {
        manin.crowded = 0;
        manin.p_c = 0;
        manin.p_c_ = 0;
        manin.t_c = 0;
        manin.t_c_ = 0;
    }

    elite[priceedge[elite.size()-1]].crowded = -1;
    elite[timeedge[elite.size()-1]].crowded = -1;

    for (int i = 1; i < elite.size() - 1; ++i) {
        if (!not_p) {
            elite[priceedge[i]].p_dis_down = qAbs(elite[priceedge[i - 1]].pricefit - elite[priceedge[i]].pricefit);
            elite[priceedge[i]].p_dis_up = qAbs(elite[priceedge[i]].pricefit - elite[priceedge[i + 1]].pricefit);
            double pownum1 = (elite[priceedge[i]].p_dis_down) / p_per_range / price_range;
            double pownum2 = (elite[priceedge[i]].p_dis_up) / p_per_range / price_range;
            elite[priceedge[i]].p_c = pownum1 * pownum1 + pownum2 * pownum2;
            elite[priceedge[i]].p_c_ = pownum1 + pownum2;
        }

        if (!not_t) {
            elite[timeedge[i]].t_dis_down = qAbs(elite[timeedge[i - 1]].timefit - elite[timeedge[i]].timefit);
            elite[timeedge[i]].t_dis_up = qAbs(elite[timeedge[i]].timefit - elite[timeedge[i + 1]].timefit);
            double pownum3 = (elite[timeedge[i]].t_dis_down) / t_per_range / time_range;
            double pownum4 = (elite[timeedge[i]].t_dis_up) / t_per_range / time_range;
            elite[timeedge[i]].t_c = pownum3 * pownum3 + pownum4 * pownum4;
            elite[timeedge[i]].t_c_ = pownum3 + pownum4;
        }
    }

    double min = 0;
    int min_index = 0;
    population* min_point = &elite[0];
    for (auto& manin : elite) {
        if (manin.crowded != -1) {
            manin.crowded = (2 - manin.power / manin.order_num) +
                            (arfa * qSqrt(manin.p_c + manin.t_c) + (1 - arfa) * (manin.p_c_ + manin.t_c_));
            if (min_index == 0) {
                min = manin.crowded;
                min_point = &manin;
            } else if (manin.crowded < min) {
                min = manin.crowded;
                min_point = &manin;
            }
            min_index++;
        }
    }

    if (min_point && min_point->crowded != -1) {
        min_index = std::distance(elite.begin(), std::find_if(elite.begin(), elite.end(),
            [&](const population& p) { return &p == min_point; }));

        population deleted = elite[min_index];

        double protect_price_range = p_max - p_min;
        double protect_time_range = t_max - t_min;
        bool protect_not_p = false, protect_not_t = false;

        if (protect_price_range <= std_per) {
            protect_price_range = 1.0;
            protect_not_p = true;
        }
        if (protect_time_range <= std_per) {
            protect_time_range = 1.0;
            protect_not_t = true;
        }

        double x1 = protect_not_p ? 0.0 : (elite[priceedge[elite.size()-1]].pricefit - p_min) / protect_price_range;
        double y1 = protect_not_t ? 0.0 : (elite[priceedge[elite.size()-1]].timefit - t_min) / protect_time_range;
        double x2 = protect_not_p ? 0.0 : (elite[timeedge[elite.size()-1]].pricefit - p_min) / protect_price_range;
        double y2 = protect_not_t ? 0.0 : (elite[timeedge[elite.size()-1]].timefit - t_min) / protect_time_range;

        double A = y2 - y1;
        double B = x1 - x2;
        double C = x2 * y1 - x1 * y2;
        double denom = std::sqrt(A * A + B * B);
        if (denom < 1e-10) denom = 1e-10;

        QVector<double> distances(elite.size());
        for (int i = 0; i < elite.size(); ++i) {
            double x = protect_not_p ? 0.0 : (elite[i].pricefit - p_min) / protect_price_range;
            double y = protect_not_t ? 0.0 : (elite[i].timefit - t_min) / protect_time_range;
            distances[i] = std::abs(A * x + B * y + C) / denom;
        }

        QVector<double> sorted_distances = distances;
        std::sort(sorted_distances.begin(), sorted_distances.end());
        int threshold_index = static_cast<int>(std::ceil(elite.size() * (1 - save_point))) - 1;
        if (threshold_index < 0) threshold_index = 0;
        if (threshold_index >= elite.size()) threshold_index = elite.size() - 1;
        double threshold = sorted_distances[threshold_index];
        is_in_top_30_percent = (distances[min_index] >= threshold);
        if (is_in_top_30_percent) {
            deleted_member = deleted;
        }

        elite.remove(min_index);
    } else {
        elite.remove(elite.size()-1);
        for (auto& manin : elite) {
            manin.crowded = 0;
        }
        priceedge.resize(max_elite);
        timeedge.resize(max_elite);
        for (int i = 0; i < max_elite; ++i) {
            priceedge[i] = i;
            timeedge[i] = i;
        }
        p_t_inline();
        QMessageBox::critical(nullptr, "错误", QString("无法找到最拥挤的解！"));
        error_ack = true;
        return false;
    }

    for (auto& manin : elite) {
        manin.crowded = 0;
    }
    priceedge.resize(max_elite);
    timeedge.resize(max_elite);
    for (int i = 0; i < max_elite; ++i) {
        priceedge[i] = i;
        timeedge[i] = i;
    }
    p_t_inline();

    return true;
}

bool result::crowded_pbest(population::order candy, int index_cr) {
    p_best[index_cr].append(candy);

    priceedge_pbest[index_cr].clear();
    timeedge_pbest[index_cr].clear();
    priceedge_pbest[index_cr].resize(p_best[index_cr].size());
    timeedge_pbest[index_cr].resize(p_best[index_cr].size());
    for (int i = 0; i < p_best[index_cr].size(); ++i) {
        priceedge_pbest[index_cr][i] = i;
        timeedge_pbest[index_cr][i] = i;
    }

    p_t_inline_pbest(index_cr);

    if (p_best[index_cr].isEmpty()) {
        p_best[index_cr].remove(p_best[index_cr].size() - 1);
        for (auto& kidin : p_best[index_cr]) {
            kidin.crowded_pbest = 0;
        }
        priceedge_pbest[index_cr].resize(max_elite_pbest);
        timeedge_pbest[index_cr].resize(max_elite_pbest);
        for (int i = 0; i < max_elite_pbest; ++i) {
            priceedge_pbest[index_cr][i] = i;
            timeedge_pbest[index_cr][i] = i;
        }
        p_t_inline_pbest(index_cr);
        QMessageBox::critical(nullptr, "错误", QString("elite 为空，无法定义归一化范围！"));
        error_ack = true;
        return false;
    }

    p_t_inline_pbest(index_cr);

    int p_max = p_best[index_cr][priceedge_pbest[index_cr][0]].cost_with;
    int p_min = p_best[index_cr][priceedge_pbest[index_cr][p_best[index_cr].size() - 1]].cost_with;
    int t_max = p_best[index_cr][timeedge_pbest[index_cr][0]].time;
    int t_min = p_best[index_cr][timeedge_pbest[index_cr][p_best[index_cr].size() - 1]].time;

    double price_range = p_max - p_min;
    double time_range = t_max - t_min;

    double std_per = 0.0001;
    bool not_p = false, not_t = false;

    if (price_range <= std_per) {
        not_p = true;
        price_range = 1.0;
    }
    if (time_range <= std_per) {
        not_t = true;
        time_range = 1.0;
    }

    for (auto& kidin : p_best[index_cr]) {
        kidin.crowded_pbest = 0;
        kidin.p_cpbest = 0;
        kidin.p_c_pbest = 0;
        kidin.t_cpbest = 0;
        kidin.t_c_pbest = 0;
    }

    double x1 = not_p ? 0.0 : (p_best[index_cr][priceedge_pbest[index_cr][p_best[index_cr].size()-1]].cost_with - p_min) / price_range;
    double y1 = not_t ? 0.0 : (p_best[index_cr][priceedge_pbest[index_cr][p_best[index_cr].size()-1]].time - t_min) / time_range;
    double x2 = not_p ? 0.0 : (p_best[index_cr][timeedge_pbest[index_cr][p_best[index_cr].size()-1]].cost_with - p_min) / price_range;
    double y2 = not_t ? 0.0 : (p_best[index_cr][timeedge_pbest[index_cr][p_best[index_cr].size()-1]].time - t_min) / time_range;

    double A = y2 - y1;
    double B = x1 - x2;
    double C = x2 * y1 - x1 * y2;
    double denom = std::sqrt(A * A + B * B);
    if (denom < 1e-10) denom = 1e-10;

    for (auto& kidin : p_best[index_cr]) {
        double price_norm = not_p ? 0.0 : (kidin.cost_with - p_min) / price_range;
        double time_norm = not_t ? 0.0 : (kidin.time - t_min) / time_range;
        kidin.crowded_pbest = std::abs(A * price_norm + B * time_norm + C) / denom;
    }

    double max_crowded = std::numeric_limits<double>::lowest();
    population::order* max_point = nullptr;
    for (auto& kidin : p_best[index_cr]) {
        if (kidin.crowded_pbest > max_crowded) {
            max_crowded = kidin.crowded_pbest;
            max_point = &kidin;
        }
    }

    if (max_point) {
        int min_index = std::distance(p_best[index_cr].begin(), std::find_if(p_best[index_cr].begin(), p_best[index_cr].end(),
            [&](const population::order& p) { return &p == max_point; }));
        p_best[index_cr].remove(min_index);
    } else {
        p_best[index_cr].remove(p_best[index_cr].size() - 1);
        for (auto& kidin : p_best[index_cr]) {
            kidin.crowded_pbest = 0;
        }
        priceedge_pbest[index_cr].resize(max_elite_pbest);
        timeedge_pbest[index_cr].resize(max_elite_pbest);
        for (int i = 0; i < max_elite_pbest; ++i) {
            priceedge_pbest[index_cr][i] = i;
            timeedge_pbest[index_cr][i] = i;
        }
        p_t_inline_pbest(index_cr);
        QMessageBox::critical(nullptr, "错误", QString("无法找到最拥挤的解！"));
        error_ack = true;
        return false;
    }

    for (auto& kidin : p_best[index_cr]) {
        kidin.crowded_pbest = 0;
    }
    priceedge_pbest[index_cr].resize(max_elite_pbest);
    timeedge_pbest[index_cr].resize(max_elite_pbest);
    for (int i = 0; i < max_elite_pbest; ++i) {
        priceedge_pbest[index_cr][i] = i;
        timeedge_pbest[index_cr][i] = i;
    }
    p_t_inline_pbest(index_cr);

    return true;
}

bool result::crowded_save(population candidate) {
    elite_save.append(candidate);

    priceedge_save.clear();
    timeedge_save.clear();
    priceedge_save.resize(elite_save.size());
    timeedge_save.resize(elite_save.size());
    for (int i = 0; i < elite_save.size(); ++i) {
        priceedge_save[i] = i;
        timeedge_save[i] = i;
    }

    p_t_inline_save();

    if (elite.isEmpty()) {
        elite_save.remove(elite_save.size() - 1);
        for (auto& manin : elite_save) {
            manin.crowded = 0;
        }
        priceedge_save.resize(max_elite);
        timeedge_save.resize(max_elite);
        for (int i = 0; i < max_elite; ++i) {
            priceedge_save[i] = i;
            timeedge_save[i] = i;
        }
        p_t_inline_save();
        QMessageBox::critical(nullptr, "错误", QString("elite 为空，无法定义归一化范围！"));
        error_ack = true;
        return false;
    }

    p_t_inline();

    int p_max = elite[priceedge[0]].pricefit;
    int p_min = elite[priceedge[elite.size() - 1]].pricefit;
    int t_max = elite[timeedge[0]].timefit;
    int t_min = elite[timeedge[elite.size() - 1]].timefit;

    double price_range = p_max - p_min;
    double time_range = t_max - t_min;

    double std_per = 0.0001;
    bool not_p = false, not_t = false;

    if (price_range <= std_per) {
        not_p = true;
        price_range = 1.0;
    }
    if (time_range <= std_per) {
        not_t = true;
        time_range = 1.0;
    }

    for (auto& manin : elite_save) {
        manin.crowded = 0;
        manin.p_c = 0;
        manin.p_c_ = 0;
        manin.t_c = 0;
        manin.t_c_ = 0;
    }

    double x1 = not_p ? 0.0 : (elite[priceedge[elite.size()-1]].pricefit - p_min) / price_range;
    double y1 = not_t ? 0.0 : (elite[priceedge[elite.size()-1]].timefit - t_min) / time_range;
    double x2 = not_p ? 0.0 : (elite[timeedge[elite.size()-1]].pricefit - p_min) / price_range;
    double y2 = not_t ? 0.0 : (elite[timeedge[elite.size()-1]].timefit - t_min) / time_range;

    double A = y2 - y1;
    double B = x1 - x2;
    double C = x2 * y1 - x1 * y2;
    double denom = std::sqrt(A * A + B * B);
    if (denom < 1e-10) denom = 1e-10;

    for (auto& manin : elite_save) {
        double price_norm = not_p ? 0.0 : (manin.pricefit - p_min) / price_range;
        double time_norm = not_t ? 0.0 : (manin.timefit - t_min) / time_range;
        manin.crowded = std::abs(A * price_norm + B * time_norm + C) / denom;
    }

    double max_crowded = std::numeric_limits<double>::lowest();
    population* max_point = nullptr;
    for (auto& manin : elite_save) {
        if (manin.crowded > max_crowded) {
            max_crowded = manin.crowded;
            max_point = &manin;
        }
    }

    if (max_point) {
        int min_index = std::distance(elite_save.begin(), std::find_if(elite_save.begin(), elite_save.end(),
            [&](const population& p) { return &p == max_point; }));
        elite_save.remove(min_index);
    } else {
        elite_save.remove(elite_save.size() - 1);
        for (auto& manin : elite_save) {
            manin.crowded = 0;
        }
        priceedge_save.resize(max_elite);
        timeedge_save.resize(max_elite);
        for (int i = 0; i < max_elite; ++i) {
            priceedge_save[i] = i;
            timeedge_save[i] = i;
        }
        p_t_inline_save();
        QMessageBox::critical(nullptr, "错误", QString("无法找到最拥挤的解！"));
        error_ack = true;
        return false;
    }

    for (auto& manin : elite_save) {
        manin.crowded = 0;
    }
    priceedge_save.resize(max_elite);
    timeedge_save.resize(max_elite);
    for (int i = 0; i < max_elite; ++i) {
        priceedge_save[i] = i;
        timeedge_save[i] = i;
    }
    p_t_inline_save();

    return true;
}

bool result::dominate(population candidate) {
    int timing = 0;
    for (auto it = elite.begin(); it != elite.end(); /* no ++it here */) {

        bool check_only_one=true;
        if(candidate.timefit==it->timefit&&candidate.pricefit==it->pricefit){
            check_only_one=false;
            for(int i=0;i<candidate.order_num;i++){
                if(candidate.particles[i].exp_path!=it->particles[i].exp_path){
                    check_only_one=true;
                    break;
                }
            }
        }
        if(!check_only_one)return true;

        if ((candidate.timefit <= it->timefit && candidate.pricefit <= it->pricefit) &&
            (candidate.timefit < it->timefit || candidate.pricefit < it->pricefit)) {

            if (timing == 0) {
                *it = candidate;
                ++it;
            } else {
                it = elite.erase(it);
                continue;
            }
            timing++;
        } else {
            if ((candidate.timefit >= it->timefit && candidate.pricefit >= it->pricefit) &&
                (candidate.timefit > it->timefit || candidate.pricefit > it->pricefit)){
                be_dominate_flag=true;
                return false;
            }
            ++it;
        }
    }
    return timing > 0;
}

bool result::dominate_save(population candidate) {
    int timing = 0;
    for (auto it = elite_save.begin(); it != elite_save.end(); /* no ++it here */) {

        bool check_only_one=true;
        if(candidate.timefit==it->timefit&&candidate.pricefit==it->pricefit){
            check_only_one=false;
            for(int i=0;i<candidate.order_num;i++){
                if(candidate.particles[i].exp_path!=it->particles[i].exp_path){
                    check_only_one=true;
                    break;
                }
            }
        }
        if(!check_only_one)return true;

        if ((candidate.timefit <= it->timefit && candidate.pricefit <= it->pricefit) &&
            (candidate.timefit < it->timefit || candidate.pricefit < it->pricefit)) {

            if (timing == 0) {
                *it = candidate;
                ++it;
            } else {
                it = elite_save.erase(it);
                continue;
            }
            timing++;
        } else {
            if ((candidate.timefit >= it->timefit && candidate.pricefit >= it->pricefit) &&
                (candidate.timefit > it->timefit || candidate.pricefit > it->pricefit)){
                be_dominate_flag=true;
                return false;
            }
            ++it;
        }
    }
    return timing > 0;
}

bool result::dominate_pbest(population::order candy, int index_do) {
    int timing = 0;
    for (auto it = p_best[index_do].begin(); it != p_best[index_do].end(); /* no ++it here */) {

        bool check_only_one=true;
        if(candy.time==it->time&&candy.cost_with==it->cost_with){
            check_only_one=false;
                if(candy.exp_path!=it->exp_path){
                    check_only_one=true;
            }
        }
        if(!check_only_one)return true;

        if ((candy.time <= it->time && candy.cost_with <= it->cost_with) &&
            (candy.time < it->time || candy.cost_with < it->cost_with)) {

            if (timing == 0) {
                *it = candy;
                ++it;
            } else {
                it = p_best[index_do].erase(it);
                continue;
            }
            timing++;
        } else {
            if ((candy.time >= it->time && candy.cost_with >= it->cost_with) &&
                (candy.time > it->time || candy.cost_with > it->cost_with)){
                be_dominate_flag_pbest=true;
                return false;
            }
            ++it;
        }
    }
    return timing > 0;
}

void result::p_t_inline(){
    std::sort(priceedge.begin(), priceedge.end(), [&](int a, int b) {
        return elite[a].pricefit > elite[b].pricefit;
    });
    std::sort(timeedge.begin(), timeedge.end(), [&](int a, int b) {
        return elite[a].timefit > elite[b].timefit;
    });
}

void result::p_t_inline_save(){
    std::sort(priceedge_save.begin(), priceedge_save.end(), [&](int a, int b) {
        return elite_save[a].pricefit > elite_save[b].pricefit;
    });
    std::sort(timeedge_save.begin(), timeedge_save.end(), [&](int a, int b) {
        return elite_save[a].timefit > elite_save[b].timefit;
    });
}

void result::p_t_inline_pbest(int index_pt){
    std::sort(priceedge_pbest[index_pt].begin(), priceedge_pbest[index_pt].end(), [&](int a, int b) {
        return p_best[index_pt][a].cost_with > p_best[index_pt][b].cost_with;
    });
    std::sort(timeedge_pbest[index_pt].begin(), timeedge_pbest[index_pt].end(), [&](int a, int b) {
        return p_best[index_pt][a].time > p_best[index_pt][b].time;
    });
}

