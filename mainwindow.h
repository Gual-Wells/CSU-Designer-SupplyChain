#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QApplication>
#include <QWidget>
#include <QTabWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include "graphscene.h"
#include "fw.h"
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QRadioButton>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

extern int carriage;  // 全局变量声明

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


    int timing; // 迭代计数


    void process_population(population& pop, int timing); // 新增PSO处理方法

protected:

private:
    Ui::MainWindow *ui;

    FW *fw;
    GraphScene *scene;
    void drawImageAt(const QPointF &scenePos);

    QDoubleSpinBox *globalProbSpinBox; // PSO global probability
    QDoubleSpinBox *indivProbSpinBox; // PSO individual probability
    QDoubleSpinBox *inertiaProbSpinBox; // PSO inertia probability
    QDoubleSpinBox *suboptimalRankSpinBox; // Suboptimal rank
    QDoubleSpinBox *timeWindowSpinBox; // Time window weight
    QSpinBox *globalLimitSpinBox; // Global Pareto limit
    QSpinBox *indivLimitSpinBox; // Individual Pareto limit
    QSpinBox *convLimitSpinBox; // Convergence limit
    QRadioButton *pruningRadio; // Pruning strategy


private slots:
    void follow1();
    void follow2();
    void search();
    void C_ACTION();
    void B_ACTION();
    void S_ACTION();


};


#endif // MAINWINDOW_H
