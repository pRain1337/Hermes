#include "Memory.h"

/*
 *   Funtetm.
 *   et:
 *       - etread
 *       - Size, et read
 *
 *   ettet.
 *
*/
void clearPacket(uint64_t packetPtr)
{
	memset((void *)packetPtr, 0, sizeof(hermes_packet));
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
void setPacketFinished(uint64_t packetPtr)
{
	// Sets the end identifier of the packet,  Handler will instantly process it 
	memcpy((void *)(packetPtr + sizeof(hermes_packet) - sizeof(uint16_t)), &end_identifier, sizeof(uint16_t));

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
void setPacketStarting(uint64_t packetPtr)
{
	// Sets the start identifier of the packet
	memcpy((void *)(packetPtr), &start_identifier, sizeof(uint16_t));
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
void setPacketCommand(uint64_t packetPtr, hermes_command_t command)
{
	memcpy((void *)(packetPtr + sizeof(uint16_t)), &command, sizeof(hermes_command_t));
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
void setPacketDataPtr(uint64_t packetPtr, uint64_t dataPtr)
{
	uint64_t tempPtr = dataPtr;
	memcpy((void *)(packetPtr + sizeof(uint16_t) + sizeof(uint8_t)), &tempPtr, sizeof(uint64_t));

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
void setPacketResultPtr(uint64_t packetPtr, uint64_t resultPtr)
{
	uint64_t tempPtr = resultPtr;
	memcpy((void *)(packetPtr + sizeof(uint16_t) + sizeof(uint8_t) + sizeof(uint64_t)), &tempPtr, sizeof(uint64_t));
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