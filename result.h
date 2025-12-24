#ifndef RESULT_H
#define RESULT_H

#include "population_and_order.h"


class result
{
public:
    result();
    void global_renew();
    double arfa=0.3;

    int max_elite=10;

    double a=0.1;
    double b=0.3;
    double c=0.6;

    bool disable_cut=false;

    int max_elite_pbest=10;

    QVector<population>elite;
    QVector<population>elite_save;


    QVector<QVector<population::order>> p_best;

    bool dominate_pbest(population::order candy,int index_do);
    bool crowded_pbest(population::order candy,int index_cr);
    bool be_dominate_flag_pbest=false;

    void catchorder(population::order kid,int index_ca);

    QVector<QVector<int>>priceedge_pbest;
    QVector<QVector<int>>timeedge_pbest;


    void catchelite(population man);
    QVector<int>priceedge;
    QVector<int>timeedge;


//    QVector<order>pbest;

    double save_point=0.5;


    QVector<int>priceedge_save;
    QVector<int>timeedge_save;

    void callcrowdedcount();
    void p_t_inline();

    QVector<double> poweredge;
    double for_power=0;

    bool elite_GA(int id);
    bool elite_PP(int id);

    population::order false_return(int my_in);

    population::order random_elite(int my_in);
    population::order random_elite_save(int my_in);
    population::order random_pbest(int my_in);

    void get_poweredge(int size);

    void p_t_inline_save();

    void p_t_inline_pbest(int index_pt);

    bool dominate(population candidate);

    bool crowded(population candidate,bool &is_in_top_30_percent, population& deleted_member);

    bool dominate_save(population candidate);

    bool crowded_save(population candidate);

    bool be_dominate_flag=false;

    bool be_dominate_flag_save=false;


    void last_change();

    int max_step=5000;




    void update_metrics(int iteration); // 更新每100次迭代的指标
    void dynamic_tune_parameters(int iteration); // 动态调整a, b, c

    QVector<population> prev_elite; // 上一次elite集合
    int iteration_count; // 迭代计数
    double last_p_max, last_p_min, last_t_max, last_t_min; // 上次边界

        // 动态调参指标
    struct Metrics {
        double boundary_step; // 边界更新步长
        double replacement_rate; // 替换率
        double avg_pricefit; // 平均价格适应度
        double avg_timefit; // 平均时间适应度
    };
    QVector<Metrics> metrics_history; // 存储每100次迭代的指标
    double prev_weighted_metric; // 上一次加权指标

};

extern result control_guy;

extern bool need_structure;

extern int hi_time;

//extern bool allow;

#endif // RESULT_H
