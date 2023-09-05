#ifndef _NETIO_H_
#define _NETIO_H_

#include <cstring>
#include "types.h"

// deal with platform differences
#if _WIN32
#include <winsock2.h>
#else
typedef int SOCKET;
#define SOCKET_ERROR (-1)
#define INVALID_SOCKET (~0)
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

typedef char Pad;

/// <summary>
/// Network protocol buffers
/// Quick and dirty implementation, just tested for happy path
/// </summary>
struct NetIO
{
	struct Handshake
	{
		char id = 'H';
		Pad pad[3] = {0};
	};
	struct Ack
	{
		char id = '~';
		Pad pad[3] = {0};
	};
#pragma pack(push, 1)
	struct Base
	{
	};
#pragma pack(pop)

#pragma pack(push, 1)
	struct NewOrder : Base
	{
		// N,1,AAPL,10,100,B,1
		char id = 'N';
		Uid user_id = 0;
		char symbol[6] = {0};
		Price price = 0;
		Qty qty = 0;
		char side = 0;
		Uid user_order_id = 0;
	};
#pragma pack(pop)

#pragma pack(push, 1)
	struct CancelOrder : Base
	{
		char id = 'C';
		Uid user_id = 0;
		Uid user_order_id = 0;
		Pad pad[3] = {0};
	};
#pragma pack(pop)

#pragma pack(push, 1)
	struct FlushBook : Base
	{
		char id = 'F';
		Pad pad[3] = {0};
	};
#pragma pack(pop)

	static void error(const char *msg)
	{
#if _WIN32
		std::cout << "ERROR : " << msg << " : " << WSAGetLastError() << std::endl;
#else
		char *details = strerror(errno);
		std::cout << "ERROR : " << msg << " : " << details << std::endl;
#endif
	}
	NetIO::Handshake handshake;
	NetIO::Handshake ack;
};

#endif
