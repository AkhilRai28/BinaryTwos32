/* -----------------------------------------------------------------------------
 * Copyright (c) 2014 ARM Ltd.
 *
 * This software is provided 'as-is', without any express or implied warranty. 
 * In no event will the authors be held liable for any damages arising from 
 * the use of this software. Permission is granted to anyone to use this 
 * software for any purpose, including commercial applications, and to alter 
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not 
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be 
 *    appreciated but is not required. 
 * 
 * 2. Altered source versions must be plainly marked as such, and must not be 
 *    misrepresented as being the original software. 
 * 
 * 3. This notice may not be removed or altered from any source distribution.
 *   
 *
 * $Date:        04. September 2014
 * $Revision:    V1.00
 *  
 * Project:      Flash Programming Functions for
 *               NXP LPC18xx/LPC43xx S25FL032 SPIFI Flash
 * --------------------------------------------------------------------------- */

/* History:
 *  Version 1.00
 *    Initial release
 */ 

#include <stdbool.h>
#include "../FlashOS.H"        // FlashOS Structures
#include "spifilib_api.h"


#define CGU_BASE_SPIFI0_CLK     (*(volatile unsigned long *)0x40050070)

#define SCU_SFSP3_3             (*(volatile unsigned long *)0x4008618C)
#define SCU_SFSP3_4             (*(volatile unsigned long *)0x40086190)
#define SCU_SFSP3_5             (*(volatile unsigned long *)0x40086194)
#define SCU_SFSP3_6             (*(volatile unsigned long *)0x40086198)
#define SCU_SFSP3_7             (*(volatile unsigned long *)0x4008619C)
#define SCU_SFSP3_8             (*(volatile unsigned long *)0x400861A0)

#define SPIFI_BASE                                          0x40003000

uint32_t memSize;
uint32_t lmem[21];
SPIFI_HANDLE_T *pSpifi;

unsigned long base_adr;

/* This function initializes all the RO, RW data in the PI library.
   It is called is called from the C library startup code.
   The flash algorithm does not execute the C library startup code
   so we must call it manually.
 */
extern void __cpp_initialize__aeabi_(void); 

/*  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int Init (unsigned long adr, unsigned long clk, unsigned long fnc) {
  SPIFI_ERR_T errCode;

  base_adr = adr;

  __cpp_initialize__aeabi_();	
	
  /* init SPIFI clock and pins */
  CGU_BASE_SPIFI0_CLK = 1<<24 | 1<<11; /* IRC 12 MHz is good enough for us */

  /* set up SPIFI I/O (undocumented bit 7 set as 1, Aug 2 2011) */
  SCU_SFSP3_3 = 0xF3; /* high drive for SCLK */
  /* IO pins */
  SCU_SFSP3_4 =
  SCU_SFSP3_5 =
  SCU_SFSP3_6 =
  SCU_SFSP3_7 = 0xD3;
  SCU_SFSP3_8 = 0x13; /* CS doesn't need feedback */

  /* Initialize LPCSPIFILIB library, reset the interface */
  errCode = spifiInit(SPIFI_BASE, true);
  if (errCode != SPIFI_ERR_NONE) {
    return(1);
  }

  /* register support for the family(s) we may want to work with */
  //  spifiRegisterFamily(SPIFI_REG_FAMILY_Spansion_2Byte_PStatus);
  spifiRegisterFamily(spifi_REG_FAMILY_CommonCommandSet);

  /* Get required memory for detected device, this may vary per device family */
  memSize = spifiGetHandleMemSize(SPIFI_BASE);
  if (memSize == 0) {
    return(1);                               /* No device detected, error */
  }

  /* Initialize and detect a device and get device context */
  /* NOTE: Since we don't have malloc enabled we are just supplying
           a chunk of memory that we know is large enough. It would be
           better to use malloc if it is available. */
  pSpifi = spifiInitDevice(&lmem, sizeof(lmem), SPIFI_BASE, base_adr);
  if (pSpifi == ((void *) 0)) {
    return(1);
  }

  spifiDevSetMemMode(pSpifi, true);          /* Enter memMode */

  return 0;                                  /* Finished without Errors */
}


/*  De-Initialize Flash Programming Functions
 *    Parameter:      fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int UnInit (unsigned long fnc) {

  /* Done, de-init will enter memory mode */
  spifiDevDeInit(pSpifi);
  
  return 0;                                  // Finished without Errors
}


/*  Erase complete Flash Memory
 *    Return Value:   0 - OK,  1 - Failed */

int EraseChip (void) {
  SPIFI_ERR_T errCode;
  
  spifiDevSetMemMode(pSpifi, false);         /* Leave memMode */
  errCode = spifiErase(pSpifi, 0, spifiDevGetInfo(pSpifi, SPIFI_INFO_ERASE_BLOCKS));
  spifiDevSetMemMode(pSpifi, true);          /* Enter memMode */
  if (errCode != SPIFI_ERR_NONE) {
    return(1);
  }
  
  return (0);
}


/*  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseSector (unsigned long adr) {
  SPIFI_ERR_T errCode;
  uint32_t    blockNr;

  blockNr = spifiGetBlockFromAddr(pSpifi, (uint32_t)adr);
  if (blockNr == 0xFFFFFFFF) {
    return(1);
  }

  spifiDevSetMemMode(pSpifi, false);         /* Leave memMode */
  errCode = spifiErase(pSpifi, blockNr, 1);
  spifiDevSetMemMode(pSpifi, true);          /* Enter memMode */
  if (errCode != SPIFI_ERR_NONE) {
    return(1);
  }
  
  return (0);
}


/*  Program Page in Flash Memory
 *    Parameter:      adr:  Page Start Address
 *                    sz:   Page Size
 *                    buf:  Page Data
 *    Return Value:   0 - OK,  1 - Failed
 */

int ProgramPage (unsigned long adr, unsigned long sz, unsigned char *buf) {
  SPIFI_ERR_T errCode;

  spifiDevSetMemMode(pSpifi, false);         /* Leave memMode */
  errCode = spifiProgram(pSpifi, (uint32_t)adr, (uint32_t *)buf, sz);
  spifiDevSetMemMode(pSpifi, true);          /* Enter memMode */
  if (errCode != SPIFI_ERR_NONE) {
    return (1);
  }

  return (0);
}
