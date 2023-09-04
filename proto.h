#ifndef _PROTO_H_
#define _PROTO_H_

#include "orderbook.h"

template <size_t _token_size = 64>
struct ProtocolParser
{
public:
	explicit ProtocolParser(OrderBookManager& manager, char delim = ',')
		: manager(manager)
		, delim(delim)
		, active_book(nullptr)
	{
	}
	const char* tokenize(const char* str, size_t& pos, size_t size)
	{
		size_t cnt = 0;
		while (pos < size)
		{
			auto c = str[pos++];
			if (c != delim)
			{
				token[cnt++] = c;
			}
			else
			{
				token[cnt] = '\0';
				break;
			}
		}
		return cnt > 0 ? token : nullptr;
	}
	bool parse(const char* str, size_t size)
	{
		size_t pos = 0;
		switch (str[pos++])
		{
		case 'F':
		{
			if (!active_book)
			{
				return false;
			}
			pos++;
#if VERBOSE
			std::cout << "F: " << active_book->symbol << std::endl;
			std::cout << (*active_book);
#endif
			active_book->flush();
			return pos >= size;
		}
		case 'N':
		{
			pos++;
			Order order;
			order.user_id = static_cast<Uid>(std::stoi(tokenize(str, pos, size)));
			std::string symbol = tokenize(str, pos, size);
			order.price = static_cast<Price>(std::stoi(tokenize(str, pos, size)));
			order.qty = static_cast<Qty>(std::stoi(tokenize(str, pos, size)));
			Side side = (tokenize(str, pos, size)[0] == 'B') ? BUY : SELL;
			order.user_order_id = static_cast<Uid>(std::stoi(tokenize(str, pos, size)));
			active_book = manager(symbol);
#if VERBOSE
			std::cout << "N: " << order.user_id << ", " << symbol << ", " <<
				order.price << ", " << order.qty << ", " << (side == BUY ? 'B' : 'S') <<
				", " << order.user_order_id << std::endl;
#endif
			active_book->add_order(side, std::forward<Order>(order));
			return pos >= size;
		}
		case 'C':
		{
			if (!active_book)
			{
				return false;
			}
			pos++;
			UserOrder user_order;
			user_order.user_id = static_cast<Uid>(std::stoi(tokenize(str, pos, size)));
			user_order.user_order_id = static_cast<Uid>(std::stoi(tokenize(str, pos, size)));
#if VERBOSE
			std::cout << "C: " << user_order.user_id << ", " <<
				user_order.user_order_id << std::endl;
#endif
			active_book->cancel_order(std::forward<UserOrder>(user_order));
			return pos >= size;
		}
		default:
		{
			assert(false);
			return false;
		}
		}
	}

	OrderBookManager& manager;
	OrderBook* active_book;
	char token[_token_size] = { 0 };
	char delim;
};

#endif