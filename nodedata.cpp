#include "nodedata.h"

nodedata::nodedata(double x,double y,bool live,bool cs)
{
    this->x=x;
    this->y=y;
    this->live=live;
    this->cs=cs;
    QVector<int> newCol(colcount,0);
    QVector<double> newColdou(colcount,0);

    this->goods_wants.append(newCol);
    this->hope_times.append(newColdou);

    for(int i=0;i<5;i++){
        live_goods[i].append(newCol);
        live_goods_copy[i].append(newCol);
    }

    for(int i=0;i<colcount;newCol[i-1]+=++i){}
    for(int i=0;i<colcount;newColdou[i-1]+=++i){}

    this->goods_wants.append(newCol);
    this->hope_times.append(newColdou);
    this->live_ID=ID++;
    can_num=0;
}
