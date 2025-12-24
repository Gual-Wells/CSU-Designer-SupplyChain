
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QCursor>
#include <QPixmap>
#include "graphscene.h"
#include "nodeitem.h"
#include <QOpenGLWidget>
#include "nodedata.h"
#include "population_and_order.h"
#include "result.h"
#include <QValueAxis>
#include <vector>
#include <QHash>
#include <cmath>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QLineSeries>
#include <Eigen/Dense>
#include <QDialog>
#include <QVBoxLayout>
#include <QTimer>
#include <QDoubleSpinBox>
#include <QLabel>
#include "chartwidget.h"
#include "choosedialog.h"
#include <QMessageBox>

#define population_num 5

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->B1, SIGNAL(clicked()), this, SLOT(follow1()));
    connect(ui->B2, SIGNAL(clicked()), this, SLOT(follow2()));
    connect(ui->C, SIGNAL(clicked()), this, SLOT(C_ACTION()));
    connect(ui->S, SIGNAL(clicked()), this, SLOT(S_ACTION()));
    connect(ui->B, SIGNAL(clicked()), this, SLOT(B_ACTION()));

    // **改成 GraphScene**
    scene = new GraphScene();

    // 设置超大场景
    scene->setSceneRect(0, 0, 8000000, 6000000);  // 设置超大场景
    scene->setItemIndexMethod(QGraphicsScene::BspTreeIndex);  // 使用四叉树索引

    ui->GV->setScene(scene);

    // 隐藏滚动条
    ui->GV->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->GV->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 确保视图初始化时居中并不会消失
    ui->GV->centerOn(scene->sceneRect().topLeft());  // 使画布初始居中

    // 启用抗锯齿和平滑图像缩放
    ui->GV->setRenderHint(QPainter::Antialiasing);
    ui->GV->setRenderHint(QPainter::SmoothPixmapTransform);

    // 确保视图的比例和场景匹配
    ui->GV->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);

    // 创建 FW 类的实例并将其设置为 'FW' 控件的子控件
    FW *fw = new FW(ui->FW);  // 创建文件浏览器控件
    ui->FW->setLayout(new QVBoxLayout);  // 清空现有布局
    ui->FW->layout()->addWidget(fw);    // 将文件浏览器控件添加到 FW 中

    ui->TW->setTabText(0, "控制台");
    ui->TW->setTabText(1, "数据导入");

    // Set up "参数设置" tab (index 2)
      QWidget *paramTab = new QWidget();
      QVBoxLayout *paramLayout = new QVBoxLayout(paramTab);
      QFormLayout *formLayout = new QFormLayout();

      // PSO Initial Global Best Probability
      globalProbSpinBox = new QDoubleSpinBox();
      globalProbSpinBox->setRange(0.0, 1.0);
      globalProbSpinBox->setSingleStep(0.01);
      globalProbSpinBox->setDecimals(2);
      globalProbSpinBox->setValue(control_guy.a);
      formLayout->addRow("PSO 初始全局最优概率:", globalProbSpinBox);

      // PSO Initial Individual Best Probability
      indivProbSpinBox = new QDoubleSpinBox();
      indivProbSpinBox->setRange(0.0, 1.0);
      indivProbSpinBox->setSingleStep(0.01);
      indivProbSpinBox->setDecimals(2);
      indivProbSpinBox->setValue(control_guy.b);
      formLayout->addRow("PSO 初始个体最优概率:", indivProbSpinBox);

      // PSO Initial Inertia Probability
      inertiaProbSpinBox = new QDoubleSpinBox();
      inertiaProbSpinBox->setRange(0.0, 1.0);
      inertiaProbSpinBox->setSingleStep(0.01);
      inertiaProbSpinBox->setDecimals(2);
      inertiaProbSpinBox->setValue(control_guy.c);
      formLayout->addRow("PSO 初始惯性概率:", inertiaProbSpinBox);

      // Suboptimal Front Rescue Rank
      suboptimalRankSpinBox = new QDoubleSpinBox();
      suboptimalRankSpinBox->setRange(0.0, 1.0);
      suboptimalRankSpinBox->setSingleStep(0.01);
      suboptimalRankSpinBox->setDecimals(2);
      suboptimalRankSpinBox->setValue(control_guy.save_point);
      formLayout->addRow("次优前沿解集营救排名:", suboptimalRankSpinBox);

      // Global Pareto Solution Limit
      globalLimitSpinBox = new QSpinBox();
      globalLimitSpinBox->setRange(1, 1000);
      globalLimitSpinBox->setValue(control_guy.max_elite);
      formLayout->addRow("全局最优帕累托解数量上限:", globalLimitSpinBox);

      // Individual Pareto Solution Limit
      indivLimitSpinBox = new QSpinBox();
      indivLimitSpinBox->setRange(1, 500);
      indivLimitSpinBox->setValue(control_guy.max_elite_pbest);
      formLayout->addRow("个体最优帕累托解数量上限:", indivLimitSpinBox);

      // Convergence Iteration Limit
      convLimitSpinBox = new QSpinBox();
      convLimitSpinBox->setRange(1, 10000);
      convLimitSpinBox->setValue(control_guy.max_step);
      formLayout->addRow("收敛上限次数:", convLimitSpinBox);

      // Time Window Perturbation Weight
      timeWindowSpinBox = new QDoubleSpinBox();
      timeWindowSpinBox->setRange(0.0, 1.0);
      timeWindowSpinBox->setSingleStep(0.01);
      timeWindowSpinBox->setDecimals(2);
      timeWindowSpinBox->setValue(control_guy.arfa);
      formLayout->addRow("拥挤度的均方根权:", timeWindowSpinBox);

      // Radio button for aggressive pruning strategy
      pruningRadio = new QRadioButton("启用剪枝激进时禁用策略");
      pruningRadio->setChecked(control_guy.disable_cut);
      formLayout->addRow(pruningRadio);

      paramLayout->addLayout(formLayout);
      paramLayout->addStretch();
      ui->TW->insertTab(2, paramTab, "参数设置");

    this->resize(1200, 800);
    S_Catch = scene;
}

MainWindow::~MainWindow()
{
    delete ui;
}

using namespace QtCharts;
using namespace Eigen;

// **设置光标**
void MainWindow::follow1() {
    carriage = 1;
    QApplication::setOverrideCursor(QCursor(QPixmap(":/qrc/storage.png")));
}

void MainWindow::follow2() {
    carriage = 2;
    QApplication::setOverrideCursor(QCursor(QPixmap(":/qrc/client.png")));
}

// **搜索函数（待实现）**
void MainWindow::search() {}

void MainWindow::C_ACTION() {
    //qDebug()<<"CC";
    for (auto &client : NodeIList) {
        if (client->cs && client->live) {
            client->DCE();
            //qDebug()<<client->live_ID;
        }
    }
}

void MainWindow::S_ACTION() {
    //qDebug()<<"SS";
    for (auto &server : NodeIList) {
        if (!server->cs && server->live) {
            server->DCE();
            //qDebug()<<server->live_ID;
        }
    }
}

void MainWindow::B_ACTION() {

    control_guy.a=0.1;
    control_guy.b=0.3;
    control_guy.c=0.6;


    control_guy.a = globalProbSpinBox->value();
    control_guy.b = indivProbSpinBox->value();
    control_guy.c = inertiaProbSpinBox->value();
    control_guy.save_point = suboptimalRankSpinBox->value();

    int max_a = control_guy.max_elite;



    int max_b = control_guy.max_elite_pbest;

    control_guy.max_elite = globalLimitSpinBox->value();
    control_guy.max_elite_pbest = indivLimitSpinBox->value();

    if(max_a>control_guy.max_elite){
        if(control_guy.elite.size()>control_guy.max_elite){
            control_guy.elite.resize(control_guy.max_elite);
        }
        if(control_guy.elite_save.size()>control_guy.max_elite){
            control_guy.elite_save.resize(control_guy.max_elite);
        }
    }

    if(max_b>control_guy.max_elite_pbest){
        for(int i=0;i<order_num_for_result_pbest;i++){
            if(control_guy.p_best[i].size()>control_guy.max_elite_pbest){
                control_guy.p_best[i].resize(control_guy.max_elite_pbest);
            }
        }
    }

    control_guy.max_step = convLimitSpinBox->value();
    control_guy.arfa = timeWindowSpinBox->value();
    control_guy.disable_cut = pruningRadio->isChecked();

    massacall=false;
    //qDebug()<<"BB";

    for (auto &path_find : NodeList) {
        path_find.path_can.clear();   // 清空所有路径
        path_find.can_num = 0;        // 初始化计数

        if (!path_find.cs && path_find.live) {
            for (int love = 0; love < 5; love++) {
                for (int i = 0; i < path_find.live_goods[love][0].size(); i++) {
                    path_find.live_goods[love][0][i] = path_find.goods_wants[0][i];
                    //qDebug()<<path_find.live_goods[0][i];
                }
            }
        }

        //qDebug()<<path_find.live_goods[0];
        if (!path_find.live) continue;

        for (int i = 0; i < ID; ++i) {
            if (Data_V[path_find.live_ID][i].live) {
                path_find.path_can.insert(i, 1.0); // 插入新路径，默认权重为 1.0
                ++path_find.can_num;               // 维护 can_num
            }
        }
    }

    QVector<population> experiment;

    //    qDebug()<<"inhere";

    order_num_for_result_pbest = 0;
    for (int i = 0; i < population_num; i++) {
        experiment.append(population(i));
        if (error_ack) {
            error_ack = false;
            return;
        }
    }

    control_guy.global_renew();
    go_fuck_no_brother.resize(experiment.size());

    for (auto &no_brother : go_fuck_no_brother) {
        no_brother.resize(NodeList.size());
    }

    for (auto &no_brother : go_fuck_no_brother) {
        for (auto &debug_fuck : no_brother) {
            debug_fuck = false;
        }
    }

    //    for(auto&A:experiment){
    //        for(auto&B:A.particles){
    //            qDebug()<<B.my_ID<<":"<<B.goods_ID;
    //        }
    //    }
    //    qDebug()<<"编译器发癫";
    //    experiment[0].particles[0].exp_path[1][0]=1;
    //    //qDebug()<<experiment[0].particles[0].exp_path[0][0]<<experiment[0].particles[0].exp_path[1][0];

    //qDebug()<<"inhere";

    //    for(auto &check : NodeList){
    //        qDebug()<<check.path_can;
    //    }
    //    for(auto&A:experiment){
    //        for(auto&B:A.particles){
    //            qDebug()<<B.my_ID<<":"<<B.goods_ID;
    //        }
    //    }
    //    qDebug()<<"inhere";
    for (auto &pop : experiment) {
        for (auto &path_find : NodeList) {
            if (!path_find.cs && path_find.live) {
                for (int i = 0; i < path_find.live_goods[pop.P_ID][0].size(); i++) {
                    path_find.live_goods[pop.P_ID][0][i] = path_find.goods_wants[0][i];
                }
                }
            }
            //        qDebug()<<"inhere";

            for (auto &ord : pop.particles) {
                //            qDebug()<<ord.my_ID<<"+"<<ord.goods_ID;
                //            qDebug()<<NodeList[0].live_goods[0];
                ord.al_random();
                //            qDebug()<<"1";
                if (error_ack) {
                    error_ack = false;
                    return;
                }
                //            qDebug()<<"2";
                if (ord.end_ID == -1) return;
                //            qDebug()<<"3";
                ord.exp(&pop);
                if (error_ack) {
                    error_ack = false;
                    return;
                }
                //            qDebug()<<"4";
                if (!ord.arriving) break;

            }
            //首次行为

            if (pop.OK_Check()) {
                if (pop.load_info()) {
                    control_guy.catchelite(pop);
                    if (error_ack) {
                        error_ack = false;
                        return;
                    }
                    //            qDebug()<<"抓住了"<<pop.P_ID;
                }
            }
        }

        //    qDebug()<<"hello";
        //    int lun=0;
        //    for(auto&pop : experiment){
        //        //qDebug()<<"第"<<++lun<<"种群";
        //        for(auto &ord : pop.particles){
        //            //qDebug()<<"订单ID"<<ord.my_ID;
        //            //qDebug()<<"订单分配仓库ID"<<ord.end_ID;
        //            //qDebug()<<"订单探询路径"<<ord.exp_path;
        //        }
        //    }
        //    for(auto&check:NodeList)
        //        if(!check.cs)
        //    qDebug()<<check.goods_wants<<check.live_goods;

        brother_check.clear();
        brother_check.resize(experiment.size());
        int timing = 0;
        while (timing<=control_guy.max_step&&control_guy.a<=0.8&&control_guy.c>=0.2) {
            qDebug() << timing;


            for (auto &pop : experiment) {
                double abc = QRandomGenerator::global()->generateDouble() * 10;
                if (abc <= control_guy.a && control_guy.elite.size() != 0) {
                    for (auto &path_find : NodeList)
                        qDebug() << path_find.live_goods[pop.P_ID][0];

                    for (auto &path_find : NodeList) {
                        if (!path_find.cs && path_find.live) {
                            for (int i = 0; i < path_find.live_goods[pop.P_ID][0].size(); i++) {
                                path_find.live_goods_copy[pop.P_ID][0][i] = path_find.live_goods[pop.P_ID][0][i];
                            }
                        }
                    }
                    qDebug() << "?1";
                    control_guy.elite_GA(pop.P_ID);

                    if (error_ack) {
                        error_ack = false;
                        return;
                    }
                    for (auto &path_find : NodeList) {
                        if (!path_find.cs && path_find.live) {
                            for (int i = 0; i < path_find.live_goods[pop.P_ID][0].size(); i++) {
                                path_find.live_goods[pop.P_ID][0][i] = path_find.live_goods_copy[pop.P_ID][0][i];
                            }
                        }
                    }
                    for (auto &path_find : NodeList)
                        qDebug() << path_find.live_goods[pop.P_ID][0];
                }
                else if (abc <= control_guy.a + control_guy.b) {
                    for (auto &path_find : NodeList) {
                        if (!path_find.cs && path_find.live) {
                            for (int i = 0; i < path_find.live_goods[pop.P_ID][0].size(); i++) {
                                path_find.live_goods_copy[pop.P_ID][0][i] = path_find.live_goods[pop.P_ID][0][i];
                            }
                        }
                    }

                    qDebug() << "?2";
                    control_guy.elite_PP(pop.P_ID);
                    if (error_ack) {
                        error_ack=false;
                        return;
                    }

                    for (auto &path_find : NodeList) {
                        if (!path_find.cs && path_find.live) {
                            for (int i = 0; i < path_find.live_goods[0].size(); i++) {
                                path_find.live_goods[pop.P_ID][0][i] = path_find.live_goods_copy[pop.P_ID][0][i];
                            }
                        }
                    }
                }
                else {
                    qDebug() << "?3";
                    pop.child_sisters.clear();

                    bool allow = true;
                    for (auto &allow_check : pop.particles) {
                        if (allow_check.end_ID < 0) {
                            allow = false;
                            break;
                        }
                    }
                    if (pop.C_or_S() && allow) {
                        for (auto &ording : pop.particles) {
                            ording.exp(&pop);
                            if (error_ack) {
                                error_ack = false;
                                return;
                            }
                            if (!ording.arriving) break;


                            //                qDebug()<<ording.reach[ording.end_ID][ording.my_ID];
                        }
                    }
                    else {
                        for (auto &path_find : NodeList)
                            qDebug() << path_find.live_goods[pop.P_ID][0];

                        for (auto &path_find : NodeList) {
                            if (!path_find.cs && path_find.live) {
                                for (int i = 0; i < path_find.live_goods[pop.P_ID][0].size(); i++) {
                                    path_find.live_goods[pop.P_ID][0][i] = path_find.goods_wants[0][i];
                                }
                            }
                        }

                        for (auto &path_find : NodeList)
                            qDebug() << path_find.live_goods[pop.P_ID][0];

                        brother_check[pop.P_ID].clear();

                        for (auto &ording : pop.particles) {
                            ording.al_random();

                            if (error_ack) {
                                error_ack = false;
                                return;
                            }

                            ording.brother_order = &ording;

                            if (ording.end_ID >= 0) {
                                QPair<int, int> pair(ording.my_ID, ording.end_ID);

                                go_fuck_no_brother[ording.Pop_ID][ording.my_ID] = false;
                                if (brother_check[pop.P_ID].contains(pair)) {
                                    ording.brother_order = &pop.particles[brother_check[pop.P_ID].value(pair)];

                                    ording.have_brother = true;
                                    ording.old_brother = pop.particles[brother_check[pop.P_ID].value(pair)].my_index;

                                    pop.particles[brother_check[pop.P_ID].value(pair)].my_brothers.insert(ording.my_index);
                                    for (auto &little_brother : pop.particles[brother_check[pop.P_ID].value(pair)].my_brothers) {
                                        pop.particles[little_brother].my_brothers = pop.particles[brother_check[pop.P_ID].value(pair)].my_brothers;
                                    }
                                }
                                else {
                                    brother_check[pop.P_ID].insert(pair, ording.my_index);
                                    ording.brother_order = &ording;

                                    ording.have_brother = true;
                                    ording.old_brother = ording.my_index;
                                }
                            }

                            if (error_ack) {
                                error_ack = false;
                                return;
                            }
                        }

                        bool allow_in = true;
                        for (auto &allow_in_check : pop.particles) {
                            if (allow_in_check.end_ID < 0) {
                                allow_in = false;
                                break;
                            }
                        }
                        if (allow_in) {
                            for (auto &ording : pop.particles) {
                                ording.exp(&pop);
                                if (error_ack) {
                                    error_ack = false;
                                    return;
                                }
                                if (!ording.arriving) break;

                            }
                        }
                    }

                    if (pop.OK_Check()) {
                        if (pop.load_info()) {
                            popu_num++;
                            //            qDebug()<<"guy";
                            control_guy.catchelite(pop);
                            //            if(popu_num==7)goto label;
                            if (error_ack) {
                                error_ack = false;
                                return;
                            }
                            qDebug() << "从多策略抓住了" << pop.P_ID;
                        }
                    }

                    //。。。统计fit
                    for (auto &guys : pop.particles) {
                        guys.merge_order = nullptr;
                        guys.wait_merge = false;
                        guys.brother_order = nullptr;
                        guys.arriving = false;
                    }
                }
            }
            timing++;
            if(timing%20==0){
                control_guy.update_metrics(timing);
                control_guy.dynamic_tune_parameters(timing);
            }
            //    qDebug()<<timing;
        }

        //测试版的多次循环

        for (auto &check : control_guy.elite) {
            //        qDebug()<<"!!种群ID"<<check.P_ID;
            //        qDebug()<<check.p_c<<" "<<check.pricefit<<"||"<<check.t_c<<" "<<check.timefit;
            for (auto &incheck : check.particles) {
                //            qDebug()<<"!订单ID"<<incheck.my_ID;
                //            qDebug()<<incheck.exp_path;
            }
        }

        if (experiment[0].order_num == 0) {
            QMessageBox::critical(nullptr, "错误", QString("不存在合法订单！"));
            error_ack = true;
            if (error_ack) {
                error_ack = false;
                return;
            }
        }
        qDebug() << control_guy.elite.size();
        control_guy.last_change();

        //    if(control_guy.elite.size()<control_guy.max_elite){
        //        qDebug()<<control_guy.elite.size();
        //        QMessageBox::critical(nullptr, "错误", QString("精英状态集不合法！"));
        //        error_ack=true;
        //        if(error_ack){
        //            error_ack=false;
        //            return;
        //        }
        //    }

    label:

        hi_time=timing;

        qDebug()<<"a:"<<control_guy.a<<",b:"<<control_guy.b<<",c:"<<control_guy.c;
        ChartWidget* chartWidget = showEliteChartWithFit(control_guy.elite);





          go_fuck_no_brother.clear();
          popu_num = 0;
          error_ack = false;



}


