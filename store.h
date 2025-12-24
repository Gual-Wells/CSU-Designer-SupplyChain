#ifndef STORE_H
#define STORE_H

#include <QHash>

class store
{
public:
    store();
    QHash<QString,int> goods;
    void goodsin(QString,int);
    void goodsout(QString,int);
};

#endif // STORE_H
