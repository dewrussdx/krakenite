#ifndef _FILESTREAM_H_
#define _FILESTREAM_H_

#include <functional>
#include <string>
#include <iostream>
#include <fstream>

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

#endif