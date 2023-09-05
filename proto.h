#ifndef _PROTO_H_
#define _PROTO_H_

#include "orderbook.h"
#include "netio.h"

class ProtocolParser
{
public:
	static constexpr size_t _buffer_size = 64;

	struct Callback
	{
		virtual void new_order(const NetIO::NewOrder &data) = 0;
		virtual void cancel_order(const NetIO::CancelOrder &data) = 0;
		virtual void flush_book(const NetIO::FlushBook &data) = 0;
		virtual void protocol_error() = 0;
	};

	explicit ProtocolParser(const char delim = ',')
		: _delim(delim)
	{
	}

	void parse(const char *str, size_t size, Callback &_callback)
	{
		size_t pos = 0;
		switch (str[pos++])
		{

		case 'F':
		{
			pos++;
			NetIO::FlushBook data;
			data.id = 'F';
			assert(pos >= size);
			_callback.flush_book(std::forward<NetIO::FlushBook>(data));
		}
		break;

		case 'N':
		{
			pos++;
			NetIO::NewOrder data;
			data.id = 'N';
			data.user_id = static_cast<Uid>(std::stoi(_tokenize(str, pos, size)));
			// strncpy_s(& data.symbol[0], sizeof(data.symbol), _tokenize(str, pos, size), sizeof(data.symbol));
			strcpy(&data.symbol[0], _tokenize(str, pos, size));
			data.price = static_cast<Price>(std::stoi(_tokenize(str, pos, size)));
			data.qty = static_cast<Qty>(std::stoi(_tokenize(str, pos, size)));
			data.side = _tokenize(str, pos, size)[0];
			data.user_order_id = static_cast<Uid>(std::stoi(_tokenize(str, pos, size)));
			assert(pos >= size);
			_callback.new_order(std::forward<NetIO::NewOrder>(data));
		}
		break;

		case 'C':
		{
			pos++;
			NetIO::CancelOrder data;
			data.id = 'C';
			data.user_id = static_cast<Uid>(std::stoi(_tokenize(str, pos, size)));
			data.user_order_id = static_cast<Uid>(std::stoi(_tokenize(str, pos, size)));
			assert(pos >= size);
			_callback.cancel_order(std::forward<NetIO::CancelOrder>(data));
		}
		break;

		default:
		{
			_callback.protocol_error();
		}
		break;
		}
	}

private:
	const char *_tokenize(const char *str, size_t &pos, size_t size)
	{
		size_t cnt = 0;
		while (pos < size)
		{
			auto c = str[pos++];
			if (c != _delim)
			{
				_token[cnt++] = c;
			}
			else
			{
				_token[cnt] = '\0';
				break;
			}
		}
		return cnt > 0 ? _token : nullptr;
	}
	char _token[_buffer_size] = {0};
	char _delim;
};

#endif