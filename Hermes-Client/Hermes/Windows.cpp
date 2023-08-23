#include "Virtual.h"
#include "Hermes.h"
#include "Memory.h"
#include "Prints.h"

#include <iostream>

/*
 *   Function .
 *   Asks :
 *       - 
 *       - 
 *
 *   Function .
 *
*/
void getModules(uint64_t packetBegin)
{
    setPacketStarting(packetBegin);
    
    std::cout << "Processname (MAX 63 CHARS): ";
    std::string prName;
    std::cin >> prName;
    
    uint8_t nLength = prName.length() + 1;
    
    char *cprName = &prName[0];
    
    setPacketCommand(packetBegin, HERMES_CMD_GET_MODULES);
    
    void *data = (void *)malloc(sizeof(uint8_t) + nLength);
    memcpy(data, &nLength, sizeof(uint8_t));
    memcpy((void *)((uint64_t)data + 1), cprName, nLength);
    
    setPacketDataPtr(packetBegin, (uint64_t)data);
    
    //if (debug)
    std::cout << "Data Ptr: " << (uint64_t)data << std::endl;
    
    void* result = _aligned_malloc(0x1000, 4096);
    uint64_t *resultToCheck = (uint64_t*)((uint64_t)result); // Status will be at the end of the memory buffer
    *resultToCheck = 0;
    
    uint8_t *temp = (uint8_t*)((uint64_t)result);
    *temp = 1;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    *temp = 0;
    
    setPacketResultPtr(packetBegin, (uint64_t)result);
    
    //if (debug)
    std::cout << "Result Ptr: " << (uint64_t)result << std::endl;
    
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
        // Get size
        uint8_t *size = (uint8_t *)(uint64_t)result + 8;
    
        // Result has changed
        std::cout << "Printing Module " << std::dec << (int)*size << " Names: " << std::endl;
    
        for (int i = 0; i < *size; i++)
        {
            char buffer[32];
            memcpy(buffer, (void *)((uint64_t)result + 8 + 1 + (i * 32)), 32);
    
            std::cout << "Name: " << buffer << std::endl;
        }
    }
    
	clearPacket(packetBegin);
}

/*
 *   Function .
 *   Asks :
 *       - 
 *       - 
 *
 *   Function .
 *
*/
void getModuledata(uint64_t packetBegin)
{
    setPacketStarting(packetBegin);
    
    std::cout << "Processname (MAX 63 CHARS): ";
    std::string prName;
    std::cin >> prName;
    
    uint8_t nLength = prName.length() + 1;
    
    char *cprName = &prName[0];
    
    std::cout << "Modulename (MAX 63 CHARS): ";
    std::string moName;
    std::cin >> moName;
    
    uint8_t mLength = moName.length() + 1;
    
    char *cmoName = &moName[0];
    
    setPacketCommand(packetBegin, HERMES_CMD_GET_MODULEDATA);
    
    void *data = (void *)malloc(sizeof(uint8_t) + nLength + sizeof(uint8_t) + mLength);
    memcpy(data,																	&nLength, sizeof(uint8_t));
    memcpy((void *) ((uint64_t)data + sizeof(uint8_t)),								cprName, nLength);
    memcpy((void*) ((uint64_t)data + sizeof(uint8_t) + nLength),					&mLength, sizeof(uint8_t));
    memcpy((void *) ((uint64_t)data + sizeof(uint8_t) + nLength + sizeof(uint8_t)),	cmoName, mLength);
    setPacketDataPtr(packetBegin, (uint64_t)data);
    
    if (debug)
        std::cout << "Data Ptr: " << (uint64_t)data << std::endl;
    
    moduleData result = { 0, 0 };
    setPacketResultPtr(packetBegin, (uint64_t)&result);
    
    if (debug)
        std::cout << "Result Ptr: " << &result << std::endl;
    
    // Data collected, set the end identifier and wait for response
    setPacketFinished(packetBegin);
    
    std::cout << "Waiting for Endpoint... " << std::endl;
    while (result.moduleBase == 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    if (result.moduleBase <= 0x9000)
    {
        // Print error explanation
        printError(result.moduleBase);
    }
    else
    {
        // Result has changed
        std::cout << "Result: Base " << std::hex << result.moduleBase << " Size " << std::hex << result.moduleSize << std::endl;
    }
    
	clearPacket(packetBegin);
    
    free(data);
}

/*
 *   Function .
 *   Asks :
 *       - 
 *       - 
 *
 *   Function .
 *
*/
void dumpModule(uint64_t packetBegin)
{
    setPacketStarting(packetBegin);
    
    std::cout << "Base: ";
    uint64_t	base;
    std::cin >> std::hex >> base;
    
    std::cout << "Dirbase: ";
    uint64_t	dirbase;
    std::cin >> std::hex >> dirbase;
    
    std::cout << "Size: ";
    uint64_t	size;
    std::cin >> std::hex >> size;
    
    setPacketCommand(packetBegin, HERMES_CMD_DUMP_MODULE);
    
    std::vector<uint64_t> data(3);
    memcpy(&(data[0]), &base, sizeof(uint64_t));
    memcpy(&(data[1]), &dirbase, sizeof(uint64_t));			
    memcpy(&(data[2]), &size, sizeof(uint64_t));
    
    setPacketDataPtr(packetBegin, (uint64_t)&(data[0]));
    
    //if (debug)
        std::cout << "Data Ptr: " << (uint64_t)&(data[0]) << std::endl;
    
    void* result = _aligned_malloc(size + sizeof(uint64_t), 4096);
    uint64_t *resultToCheck = (uint64_t*)((uint64_t)result + (uint64_t)size); // Status will be at the end of the memory buffer
    *resultToCheck = 0;
    
    // Loop ones through the allocated pages to make sure its in the dirbase
    for (int i = 1; i < size + sizeof(uint64_t); i += 0x1000)
    {
        uint8_t *temp = (uint8_t*)((uint64_t)result + (uint64_t)i);
        *temp = 1;
        //std::this_thread::sleep_for(std::chrono::milliseconds(2));
        *temp = 0;
    }
    
    setPacketResultPtr(packetBegin, (uint64_t)result);
    
    //if (debug)
        std::cout << "Result Ptr: " << (uint64_t)result << " " << *resultToCheck << std::endl;
    
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
        std::cout << "Finished dumping... " << *resultToCheck << std::endl;
    }
    
	clearPacket(packetBegin);
    
    // Dump successfully, ask for filename
    std::cout << "Filename: ";
    std::string fileName;
    std::cin >> fileName;
    
    std::ofstream ofs(fileName, std::ios::trunc | std::ios::out | std::ofstream::binary);
    
    ofs.write((const char *)result, size);
    
    ofs.close();
    
    _aligned_free(result);
}