#ifndef CLIENT_H
#define CLIENT_H

#include <QHash>

class client
{
public:
    client();
    QHash<QString,int> needs;
    void needsadd(QString,int);
    void needsdeal(QString,int);
    int priority;
};

#endif // CLIENT_H
