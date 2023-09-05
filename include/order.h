#ifndef _ORDER_H_
#define _ORDER_H_

#include "types.h"
#include <functional>

// Defines an order
// Note: The comparison operators define price as primary key,
//       and epoch as secondary key
struct Order final
{
	Order()
		: user_id(0), user_order_id(0), price(0), epoch(0), qty(0)
	{
	}
	explicit Order(Uid user_id, Uid user_order_id, Price price, Epoch epoch, Qty qty)
		: user_id(user_id), user_order_id(user_order_id), price(price), epoch(epoch), qty(qty)
	{
	}
	bool operator<(const Order &rhs) const
	{
		// primary key
		if (price < rhs.price)
			return true;
		if (price > rhs.price)
			return false;
		// secondary key
		return epoch < rhs.epoch;
	}
	bool operator>(const Order &rhs) const
	{
		if (price > rhs.price)
			return true;
		if (price < rhs.price)
			return false;
		return epoch < rhs.epoch;
	}
	bool operator<=(const Order &rhs) const
	{
		if (operator<(rhs))
			return true;
		return price == rhs.price;
	}
	bool operator>=(const Order &rhs) const
	{
		if (operator>(rhs))
			return true;
		return price == rhs.price;
	}

	friend std::ostream &operator<<(std::ostream &os, const Order &order)
	{
		os << "(" << order.user_id << "," << order.user_order_id << "): $" << order.price << " x" << order.qty;
		return os;
	}
	// Order members
	// Note: side can be inferred by the specified book
	Uid user_id;
	Uid user_order_id;
	Price price;
	Epoch epoch;
	Qty qty;
};

// Order Comparison Functor
template <typename T = Order>
struct OrderComparator final
{
	typedef std::function<bool(const T &lhs, const T &rhs)> Handler;

	explicit OrderComparator(Handler handler = OrderComparator::less)
		: handler(std::bind(handler, std::placeholders::_1, std::placeholders::_2))
	{
	}
	OrderComparator(const OrderComparator &rhs) : handler(rhs.handler)
	{
	}
	bool operator()(const T &lhs, const T &rhs) const
	{
		return handler(lhs, rhs);
	}
	static bool less(const T &lhs, const T &rhs)
	{
		return lhs < rhs;
	}
	static bool less_equal(const T &lhs, const T &rhs)
	{
		return lhs <= rhs;
	}
	static bool greater(const T &lhs, const T &rhs)
	{
		return lhs > rhs;
	}
	static bool greater_equal(const T &lhs, const T &rhs)
	{
		return lhs >= rhs;
	}
	Handler handler;
};
#endif