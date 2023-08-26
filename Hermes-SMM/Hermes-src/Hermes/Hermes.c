#include "Hermes.h"

// From NtKernelTools.c
extern WinCtx *winGlobal;

UINT64 found_packet = 0;

const static unsigned char hermes_identifier[128] = {
0x0f, 0x2b, 0x34, 0x7f, 0x58, 0x40, 0x22, 0x61, 0x9c, 0xcc, 0x0f, 0xfa, 0x44, 0x0f, 0x59, 0x61,
0x40, 0x8b, 0x37, 0x6e, 0x8f, 0xe9, 0x1d, 0x2d, 0x81, 0x12, 0x65, 0x48, 0x6b, 0x8b, 0xcb, 0x0c,
0xa8, 0x25, 0x6a, 0xac, 0x9e, 0x66, 0xe3, 0x5d, 0x14, 0x24, 0x19, 0x45, 0x75, 0xab, 0xe8, 0xa1,
0x5a, 0x89, 0x99, 0xbe, 0xf9, 0xcc, 0x71, 0x81, 0x98, 0xe1, 0xce, 0x07, 0x9b, 0xc8, 0x59, 0x49,
0xa7, 0x5b, 0xfe, 0xa4, 0x05, 0x6a, 0xa6, 0x70, 0xd3, 0xe9, 0x0e, 0x85, 0xb3, 0xb1, 0x43, 0xde,
0xdc, 0xef, 0x3b, 0xfe, 0x0d, 0x20, 0xf1, 0xc6, 0x65, 0x92, 0x20, 0x97, 0xad, 0xa7, 0xcc, 0x32,
0x46, 0x05, 0x5c, 0xc9, 0xf2, 0xa1, 0xb8, 0x79, 0x92, 0x34, 0x03, 0xa4, 0x17, 0x20, 0x12, 0x0b,
0x35, 0x85, 0x81, 0x98, 0xe8, 0x76, 0xf5, 0x61, 0x0b, 0x7c, 0xe7, 0xfc, 0xcf, 0x97, 0x88, 0x81 };

const static UINT16 hermes_start_identifier = 0xf345;
const static UINT16 hermes_end_identifier = 0xa13f;

BOOLEAN HermesPollCommands()
{
    // Dump Process List and search for process
    BOOLEAN status = FALSE;
    BOOLEAN verbose = FALSE;

    SerialPrintString("Polling for Hermes commands \r\n");

    WinProc process;

    CHAR8* hermesProcessName = "hermes.exe";

    status = DumpSingleProcess(winGlobal, hermesProcessName, &process, verbose);

    if (status == FALSE)
    {
        SerialPrintString("Failed finding Hermes UM process \r\n");

        return FALSE;
    }
    else
    {
        SerialPrintString("Found Hermes UM Dir 0x");
        SerialPrintNumber(process.dirBase, 16);
        SerialPrintString("\r\n");
    }

    // Get hermes.exe module
    // Prepare Module list
    WinModule hermes;
    hermes.name = hermesProcessName;
    status = DumpSingleModule(winGlobal, &process, &hermes, verbose);

    if (status == FALSE)
    {
        SerialPrintString("Failed finding Hermes module \r\n");

        return FALSE;
    }

    // Find Memory Communication buffer
    if (hermes.baseAddress == 0 || hermes.sizeOfModule == 0)
    {
        SerialPrintString("Invalid Module info \r\n");

        return FALSE;
    }
    else
    {
        SerialPrintString("Hermes Module Base 0x");
        SerialPrintNumber(hermes.baseAddress, 16);
        SerialPrintString("Size 0x");
        SerialPrintNumber(hermes.sizeOfModule, 16);
    }

    for (UINT64 i = hermes.baseAddress; i < (hermes.baseAddress + hermes.sizeOfModule); i += 0x1000)
    {

        UINT64 pSrc = VTOP(i, process.dirBase, FALSE);

        if (pSrc == 0)
            continue;

        for (UINT64 k = 0; k < (0x1000 - 128); k++)
        {
            BOOLEAN bFound = TRUE;

            for (UINT32 j = 0; j < 128; j++)
            {
                if (hermes_identifier[j] != ((unsigned char *)pSrc)[k + j])
                {
                    bFound = FALSE;
                    break;
                }
            }

            if (bFound != FALSE)
            {
                found_packet = (UINT64)pSrc + k + 128;
            }
        }
    }

    SerialPrintString("Found Packet at: ");
    SerialPrintNumber(found_packet, 16);
    SerialPrintString(" sizeof(packet): ");
    SerialPrintNumber(sizeof(hermes_packet), 16);
    SerialPrintString("\r\n");

    if (found_packet != 0)
    {
        hermes_packet* pkt = (hermes_packet*)found_packet;
        
        // Check if there's a packet at the queue
        if (pkt->begin == hermes_start_identifier)
        {
            if (pkt->end == hermes_end_identifier)
            {
                if (pkt->command == HERMES_CMD_GET_DIRBASE)
                {
                    SerialPrintString("HERMES_CMD_GET_DIRBASE request\r\n");

                    // Null the startidentifer to act as mutex
                    pkt->begin = 0;

                    if (pkt->resultPointer == 0)
                    {
                        SerialPrintString("Error: resultPtr not set \r\n");
                        goto clear_packet;
                    }

                    if (pkt->dataPointer == 0)
                    {
                        SerialPrintString("Error: dataPtr not set \r\n");

                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_INVALID_DATA_PTR;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    // Get Size of Process name
                    UINT8 size = 0;
                    v_memRead((UINT64)&size, pkt->dataPointer, sizeof(UINT8), process.dirBase, FALSE);

                    if (size == 0 || size >= 64)
                    {
                        SerialPrintStringDebug("Error: Size is ");
                        SerialPrintNumberDebug(size, 10);
                        SerialPrintStringDebug("\r\n");

                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_PROCNAME_TOO_LONG;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);

                        return FALSE;
                    }

                    // Read Processname
                    CHAR8 bName[64];

                    v_memRead((UINT64)&bName, pkt->dataPointer + sizeof(UINT8), size, process.dirBase, FALSE);

                    SerialPrintStringDebug("Read Name: ");
                    SerialPrintStringDebug(bName);
                    SerialPrintStringDebug(".\r\n");

                    // Get Dirbase
                    UINT64 dirBase = 0;

                    WinProc getDirBaseProcess;
                    UINT64 status = DumpSingleProcess(winGlobal, bName, &getDirBaseProcess, FALSE);

                    if (status == FALSE)
                    {
                        // Set result
                        UINT64 temp = HERMES_STATUS_FAIL_FIND_PROC;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    if (getDirBaseProcess.dirBase != 0)
                    {
                        dirBase = getDirBaseProcess.dirBase;
                    }
                    else
                    {
                        dirBase = HERMES_STATUS_FAIL_FIND_DIRBASE;
                    }

                    // Set Result         
                    v_memWrite(pkt->resultPointer, (UINT64)&dirBase, sizeof(UINT64), process.dirBase, FALSE);

                    return FALSE;
                }
                else if (pkt->command == HERMES_CMD_GET_MODULEDATA)
                {
                    SerialPrintString("HERMES_CMD_GET_MODULEDATA request\r\n");

                    // Null the startidentifer to act as mutex
                    pkt->begin = 0;

                    if (pkt->resultPointer == 0)
                    {
                        SerialPrintString("Error: resultPtr not set \r\n");
                        goto clear_packet;
                    }

                    if (pkt->dataPointer == 0)
                    {
                        SerialPrintString("Error: dataPtr not set \r\n");

                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_INVALID_DATA_PTR;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    // Get Size of Process name
                    UINT8 bProcessSize = 0;
                    v_memRead((UINT64)&bProcessSize, pkt->dataPointer, sizeof(UINT8), process.dirBase, FALSE);

                    if (bProcessSize == 0 || bProcessSize >= 64)
                    {
                        SerialPrintString("Error: Size is ");
                        SerialPrintNumber(bProcessSize, 10);
                        SerialPrintString("\r\n");

                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_PROCNAME_TOO_LONG;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);

                        return FALSE;
                    }

                    // Read Processname
                    CHAR8 bProcessName[64];

                    v_memRead((UINT64)&bProcessName, pkt->dataPointer + sizeof(UINT8), bProcessSize, process.dirBase, FALSE);

                    SerialPrintStringDebug("Processname: ");
                    SerialPrintStringDebug(bProcessName);
                    SerialPrintStringDebug(".\r\n");

                    // Get Size of Module name
                    UINT8 pModuleSize = 0;
                    v_memRead((UINT64)&pModuleSize, pkt->dataPointer + sizeof(UINT8) + bProcessSize, sizeof(UINT8), process.dirBase, FALSE);

                    if (pModuleSize == 0 || pModuleSize >= 64)
                    {
                        SerialPrintString("Error: Size is ");
                        SerialPrintNumber(pModuleSize, 10);
                        SerialPrintString("\r\n");

                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_MODNAME_TOO_LONG;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);

                        return FALSE;
                    }

                    // Read Modulename
                    CHAR8 bModuleName[64];

                    v_memRead((UINT64)&bModuleName, pkt->dataPointer + sizeof(UINT8) + bProcessSize + sizeof(UINT8), pModuleSize, process.dirBase, FALSE);

                    SerialPrintStringDebug("Modulename: ");
                    SerialPrintStringDebug(bModuleName);
                    SerialPrintStringDebug(".\r\n");

                    // Initialize process struct 
                    WinProc getModuleDataProcess;
                    UINT64 status = DumpSingleProcess(winGlobal, bProcessName, &getModuleDataProcess, FALSE);

                    // Check status
                    if (status == FALSE)
                    {
                        // Set result
                        UINT64 temp = HERMES_STATUS_FAIL_FIND_PROC;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    // Process is ok, search module
                    WinModule getModuleDataModule;
                    getModuleDataModule.name = bModuleName;
                    status = DumpSingleModule(winGlobal, &getModuleDataProcess, &getModuleDataModule, FALSE);

                    // Check status
                    if (status == FALSE)
                    {
                        // Set result
                        UINT64 temp = HERMES_STATUS_FAIL_FIND_MOD;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    if (getModuleDataModule.baseAddress == 0)
                    {
                        // Set result
                        UINT64 temp = HERMES_STATUS_INVALID_MOD_BASE;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    if (getModuleDataModule.sizeOfModule == 0)
                    {
                        // Set result
                        UINT64 temp = HERMES_STATUS_INVALID_MOD_SIZE;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    // Module is ok, prepare module dataPointer
                    moduleData modData;
                    modData.moduleBase = getModuleDataModule.baseAddress;
                    modData.moduleSize = getModuleDataModule.sizeOfModule;

                    // Set Result         
                    v_memWrite(pkt->resultPointer, (UINT64)&modData, sizeof(moduleData), process.dirBase, FALSE);

                    return FALSE;
                }
                else if (pkt->command == HERMES_CMD_READ_VIRTUAL)
                {
                    SerialPrintString("HERMES_CMD_READ_VIRTUAL request\r\n");

                    // Null the startidentifer to act as mutex
                    pkt->begin = 0;

                    if (pkt->resultPointer == 0)
                    {
                        SerialPrintString("Error: resultPtr not set \r\n");
                        goto clear_packet;
                    }

                    if (pkt->dataPointer == 0)
                    {
                        SerialPrintString("Error: dataPtr not set \r\n");

                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_INVALID_DATA_PTR;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    UINT64 source = 0;
                    v_memRead((UINT64)&source, pkt->dataPointer, sizeof(UINT64), process.dirBase, FALSE);

                    if (source == 0)
                    {
                        SerialPrintString("Error: Source 0 \r\n");
                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_INVALID_DATA_SOURCE;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    UINT64 dirbase = 0;
                    v_memRead((UINT64)&dirbase, pkt->dataPointer + sizeof(UINT64), sizeof(UINT64), process.dirBase, FALSE);

                    if (dirbase == 0)
                    {
                        SerialPrintString("Error: Dirbase 0 \r\n");
                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_INVALID_DATA_DIRBASE;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    UINT64 size = 0;
                    v_memRead((UINT64)&size, pkt->dataPointer + sizeof(UINT64) + sizeof(UINT64), sizeof(UINT64), process.dirBase, FALSE);

                    if (size == 0)
                    {
                        SerialPrintString("Error: Size 0 \r\n");
                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_INVALID_DATA_SIZE;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    if (size > 0x1000)
                    {
                        SerialPrintString("Error: Size >0x1000 \r\n");
                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_REQ_TOO_LARGE;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    SerialPrintString("Addr: ");
                    SerialPrintNumber(source, 16);
                    SerialPrintString(" dr: ");
                    SerialPrintNumber(dirbase, 16);
                    SerialPrintString(" si: ");
                    SerialPrintNumber(size, 16);
                    SerialPrintString("\r\n");

                    // Try to read it
                    unsigned char readBuffer[0x1000];

                    UINT64 status = v_memRead((UINT64)readBuffer, source, size, dirbase, FALSE);

                    if (status == FALSE)
                    {
                        SerialPrintString("Error: vMem Fail \r\n");
                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_FAIL_VIRT_READ;
                        v_memWrite(pkt->resultPointer + size, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    // Set Result       
                    UINT16 temp = HERMES_STATUS_OK;
                    v_memWrite(pkt->resultPointer + size, (UINT64)&temp, sizeof(UINT16), process.dirBase, FALSE);
                    v_memWrite(pkt->resultPointer, (UINT64)readBuffer, size, process.dirBase, FALSE);

                    return FALSE;
                }
                else if (pkt->command == HERMES_CMD_WRITE_VIRTUAL)
                {
                    SerialPrintString("HERMES_CMD_WRITE_VIRTUAL request\r\n");

                    // Null the startidentifer to act as mutex
                    pkt->begin = 0;

                    if (pkt->resultPointer == 0)
                    {
                        SerialPrintString("Error: resultPtr not set \r\n");
                        goto clear_packet;
                    }

                    if (pkt->dataPointer == 0)
                    {
                        SerialPrintString("Error: dataPtr not set \r\n");

                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_INVALID_DATA_PTR;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    UINT64 destination = 0;
                    v_memRead((UINT64)&destination, pkt->dataPointer, sizeof(UINT64), process.dirBase, FALSE);

                    if (destination == 0)
                    {
                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_INVALID_DATA_DEST;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    UINT64 dirbase = 0;
                    v_memRead((UINT64)&dirbase, pkt->dataPointer + sizeof(UINT64), sizeof(UINT64), process.dirBase, FALSE);

                    if (dirbase == 0)
                    {
                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_INVALID_DATA_DIRBASE;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    UINT64 size = 0;
                    v_memRead((UINT64)&size, pkt->dataPointer + sizeof(UINT64) + sizeof(UINT64), sizeof(UINT64), process.dirBase, FALSE);

                    if (size == 0)
                    {
                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_INVALID_DATA_SIZE;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    if (size > 0x1000)
                    {
                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_REQ_TOO_LARGE;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    // Try to read the buffer to write
                    unsigned char writeBuffer[0x1000];

                    UINT64 status = v_memRead((UINT64)writeBuffer, (UINT64)pkt->dataPointer + sizeof(UINT64) + sizeof(UINT64) + sizeof(UINT64), size, process.dirBase, FALSE);

                    if (status == FALSE)
                    {
                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_FAIL_VIRT_WRITE;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    // Try to write the buffer to destination
                    status = v_memWrite(destination, (UINT64)writeBuffer, size, process.dirBase, FALSE);

                    if (status == FALSE)
                    {
                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_FAIL_SBUFF_VIRTW;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    // Set Result  
                    UINT16 temp = HERMES_STATUS_OK;
                    v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT16), process.dirBase, FALSE);

                    return FALSE;
                }
                else if (pkt->command == HERMES_CMD_READ_PHYSICAL)
                {
                    SerialPrintString("HERMES_CMD_READ_PHYSICAL request\r\n");

                    // Null the startidentifer to act as mutex
                    pkt->begin = 0;

                    if (pkt->resultPointer == 0)
                    {
                        SerialPrintString("Error: resultPtr not set \r\n");
                        goto clear_packet;
                    }

                    if (pkt->dataPointer == 0)
                    {
                        SerialPrintString("Error: dataPtr not set \r\n");

                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_INVALID_DATA_PTR;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    UINT64 source = 0;
                    v_memRead((UINT64)&source, pkt->dataPointer, sizeof(UINT64), process.dirBase, FALSE);

                    if (source == 0)
                    {
                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_INVALID_DATA_SOURCE;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    UINT64 size = 0;
                    v_memRead((UINT64)&size, pkt->dataPointer + sizeof(UINT64), sizeof(UINT64), process.dirBase, FALSE);

                    if (size == 0)
                    {
                        SerialPrintString("Error: Size 0 \r\n");
                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_INVALID_DATA_SIZE;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    if (size > 0x1000)
                    {
                        SerialPrintString("Error: Size >0x1000 \r\n");
                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_REQ_TOO_LARGE;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    // Try to read it
                    unsigned char readBuffer[0x1000];

                    BOOLEAN status = p_memCpy((UINT64)readBuffer, source, size, FALSE);

                    if (status == FALSE)
                    {
                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_FAIL_PHYS_READ;
                        v_memWrite(pkt->resultPointer + size, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    // Set Result       
                    UINT16 temp = HERMES_STATUS_OK;
                    v_memWrite(pkt->resultPointer + size, (UINT64)&temp, sizeof(UINT16), process.dirBase, FALSE);
                    v_memWrite(pkt->resultPointer, (UINT64)readBuffer, size, process.dirBase, FALSE);

                    return FALSE;
                }
                else if (pkt->command == HERMES_CMD_WRITE_PHYSICAL)
                {
                    SerialPrintString("HERMES_CMD_WRITE_PHYSICAL request\r\n");

                    // Null the startidentifer to act as mutex
                    pkt->begin = 0;

                    if (pkt->resultPointer == 0)
                    {
                        SerialPrintString("Error: resultPtr not set \r\n");
                        goto clear_packet;
                    }

                    if (pkt->dataPointer == 0)
                    {
                        SerialPrintString("Error: dataPtr not set \r\n");

                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_INVALID_DATA_PTR;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    UINT64 destination = 0;
                    v_memRead((UINT64)&destination, pkt->dataPointer, sizeof(UINT64), process.dirBase, FALSE);

                    if (destination == 0)
                    {
                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_INVALID_DATA_SOURCE;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    UINT64 size = 0;
                    v_memRead((UINT64)&size, pkt->dataPointer + sizeof(UINT64), sizeof(UINT64), process.dirBase, FALSE);

                    if (size == 0)
                    {
                        SerialPrintString("Error: Size 0 \r\n");
                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_INVALID_DATA_SIZE;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    if (size > 0x1000)
                    {
                        SerialPrintString("Error: Size >0x1000 \r\n");
                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_REQ_TOO_LARGE;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    // Try to read the buffer to write
                    unsigned char writeBuffer[0x1000];

                    BOOLEAN status = v_memRead((UINT64)writeBuffer, pkt->dataPointer + sizeof(UINT64) + sizeof(UINT64), size, process.dirBase, FALSE);

                    if (status == FALSE)
                    {
                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_FAIL_SBUFF_PHYSW;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    // Try to write the buffer to destination
                    status = p_memCpy(destination, (UINT64)writeBuffer, size, FALSE);

                    if (status == FALSE)
                    {
                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_FAIL_PHYS_WRITE;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    // Set Result       
                    UINT16 temp = HERMES_STATUS_OK;
                    v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT16), process.dirBase, FALSE);

                    return FALSE;
                }
                else if (pkt->command == HERMES_CMD_VIRTUAL_TO_PHYSICAL)
                {
                    SerialPrintString("HERMES_CMD_VIRTUAL_TO_PHYSICAL request\r\n");

                    // Null the startidentifer to act as mutex
                    pkt->begin = 0;

                    if (pkt->resultPointer == 0)
                    {
                        SerialPrintString("Error: resultPtr not set \r\n");
                        goto clear_packet;
                    }

                    if (pkt->dataPointer == 0)
                    {
                        SerialPrintString("Error: dataPtr not set \r\n");

                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_INVALID_DATA_PTR;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    UINT64 source = 0;
                    v_memRead((UINT64)&source, pkt->dataPointer, sizeof(UINT64), process.dirBase, FALSE);

                    if (source == 0)
                    {
                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_INVALID_DATA_SOURCE;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    UINT64 dirbase = 0;
                    v_memRead((UINT64)&dirbase, pkt->dataPointer + sizeof(UINT64), sizeof(UINT64), process.dirBase, FALSE);

                    if (dirbase == 0)
                    {
                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_INVALID_DATA_DIRBASE;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    UINT64 temp = VTOP(source, dirbase, FALSE);

                    UINT64 physical = 0;

                    if (temp == 0)
                    {
                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_FAIL_ADDR_TRANSLATION;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }
                    else
                    {
                        physical = temp;
                    }

                    // Set Result         
                    v_memWrite(pkt->resultPointer, (UINT64)&physical, sizeof(UINT64), process.dirBase, FALSE);

                    return FALSE;
                }
                else if (pkt->command == HERMES_CMD_DUMP_MODULE)
                {
                    SerialPrintString("HERMES_CMD_DUMP_MODULE request\r\n");

                    // Null the startidentifer to act as mutex
                    pkt->begin = 0;

                    if (pkt->resultPointer == 0)
                    {
                        SerialPrintString("Error: resultPtr not set \r\n");
                        goto clear_packet;
                    }

                    if (pkt->dataPointer == 0)
                    {
                        SerialPrintString("Error: dataPtr not set \r\n");

                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_INVALID_DATA_PTR;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    UINT64 base = 0;
                    v_memRead((UINT64)&base, pkt->dataPointer, sizeof(UINT64), process.dirBase, FALSE);

                    if (base == 0)
                    {
                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_INVALID_MOD_BASE;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    UINT64 dirbase = 0;
                    v_memRead((UINT64)&dirbase, pkt->dataPointer + sizeof(UINT64), sizeof(UINT64), process.dirBase, FALSE);

                    if (dirbase == 0)
                    {
                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_INVALID_DATA_DIRBASE;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    UINT64 size = 0;
                    v_memRead((UINT64)&size, pkt->dataPointer + sizeof(UINT64) + sizeof(UINT64), sizeof(UINT64), process.dirBase, FALSE);

                    if (size == 0 || size < 0x1000)
                    {
                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_INVALID_MOD_SIZE;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    // Copy page per page
                    UINT16 invalid_counter = 0;

                    for (UINT64 i = 0; i < size; i += 0x1000)
                    {
                        BOOLEAN status = v_to_v_memCpy((UINT64)(pkt->resultPointer) + i, base + i, 0x1000, process.dirBase, dirbase, FALSE);

                        if (status == FALSE)
                        {
                            invalid_counter += 1;

                            if (invalid_counter >= 100)
                            {
                                // Abort
                                SerialPrintStringDebug("Many errors... \r\n");
                                //goto clear_packet;
                            }
                            SerialPrintStringDebug("FP ");
                            SerialPrintNumberDebug(i, 16);
                            SerialPrintStringDebug("\r\n");
                            // Maybe do some reporting? Something to think about
                            // Dont have to fill the pages with 0 as they are 0 initialized
                        }
                    }

                    // Set Result after the dump  
                    UINT16 temp = HERMES_STATUS_OK;
                    v_memWrite(pkt->resultPointer + size, (UINT64)&temp, sizeof(UINT16), process.dirBase, FALSE);

                    return FALSE;
                }
                else if (pkt->command == HERMES_CMD_GET_MODULES)
                {
                    SerialPrintString("HERMES_CMD_GET_MODULES request\r\n");

                    // Null the startidentifer to act as mutex
                    pkt->begin = 0;

                    if (pkt->resultPointer == 0)
                    {
                        SerialPrintString("Error: resultPtr not set \r\n");
                        goto clear_packet;
                    }

                    if (pkt->dataPointer == 0)
                    {
                        SerialPrintString("Error: dataPtr not set \r\n");

                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_INVALID_DATA_PTR;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    // Get Size of Process name
                    UINT8 size = 0;
                    v_memRead((UINT64)&size, pkt->dataPointer, sizeof(UINT8), process.dirBase, FALSE);

                    if (size == 0 || size >= 64)
                    {
                        SerialPrintString("Error: Size is ");
                        SerialPrintNumber(size, 10);
                        SerialPrintString("\r\n");

                        // Set Error Code as Result
                        UINT64 temp = HERMES_STATUS_PROCNAME_TOO_LONG;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);

                        return FALSE;
                    }

                    // Read Processname
                    CHAR8 bName[64];

                    v_memRead((UINT64)&bName, pkt->dataPointer + sizeof(UINT8), size, process.dirBase, FALSE);

                    SerialPrintStringDebug("Read Name: ");
                    SerialPrintStringDebug(bName);
                    SerialPrintStringDebug(".\r\n");

                    WinProc getModulesProcess;
                    UINT64 status = DumpSingleProcess(winGlobal, bName, &getModulesProcess, FALSE);

                    // Check status
                    if (status == FALSE)
                    {
                        // Set result
                        UINT64 temp = HERMES_STATUS_FAIL_FIND_PROC;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    // Process is ok, get list of modules
                    unsigned char names[0x1000];
                    UINT64 moduleListCount = 0;
                    status = DumpModuleNames(winGlobal, &getModulesProcess, FALSE, (UINT64)names, &moduleListCount);

                    SerialPrintString("Modules: ");
                    SerialPrintNumber(moduleListCount, 10);
                    SerialPrintString("\r\n");

                    // Check status
                    if (status == FALSE)
                    {
                        // Set result
                        UINT64 temp = HERMES_STATUS_FAIL_FIND_MOD;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        return FALSE;
                    }

                    // Set Result       
                    UINT16 temp = HERMES_STATUS_OK;
                    v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT16), process.dirBase, FALSE);
                    v_memWrite(pkt->resultPointer + sizeof(UINT64), (UINT64)&moduleListCount, sizeof(UINT8), process.dirBase, FALSE);
                    v_memWrite(pkt->resultPointer + sizeof(UINT64) + sizeof(UINT8), (UINT64)names, 0x1000 - sizeof(UINT64) - sizeof(UINT8), process.dirBase, FALSE);

                    return FALSE;
                }
                else
                {
                    // No command matched, return error code in status if possible
                    if (pkt->resultPointer == 0)
                    {
                        UINT64 temp = HERMES_STATUS_INVALID_COMMAND;
                        v_memWrite(pkt->resultPointer, (UINT64)&temp, sizeof(UINT64), process.dirBase, FALSE);
                        SerialPrintString("Error: command invalid \r\n");
                        return FALSE;
                    }
                }

                // If it reaches here we should clear the found_packet as we cant return an error code
                clear_packet:
                SerialPrintString("Nulling Packet.. \r\n");
                for (UINT8 k = 0; k < sizeof(UINT16) + sizeof(UINT8) + sizeof(UINT64) + sizeof(UINT64) + sizeof(UINT16); k++)
                {
                    UINT8 *nuller = (UINT8 *)found_packet + k;
                    *nuller = 0x0;
                }
            }
        }
    }
    return TRUE;
}