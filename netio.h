#ifndef _NETIO_H_
#define _NETIO_H_

#include "types.h"

typedef char Pad;

/// <summary>
/// Network protocol buffers
/// Note: This isn't production ready,
///       UDP is ... unreliable.
/// </summary>
struct NetIO
{
	struct Handshake
	{
		char id = 'H';
		Pad pad[3] = { 0 };
	};

#pragma pack( push, 1 )
	struct Base
	{
	};
#pragma pack( pop )

#pragma pack( push, 1 )
	struct NewOrder : Base
	{
		// N,1,AAPL,10,100,B,1
		char id = 'N';
		Uid user_id = 0;
		char symbol[6] = { 0 };
		Price price = 0;
		Qty qty = 0;
		char side = 0;
		Uid user_order_id = 0;
	};
#pragma pack( pop )

#pragma pack( push, 1 )
	struct CancelOrder : Base
	{
		char id = 'C';
		Uid user_id = 0;
		Uid user_order_id = 0;
		Pad pad[3] = { 0 };
	};
#pragma pack( pop )

#pragma pack( push, 1 )
	struct FlushBook : Base
	{
		char id = 'F';
		Pad pad[3] = { 0 };
	};
#pragma pack (pop)

	const NetIO::Handshake handshake;
};

#endif
