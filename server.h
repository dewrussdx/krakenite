#ifndef _SERVER_H_
#define _SERVER_H_

#include <iostream>
#include <vector>
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
   
    void add_protobuf(NetIO::Base* pbuf)
    {
        _protobufs.push_back(pbuf);
    }

private:
    bool _read_csv(const char* path);

    unsigned short _port;
    SOCKET _socket;
    sockaddr_in _server;
    sockaddr_in _client;
    std::vector<NetIO::Base*> _protobufs;
};

#endif
