#include "test.h"
#include "orderbook.h"
#include "proto.h"
#include <chrono>
#include <vector>
#include <string>

struct StreamHandler : public ProtocolParser::Callback
{
	StreamHandler(OrderBookManager& manager)
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
			_active_book->add_order(data.side == 'B' ? BUY : SELL,std::forward<Order>(
				Order{ data.user_id,data.user_order_id,data.price,0,data.qty }));
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
				UserOrder{ data.user_id,data.user_order_id }));
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
	ProtocolParser parser;
	OrderBookManager manager;
	StreamHandler callback(manager);

	const char* inputs[] = {
		// 1
		"N,1,IBM,10,100,B,1",
		"N,1,IBM,12,100,S,2",
		"N,2,IBM,9,100,B,101",
		"N,2,IBM,11,100,S,102",
		"N,1,IBM,11,100,B,3",
		"N,2,IBM,10,100,S,103",
		"N,1,IBM,10,100,B,4",
		"N,2,IBM,11,100,S,104",
		"F",
		// 2
		"N,1,AAPL,10,100,B,1",
		"N,1,AAPL,12,100,S,2",
		"N,2,AAPL,11,100,S,102",
		"N,2,AAPL,10,100,S,103",
		"N,1,AAPL,10,100,B,3",
		"F",
		// 3
		"N,1,VAL,10,100,B,1",
		"N,2,VAL,9,100,B,101",
		"N,2,VAL,11,100,S,102",
		"N,1,VAL,11,100,B,2",
		"N,2,VAL,11,100,S,103",
		"F",
		// 4
		"N,1,IBM,10,100,B,1",
		"N,1,IBM,12,100,S,2",
		"N,2,IBM,9,100,B,101",
		"N,2,IBM,11,100,S,102",
		"N,2,IBM,9,100,S,103",
		"F",
		// 5
		"N,1,IBM,10,100,B,1",
		"N,1,IBM,12,100,S,2",
		"N,2,IBM,9,100,B,101",
		"N,2,IBM,11,100,S,102",
		"N,1,IBM,12,100,B,103",
		"F",
	
		// 6
		"N,1,IBM,10,100,B,1",
		"N,1,IBM,12,100,S,2",
		"N,2,IBM,9,100,B,101",
		"N,2,IBM,11,100,S,102",
		"N,2,IBM,0,100,S,103",
		"F",
		
		// 7
		"N,1,IBM,10,100,B,1",
		"N,1,IBM,12,100,S,2",
		"N,2,IBM,9,100,B,101",
		"N,2,IBM,11,100,S,102",
		"N,1,IBM,0,100,B,3",
		"F",
	
		// 8
		"N,1,IBM,10,100,B,1",
		"N,1,IBM,16,100,S,2",
		"N,2,IBM,9,100,B,101",
		"N,2,IBM,15,100,S,102",
		"N,2,IBM,11,100,B,103",
		"N,1,IBM,14,100,S,3",
		"F",

		// 9
		"N,1,IBM,10,100,B,1",
		"N,1,IBM,12,100,S,2",
		"N,2,IBM,9,100,B,101",
		"N,2,IBM,11,100,S,102",
		"N,2,IBM,0,20,S,103",
		"F",

		// 10
		"N,1,IBM,10,100,B,1",
		"N,1,IBM,12,100,S,2",
		"N,2,IBM,9,100,B,101",
		"N,2,IBM,11,100,S,102",
		"N,1,IBM,0,20,B,3",
		"F",

		// 11
		"N,1,IBM,10,100,B,1",
		"N,1,IBM,12,100,S,2",
		"N,2,IBM,9,100,B,101",
		"N,2,IBM,11,100,S,102",
		"N,2,IBM,10,20,S,103",
		"F",

		// 12
		"N,1,IBM,10,100,B,1",
		"N,1,IBM,12,100,S,2",
		"N,2,IBM,9,100,B,101",
		"N,2,IBM,11,100,S,102",
		"N,1,IBM,11,20,B,3",
		"F",

		// 13
		"N,1,IBM,10,100,B,1",
		"N,1,IBM,12,100,S,2",
		"N,2,IBM,9,100,B,101",
		"N,2,IBM,11,100,S,102",
		"N,2,IBM,10,50,B,103",
		"N,1,IBM,11,50,S,3",
		"N,1,IBM,11,100,B,4",
		"N,2,IBM,10,100,S,104",
		"F",

		// 14
		"N,1,IBM,10,100,B,1",
		"N,1,IBM,12,100,S,2",
		"N,2,IBM,9,100,B,101",
		"N,2,IBM,11,100,S,102",
		"C,1,1",
		"C,2,102",
		"F",

		// 15
		"N,1,IBM,10,100,B,1",
		"N,1,IBM,12,100,S,2",
		"N,2,IBM,9,100,B,101",
		"N,2,IBM,11,100,S,102",
		"C,1,2",
		"C,2,101",
		"F",

		// 16
		"N,1,IBM,10,100,B,1",
		"N,1,IBM,12,100,S,2",
		"N,2,IBM,9,100,B,101",
		"N,2,IBM,11,100,S,102",
		"C,1,1",
		"C,2,101",
		"F",
	};
	const char* outputs[] = {
		// Scenario 1
		"A, 1, 1",
		"B, B, 10, 100",
		"A, 1, 2",
		"B, S, 12, 100",
		"A, 2, 101",
		"A, 2, 102",
		"B, S, 11, 100",
		"A, 1, 3",
		"T, 1, 3, 2, 102, 11, 100",
		"B, S, 12, 100",
		"A, 2, 103",
		"T, 1, 1, 2, 103, 10, 100",
		"B, B, 9, 100",
		"A, 1, 4",
		"B, B, 10, 100",
		"A, 2, 104",
		"B, S, 11, 100",

		// Scenario 2
		"A, 1, 1",
		"B, B, 10, 100",
		"A, 1, 2",
		"B, S, 12, 100",
		"A, 2, 102",
		"B, S, 11, 100",
		"A, 2, 103",
		"T, 1, 1, 2, 103, 10, 100",
		"B, B, -, -",
		"A, 1, 3",
		"B, B, 10, 100",

		// Scenario 3
		"A, 1, 1",
		"B, B, 10, 100",
		"A, 2, 101",
		"A, 2, 102",
		"B, S, 11, 100",
		"A, 1, 2",
		"T, 1, 2, 2, 102, 11, 100",
		"B, S, -, -",
		"A, 2, 103",
		"B, S, 11, 100",

		// Scenario 4
		"A, 1, 1",
		"B, B, 10, 100",
		"A, 1, 2",
		"B, S, 12, 100",
		"A, 2, 101",
		"A, 2, 102",
		"B, S, 11, 100",
		"A, 2, 103",
		"T, 1, 1, 2, 103, 9, 100",
		"B, B, 9, 100",
		
		// Scenario 5		
		"A, 1, 1",
		"B, B, 10, 100",
		"A, 1, 2",
		"B, S, 12, 100",
		"A, 2, 101",
		"A, 2, 102",
		"B, S, 11, 100",
		"A, 1, 103",
		"T, 1, 103, 2, 102, 11, 100",
		"B, S, 12, 100",

		// Scenario 6 (*)
		"A, 1, 1",
		"B, B, 10, 100",
		"A, 1, 2",
		"B, S, 12, 100",
		"A, 2, 101",
		"A, 2, 102",
		"B, S, 11, 100",
		"A, 2, 103",
		"T, 1, 1, 2, 103, 10, 100",
		"B, B, 9, 100",

		// Scenario 7
		"A, 1, 1",
		"B, B, 10, 100",
		"A, 1, 2",
		"B, S, 12, 100",
		"A, 2, 101",
		"A, 2, 102",
		"B, S, 11, 100",
		"A, 1, 3",
		"T, 1, 3, 2, 102, 11, 100",
		"B, S, 12, 100",

		// Scenario 8 (*)
		"A, 1, 1",
		"B, B, 10, 100",
		"A, 1, 2",
		"B, S, 16, 100",
		"A, 2, 101",
		"A, 2, 102",
		"B, S, 15, 100",
		"A, 2, 103",
		"B, B, 11, 100",
		"A, 1, 3",
		"B, S, 14, 100",

		// Scenario 9
		"A, 1, 1",
		"B, B, 10, 100",
		"A, 1, 2",
		"B, S, 12, 100",
		"A, 2, 101",
		"A, 2, 102",
		"B, S, 11, 100",
		"A, 2, 103",
		"T, 1, 1, 2, 103, 10, 20",
		"B, B, 10, 80",

		// Scenario 10 (*)
		"A, 1, 1",
		"B, B, 10, 100",
		"A, 1, 2",
		"B, S, 12, 100",
		"A, 2, 101",
		"A, 2, 102",
		"B, S, 11, 100",
		"A, 1, 3",
		"T, 1, 3, 2, 102, 11, 20",
		"B, S, 11, 80",
		
		// Scenario 11
		"A, 1, 1",
		"B, B, 10, 100",
		"A, 1, 2",
		"B, S, 12, 100",
		"A, 2, 101",
		"A, 2, 102",
		"B, S, 11, 100",
		"A, 2, 103",
		"T, 1, 1, 2, 103, 10, 20",
		"B, B, 10, 80",

		// Scenario 12 (*)
		"A, 1, 1",
		"B, B, 10, 100",
		"A, 1, 2",
		"B, S, 12, 100",
		"A, 2, 101",
		"A, 2, 102",
		"B, S, 11, 100",	
		"A, 1, 3",
		"T, 1, 3, 2, 102, 11, 20",
		"B, S, 11, 80",
		
		// Scenario 13
		"A, 1, 1",
		"B, B, 10, 100",
		"A, 1, 2",
		"B, S, 12, 100",
		"A, 2, 101",
		"A, 2, 102",
		"B, S, 11, 100",
		"A, 2, 103",
		"B, B, 10, 150",
		"A, 1, 3",
		"B, S, 11, 150",
		"A, 1, 4",
		"T, 1, 4, 2, 102, 11, 100",
		"B, S, 11, 50",
		"A, 2, 104",
		"T, 1, 1, 2, 104, 10, 100",
		"B, B, 10, 50",

		// Scenario 14 (*)
		"A, 1, 1",
		"B, B, 10, 100",
		"A, 1, 2",
		"B, S, 12, 100",
		"A, 2, 101",
		"A, 2, 102",
		"B, S, 11, 100",
		"C, 1, 1",
		"B, B, 9, 100",
		"C, 2, 102",
		"B, S, 12, 100",
	
		// Scenario 15
		"A, 1, 1",
		"B, B, 10, 100",
		"A, 1, 2",
		"B, S, 12, 100",
		"A, 2, 101",
		"A, 2, 102",
		"B, S, 11, 100",
		"C, 1, 2",
		"C, 2, 101",

		// 16 (*)
		"A, 1, 1",
		"B, B, 10, 100",
		"A, 1, 2",
		"B, S, 12, 100",
		"A, 2, 101",
		"A, 2, 102",
		"B, S, 11, 100",
		"C, 1, 1",
		"B, B, 9, 100",
		"C, 2, 101",
		"B, B, -, -",
	};
	LogPub::instance().start_recording();
	auto scenario = 1;
	LogPub::instance().queue_ext("Scenario %d:", scenario);
	for (size_t i = 0,max = sizeof(inputs) / sizeof(char*); i < max; i++)
	{
		parser.parse(inputs[i],strlen(inputs[i]),callback);
		if (inputs[i][0] == 'F')
		{	
			scenario++;
			LogPub::instance().queue_ext("Scenario %d:", scenario);
		}
	}
	const std::vector<std::string>& out = LogPub::instance().stop_recording();
	
	size_t should_have = sizeof(outputs) / sizeof(char*);
	size_t actually_have = out.size();
	std::cout << "Should have: " << should_have << "; Actually have: " << actually_have << std::endl;

	assert(should_have == actually_have);;

	// validate
	auto success = 0;
	auto failure = 0;
	auto total = 0;
	for (size_t i = 0, max = sizeof(outputs) / sizeof(char*); i < max && i < out.size(); i++)
	{
		auto result = out[i].compare(outputs[i]);
		std::cout << "" << outputs[i] << " : " << out[i] << " -> " <<
			(result == 0 ? "PASS" : "FAILURE") << std::endl;
		if (result == 0)
		{
			success++;
		}
		else
		{
			failure++;
		}
		total++;
	}
	std::cout << "Success: " << success << ", Failure: " << failure << ", Total: " << total << std::endl;
	return true;
}