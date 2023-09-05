#ifndef _LOGPUB_H_
#define _LOGPUB_H_

#include <thread>
#include <mutex>
#include <queue>
#include <iostream>
#include <cassert>
#include <cstdarg>

#include "order.h"
#include "user.h"
#include "tob.h"

// LogPub
class LogPub final
{
public:
	static constexpr size_t _buffer_size = 256;

	static LogPub &instance()
	{
		static LogPub _logpub;
		return _logpub;
	}
	~LogPub()
	{
		_stop = true;
		_thread.join();
	}
	void start_recording()
	{
		_recording_buffer.clear();
		_recording = true;
	}
	const std::vector<std::string> &stop_recording()
	{
		_flush_queue();
		_recording = false;
		return _recording_buffer;
	}
	void queue_new_order(const Order &data)
	{
		snprintf(_buffer, sizeof(_buffer), "A, %d, %d", data.user_id, data.user_order_id);
		_add(std::forward<std::string>(std::string(_buffer)));
	}

	void queue_cancel_order(const UserOrder &data)
	{
		snprintf(_buffer, sizeof(_buffer), "C, %d, %d", data.user_id, data.user_order_id);
		_add(std::forward<std::string>(std::string(_buffer)));
	}

	void queue_trade(Side side, const Order &lhs, const Order &rhs, Qty qty)
	{
		const Order &buyer = (side == BUY) ? lhs : rhs;
		const Order &seller = (side == BUY) ? rhs : lhs;
		snprintf(_buffer, sizeof(_buffer), "T, %d, %d, %d, %d, %d, %d",
				 buyer.user_id, buyer.user_order_id, seller.user_id, seller.user_order_id, seller.price, qty);
		_add(std::forward<std::string>(std::string(_buffer)));
	}
	void queue_tob_changes(Side side, const TopOfBook &tob)
	{
		char token = (side == BUY) ? 'B' : 'S';
		if (tob.qty != 0)
		{
			assert(tob.qty >= 0);
			snprintf(_buffer, sizeof(_buffer), "B, %c, %d, %d", token, tob.price, tob.qty);
		}
		else
		{
			snprintf(_buffer, sizeof(_buffer), "B, %c, -, -", token);
		}
		_add(std::forward<std::string>(std::string(_buffer)));
	}

	void queue_ext(const char *format, ...)
	{
		va_list args;
		va_start(args, format);
		vsnprintf(_buffer, sizeof(_buffer), format, args);
		va_end(args);
		// skip recording on purpose
		std::lock_guard<std::mutex> lock(_mutex);
		_queue.emplace(std::forward<std::string>(std::string(_buffer)));
	}

private:
	LogPub()
		: _stop(false), _thread(&LogPub::_process, this), _recording_buffer(64), _recording(false)
	{
	}
	void _add(std::string &&message)
	{
		if (_recording)
		{
			_recording_buffer.push_back(message);
		}
		std::lock_guard<std::mutex> lock(_mutex);
		_queue.emplace(std::forward<std::string>(message));
	}

	void _process()
	{
		while (!_stop)
		{
			_flush_queue();
			std::this_thread::yield();
		}
	}
	void _flush_queue()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		while (!_queue.empty())
		{
			std::string &msg = _queue.front();
			std::cout << msg << std::endl;
			_queue.pop();
		}
	}
	char _buffer[_buffer_size] = {0};
	std::atomic<bool> _stop;
	std::queue<std::string> _queue;
	std::mutex _mutex;
	std::thread _thread;
	std::vector<std::string> _recording_buffer;
	bool _recording;
};

#endif
