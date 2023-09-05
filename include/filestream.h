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
	FileStreamer(const std::string &path)
		: _handle(path.c_str())
	{
	}
	~FileStreamer()
	{
		close();
	}
	bool file_exists() const
	{
		return _handle.good();
	}
	bool process(std::function<bool(std::string &&data)> handler)
	{
		if (!_handle.is_open())
		{
			std::cout << "ERROR: File not open.";
			return false;
		}
		std::string line;
		while (std::getline(_handle, line))
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
		_handle.clear();
		_handle.seekg(0);
	}
	void close()
	{
		_handle.close();
	}

private:
	bool _error(const char *msg)
	{
		std::cout << msg << std::endl;
		_handle.close();
		return false;
	}
	std::ifstream _handle;
};

#endif