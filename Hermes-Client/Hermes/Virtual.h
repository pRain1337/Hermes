#pragma once

#include "GlobalVars.h"

#include <cstdint>

void getDirbase(uint64_t packetBegin);

void virtualToPhysical(uint64_t packetBegin);

void readVirtual(uint64_t packetBegin);

void writeVirtual(uint64_t packetBegin);