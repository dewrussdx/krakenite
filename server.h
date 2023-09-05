#ifndef _SERVER_H_
#define _SERVER_H_

#include <iostream>
#if _WIN32
#include <winsock2.h>
#endif
#include "netio.h"

class Server
{
public:
    Server(unsigned short port = 1234);
    ~Server();

    bool init();
    bool run();
   
private:
    bool _read_csv(const char* path);

    unsigned short _port;
    SOCKET _socket;
    sockaddr_in _server;
    sockaddr_in _client;
};

#endif
