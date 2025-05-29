#include "Virtual.h"
#include "Hermes.h"
#include "Memory.h"
#include "Prints.h"

#include <iostream>

/*
 *   Function for requesting the dirbase of a given process name.
 *   Asks for the following parameter:
 *       - packetBegin, pointer to the start of the memory mapping buffer
 *
 *   The function itself asks for further information (process name)
 *
*/
void getDirbase(uint64_t packetBegin)
{
    setPacketStarting(packetBegin);

    std::cout << "Processname (MAX 63 CHARS): ";
    std::string prName;
    std::cin >> prName;

	// Fill the rest of the string
	prName.resize(64, '\0');

    const char *cprName = prName.c_str();
	uint8_t nLength = std::strlen(cprName) + 1;

    setPacketCommand(packetBegin, HERMES_CMD_GET_DIRBASE);

    void *data = (void *)malloc(sizeof(uint8_t) + nLength);
    memcpy(data, &nLength, sizeof(uint8_t));
    memcpy((void *)((uint64_t)data + 1), cprName, nLength);

    setPacketDataPtr(packetBegin, (uint64_t)data);

    if (debug)
        std::cout << "Data Ptr: " << (uint64_t)data << std::endl;

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

    if (result <= 0x9000)
    {
        // Print error explanation
        printError(result);
    }
    else
    {
        // Result has changed
        std::cout << "Result: " << std::hex << result << std::endl;
    }

    clearPacket(packetBegin);

    free(data);
}


/*
 *   Function for requesting the translation of a virtual to a physical memory address.
 *   Asks for the following parameter:
 *       - packetBegin, pointer to the start of the memory mapping buffer
 *
 *   The function itself asks for further information (virtual address and dirbase)
 *
*/
void virtualToPhysical(uint64_t packetBegin)
{
    setPacketStarting(packetBegin);

    std::cout << "Virtual: ";
    uint64_t	address;
    std::cin >> std::hex >> address;

    std::cout << "Dirbase: ";
    uint64_t	dirbase;
    std::cin >> std::hex >> dirbase;

    setPacketCommand(packetBegin, HERMES_CMD_VIRTUAL_TO_PHYSICAL);

    std::vector<uint64_t> data(2);
    memcpy(&(data[0]), &address, sizeof(uint64_t));
    memcpy(&(data[1]), &dirbase, sizeof(uint64_t));

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
        std::cout << "Converted " << address << " to: " << result << std::endl;
    }

    clearPacket(packetBegin);
}

/*
 *   Function for requesting the reading of a given virtual address.
 *   Asks for the following parameter:
 *       - packetBegin, pointer to the start of the memory mapping buffer
 *
 *   The function itself asks for further information (virtual address, dirbase and size to read)
 *
*/
void readVirtual(uint64_t packetBegin)
{
    setPacketStarting(packetBegin);

    std::cout << "Address: ";
    uint64_t	address;
    std::cin >> std::hex >> address;

    std::cout << "Dirbase: ";
    uint64_t	dirbase;
    std::cin >> std::hex >> dirbase;

    std::cout << "Size: ";
    uint64_t	size;
    std::cin >> std::dec >> size;

    if (debug)
        std::cout << "Size: " << size << "Dirbase: " << dirbase << "Adrress " << address << std::endl;

    setPacketCommand(packetBegin, HERMES_CMD_READ_VIRTUAL);

    std::vector<uint64_t> data(3);
    memcpy(&(data[0]), &address, sizeof(uint64_t));
    memcpy(&(data[1]), &dirbase, sizeof(uint64_t));
    memcpy(&(data[2]), &size, sizeof(uint64_t));

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
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
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

    if (*resultToCheck != HERMES_STATUS_OK)
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
 *   Function for requesting the writing to a given virtual address.
 *   Asks for the following parameter:
 *       - packetBegin, pointer to the start of the memory mapping buffer
 *
 *   The function itself asks for further information (virtual address, dirbase, size to write and value to write)
 *
*/
void writeVirtual(uint64_t packetBegin)
{
    setPacketStarting(packetBegin);

    std::cout << "Destination: ";
    uint64_t	address;
    std::cin >> std::hex >> address;

    std::cout << "Dirbase: ";
    uint64_t	dirbase;
    std::cin >> std::hex >> dirbase;

    std::cout << "Size: ";
    uint64_t	size;
    std::cin >> std::dec >> size;

    std::cout << "Value: (MAX 8BYTES)";
    uint64_t	value;
    std::cin >> std::hex >> value;

    setPacketCommand(packetBegin, HERMES_CMD_WRITE_VIRTUAL);

    std::vector<uint64_t> data(4);
    memcpy(&(data[0]), &address, sizeof(uint64_t));
    memcpy(&(data[1]), &dirbase, sizeof(uint64_t));
    memcpy(&(data[2]), &size, sizeof(uint64_t));
    memcpy(&(data[3]), &value, sizeof(uint64_t));

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