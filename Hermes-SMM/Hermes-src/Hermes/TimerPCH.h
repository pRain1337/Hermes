#ifndef __hermes_timer_pch_h__
#define __hermes_timer_pch_h__

#include <Uefi.h>
#include <Library/IoLib.h>
#include <Library/PciLib.h>
#include <Protocol/PciIo.h>

#define PERIODIC_EN BIT14
#define GBL_SMI_EN BIT0

#define SMI_EN   0x30
#define SMI_STS   0x34

VOID PchInitTimer();

VOID ClearBits();

#endif