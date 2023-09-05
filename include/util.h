#ifndef _UTIL_H_
#define _UTIL_H_

#include <chrono>
#include <cassert>

struct StopWatch
{
	StopWatch()
	{
		start();
	}

	void start()
	{
		_start = std::chrono::high_resolution_clock::now();
	}

	template <typename _resolution = std::chrono::microseconds>
	auto stop()
	{
		_finish = std::chrono::high_resolution_clock::now();
		assert(_finish > _start);
		return std::chrono::duration_cast<_resolution>(_finish - _start).count();
	}

	// Return elapsed timer, but don't stop (checkpoint)
	template <typename _resolution = std::chrono::nanoseconds>
	auto elapsed()
	{
		auto now = std::chrono::high_resolution_clock::now();
		return std::chrono::duration_cast<_resolution>(now - _start).count();
	}

private:
	// std::cout << << "ns\n";
	std::chrono::time_point<std::chrono::steady_clock> _start;
	std::chrono::time_point<std::chrono::steady_clock> _finish;
};

#endif