#ifndef POPULATION_AND_ORDER_H
#define POPULATION_AND_ORDER_H

#include <QVector>
#include <QSet>


class population
{
public:
    population(int id);
    population();
    bool sleeping;
    double fitness;

    double p_c;
    double t_c;

    int power;

    double p_dis_up;
    double t_dis_up;

    double p_dis_down;
    double t_dis_down;

    double ang_up;
    double ang_down;

    double p_c_;
    double t_c_;

    double pricefit;
    double timefit;

    double crowded;


    int order_num;
    int P_ID;

    int linedex;

    int population_goods_num=5;

    bool c_o_s;

    bool C_or_S();

    bool arrived;
    void pricecount();
    void timecount();

    bool OK_Check();

    bool load_info();


    QSet<int> child_sisters;

    class order
    {
    public:
        order();
        order(int goods_ID,int order_num,int my_ID,int Pop_ID);
        int order_need=-5;
        int my_ID=-5;
        int goods_ID=-5;
        int end_ID=-5;
        int Pop_ID=-5;

        bool massage_order=true;

        bool false_are_not_brother_guard=false;



        bool have_brother=false;
        int old_brother=-1;
        QSet<int> my_brothers;

        bool and_0=false;
        bool and_1=false;
        int point_0=-1;
        int point_1=-1;



        int my_index=-5;

        double crowded_pbest=-5;

        double p_c_pbest=-5;
        double t_c_pbest=-5;

        double p_cpbest=-5;
        double t_cpbest=-5;

        bool merge_ing=false;

        order* merge_order=nullptr;


        double wait=0.5;
        double brother_point=0.5;

        int order_goods_num=5;

        order* brother_order=this;


        int index_ID=-5;
        static const int max_path=10;
        QVector<QVector<int>> exp_path;
    //    QVector<QVector<int>> last_path;

        double time_hold=-5;
        double time=-5;
        double cost=-5;
        double cost_with=-5;
        double hope_time=-5;
        double time_per=-5;

        bool wait_merge=false;

        void hello_help(int please_ID);



        void bad_review();
        double fitness=-5;
        int rank=-5;
        int server_num=-5;

        int follow=-5;

        int point_ID=-5;
        int now_ID=-5;

        int index=-5;

        int last_ID=-5;

        QVector<QVector<int>> allocation;
        QVector<QVector<QVector<bool>>> reach;
        void old_time();
        void allocate();

        void al_random();
        void older();
        void al_fit();
        void believe();

        void exp(population*boss);
        void simply_exp();
        int randomChoice();
        int randomHightway();



        bool arriving=false;

    };


    QVector<order> particles;
    QVector<QVector<int>> ID_crad;




};


extern bool error_ack;

extern bool debug_cut;

extern int message_num;

extern QVector<QHash<QPair<int,int>,int>> brother_check;

extern int order_num_for_result_pbest;

extern QVector<QVector<bool>> go_fuck_no_brother;

extern int popu_num;

extern bool massacall;
#endif // POPULATION_AND_ORDER_H
