#ifndef EDGEDATA_H
#define EDGEDATA_H

#include <QVector>

class edgedata
{
public:
    edgedata();
    edgedata(double edgeLength,bool highway,double edgePrice,double edgeHighwayPrice);
    void setedgedata(double edgeLength,bool highway,double edgePrice,double edgeHighwayPrice);
    double edgeLength = 0;
    bool highway = false;
    double edgePrice = 0;
    double edgeHighwayPrice = 0;
    bool live=false;
    void takedown();
};


extern QVector<QVector<edgedata>> Data_V;

#endif // EDGEDATA_H
