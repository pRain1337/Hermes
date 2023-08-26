#pragma once

#include "GlobalVars.h"
#include "Hermes.h"

void clearPacket(uint64_t packetPtr);

void setPacketFinished(uint64_t packetPtr);

void setPacketStarting(uint64_t packetPtr);

void setPacketCommand(uint64_t packetPtr, hermes_command_t command);

void setPacketDataPtr(uint64_t packetPtr, uint64_t dataPtr);

void setPacketResultPtr(uint64_t packetPtr, uint64_t resultPtr);

bool checkPacketStatus(uint64_t packetPtr);