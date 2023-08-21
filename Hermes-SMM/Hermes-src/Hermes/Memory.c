#include "Memory.h"

// from Main.c
extern EFI_SMM_SYSTEM_TABLE2 *gSmst2;

BOOLEAN p_memCpy(UINT64 dest, UINT64 src, size_t n, BOOLEAN verbose)
{
  // Check if the address ranges are in allowed range
  if ((IsAddressValid((UINT64)src) == FALSE || IsAddressValid((UINT64)(src + n - 1)) == FALSE))
  {
    SerialPrintString("[p_memCpy] Aborted duo to disallowed memory range \r\n");

    return FALSE;
  }

  // Typecast src and dest addresses to (char *)
  CHAR8 *csrc = (char *)src;
  CHAR8 *cdest = (char *)dest;

  // Copy contents of src[] to dest[]
  for (INT32 i = 0; i < n; i++)
    cdest[i] = csrc[i];

  return TRUE;
}

UINT64 VTOP(UINT64 address, UINT64 directoryBase, BOOLEAN verbose)
{
  if (address == 0 && verbose)
  {
    SerialPrintStringDebug("[VTOP] address is 0 \r\n");
    return 0;
  }

  if (directoryBase == 0 && verbose)
  {
    SerialPrintStringDebug("[VTOP] directoryBase is 0 \r\n");
    return 0;
  }

  directoryBase &= ~0xf;

  UINT64 pageOffset = address & ~(~0ul << PAGE_OFFSET_SIZE);
  UINT64 pte = ((address >> 12) & (0x1ffll));
  UINT64 pt = ((address >> 21) & (0x1ffll));
  UINT64 pd = ((address >> 30) & (0x1ffll));
  UINT64 pdp = ((address >> 39) & (0x1ffll));

  if (verbose)
  {
    SerialPrintString("Dirbase:  ");
    SerialPrintNumber(directoryBase, 16);
    SerialPrintString(" VA ");
    SerialPrintNumber(address, 16);
    SerialPrintString(" PO:  ");
    SerialPrintNumber(pageOffset, 16);
    SerialPrintString(" PTE ");
    SerialPrintNumber(pte, 16);
    SerialPrintString(" PT ");
    SerialPrintNumber(pt, 16);
    SerialPrintString(" PD ");
    SerialPrintNumber(pd, 16);
    SerialPrintString(" PDP ");
    SerialPrintNumber(pdp, 16);
    SerialPrintString("\r\n");
  }

  UINT64 pdpe = 0;
  p_memCpy((UINT64)&pdpe, directoryBase + 8 * pdp, sizeof(UINT64), verbose);

  if (verbose)
  {
    SerialPrintString("Dump PDPE at ");
    SerialPrintNumber(directoryBase + 8 * pdp, 16);
    SerialPrintString("results ");
    SerialPrintNumber(pdpe, 16);
    SerialPrintString("\r\n");
  }

  if (~pdpe & 1)
    return 0;

  UINT64 pde = 0;
  p_memCpy((UINT64)&pde, (UINT64)(pdpe & PMASK2) + 8 * pd, sizeof(UINT64), verbose);

  if (verbose)
  {
    SerialPrintString("Dump pde at ");
    SerialPrintNumber((pdpe & PMASK2) + 8 * pd, 16);
    SerialPrintString("results ");
    SerialPrintNumber(pde, 16);
    SerialPrintString("\r\n");
  }

  if (~pde & 1)
    return 0;

  /* 1GB large page, use pde's 12-34 bits */
  if (pde & 0x80)
    return (pde & (~0ull << 42 >> 12)) + (address & ~(~0ull << 30));

  UINT64 pteAddr = 0;
  p_memCpy((UINT64)&pteAddr, (UINT64)(pde & PMASK2) + 8 * pt, sizeof(UINT64), verbose);

  if (verbose)
  {
    SerialPrintString("Dump pteAddr at ");
    SerialPrintNumber((pde & PMASK2) + 8 * pt, 16);
    SerialPrintString("results ");
    SerialPrintNumber(pteAddr, 16);
    SerialPrintString("\r\n");
  }

  if (~pteAddr & 1)
    return 0;

  /* 2MB large page */
  if (pteAddr & 0x80)
    return (pteAddr & PMASK) + (address & ~(~0ull << 21));

  p_memCpy((UINT64)&address, (UINT64)(pteAddr & PMASK) + 8 * pte, sizeof(UINT64), verbose);

  address = address & PMASK;

  if (verbose)
  {
    SerialPrintString("Dump address at ");
    SerialPrintNumber((pteAddr & PMASK) + 8 * pte, 16);
    SerialPrintString("results ");
    SerialPrintNumber(address, 16);
    SerialPrintString("\r\n");
  }

  if (!address)
    return 0;

  // UINT64 tempPhys = address & 0xFFFFFFFFFFFFF000;
  // UINT64 physAddress = tempPhys + virtOffs;

  return address + pageOffset;
}

BOOLEAN v_memWrite(UINT64 dest, UINT64 src, size_t n, UINT64 directoryBase, BOOLEAN verbose)
{
  // Translate to physical
  UINT64 pDest = VTOP(dest, directoryBase, FALSE);

  if (pDest == 0)
  {
    return FALSE;
  }

  // Read physical
  return p_memCpy(pDest, src, n, verbose);
}

BOOLEAN v_memReadMultiPage(UINT64 dest, UINT64 src, size_t n, UINT64 directoryBase, BOOLEAN verbose)
{
  UINT64 curr_vAddr = src;
  UINT64 read = 0;

  while (n > 0)
  {
    UINT64 nextPage = (curr_vAddr + 0x1000) & ~0xfff;
    UINT64 to_read = nextPage - curr_vAddr;

    // if it's the "last" read
    if (n < to_read)
      to_read = n;

    // Translate to physical
    UINT64 pSrc = VTOP(curr_vAddr, directoryBase, FALSE);
    if (pSrc == 0)
    {
      return FALSE;
    }
    // read physical
    p_memCpy(dest + read, pSrc, to_read, verbose);
    n -= to_read;
    read += to_read;
    curr_vAddr += to_read;
  }
  return TRUE;
}

BOOLEAN v_memRead(UINT64 dest, UINT64 src, size_t n, UINT64 directoryBase, BOOLEAN verbose)
{
  // Translate to physical
  UINT64 pSrc = VTOP(src, directoryBase, FALSE);

  if (pSrc == 0)
  {
    return FALSE;
  }

  // Read physical
  return p_memCpy(dest, pSrc, n, verbose);
}

BOOLEAN v_to_v_memCpy(UINT64 dest, UINT64 src, size_t n, UINT64 directoryBaseDest, UINT64 directoryBaseSource, BOOLEAN verbose)
{
  // Translate source virtual address to physical
  UINT64 pSrc = VTOP(src, directoryBaseSource, verbose);
    
  if (pSrc == 0)
  {
    return FALSE;
  }

  // Translate destination virtual address to physical
  UINT64 pDest = VTOP(dest, directoryBaseDest, verbose);
    
  if (pDest == 0)
  {
    return FALSE;
  }

  // Read physical
  return p_memCpy(pDest, pSrc, n, verbose);
}