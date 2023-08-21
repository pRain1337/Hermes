#include "Physical.h"
#include "Virtual.h"
#include "Windows.h"
#include "Hermes.h"
#include "Memory.h"
#include "Prints.h"

#include <iostream>
#include <map>
#include <string>

using hermes_func = void(*)(uint64_t);

void close(uint64_t dummy)
{
	exit(0);
	return;
}

std::map<std::string, hermes_func> hermes_funcs =
{
  { "gd", getDirbase },
  { "gmd", getModuledata },
  { "vr", readVirtual },
  { "vw", writeVirtual },
  { "pr", readPhysical },
  { "pw", writePhysical },
  { "vto", virtualToPhysical },
  { "dm", dumpModule },
  { "gm", getModules },
  { "help", printCommands },
  { "exit", close }
};

int main()
{
	// Starting
	std::cout << "Initializing Atlas " << std::endl;
	printCommands(0);

	// Initialize communication buffer
	std::cout << "Combuffer at: " << std::hex << (uint64_t)identifier << std::endl;

	uint64_t packetBegin = (uint64_t)identifier + 128;

	// Wait for Input and Parse Commands
	while (true)
	{
		clearPacket(packetBegin);

		std::cout << "# ";

		std::string input;
		std::cin >> input;

		// Check if there is a pending packet
		if (checkPacketStatus(packetBegin) == true)
		{
			std::cout << "Pending packet! Stopping... " << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			continue;
		}

		try
		{
			hermes_funcs.at(input)(packetBegin);
		}
		catch (...)
		{
			// Invalid command passed, write help
			printCommands(0);
		}
		
	}
}