#if _WIN32
#include <ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib") // Winsock Library
//#pragma warning(disable:4996) 
#endif

#include "server.h"
#include "filestream.h"
#include "proto.h"

Server::Server(unsigned short port)
    : _port(port)
    , _socket(INVALID_SOCKET)
    , _server({ 0 })
    , _client({ 0 })
{
}

Server::~Server()
{
    closesocket(_socket);
    WSACleanup();
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
    if ((_socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
    {
        std::cout << "ERROR: Unable to create socket: " << WSAGetLastError() << std::endl;
        return false;
    }

    // prepare the sockaddr_in structure
    _server.sin_family = AF_INET;
    _server.sin_addr.s_addr = INADDR_ANY;
    _server.sin_port = htons(_port);

    // bind
    if (bind(_socket, (sockaddr*)&_server, sizeof(_server)) == SOCKET_ERROR)
    {
        std::cout << "ERROR: Bind failed: " << WSAGetLastError() << std::endl;
        return false;
    }
    return true;
}

struct Callback : public ProtocolParser::Callback
{
    virtual void new_order(const NetIO::NewOrder&)
    {
    }
    virtual void cancel_order(const NetIO::CancelOrder&)
    {
    }
    virtual void flush_book(const NetIO::FlushBook&)
    {
    }
    virtual void protocol_error()
    {
    }
};

bool Server::_read_csv(const char* path)
{
    Callback callback;
    FileStreamer streamer(path);
    ProtocolParser parser;

    streamer.process([&parser, &callback](std::string&& data) -> bool {
        parser.parse(data.c_str(), data.size(), callback);
        return true;
        });

    return true;
}

bool Server::run()
{
    char client_addr[64] = { 0 };
    char buffer[1024] = { 0 };
    while (true)
    {
        std::cout << "Waiting for client handshake..." << std::endl;

        // try to receive some data, this is a blocking call
        int msg_size;
        int slen = sizeof(sockaddr_in);
        if ((msg_size = recvfrom(_socket, buffer, sizeof(buffer), 0, (sockaddr*)&_client, &slen)) == SOCKET_ERROR)
        {
            std::cout << "ERROR: recvfrom() failed: " << WSAGetLastError() << std::endl;
            return false;
        }

        // print details of the client/peer and the data received
        inet_ntop(AF_INET, &_client.sin_addr, client_addr, sizeof(client_addr));
        std::cout << "recvfrom(): " << msg_size << " bytes " << client_addr
            << ":" << _client.sin_port << std::endl;
        std::cout << buffer << std::endl;

#if 0 // TODO: Loop send data
        // reply the client with 2the same data
        if (sendto(_socket, message, strlen(message), 0, (sockaddr*)&client, sizeof(sockaddr_in)) == SOCKET_ERROR)
        {
            printf("sendto() failed with error code: %d", WSAGetLastError());
            return 3;
        }
#endif

    }
}