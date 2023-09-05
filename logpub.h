#ifndef _LOGPUB_H_
#define _LOGPUB_H_

#include <thread>
#include <mutex>
#include <queue>
#include <iostream>
#include <cassert>

#include "order.h"
#include "user.h"
#include "tob.h"

// LogPub
class LogPub final
{
public:
	static constexpr size_t _buffer_size = 256;
	LogPub()
		: quit(false)
		, thread(&LogPub::process, this)
	{
	}
	~LogPub()
	{
		quit = true;
		thread.join();
		_flush_queue();
	}
	void queue_new_order(const Order& data)
	{
		snprintf(buffer, sizeof(buffer), "A: %d, %d", data.user_id, data.user_order_id);
		const std::lock_guard<std::mutex> lock(mutex);
		messages.emplace(std::forward<std::string>(std::string(buffer)));
	}

	void queue_cancel_order(const UserOrder& data)
	{
		snprintf(buffer, sizeof(buffer), "C: %d, %d", data.user_id, data.user_order_id);
		const std::lock_guard<std::mutex> lock(mutex);
		messages.emplace(std::forward<std::string>(std::string(buffer)));
	}

	void queue_trade(Side side, const Order& lhs, const Order& rhs, Qty qty)
	{
		const Order& buyer = (side == BUY) ? lhs : rhs;
		const Order& seller = (side == BUY) ? rhs : lhs;
		snprintf(buffer, sizeof(buffer), "T: %d, %d, %d, %d, %d, %d",
			buyer.user_id, buyer.user_order_id, seller.user_id, seller.user_order_id, seller.price, qty);
		const std::lock_guard<std::mutex> lock(mutex);
		messages.emplace(std::forward<std::string>(std::string(buffer)));
	}
	void queue_tob_changes(Side side, const TopOfBook& tob)
	{
		char token = (side == BUY) ? 'B' : 'S';
		if (tob.qty != 0)
		{
			assert(tob.qty >= 0);
			snprintf(buffer, sizeof(buffer), "B: %c, %d, %d", token, tob.price, tob.qty);
		}
		else
		{
			snprintf(buffer, sizeof(buffer), "B: %c, -, -", token);
		}
		const std::lock_guard<std::mutex> lock(mutex);
		messages.emplace(std::forward<std::string>(std::string(buffer)));
	}

	void process()
	{
		while (!quit)
		{
			_flush_queue();
		}
	}
private:
	void _flush_queue()
	{
		const std::lock_guard<std::mutex> lock(mutex);
		while (!messages.empty())
		{
			std::cout << messages.front() << std::endl;
			messages.pop();
		}
	}
	char buffer[_buffer_size] = { 0 };
	std::atomic<bool> quit;
	std::queue<std::string> messages;
	std::mutex mutex;
	std::thread thread;
};

#endif