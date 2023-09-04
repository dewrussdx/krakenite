#ifndef _USER_H_
#define _USER_H_

#include "types.h"
#include <string>

struct UserOrder final
{
	Uid user_id;
	Uid user_order_id;

	bool operator == (const UserOrder& rhs) const
	{
		return user_id == rhs.user_id && user_order_id == rhs.user_order_id;
	}
};

template<>
struct std::hash<UserOrder> final
{
	size_t operator()(const UserOrder& order) const
	{
		// simple hashfunction for 32-bit values which 
		// should be sufficient for this exercise
		return
			static_cast<size_t>(order.user_id) << 32 |
			static_cast<size_t>(order.user_order_id);
	}
};

#endif
