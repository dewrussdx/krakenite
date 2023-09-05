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
		: scenario_count(0)
		, scenario_pick(0)
		, file_streamer(file_path.c_str())
	{
		assert(file_streamer.file_exists());
	}

	bool run();
	
private:
	int scenario_pick;
	int scenario_count;
	FileStreamer file_streamer;
	ProtocolParser parser;
	OrderBookManager manager;
};

#endif