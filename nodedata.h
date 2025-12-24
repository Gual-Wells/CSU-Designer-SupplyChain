#ifndef NODEDATA_H
#define NODEDATA_H
#include <QVector>
#include <QList>
#include <QHash>


class nodedata
{
public:
    nodedata(double x,double y,bool live,bool cs);


    int colcount=5;
    bool live;
    bool cs;
    double x;
    double y;
    QVector <QVector<int>> goods_wants;
    int live_ID;
    QHash<int, double> path_can;
    int can_num;
    QVector <QVector<int>> live_goods[5];

    QVector <QVector<int>> live_goods_copy[5];

    QVector <QVector<double>> hope_times;

    bool explored;

};


extern QVector<nodedata>NodeList;
extern int ID;

#endif // NODEDATA_H
