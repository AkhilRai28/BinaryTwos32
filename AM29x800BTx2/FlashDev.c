/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2006                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Device Description for Dual AM29x800BT (32-bit Bus)   */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures


struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "AM29x800BT Dual Flash",    // Device Name
   EXT32BIT,                   // Device Type
   0x000000,                   // Device Start Address
   0x200000,                   // Device Size in Bytes (2MB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   3000,                       // Erase Sector Timeout 3000 mSec

// Specify Size and Address of Sectors
   0x20000, 0x000000,          // Sector Size Dual 64kB (14 Sectors)
   0x08000, 0x1C0000,          // Sector Size Dual 16kB (1 Sector)
   0x10000, 0x1C8000,          // Sector Size Dual 32kB (1 Sector)
   0x04000, 0x1D8000,          // Sector Size Dual  8kB (4 Sectors)
   0x10000, 0x1E8000,          // Sector Size Dual 32kB (1 Sector)
   0x08000, 0x1F8000,          // Sector Size Dual 16kB (1 Sector)
   SECTOR_END
};
