#ifndef SOCKETCOMM_H
#define SOCKETCOMM_H
#include <QString>

class SocketComm
{
public:
    static QString tcpFetch(const QString& args);
};

#endif // SOCKETCOMM_H
