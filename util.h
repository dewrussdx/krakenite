#ifndef _UTIL_H_
#define _UTIL_H_

#include <chrono>

// Return the elapsed time in specified resolution
template <
	class result_t = std::chrono::milliseconds,
	class clock_t = std::chrono::steady_clock,
	class duration_t = std::chrono::milliseconds>
auto since(const std::chrono::time_point<clock_t, duration_t>& start)
{
	return std::chrono::duration_cast<result_t>(clock_t::now() - start);
}

#endif