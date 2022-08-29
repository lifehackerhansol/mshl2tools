/*-----------------------------------------------------------------
 boot.c
 
 BootLoader
 Loads a file into memory and runs it

 All resetMemory and startBinary functions are based 
 on the MultiNDS loader by Darkain.
 Original source available at:
 http://cvs.sourceforge.net/viewcvs.py/ndslib/ndslib/examples/loader/boot/main.cpp

License:
 Copyright (C) 2005  Michael "Chishm" Chisholm

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 If you use this code, please give due credit and email me about your
 project at chishm@hotmail.com
 
Helpful information:
 This code runs from the first Shared IWRAM bank:
 0x037F8000 to 0x037FC000
------------------------------------------------------------------*/

#include <nds/ndstypes.h>
#include <nds/registers_alt.h>
#include <nds/dma.h>
#include <nds/system.h>
#include <nds/interrupts.h>
#include <nds/timers.h>
#define ARM9
#undef ARM7
#include <nds/memory.h>
#include <nds/arm9/video.h>
#include <nds/arm9/input.h>
#undef ARM9
#define ARM7
#include <nds/arm7/audio.h>
#include <nds/fifocommon.h>

#include "fat.h"
#include "dldi_patcher.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Important things
#define TEMP_MEM 0x027FE000
#define NDS_HEAD 0x027FFE00
#define TEMP_ARM9_START_ADDRESS (*(vu32*)0x027FFFF4)


//#define STORED_FILE_CLUSTER (*(vu32*)0x027FFFFC)

#define ARM9_START_FLAG (*(vu8*)0x027FFDFF)
const char* bootName = "_BOOT_DS.NDS";

extern unsigned long storedFileCluster;
extern unsigned long initDisc;
extern unsigned long wantToPatchDLDI;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Firmware stuff

#define FW_READ        0x03

void boot_readFirmware (uint32 address, uint8 * buffer, uint32 size) {
  uint32 index;

  // Read command
  while (REG_SPICNT & SPI_BUSY);
  REG_SPICNT = SPI_ENABLE | SPI_CONTINUOUS | SPI_DEVICE_NVRAM;
  REG_SPIDATA = FW_READ;
  while (REG_SPICNT & SPI_BUSY);

  // Set the address
  REG_SPIDATA =  (address>>16) & 0xFF;
  while (REG_SPICNT & SPI_BUSY);
  REG_SPIDATA =  (address>>8) & 0xFF;
  while (REG_SPICNT & SPI_BUSY);
  REG_SPIDATA =  (address) & 0xFF;
  while (REG_SPICNT & SPI_BUSY);

  for (index = 0; index < size; index++) {
    REG_SPIDATA = 0;
    while (REG_SPICNT & SPI_BUSY);
    buffer[index] = REG_SPIDATA & 0xFF;
  }
  REG_SPICNT = 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Loader functions

void CpuFastSet (u32 src, u32 dest, u32 ctrl)
{
	__asm volatile ("swi 0x0C0000\n");
}

static inline void MYdmaFillWords(const void* src, void* dest, uint32 size) {
	DMA_SRC(3)  = (uint32)src;
	DMA_DEST(3) = (uint32)dest;
	DMA_CR(3)   = DMA_COPY_WORDS | DMA_SRC_FIX | (size>>2);
	while(DMA_CR(3) & DMA_BUSY);
}

static inline void copyLoop (u32* dest, const u32* src, u32 size) {
	do {
		*dest++ = *src++;
	} while (size -= 4);
}

#define resetCpu() \
		__asm volatile("swi 0x000000")

/*-------------------------------------------------------------------------
resetMemory1_ARM9
Clears the ARM9's icahce and dcache
Written by Darkain.
Modified by Chishm:
 Changed ldr to mov & add
 Added clobber list
--------------------------------------------------------------------------*/
/*
#define resetMemory1_ARM9_size 0x400
void __attribute__ ((long_call)) resetMemory1_ARM9 (void) 
{
	// Return to loop
	*((vu32*)0x027FFE04) = (u32)0xE59FF018;		// ldr pc, 0x027FFE24
	*((vu32*)0x027FFE24) = (u32)0x027FFE04;		// Set ARM9 Loop address
	__asm volatile("bx %0" : : "r" (0x027FFE04) ); 
}
*/
/*-------------------------------------------------------------------------
resetMemory2_ARM9
Clears the ARM9's DMA channels and resets video memory
Written by Darkain.
Modified by Chishm:
 * Changed MultiNDS specific stuff
--------------------------------------------------------------------------*/
#define resetMemory2_ARM9_size 0x400
void __attribute__ ((long_call)) resetMemory2_ARM9 (void) 
{
 	register int i;

	REG_IME = 0;
	REG_IE = 0;
	REG_IF = ~0;
	(*(vu32*)0x00803FFC) = 0;   //IRQ_HANDLER ARM9 version
	(*(vu32*)0x00803FF8) = ~0;  //VBLANK_INTR_WAIT_FLAGS ARM9 version

	//clear out ARM9 DMA channels
	for (i=0; i<4; i++) {
		DMA_CR(i) = 0;
		DMA_SRC(i) = 0;
		DMA_DEST(i) = 0;
		TIMER_CR(i) = 0;
		TIMER_DATA(i) = 0;
	}

	VRAM_CR = (VRAM_CR & 0xffff0000) | 0x00008080 ;
	(*(vu32*)0x027FFE04) = 0;   // temporary variable
	BG_PALETTE[0] = 0xFFFF;
	MYdmaFillWords((void*)0x027FFE04, BG_PALETTE+1, (2*1024)-2);
	MYdmaFillWords((void*)0x027FFE04, OAM,     2*1024);
	MYdmaFillWords((void*)0x027FFE04, (void*)0x04000000, 0x56);  //clear main display registers
	MYdmaFillWords((void*)0x027FFE04, (void*)0x04001000, 0x56);  //clear sub  display registers
	MYdmaFillWords((void*)0x027FFE04, VRAM,  656*1024);

	
	REG_DISPSTAT = 0;
	videoSetMode(0);
	videoSetModeSub(0);
	VRAM_A_CR = 0;
	VRAM_B_CR = 0;
// Don't mess with the ARM7's VRAM
//	VRAM_C_CR = 0;
	VRAM_D_CR = 0;
	VRAM_E_CR = 0;
	VRAM_F_CR = 0;
	VRAM_G_CR = 0;
	VRAM_H_CR = 0;
	VRAM_I_CR = 0;
//	VRAM_CR   = 0x03000000;
	REG_POWERCNT  = 0x820F;

	//set shared ram to ARM7
	WRAM_CR = 0x03;

	// Return to loop
	*((vu32*)0x027FFE04) = (u32)0xE59FF018;		// ldr pc, 0x027FFE24
	*((vu32*)0x027FFE24) = (u32)0x027FFE04;		// Set ARM9 Loop address
	__asm volatile("bx %0" : : "r" (0x027FFE04) ); 
}


/*-------------------------------------------------------------------------
startBinary_ARM9
Jumps to the ARM9 NDS binary in sync with the display and ARM7
Written by Darkain.
Modified by Chishm:
 * Removed MultiNDS specific stuff
--------------------------------------------------------------------------*/
#define startBinary_ARM9_size 0xA0
void __attribute__ ((long_call)) startBinary_ARM9 (void)
{
	REG_EXMEMCNT = 0xE880;
	// set ARM9 load address to 0 and wait for it to change again
	ARM9_START_FLAG = 0;
	while(REG_VCOUNT!=191);
	while(REG_VCOUNT==191);
	while ( ARM9_START_FLAG != 1 );
	// wait for vblank
	while(REG_VCOUNT!=191);
	while(REG_VCOUNT==191);
	resetCpu();
}

/*-------------------------------------------------------------------------
resetMemory_ARM7
Clears all of the NDS's RAM that is visible to the ARM7
Written by Darkain.
Modified by Chishm:
 * Added STMIA clear mem loop
--------------------------------------------------------------------------*/
void resetMemory_ARM7 (void)
{
	int i;
	u8 settings1, settings2;
	u32 settingsOffset = 0;
	
	REG_IME = 0;

	for (i=0; i<16; i++) {
		SCHANNEL_CR(i) = 0;
		SCHANNEL_TIMER(i) = 0;
		SCHANNEL_SOURCE(i) = 0;
		SCHANNEL_LENGTH(i) = 0;
	}

	REG_SOUNDCNT = 0;

	//clear out ARM7 DMA channels and timers
	for (i=0; i<4; i++) {
		DMA_CR(i) = 0;
		DMA_SRC(i) = 0;
		DMA_DEST(i) = 0;
		TIMER_CR(i) = 0;
		TIMER_DATA(i) = 0;
	}
	
	arm7clearRAM();


	REG_IE = 0;
	REG_IF = ~0;
	(*(vu32*)(0x04000000-4)) = 0;  //IRQ_HANDLER ARM7 version
	(*(vu32*)(0x04000000-8)) = ~0; //VBLANK_INTR_WAIT_FLAGS, ARM7 version
	REG_POWERCNT = 1;  //turn off power to stuff
	
	// Get settings location
	boot_readFirmware((u32)0x00020, (u8*)&settingsOffset, 0x2);
	settingsOffset *= 8;
	
	// Reload DS Firmware settings
	boot_readFirmware(settingsOffset + 0x070, &settings1, 0x1);
	boot_readFirmware(settingsOffset + 0x170, &settings2, 0x1);
	
	if ((settings1 & 0x7F) == ((settings2+1) & 0x7F)) {
		boot_readFirmware(settingsOffset + 0x000, (u8*)0x02FFFC80, 0x70);
	} else {
		boot_readFirmware(settingsOffset + 0x100, (u8*)0x02FFFC80, 0x70);
	}
}


void loadBinary_ARM7 (u32 fileCluster)
{
	u32 ndsHeader[0x170>>2];

	// read NDS header
	fileRead ((char*)ndsHeader, fileCluster, 0, 0x170);
	// read ARM9 info from NDS header
	u32 ARM9_SRC = ndsHeader[0x020>>2];
	char* ARM9_DST = (char*)ndsHeader[0x028>>2];
	u32 ARM9_LEN = ndsHeader[0x02C>>2];
	// read ARM7 info from NDS header
	u32 ARM7_SRC = ndsHeader[0x030>>2];
	char* ARM7_DST = (char*)ndsHeader[0x038>>2];
	u32 ARM7_LEN = ndsHeader[0x03C>>2];
	
	// Load binaries into memory
	fileRead(ARM9_DST, fileCluster, ARM9_SRC, ARM9_LEN);
	fileRead(ARM7_DST, fileCluster, ARM7_SRC, ARM7_LEN);

	// first copy the header to its proper location, excluding
	// the ARM9 start address, so as not to start it
	TEMP_ARM9_START_ADDRESS = ndsHeader[0x024>>2];		// Store for later
	ndsHeader[0x024>>2] = 0;
	dmaCopyWords(3, (void*)ndsHeader, (void*)NDS_HEAD, 0x170);
}

/*-------------------------------------------------------------------------
startBinary_ARM7
Jumps to the ARM7 NDS binary in sync with the display and ARM9
Written by Darkain.
Modified by Chishm:
 * Removed MultiNDS specific stuff
--------------------------------------------------------------------------*/
#define startBinary_ARM7_size 0xB0
void startBinary_ARM7 (void)
{
	while(REG_VCOUNT!=191);
	while(REG_VCOUNT==191);
	
	// copy NDS ARM9 start address into the header, starting ARM9
	ARM9_START_FLAG = 1;

	while(REG_VCOUNT!=191);
	while(REG_VCOUNT==191);
	// Start ARM7
	resetCpu();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Main function

int main (void) {
	u32 fileCluster = storedFileCluster;
	// Init card
	if(!FAT_InitFiles(initDisc))
	{
		return -1;
	}
	if ((fileCluster < CLUSTER_FIRST) || (fileCluster >= CLUSTER_EOF)) 	/* Invalid file cluster specified */
	{
		fileCluster = getBootFileCluster(bootName);
	}
	if (fileCluster == CLUSTER_FREE)
	{
		return -1;
	}

	//writePowerManagement(0, readPowerManagement(0) | PM_BACKLIGHT_BOTTOM | PM_BACKLIGHT_TOP);

	// ARM9 clears its memory part 2
	// copy ARM9 function to RAM, and make the ARM9 jump to it
	copyLoop((void*)TEMP_MEM, (void*)resetMemory2_ARM9, resetMemory2_ARM9_size);
	(*(vu32*)0x027FFE24) = (u32)TEMP_MEM;	// Make ARM9 jump to the function
	// Wait until the ARM9 has completed its task
	while ((*(vu32*)0x027FFE24) == (u32)TEMP_MEM);

	// Get ARM7 to clear RAM
	resetMemory_ARM7();	
	
	// ARM9 enters a wait loop
	// copy ARM9 function to RAM, and make the ARM9 jump to it
	copyLoop((void*)TEMP_MEM, (void*)startBinary_ARM9, startBinary_ARM9_size);
	(*(vu32*)0x027FFE24) = (u32)TEMP_MEM;	// Make ARM9 jump to the function

	writePowerManagement(0, readPowerManagement(0) | PM_LED_BLINK);
	//REG_POWERCNT|=PM_LED_BLINK;
	// Load the NDS file
	loadBinary_ARM7(fileCluster);
	writePowerManagement(0, readPowerManagement(0) &0xffffffcf);
	//REG_POWERCNT&=0xffffffcf;

	// Patch with DLDI if desired
	if (wantToPatchDLDI) {
		dldi((u8*)((u32*)NDS_HEAD)[0x0A], ((u32*)NDS_HEAD)[0x0B]);
	}
	
	startBinary_ARM7();

	return 0;
}

