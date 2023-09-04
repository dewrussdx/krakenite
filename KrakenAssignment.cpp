#ifdef _WIN32
#pragma warning(disable : 4710) // function not inlined
#pragma warning(disable : 4711) // function selected for inline expansion
// #pragma warning(disable : 4514) // unreferenced inline function has been removed

#define VERBOSE (CODE_DEBUG || 0)
#if !CODE_DEBUG
#define _HAS_EXCEPTIONS (0)
#endif
#endif

#include <set>
#include <vector>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <functional>
#include <cassert>
#include <array>
#include <fstream>
#include <string>
#include <unordered_map>

// Custom types
typedef int Uid;	//!< Order UID
typedef int Qty;	//!< Order Quantity
typedef int Price;	//!< Order Pricing
typedef int Epoch;	//!< Order Epoch
typedef char Side;	//!< Side 'B' or 'S' 
typedef char Type;	//!< Order Type (MARKET or LIMIT)

// Order side mapping to (0,1) which simplies indexing of buy/sell book
constexpr Side BUY = 0;
constexpr Side SELL = 1;

// Order types
constexpr Type MARKET = 'M';
constexpr Type LIMIT = 'L';

// Initial history size as elements in memory
constexpr size_t kHistorySize = 1000;

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
		: user_id(user_id), user_order_id(user_order_id), price(price), epoch(epoch),qty(qty)
	{
	}
	bool operator<(const Order& rhs) const
	{
		// primary key
		if (price < rhs.price)
			return true;
		if (price > rhs.price)
			return false;
		// secondary key
		return epoch > rhs.epoch;
	}
	bool operator>(const Order& rhs) const
	{
		if (price > rhs.price)
			return true;
		if (price < rhs.price)
			return false;
		return epoch < rhs.epoch;
	}
	bool operator<=(const Order& rhs) const
	{
		if (operator<(rhs))
			return true;
		return price == rhs.price;
	}
	bool operator>=(const Order& rhs) const
	{
		if (operator>(rhs))
			return true;
		return price == rhs.price;
	}

	friend std::ostream& operator<<(std::ostream& os, const Order& order)
	{
		os << "(" << order.user_id << "," << order.user_order_id << 
			"): $" << order.price << " x" << order.qty;
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
	typedef std::function<bool(const T& lhs, const T& rhs)> Handler;

	explicit OrderComparator(Handler handler = OrderComparator::less)
		: handler(std::bind(handler, std::placeholders::_1, std::placeholders::_2))
	{
	}
	OrderComparator(const OrderComparator& rhs) : handler(rhs.handler)
	{
	}
	bool operator()(const T& lhs, const T& rhs) const
	{
		return handler(lhs, rhs);
	}
	static bool less(const T& lhs, const T& rhs)
	{
		return lhs < rhs;
	}
	static bool less_equal(const T& lhs, const T& rhs)
	{
		return lhs <= rhs;
	}
	static bool greater(const T& lhs, const T& rhs)
	{
		return lhs > rhs;
	}
	static bool greater_equal(const T& lhs, const T& rhs)
	{
		return lhs >= rhs;
	}
	Handler handler;
};

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

// History Record
struct Record final
{
	Record() = delete;
	Side side = 0;
	Order order;
};
typedef std::unordered_map<UserOrder, Record> History;

// Storage Types
typedef std::multiset<Order, OrderComparator<Order>> Storage;
typedef Storage BuyStorage;
struct SellStorage : Storage
{
	using Storage::Storage;
};

// Forward storage iteration
std::ostream& operator<<(std::ostream& os, const BuyStorage& storage)
{
	os << " BUY" << std::endl;
	if (!storage.size())
		os << "  --EMPTY--" << std::endl;
	for (auto& order : storage)
		os << "   " << order << std::endl;
	return os;
}

// Reverse storage iteration
std::ostream& operator<<(std::ostream& os, const SellStorage& storage)
{
	os << " SELL" << std::endl;
	if (!storage.size())
		os << "  --EMPTY--" << std::endl;
	for (auto it = storage.rbegin(); it != storage.rend(); ++it)
		os << "   " << (*it) << std::endl;
	return os;
}

#include <queue>
// Publisher
struct Publisher final
{
	void queue(std::string&& msg)
	{
		messages.emplace(std::forward<std::string>(msg));
		process();
	}

	void process()
	{
		while (!messages.empty())
		{
			std::string msg = messages.front();
			std::cout << msg << std::endl;
			messages.pop();
		}
	}
	std::queue<std::string> messages;
};

// Defines an order book
class OrderBook final
{
public:
	typedef OrderComparator<Order> Comparator;
	std::string symbol;
	OrderBook(const std::string symbol, size_t history_size = kHistorySize)
		: symbol(symbol)
	{
		history.reserve(history_size);
	}
	OrderBook(const OrderBook& rhs) = delete;
	OrderBook& operator=(const OrderBook& rhs) = delete;

	void add_order(Side side, Order&& order)
	{
		char buf[128];
		snprintf(buf, sizeof(buf), "A: %d, %d", order.user_id, order.user_order_id);
		publisher.queue(buf);
		order.epoch = ++uid; // add "timestamp" (FIFO) for price>time ordering
		history.emplace(std::forward<std::pair<UserOrder, Record>>(std::pair<UserOrder, Record>(
			std::forward<UserOrder>(UserOrder({order.user_id, order.user_order_id})), 
			std::forward<Record>(Record({side, order})))));
		
		// Find matching orders, handling MARKET and LIMIT orders
		if (!match_order(side, order))
		{
			assert(order.price != 0); // Place only LIMIT orders in book
			books[side].emplace(std::forward<Order>(order));
		}
	}

	bool cancel_order(UserOrder&& lhs_user_order)
	{
		// find order in history
		History::iterator _it = history.find(lhs_user_order);
		if (_it == history.end())
		{
			return false; // not found
		}
		auto& rhs_user_order = (*_it).first;
		auto& record = (*_it).second;
		Storage& storage = books[record.side];
		auto& rhs = record.order;
		assert(lhs_user_order == rhs_user_order);

		// iterate over LIMIT orders (primary,secondary) key (price,time)
		Storage::iterator it = storage.find(rhs);
		while (it != storage.end())
		{
			auto& lhs = (*it);
			if (lhs.user_id == rhs.user_id &&
				lhs.user_order_id == rhs.user_order_id)
			{
				// delete from book
				storage.erase(it);
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
		bids.clear();
		asks.clear();
		history.clear();
		uid = 0;
	}

	// Print book
	friend std::ostream& operator << (std::ostream& os, const OrderBook& book)
	{
		os << book.symbol << std::endl;
		os << static_cast<SellStorage&>(book.asks);
		os << static_cast<BuyStorage&>(book.bids);
		return os;
	}
private:
	bool match_order(Side side, Order& lhs)
	{
		// side specialization
		auto& cmp = comparator[side];
		auto& rhs_book = books[side ^ 1];
#if VERBOSE
		std::cout << (*this);
#endif
		Type type = (lhs.price != 0) ? LIMIT : MARKET;

		// init iterator range for erase 
		Storage::iterator rb(rhs_book.end());
		Storage::iterator re(rb);

		// find a match for the incoming order
		// note: if there's a match it should be at the beginning of the book
		auto it = rhs_book.begin();
		while (it != rhs_book.end())
		{
			Order& rhs = const_cast<Order&>(*it);
			if (!lhs.price)
			{
				// For MARKET price set to first entry
				lhs.price = rhs.price;
			}
			if (cmp(lhs, rhs))
			{
#if VERBOSE
				std::cout << (type==LIMIT ? "LIMIT" : "MARKET") << " Match : " << lhs << " with " << rhs << std::endl;
#endif
				Qty qty = (lhs.qty <= rhs.qty) ? lhs.qty : rhs.qty;
				lhs.qty -= qty;
				rhs.qty -= qty;

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
		// return True whether this incoming order was MARKET
		// or a LIMIT order completely satisfied
		return (type != LIMIT) || !lhs.qty;
	}

	// Book storage
	std::array<Storage, 2> books = {
		Storage(Comparator(Comparator::greater)),
		Storage(Comparator(Comparator::less)),
	};
	// Set comparison functors
	std::array<OrderComparator<Order>, 2> comparator = {
		Comparator(Comparator::greater_equal),
		Comparator(Comparator::less_equal),
	};
	Storage& bids = books[0];
	Storage& asks = books[1];
	History history;
	Uid uid = 0;
	Publisher publisher;
};

// Return the elapsed time in specified resolution
template <
	class result_t = std::chrono::milliseconds,
	class clock_t = std::chrono::steady_clock,
	class duration_t = std::chrono::milliseconds>
auto since(const std::chrono::time_point<clock_t, duration_t>& start)
{
	return std::chrono::duration_cast<result_t>(clock_t::now() - start);
}

// File streamer
struct FileStreamer final
{
	static constexpr std::streamsize kDefaultBufferSize = 16;
	FileStreamer(const std::string& path)
		: handle(path.c_str())
	{		
	}	
	~FileStreamer()
	{
		close();
	}
	bool file_exists() const 
	{
		return handle.good();
	}
	bool process(std::function<bool(std::string&& data)> handler)
	{
		if (!handle.is_open())
		{
			std::cout << "ERROR: File not open.";
			return false;
		}
		std::string line;
		while (std::getline(handle, line))
		{
			// Note: std::forward will invalidate line
			if (!handler(std::forward<std::string>(line)))
			{
				return false;
			}
		}
		return true;
	}
	void rewind()
	{
		handle.clear();
		handle.seekg(0);
	}
	void close()
	{
		handle.close();
	}
private:
	bool _error(const char* msg)
	{
		std::cout << msg << std::endl;
		handle.close();
		return false;
	}
	std::ifstream handle;
};

// Orderbook manager
struct OrderBookManager final
{
	~OrderBookManager()
	{
		clear();
	}
	OrderBook* operator()(const std::string& name)
	{
		auto it = books.find(name);
		if (it == books.end())
		{
			auto ref = books.emplace(std::forward<std::pair<std::string, OrderBook*>>(
				std::pair<std::string, OrderBook*>(name, new OrderBook(name))));
			assert(ref.second); 
			it = ref.first;
		}
		assert(it != books.end());
		return (*it).second;
	}
	void clear()
	{
		auto it = books.begin();
		while (it != books.end())
		{
			delete (*it).second;
			++it;
		}
		books.clear();
	}
	typedef std::unordered_map<std::string, OrderBook*> Books;
	Books books;
};

template <size_t _token_size=64>
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
	bool parse(const char* str, size_t size )
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
#endif
				std::cout << (*active_book);
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

class UnitTests
{
public:
	UnitTests(const std::string& file_path)
		: book_manager()
		, file_streamer(file_path.c_str())
		, scenario_count(0)
		, parser(book_manager)
	{
		assert(file_streamer.file_exists());
	}
	bool run()
	{
		scenario_count = 0;	
		std::cout << "Scenario #1" << std::endl;
		auto result = file_streamer.process([this](std::string&& data) -> bool {
			if (data[0] == 'F')
			{
				scenario_count++;
				std::cout << "\nScenario #" << scenario_count << std::endl;
			}
			auto result = parser.parse(data.c_str(), data.size());
			return true;
			});
		std::cout << "Scenarios processed: " << scenario_count << std::endl;
		return result;
	}
	OrderBookManager book_manager;
	FileStreamer file_streamer;
	uint32_t scenario_count;
	ProtocolParser<64> parser;
};

// Entry point
int main()
{
	// set the default precision to two decimal points
	std::cout << std::fixed << std::setprecision(2) << std::setfill(' ');
	assert(UnitTests("input_trimmed.csv").run());
}
