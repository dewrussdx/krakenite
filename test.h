#ifndef _TEST_H_
#define _TEST_H_

#include <string>
#include <chrono>
#include <cassert>

#include "orderbook.h"
#include "filestream.h"
#include "proto.h"

class UnitTests
{
public:
	UnitTests(const std::string& file_path)
		: file_streamer(file_path.c_str())
		, scenario_count(0)
		, parser(book_manager)
	{
		assert(file_streamer.file_exists());
	}
	bool run()
	{
		using namespace std::chrono_literals;
		scenario_count = 1;
#if VERBOSE
		std::cout << "Scenario #1" << std::endl;
#endif
		int scenario_pick = 16;

		auto result = file_streamer.process([this, scenario_pick](std::string&& data) -> bool {
			if (scenario_count == scenario_pick)
			{
				parser.parse(data.c_str(), data.size());
			}
			if (data[0] == 'F')
			{
				scenario_count++;
#if VERBOSE
				std::cout << "\nScenario #" << scenario_count << std::endl;
#endif
			}
			return true;
			});
#if VERBOSE
		std::cout << "Scenarios processed: " << scenario_count << std::endl;
#endif
		return result;
	}
	OrderBookManager book_manager;
	FileStreamer file_streamer;
	int scenario_count;
	ProtocolParser<64> parser;
};

#endif