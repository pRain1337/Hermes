// Basic UEFI Libraries
#include <Uefi.h>

// Protocols
#include <Protocol/SmmCpu.h>
#include <Protocol/SmmBase2.h>
#include <Protocol/SmmSwDispatch2.h>
#include <Protocol/SerialIo.h>
#include <Library/PcdLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/PciIo.h>
#include <Library/PciLib.h>

// Our includes
#include "MemoryMapUEFI.h"
#include "TimerRTC.h"
#include "TimerPCH.h"

#include "serial.h"
#include "Memory.h"
#include "Hermes.h"

/*
 * Just a workaround for stupid MVSC pragmas
 * and other GCC-only warnings treated as errors.
 */
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-value"
#endif

 //
 // On windows these can be changed
 //
#ifndef __GNUC__
extern CONST UINT32 _gUefiDriverRevision = 0;
CHAR8 *gEfiCallerBaseName = "Hermes";
#endif

EFI_SMM_SYSTEM_TABLE2       *gSmst2;

// UEFI Tables (will be gone after exiting DXE stage) 
extern EFI_SYSTEM_TABLE *gST;
extern EFI_BOOT_SERVICES *gBS;
extern EFI_RUNTIME_SERVICES *gRT;

// NTKernelTools.c
extern WinCtx *winGlobal;


// System initialization vars
UINT32 SystemStartTime;
UINT32 SystemUptime;
BOOLEAN SystemInitOS;


VOID SmmCallHandle()
{
    if (!SystemInitOS)
    {
        // try to grab the windows Context
        SystemInitOS = InitGlobalWindowsContext();

        // give more time if it still failed
        if (!SystemInitOS)
        {
            SerialPrintString("   System OS not found...\r\n");
            SystemStartTime = SystemUptime;
            return;
        }
    }

    // If system has been initialized, poll for user commands
    if (FindProcess(winGlobal, "hermes.exe", FALSE))
    {
        // Do Hermes Stuff
        HermesPollCommands();
    }

    return;
}

EFI_STATUS EFIAPI SmmHandler(IN EFI_HANDLE DispatchHandle, IN CONST VOID *Context OPTIONAL, IN OUT VOID *CommBuffer OPTIONAL, IN OUT UINTN *CommBufferSize OPTIONAL)
{
    // if the OS has not been initialized
    if (!SystemInitOS)
    {
        // count if the OS SHOULD be initialized
        UINT16 TimeSinceLastSMI = CmosGetCurrentTime();

        // Did we overflow? This happens once every hour
        if (TimeSinceLastSMI < SystemUptime)
            SystemUptime += TimeSinceLastSMI;
        else
            SystemUptime = TimeSinceLastSMI;

        // ctx not initialized and system hasn't booted completely
        if (SystemUptime - SystemStartTime < 10)
        {
            goto end;
        }
    }

    SerialPortInitialize(SERIAL_PORT_0, SERIAL_BAUDRATE);
    SmmCallHandle();
    
    end:
    PchInitTimer();
    ClearBits();

    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI UefiMain(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
    // Write to serial port
    SerialPortInitialize(SERIAL_PORT_0, SERIAL_BAUDRATE);
    SerialPrintString("\r\n");
    SerialPrintString("----------------------------------------------\r\n");
    SerialPrintString("|                                            |\r\n");
    SerialPrintString("|             Initializing HERMES            |\r\n");
    SerialPrintString("|                                            |\r\n");
    SerialPrintString("|                   Made by                  |\r\n");
    SerialPrintString("|                 rain jussihi               |\r\n");
    SerialPrintString("|                                            |\r\n");
    SerialPrintString("|                Shoutout to                 |\r\n");
    SerialPrintString("|          zyklon   vasyan    balrog         |\r\n");
    SerialPrintString("|   floesen    niceone   sh4rk   akandesh    |\r\n");
    SerialPrintString("----------------------------------------------\r\n");
    SerialPrintString("                                              \r\n");
    SerialPrintString("\r\n");

    // Save the system tables etc. in global variable for further usage
    gST = SystemTable;
    gBS = SystemTable->BootServices;
    gRT = SystemTable->RuntimeServices;

    EFI_STATUS Status = EFI_SUCCESS;
    EFI_SMM_BASE2_PROTOCOL *SmmBase2;

    // lookup the SmmBase2 protocol by its GUID
    EFI_GUID SmmBase2Guid = EFI_SMM_BASE2_PROTOCOL_GUID;
    if ((Status = SystemTable->BootServices->LocateProtocol(&SmmBase2Guid, NULL, (void **)&SmmBase2)) != EFI_SUCCESS)
    {
        SerialPrintString("Could not locate SmmBase2 protocol!\r\n");
        return Status;
    }
    
    // get EFI_SMM_SYSTEM_TABLE2 in global var
    if ((Status = SmmBase2->GetSmstLocation(SmmBase2, &gSmst2)) != EFI_SUCCESS)
    {
        SerialPrintString("Could not locate SMST!\r\n");
        return Status;
    }

    // Register SMI Root Handler, discard the returning handle (we never unload the handler)
    EFI_HANDLE hSmmHandler;
    if ((Status = gSmst2->SmiHandlerRegister(&SmmHandler, NULL, &hSmmHandler)) != EFI_SUCCESS)
    {
        return Status;
    }

    SerialPrintString("Allocating initial memory \r\n");

    EFI_PHYSICAL_ADDRESS winGlobalAddress;
    Status = gSmst2->SmmAllocatePages(AllocateAnyPages, EfiRuntimeServicesData, 1, &winGlobalAddress);
    winGlobal = (WinCtx *)winGlobalAddress;
    SerialPrintString("WinGlobal: 0x");
    SerialPrintNumber((UINT64)winGlobal, 16);
    SerialPrintString("\r\n");

    // Initialize our own heap with some memory to be used
    if (InitMemManager(100))
    {
        SerialPrintString("memory manager successfully initialized!\r\n");
    }

    // Initialize the virtual memory map for UEFI
    SerialPrintString("Initializing UEFI Memory Map \r\n");
    if (!InitUefiMemoryMap())
    {
        SerialPrintString("Failed dumping Memory Map for UEFI \r\n");
        return EFI_ERROR_MAJOR;
    }
    SerialPrintString("Successfully dumped Memory Map \r\n");

    SerialPrintString("Memory Map at: 0x");
    SerialPrintNumber((UINT64)GetUefiMemoryMap(), 16);
    SerialPrintString("\r\n");

    // Set the start time of the PC
    SystemStartTime = CmosGetCurrentTime();
    SystemUptime = SystemStartTime;
    SerialPrintStringDebug("Start time was: ");
    SerialPrintNumberDebug(SystemStartTime, 10);
    SerialPrintStringDebug("\r\n");

    // Initialize the os ctx value, so no useless
    // probing is done while the OS hasn't even booted
    SystemInitOS = FALSE;

    PchInitTimer();

    return Status;
}


#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif