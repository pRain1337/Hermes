#include "Physical.h"
#include "Hermes.h"
#include "Memory.h"
#include "Prints.h"

#include <iostream>
#include <vector>

/*
 *   Function for reading arbitrary physical memory using smm.
 *   Asks for the following two parameters:
 *       - Address, from where should be read
 *       - Size, how much memory should be read
 *
 *   Function skips unknown memory which is not in the uefi memory map and deemed unsafe.
 *
*/
void readPhysical(uint64_t packetBegin)
{
	setPacketStarting(packetBegin);
    
    std::cout << "Address: ";
    uint64_t	address;
    std::cin >> std::hex >> address;
    
    std::cout << "Size: ";
    uint64_t	size;
    std::cin >> std::dec >> size;
    
    setPacketCommand(packetBegin, HERMES_CMD_READ_PHYSICAL);
    
    std::vector<uint64_t> data(2);
    memcpy(&(data[0]), &address, sizeof(uint64_t));
    memcpy(&(data[1]), &size, sizeof(uint64_t));
    
    setPacketDataPtr(packetBegin, (uint64_t)&(data[0]));
    
    if (debug)
        std::cout << "Data Ptr: " << (uint64_t)&(data[0]) << std::endl;
    
    char* result = (char *)_aligned_malloc(size + sizeof(uint64_t), 4096);
    uint64_t *resultToCheck = (uint64_t*)((uint64_t)result + (uint64_t)size); // Status will be at the end of the memory buffer
    *resultToCheck = 0;
    
    // Loop ones through the allocated pages to make sure its in the dirbase
    for (int i = 1; i < size + sizeof(uint64_t); i += 0x1000)
    {
        uint8_t *temp = (uint8_t*)((uint64_t)result + (uint64_t)i);
        *temp = 1;
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        *temp = 0;
    }
    
    setPacketResultPtr(packetBegin, (uint64_t)result);
    
    if (debug)
        std::cout << "Result Ptr: " << result << std::endl;
    
    // Data collected, set the end identifier and wait for response
    setPacketFinished(packetBegin);
    
    std::cout << "Waiting for Endpoint... " << std::endl;
    
    while (*resultToCheck == 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    if (*resultToCheck < 0x9000)
    {
        // Print error explanation
        printError(*resultToCheck);
    }
    else
    {
        // Result has changed
        std::cout << "Result: ";

        // Print out the read memory
		printBuffer((unsigned char *)result, size);
    }
    
	clearPacket(packetBegin);
    
    _aligned_free(result);
}

/*
 *   Funtetm.
 *   et:
 *       - etread
 *       - Size, et read
 *
 *   ettet.
 *
*/
void writePhysical(uint64_t packetBegin)
{
    setPacketStarting(packetBegin);
    
    std::cout << "Destination: ";
    uint64_t	address;
    std::cin >> std::hex >> address;
    
    std::cout << "Size: ";
    uint64_t	size;
    std::cin >> std::dec >> size;
    
    std::cout << "Value: (MAX 8BYTES) ";
    uint64_t	value;
    std::cin >> std::hex >> value;
    
    setPacketCommand(packetBegin, HERMES_CMD_WRITE_PHYSICAL);
    
    std::vector<uint64_t> data(3);
    memcpy(&(data[0]), &address, sizeof(uint64_t));
    memcpy(&(data[1]), &size, sizeof(uint64_t));
    memcpy(&(data[2]), &value, sizeof(uint64_t));
    
    setPacketDataPtr(packetBegin, (uint64_t)&(data[0]));
    
    if (debug)
        std::cout << "Data Ptr: " << (uint64_t)&(data[0]) << std::endl;
    
    uint64_t result = 0;
    setPacketResultPtr(packetBegin, (uint64_t)&result);
    
    if (debug)
        std::cout << "Result Ptr: " << &result << std::endl;
    
    // Data collected, set the end identifier and wait for response
    setPacketFinished(packetBegin);
    
    std::cout << "Waiting for Endpoint... " << std::endl;
    
    while (result == 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    if (result < 0x9000)
    {
        // Print error explanation
        printError(result);
    }
    else
    {
        // Result has changed
        std::cout << "Successfully written!" << std::endl;
    }
    
	clearPacket(packetBegin);
}