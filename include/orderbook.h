#ifndef _ORDERBOOK_H_
#define _ORDERBOOK_H_

#include <iostream>
#include <set>
#include <array>

#include "types.h"
#include "order.h"
#include "history.h"
#include "logpub.h"

// Storage Types
typedef std::multiset<Order, OrderComparator<Order>> Storage;
typedef Storage BuyStorage;
struct SellStorage : Storage
{
	using Storage::Storage;
};

// Forward storage iteration
static std::ostream &operator<<(std::ostream &os, const BuyStorage &storage)
{
	os << " BUY" << std::endl;
	if (!storage.size())
		os << "  --EMPTY--" << std::endl;
	for (auto &order : storage)
		os << "   " << order << std::endl;
	return os;
}

// Reverse storage iteration
static std::ostream &operator<<(std::ostream &os, const SellStorage &storage)
{
	os << " SELL" << std::endl;
	if (!storage.size())
		os << "  --EMPTY--" << std::endl;
	for (auto it = storage.rbegin(); it != storage.rend(); ++it)
		os << "   " << (*it) << std::endl;
	return os;
}

// Defines an order book
class OrderBook final
{
public:
	std::string symbol;
	typedef OrderComparator<Order> Comparator;
	OrderBook(const std::string &symbol, size_t history_size = kHistorySize)
		: symbol(symbol)
	{
		_history.reserve(history_size);
	}
	OrderBook(const OrderBook &rhs) = delete;
	OrderBook &operator=(const OrderBook &rhs) = delete;

	void new_order(Side side, Order &&order)
	{
		order.epoch = ++_uid; // add "timestamp" (FIFO) for price>time ordering
		_history.emplace(std::forward<std::pair<UserOrder, Record>>(std::pair<UserOrder, Record>(
			std::forward<UserOrder>(UserOrder({order.user_id, order.user_order_id})),
			std::forward<Record>(Record({side, order})))));

		// Ack order received
		LogPub::instance().queue_new_order(order);

		// Find matching orders, handling MARKET and LIMIT orders
		if (!_match_order(side, order))
		{
			assert(order.price != 0); // Place only LIMIT orders in book

			// Update TOB $HACK$
			TopOfBook &tob = _tobs[side];
			if (order.price == tob.price)
			{
				tob.qty += order.qty;
				LogPub::instance().queue_tob_changes(side, tob);
			}
			else
			{
				if (!tob.price ||
					(side == BUY ? order.price > tob.price : order.price < tob.price))
				{
					tob.price = order.price;
					tob.qty = order.qty;
					LogPub::instance().queue_tob_changes(side, tob);
				}
			}
			_books[side].emplace(std::forward<Order>(order));
		}
	}

	bool cancel_order(UserOrder &&lhs_user_order)
	{
		// find order in _history
		History::iterator _it = _history.find(lhs_user_order);
		if (_it == _history.end())
		{
			return false; // not found
		}
		auto &rhs_user_order = (*_it).first;
		auto &record = (*_it).second;
		Storage &book = _books[record.side];
		auto &rhs = record.order;
		assert(lhs_user_order == rhs_user_order);
		TopOfBook &tob = _tobs[record.side];

		// iterate over LIMIT orders key (price > time)
		Storage::iterator it = book.find(rhs);
		while (it != book.end())
		{
			auto &lhs = (*it);
			if (lhs.user_id == rhs.user_id &&
				lhs.user_order_id == rhs.user_order_id)
			{
				// manage TOB
				bool tob_change = false;
				if (tob.price == lhs.price)
				{
					tob.qty -= lhs.qty;
					assert(tob.qty >= 0);
					tob_change = true;
				}

				// delete from book
				book.erase(it);

				// publish
				LogPub::instance().queue_cancel_order(lhs_user_order);

				if (tob_change)
				{
					_tob_change(record.side, tob, book);
				}

				return true;
			}
			else if (lhs.price != rhs.price)
			{
				// primary key has changed, no match
				break;
			}
			++it;
		}
		return false;
	}

	void flush()
	{
		_bids.clear();
		_asks.clear();
		_bidstob.flush();
		_askstob.flush();
		_history.clear();
		_uid = 0;
	}

	// Print book
	friend std::ostream &operator<<(std::ostream &os, const OrderBook &book)
	{
		os << book.symbol << std::endl;
		os << static_cast<SellStorage &>(book._asks);
		os << static_cast<BuyStorage &>(book._bids);
		return os;
	}

private:
	void _tob_change(Side side, TopOfBook &tob, Storage &book)
	{
		if (!tob.qty)
		{
			Price price = 0;
			Qty qty = 0;

			// count new prices
			// TODO: this is lame, but i'm running out of time
			// FIXME: Change data structure of orderbook to following
			//        unordered_map<Price, Level>
			//        Add struct Level
			//				- Qty total_qty;
			//				- std::map<Epoch, _sort_direction> bids;
			//				- std::map<Epoch, _sort_direction> asks;
			//		  Maintain total_qty at all times for the level
			//        => If ToB changes, and level changes, it's a simple lookup in the
			//			 hashmap, to get the total_qty
			for (auto it = book.begin(); it != book.end(); ++it)
			{
				if (!price)
				{
					price = (*it).price;
				}
				else if (price != (*it).price)
				{
					break;
				}
				qty += (*it).qty;
			}
			tob.price = price;
			tob.qty = qty;
		}
		LogPub::instance().queue_tob_changes(side, tob);
	}

	bool _match_order(Side side, Order &lhs)
	{
		// side specialization
		auto &cmp = comparator[side];
		Side oppo = side ^ 1;
		auto &rhs_book = _books[oppo];
		Type type = (lhs.price != 0) ? LIMIT : MARKET;

		// init iterator range for erase
		Storage::iterator rb(rhs_book.end());
		Storage::iterator re(rb);

		// find a match for the incoming order
		// note: if there's a match it should be at the beginning of the book
		bool tob_change = false;
		auto it = rhs_book.begin();
		while (it != rhs_book.end())
		{
			Order &rhs = const_cast<Order &>(*it);
			if (!lhs.price)
			{
				// For MARKET price set to first entry
				lhs.price = rhs.price;
			}
			if (cmp(lhs, rhs))
			{
				Qty qty = (lhs.qty <= rhs.qty) ? lhs.qty : rhs.qty;
				lhs.qty -= qty;
				rhs.qty -= qty;

				// Publish trade
				LogPub::instance().queue_trade(side, lhs, rhs, qty);

				// Modify TOB
				assert(rhs.price == _tobs[oppo].price);
				_tobs[oppo].qty -= qty;
				assert(_tobs[oppo].qty >= 0);
				tob_change = true;

				// check if the book entry needs to be removed (qty==0)
				if (!rhs.qty)
				{
					// obtain iterator range
					if (rb == rhs_book.end())
					{
						rb = it;
					}
					re = ++Storage::iterator(it);
				}
				// if incoming order was completely satisfied, we're done here
				if (!lhs.qty)
				{
					break;
				}
				// check next book entry for a match
				++it;
			}
			else
			{
				// no (additional) match
				break;
			}
		}
		// delete filled entries in the order book
		if (rb != rhs_book.end())
		{
			rhs_book.erase(rb, re);
		}
		if (tob_change)
		{
			_tob_change(oppo, _tobs[oppo], rhs_book);
		}

		// return True whether this incoming order was MARKET
		// or a LIMIT order completely satisfied
		return (type != LIMIT) || !lhs.qty;
	}
	// Book storage
	std::array<Storage, 2> _books = {
		Storage(Comparator(Comparator::greater)),
		Storage(Comparator(Comparator::less)),
	};
	// Set comparison functors
	std::array<OrderComparator<Order>, 2> comparator = {
		Comparator(Comparator::greater_equal),
		Comparator(Comparator::less_equal),
	};
	Storage &_bids = _books[BUY];
	Storage &_asks = _books[SELL];
	std::array<TopOfBook, 2> _tobs;
	TopOfBook &_bidstob = _tobs[BUY];
	TopOfBook &_askstob = _tobs[SELL];
	History _history;
	Uid _uid = 0;
};

// Orderbook manager
struct OrderBookManager final
{
	~OrderBookManager()
	{
		_clear();
	}
	OrderBook *operator()(const std::string &name)
	{
		auto it = _books.find(name);
		if (it == _books.end())
		{
			auto ref = _books.emplace(std::forward<std::pair<std::string, OrderBook *>>(
				std::pair<std::string, OrderBook *>(name, new OrderBook(name))));
			assert(ref.second);
			it = ref.first;
		}
		assert(it != _books.end());
		return (*it).second;
	}

private:
	void _clear()
	{
		auto it = _books.begin();
		while (it != _books.end())
		{
			delete (*it).second;
			++it;
		}
		_books.clear();
	}
	typedef std::unordered_map<std::string, OrderBook *> Books;
	Books _books;
};
#endif
