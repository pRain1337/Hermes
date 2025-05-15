#pragma once
#include "GlobalVars.h"
#include <cstdint>
#include <windows.h>
#include <string>

void getModules(uint64_t packetBegin);

void getModuledataByName(uint64_t packetBegin, std::string processName, std::string moduleName);

void getModuledata(uint64_t packetBegin);

void dumpModule(uint64_t packetBegin);

void speedTest(uint64_t packetBegin);