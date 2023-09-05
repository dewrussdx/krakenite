#if _WIN32
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib") // Winsock Library
#endif
#include <string.h>
#include "server.h"
#include "filestream.h"
#include "proto.h"
#include "netio.h"

Server::Server(unsigned short port)
    : _port(port), _socket(INVALID_SOCKET), _server({0}), _client({0})
{
    _protobufs.reserve(64);
}

Server::~Server()
{
#if _WIN32
    closesocket(_socket);
    WSACleanup();
#else
    close(_socket);
#endif
    for (auto &pbuf : _protobufs)
    {
        delete pbuf;
    }
}

bool Server::init()
{
#if _WIN32
    // initialise winsock
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        std::cout << "ERROR: Winsock initialization failed: " << WSAGetLastError() << std::endl;
        return false;
    }
#endif
    // create a socket
    if ((_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
    {
        NetIO::error("socket() failed");
        return false;
    }

    // prepare the sockaddr_in structure
    memset((char *)&_server, 0, sizeof(_server));
    _server.sin_family = AF_INET;
    _server.sin_addr.s_addr = INADDR_ANY;
    _server.sin_port = htons(_port);

    // bind
    if (bind(_socket, (sockaddr *)&_server, sizeof(_server)) == SOCKET_ERROR)
    {
        NetIO::error("bind() failed");
        return false;
    }
    return true;
}

struct Callback : public ProtocolParser::Callback
{
    Callback(Server &server)
        : _server(server)
    {
    }
    virtual void new_order(const NetIO::NewOrder &data)
    {
        _server.add_protobuf(new NetIO::NewOrder(data));
    }

    virtual void cancel_order(const NetIO::CancelOrder &data)
    {
        _server.add_protobuf(new NetIO::CancelOrder(data));
    }

    virtual void flush_book(const NetIO::FlushBook &data)
    {
        _server.add_protobuf(new NetIO::FlushBook(data));
    }
    virtual void protocol_error()
    {
        std::cout << "ERROR: Protocol error (while parsing .csv file)" << std::endl;
        // TODO: _server.panic()
    }

private:
    Server &_server;
};

bool Server::_read_csv(const char *path)
{
    Callback callback(*this);
    FileStreamer streamer(path);
    ProtocolParser parser;

    streamer.process([&parser, &callback](std::string &&data) -> bool
                     {
        parser.parse(data.c_str(), data.size(), callback);
        return true; });

    return true;
}

bool Server::run()
{
#if VERBOSE
    std::cout << "sizeof():" << std::endl;
    std::cout << "- HandShake: " << sizeof(NetIO::Handshake) << std::endl;
    std::cout << "- NewOrder: " << sizeof(NetIO::NewOrder) << std::endl;
    std::cout << "- CancelOrder: " << sizeof(NetIO::CancelOrder) << std::endl;
    std::cout << "- FlushBook: " << sizeof(NetIO::FlushBook) << std::endl;
#endif
    _read_csv("../etc/input.csv");

    char client_addr[64] = {0};
    char buffer[1024] = {0};
#if PERF_TEST
    size_t num_trials = _perf_num_trials;
#else
    size_t num_trials = 1;
#endif
    while (true)
    {
        std::cout << "Waiting for client handshake..." << std::endl;

        // try to receive some data, this is a blocking call
        int msg_size;
        socklen_t slen = sizeof(_client);
        if ((msg_size = recvfrom(_socket, buffer, sizeof(buffer), 0, (sockaddr *)&_client, &slen)) == SOCKET_ERROR)
        {
            NetIO::error("recvfrom() failed");
            return false;
        }
        assert(msg_size == sizeof(NetIO::Handshake));

        // print details of the client/peer and the data received
        inet_ntop(AF_INET, &_client.sin_addr, client_addr, sizeof(client_addr));
        std::cout << "Handshake from " << client_addr << ":" << _client.sin_port << std::endl;

        for (auto i = 0; i < num_trials; i++)
        {
            for (NetIO::Base *pbuf : _protobufs)
            {
                int size = 0;
                switch (reinterpret_cast<const char *>(pbuf)[0])
                {
                case 'N':
                    size = sizeof(NetIO::NewOrder);
                    break;
                case 'C':
                    size = sizeof(NetIO::CancelOrder);
                    break;
                case 'F':
                    size = sizeof(NetIO::FlushBook);
                    break;
                }
                assert(size > 0 && !(size & 3));
                if (sendto(_socket, reinterpret_cast<const char *>(pbuf), size, 0,
                           (sockaddr *)&_client, sizeof(sockaddr_in)) == SOCKET_ERROR)
                {
                    NetIO::error("sendto() failed");
                }

                // Wait for client 'ack'
                if ((msg_size = recvfrom(_socket, buffer, sizeof(buffer), 0, (sockaddr *)&_client, &slen)) == SOCKET_ERROR)
                {
                    NetIO::error("recvfrom() failed");
                    return false;
                }
                assert(msg_size == sizeof(NetIO::Ack));
            }
        }
    }
    return true;
}
