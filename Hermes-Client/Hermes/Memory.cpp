#include "Memory.h"

/*
 *   Function for clearing the current data in the memory communication buffer.
 *   Asks for the following parameter:
 *       - packetPtr, pointer to the memory communication buffer
 *
 *   It will clear the buffer regardless of data inside.
 *
*/
void clearPacket(uint64_t packetPtr)
{
    memset((void *)packetPtr, 0, sizeof(hermes_packet));
}

/*
 *   Function for setting the packet in the memory communication buffer as finished.
 *   Asks for the following parameter:
 *       - packetPtr, pointer to the memory communication buffer
 *
 *   As soon as this identifier is set, smm will recognize it as valid packet and process it.
 *
*/
void setPacketFinished(uint64_t packetPtr)
{
    // Sets the end identifier of the packet,  Handler will instantly process it 
    memcpy((void *)(packetPtr + sizeof(hermes_packet) - sizeof(uint16_t)), &end_identifier, sizeof(uint16_t));
}

/*
 *   Function for setting the packet in the memory communication buffer as starting.
 *   Asks for the following parameter:
 *       - packetPtr, pointer to the memory communication buffer
 *
 *   This identifier acts as mutex, in case the communication would be paralellized using multiple threads.
 *
*/
void setPacketStarting(uint64_t packetPtr)
{
    // Sets the start identifier of the packet
    memcpy((void *)(packetPtr), &start_identifier, sizeof(uint16_t));
}

/*
 *   Function for setting the command in the packet.
 *   Asks for the following two parameters:
 *       - packetPtr, pointer to the memory communication buffer
 *       - command, the command which should be executed by smm
 *
*/
void setPacketCommand(uint64_t packetPtr, hermes_command_t command)
{
    memcpy((void *)(packetPtr + sizeof(uint16_t)), &command, sizeof(hermes_command_t));
}

/*
 *   Function for setting the pointer to the data buffer in the packet.
 *   Asks for the following two parameters:
 *       - packetPtr, pointer to the memory communication buffer
 *       - dataPtr, pointer to the data (contains instructions for the command)
 *
*/
void setPacketDataPtr(uint64_t packetPtr, uint64_t dataPtr)
{
    uint64_t tempPtr = dataPtr;
    memcpy((void *)(packetPtr + sizeof(uint16_t) + sizeof(uint8_t)), &tempPtr, sizeof(uint64_t));

}

/*
 *   Function for setting the pointer to the result buffer in the packet.
 *   Asks for the following two parameters:
 *       - packetPtr, pointer to the memory communication buffer
 *       - resultPtr, pointer to the result (contains the result of the execution)
 *
*/
void setPacketResultPtr(uint64_t packetPtr, uint64_t resultPtr)
{
    uint64_t tempPtr = resultPtr;
    memcpy((void *)(packetPtr + sizeof(uint16_t) + sizeof(uint8_t) + sizeof(uint64_t)), &tempPtr, sizeof(uint64_t));
}

/*
 *   Function for checking if the start identifier is already set.
 *   Asks for the following parameter:
 *       - packetPtr, pointer to the memory communication buffer
 *
 *  Implemented to act as a sort of mutex to prevent double commands.
 *
*/
bool checkPacketStatus(uint64_t packetPtr)
{
    // Checks if the start identifier is already set, use it as a mutex
    uint16_t start = 0;
    memcpy(&start, (void *)packetPtr, sizeof(uint16_t));

    if (start == 0)
    {
        return false;
    }

    return true;
}