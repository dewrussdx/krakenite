#ifndef _HISTORY_H_
#define _HISTORY_H_

#include <unordered_map>
#include "user.h"
#include "order.h"

// Initial history size as elements in memory
constexpr size_t kHistorySize = 1000;

// History Record
struct Record final
{
	Record() = delete;
	Side side = 0;
	Order order;
};

typedef std::unordered_map<UserOrder, Record> History;

#endif
