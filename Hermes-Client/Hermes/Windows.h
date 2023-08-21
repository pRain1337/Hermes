#pragma once

#include "GlobalVars.h"

#include <cstdint>

void getModules(uint64_t packetBegin);

void getModuledata(uint64_t packetBegin);

void dumpModule(uint64_t packetBegin);