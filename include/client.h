#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <string>
#include <iostream>
#include "netio.h"

class Client
{
public:
    Client(const char *target = "127.0.0.1", unsigned short port = 1234);
    ~Client();

    bool init();
    bool run();

private:
    std::string _target;
    unsigned short _port;
    SOCKET _socket;
    struct sockaddr_in _server;
    NetIO _netio;
};

#endif
