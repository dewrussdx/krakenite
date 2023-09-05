#include "client.h"

#if _WIN32
#include <ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib") 
//#pragma warning(disable:4996) 
#endif

Client::Client(const char* target, unsigned short port)
    : _target(target)
    , _port(port)
    , _socket(INVALID_SOCKET)
    , _server({ 0 })
{
}
Client::~Client()
{
    closesocket(_socket);
    WSACleanup();
}

bool Client::init()
{
#if _WIN32
    WSADATA ws;
    if (WSAStartup(MAKEWORD(2, 2), &ws) != 0)
    {
        std::cout << "ERROR: Winsock initialization failed: " << WSAGetLastError() << std::endl;
        return false;
    }
#endif
    // create socket
    if ((_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
    {
        std::cout << "Unable to create socket: " << WSAGetLastError() << std::endl;
        return false;
    }

    // setup address structure
    memset(&_server, 0, sizeof(_server));
    _server.sin_family = AF_INET;
    _server.sin_port = htons(_port);
    inet_pton(AF_INET, _target.c_str(), &_server.sin_addr);
    //_server.sin_addr.S_un.S_addr = inet_addr(_target.c_str());
    return true;
}

bool Client::run()
{
    // start communication
    char buffer[1024];
    while (true)
    {
        // send handshake to server
        if (sendto(_socket, reinterpret_cast<const char*>(& _netio.handshake), sizeof(NetIO::Handshake),
            0, (sockaddr*)&_server, sizeof(sockaddr_in)) == SOCKET_ERROR)
        {
            std::cout << "sendto() failed: " << WSAGetLastError() << std::endl;
            return false;
        }
   
        int slen = sizeof(sockaddr_in);
        int msg_size;
        if ( (msg_size = recvfrom(_socket, buffer, sizeof(buffer), 0, (sockaddr*)&_server, &slen)) == SOCKET_ERROR)
        {
            std::cout << "recvfrom() failed: " << WSAGetLastError() << std::endl;
            return false;
        }
        std::cout << buffer << std::endl;
    }
    return true;
}
