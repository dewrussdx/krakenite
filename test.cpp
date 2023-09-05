#include "test.h"

struct Callback : public ProtocolParser::Callback
{
	Callback(OrderBookManager& manager)
		: _manager(manager)
		, _active_book(nullptr)
	{

	}
	virtual void new_order(const NetIO::NewOrder& data)
	{
		if (!_active_book)
		{
			_active_book = _manager(data.symbol);
		}
		if (!_active_book)
		{
			protocol_error();
		}
		else
		{
			_active_book->add_order(data.side == 'B' ? BUY : SELL, std::forward<Order>(
				Order{ data.user_id, data.user_order_id, data.price, 0, data.qty }));
		}
	}
	virtual void cancel_order(const NetIO::CancelOrder& data)
	{
		if (!_active_book)
		{
			protocol_error();
		}
		else
		{
			_active_book->cancel_order(std::forward<UserOrder>(
				UserOrder{ data.user_id, data.user_order_id }));
		}
	}
	virtual void flush_book(const NetIO::FlushBook&)
	{
		if (!_active_book)
		{
			protocol_error();
		}
		else
		{
			_active_book->flush();
		}
	}
	virtual void protocol_error()
	{
		std::cout << "ERROR: Protocol error" << std::endl;
	}
private:
	OrderBookManager& _manager;
	OrderBook* _active_book;
};

bool UnitTests::run()
{
	Callback callback(manager);
	scenario_count = 1;
	scenario_pick = 0;

	auto result = file_streamer.process([this, &callback](std::string&& data) -> bool {
		if (!scenario_pick || scenario_count == scenario_pick)
		{
			parser.parse(data.c_str(), data.size(), callback);
		}
		if (data[0] == 'F')
		{
			scenario_count++;
		}
		return true;
		});

	return result;
}