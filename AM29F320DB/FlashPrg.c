/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2006                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Flash Programming Functions adapted                   */
/*               for AM29F320DB (16-bit Bus)                           */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures

#define M8(adr)  (*((volatile unsigned char  *) (adr)))
#define M16(adr) (*((volatile unsigned short *) (adr)))
#define M32(adr) (*((volatile unsigned long  *) (adr)))

#define STACK_SIZE   64        // Stack Size


union fsreg {                  // Flash Status Register
  struct b  {
    unsigned int q0:1;
    unsigned int q1:1;
    unsigned int q2:1;
    unsigned int q3:1;
    unsigned int q4:1;
    unsigned int q5:1;
    unsigned int q6:1;
    unsigned int q7:1;
  } b;
  unsigned int v;
} fsr;

unsigned long base_adr;


/*
 * Check if Program/Erase completed
 *    Parameter:      adr:  Block Start Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int Polling (unsigned long adr) {
  unsigned int q6;

  fsr.v = M16(adr);
  q6 = fsr.b.q6;
  do {
    fsr.v = M16(adr);
    if (fsr.b.q6 == q6) return (0);  // Done
    q6 = fsr.b.q6;
  } while (fsr.b.q5 == 0);           // Check for Timeout
  fsr.v = M16(adr);
  q6 = fsr.b.q6;
  fsr.v = M16(adr);
  if (fsr.b.q6 == q6) return (0);    // Done
  M16(adr) = 0xF0;                   // Reset Device
  return (1);                        // Failed
}


/*
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int Init (unsigned long adr, unsigned long clk, unsigned long fnc) {
  base_adr = adr;
  return (0);
}


/*
 *  De-Initialize Flash Programming Functions
 *    Parameter:      fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int UnInit (unsigned long fnc) {
  return (0);
}


/*
 *  Erase complete Flash Memory
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseChip (void) {

  // Start Chip Erase Command
  M16(base_adr + 0xAAA) = 0xAA;
  M16(base_adr + 0x554) = 0x55;
  M16(base_adr + 0xAAA) = 0x80;
  M16(base_adr + 0xAAA) = 0xAA;
  M16(base_adr + 0x554) = 0x55;
  M16(base_adr + 0xAAA) = 0x10;

  return (Polling(base_adr));  // Wait until Erase completed
}


/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseSector (unsigned long adr) {

  // Start Erase Sector Command
  M16(base_adr + 0xAAA) = 0xAA;
  M16(base_adr + 0x554) = 0x55;
  M16(base_adr + 0xAAA) = 0x80;
  M16(base_adr + 0xAAA) = 0xAA;
  M16(base_adr + 0x554) = 0x55;
  M16(adr) = 0x30;

  do {
    fsr.v = M16(adr);
  } while (fsr.b.q3 == 0);     // Wait for Sector Erase Timeout

  return (Polling(adr));       // Wait until Erase completed
}


/*
 *  Program Page in Flash Memory
 *    Parameter:      adr:  Page Start Address
 *                    sz:   Page Size
 *                    buf:  Page Data
 *    Return Value:   0 - OK,  1 - Failed
 */

int ProgramPage (unsigned long adr, unsigned long sz, unsigned char *buf) {
  int i;

  for (i = 0; i < ((sz+1)/2); i++)  {
    // Start Program Command
    M16(base_adr + 0xAAA) = 0xAA;
    M16(base_adr + 0x554) = 0x55;
    M16(base_adr + 0xAAA) = 0xA0;
    M16(adr) = *((unsigned short *) buf);
    if (Polling(adr) != 0) return (1);
    buf += 2;
    adr += 2;
  }
  return (0);
}
