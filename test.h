#ifndef _TEST_H_
#define _TEST_H_

#include <string>
#include <chrono>
#include <cassert>

#include "filestream.h"

class UnitTests
{
public:
	UnitTests(const std::string& file_path)
		: streamer(file_path.c_str())
	{
		assert(streamer.file_exists());
	}

	bool run();
	
private:
	FileStreamer streamer;
};

#endif