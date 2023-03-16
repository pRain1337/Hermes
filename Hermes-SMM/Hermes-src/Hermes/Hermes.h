#ifndef __hermes_h__
#define __hermes_h__

#include <ProcessorBind.h>

#include "windows.h"
#include "NTKernelTools.h"
#include "serial.h"

// Command Data structures
typedef struct {
	UINT64 moduleBase;
    UINT64 moduleSize;
} moduleData;

typedef struct {
	UINT64 source;
    UINT64 dirbase;
    UINT64 size;
} readRequest;


// Results
typedef enum {
    HERMES_STATUS_INVALID_COMMAND       = 0x900,  // Invalid command
    HERMES_STATUS_INVALID_DATA_PTR      = 0x901,  // Invalid data pointer
    HERMES_STATUS_INVALID_RES_PTR       = 0x902,  // Invalid result pointer
    HERMES_STATUS_INVALID_IDENTIFIERS   = 0x903,  // Invalid identifiers
    
    HERMES_STATUS_FAIL_FIND_PROC        = 0x1000, // Failed finding the process
    HERMES_STATUS_FAIL_FIND_DIRBASE     = 0x1001, // Failed getting dirbase
    
    HERMES_STATUS_FAIL_FIND_MOD         = 0x1010, // Failed finding the module
    HERMES_STATUS_FAIL_READ_MOD_BASE    = 0x1011, // Failed reading module base
    HERMES_STATUS_FAIL_READ_MOD_SIZE    = 0x1012, // Failed reading module size
    HERMES_STATUS_PROCNAME_TOO_LONG     = 0x1013, // Too big process name size
    HERMES_STATUS_MODNAME_TOO_LONG      = 0x1014, // Too big module name size
    
    HERMES_STATUS_FAIL_VIRT_READ        = 0x1020,  // Failed reading virtual address
    HERMES_STATUS_FAIL_VIRT_WRITE       = 0x1021,  // Failed writing virtual address
    HERMES_STATUS_INVALID_DATA_SOURCE   = 0x1022,  // Invalid data source
    HERMES_STATUS_INVALID_DATA_DIRBASE  = 0x1023,  // Invalid data dirbase
    HERMES_STATUS_INVALID_DATA_SIZE     = 0x1024,  // Invalid data size
    HERMES_STATUS_INVALID_DATA_DEST     = 0x1025,  // Invalid data destination
    HERMES_STATUS_REQ_TOO_LARGE         = 0x1026,  // Requested size is too big
    HERMES_STATUS_FAIL_SBUFF_VIRTW      = 0x1027,  // Failed reading source buffer for vWrite
    HERMES_STATUS_FAIL_ADDR_TRANSLATION = 0x1028,  // Failed translating address
    HERMES_STATUS_FAIL_SBUFF_PHYSW      = 0x1029,  // Failed reading source buffer for pWrite
    
    HERMES_STATUS_INVALID_MOD_BASE      = 0x1040,  // Invalid module base
    HERMES_STATUS_INVALID_MOD_SIZE      = 0x1041,  // Invalid module size
    
    HERMES_STATUS_FAIL_PHYS_READ        = 0x1030,  // Failed reading physical address
    HERMES_STATUS_FAIL_PHYS_WRITE       = 0x1031,  // Failed writing physical address
    HERMES_STATUS_OK                    = 0x9000,  // Everything worked
} hermes_status_t;

// Commands
typedef enum {
    getDirbase          =  1,
    getModuleData       =  2,
	virtualRead         =  3,
	virtualWrite        =  4,
	physicalRead        =  5,
    physicalWrite       =  6,
    virtualToPhysicalA  =  7, 
    dumpModule          =  8,
    getModules          =  9,
    maxInt              =  0xFF
} hermes_command_t;

// Main Packet
typedef struct {
	UINT16          begin;
	UINT8           command;
    UINT64          dataPointer;
	UINT64          resultPointer;
	UINT16 end;
} hermes_packet;


BOOLEAN HermesPollCommands();


#endif