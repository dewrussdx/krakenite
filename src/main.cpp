#include <iomanip>
#include <iostream>
#include <cstring>
#include "test.h"
#include "server.h"
#include "client.h"

// Entry point
int main(int argc, const char **argv)
{
	// set the default precision to two decimal points
	std::cout << std::fixed << std::setprecision(2) << std::setfill(' ');

	const char *mode = (argc == 2) ? argv[1] : "--test";
	if (!strcmp(mode, "--test"))
	{
		UnitTests tests;
		auto result = tests.run();
		assert(result);
	}
	else if (!strcmp(mode, "--server"))
	{
		Server server;
		server.init();
		server.run();
	}
	else if (!strcmp(mode, "--client"))
	{
		Client client;
		client.init();
		client.run();
	}
}