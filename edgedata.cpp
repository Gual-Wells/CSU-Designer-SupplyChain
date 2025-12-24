#include "edgedata.h"

edgedata::edgedata()
{

}
edgedata::edgedata(double edgeLength,bool highway,double edgePrice,double edgeHighwayPrice){
    this->edgeLength=edgeLength;
    this->highway=highway;
    this->edgePrice=edgePrice;
    this->edgeHighwayPrice=edgeHighwayPrice;
}
void edgedata::setedgedata(double edgeLength,bool highway,double edgePrice,double edgeHighwayPrice){
    this->edgeLength=edgeLength;
    this->highway=highway;
    this->edgePrice=edgePrice;
    this->edgeHighwayPrice=edgeHighwayPrice;
}
void edgedata::takedown(){
    edgeLength = 0;
    highway = false;
    edgePrice = 0;
    edgeHighwayPrice = 0;
    live=false;
}
