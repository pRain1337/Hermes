#include "TimerPCH.h"
    
VOID PchInitTimer()
{   
    // According to Intel Chipset Datasheet Vol 2 you read the BAR2 through PCI functions.
    // This is incorrect, the Intel FSP Integration Guide starting from series 300 chipsets state, that you must use 0x1800 and the pci itself will be hidden after PEI stage.
    UINT32 BAR2 = 0x1800;

    UINT32 mSmiEnable = BAR2 + SMI_EN;
    UINT32 SmiEnableVal = 0;
    SmiEnableVal = IoRead32(mSmiEnable);
    
    SerialPrintStringDebug("SMI_EN: ");
    SerialPrintNumberDebug(SmiEnableVal, 16);
    SerialPrintStringDebug("\r\n");
    
    if ((SmiEnableVal & PERIODIC_EN) == 0)
    {
        SmiEnableVal |= PERIODIC_EN;
        IoWrite32(BAR2 + SMI_EN, SmiEnableVal);
    }
    
    if ((SmiEnableVal & GBL_SMI_EN) == 0)
    {
        SmiEnableVal |= GBL_SMI_EN;
        IoWrite32(BAR2 + SMI_EN, SmiEnableVal);
    }
   
	return;
}

VOID ClearBits()
{
    // According to Intel Chipset Datasheet Vol 2 you read the BAR2 through PCI functions.
    // This is incorrect, the Intel FSP Integration Guide starting from series 300 chipsets state, that you must use 0x1800 and the pci itself will be hidden after PEI stage.
    UINT32 BAR2 = 0x1800;

    UINT32 mSmiSts = BAR2 + SMI_STS;
    UINT32 SmiStatusVal = 0;
    SmiStatusVal = IoRead32(mSmiSts);
    
    if ((SmiStatusVal & PERIODIC_EN) != 0)
    {
        // Timer SMI Status by writing 1 to it
        SmiStatusVal |= PERIODIC_EN;
        IoWrite32(BAR2 + SMI_STS, SmiStatusVal);
    }
}