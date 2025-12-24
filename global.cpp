#include "nodedata.h"
#include "edgedata.h"
#include "nodeitem.h"
#include "population_and_order.h"
#include "graphscene.h"
#include "chartwidget.h"
#include "result.h"

QVector<QVector<edgedata>> Data_V;
QVector<nodedata>NodeList;
QVector<NodeItem*>NodeIList;
int ID=0;

bool error_ack=false;
bool calledge=true;

GraphScene* S_Catch;

int carriage=0;

int warehouseCount=1;
int clientCount=1;

double choose=0.7;
population* survive=nullptr;


int hi_time=0;


bool debug_cut=false;

int message_num=0;

QVector<QHash<QPair<int,int>,int>> brother_check;

int order_num_for_result_pbest=0;


result control_guy;

QVector<QVector<bool>> go_fuck_no_brother;

int popu_num=0;

bool need_structure=false;

bool massacall=false;

//bool allow=true;


//只能main引用一次
