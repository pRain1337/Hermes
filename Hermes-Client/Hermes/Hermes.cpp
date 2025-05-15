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

/*
 *   Pseudo function to properly close the hermes process.
 *   Asks for the following parameter:
 *       - dummy, can be left empty. Is just needed because all the other function in the
 *                function map have a parameter.
 *
*/
void close(uint64_t dummy)
{
    exit(0);
    return;
}

/*
 *   Function map of the different hermes commands.
 *
*/
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
  { "st", speedTest },
  { "help", printCommands },
  { "exit", close }
};

/*
 *  Main function of hermes.
 *
 *  Initializes a loop which will continually ask for commands to communicate with the smm.
 *
*/
int main()
{
    // Starting
    std::cout << "Initializing Hermes " << std::endl;
    printCommands(0);

    // Initialize communication buffer
    std::cout << "Combuffer at: " << std::hex << (uint64_t)identifier << std::endl;
    std::cout << "Data begins at: " << std::hex << (uint64_t)identifier + 128 << std::endl;

    uint64_t packetBegin = (uint64_t)identifier + 128;

    // Wait for Input and Parse Commands
    while (true)
    {
        clearPacket(packetBegin);

        std::cout << "# ";

        // Ask for the command
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
            // Try to call the requested function by searching for the name in the map
            hermes_funcs.at(input)(packetBegin);
        }
        catch (...)
        {
            // Invalid command passed, write help
            printCommands(0);
        }

    }
}