/*---------------------------------------------------------------------------------
	$Id: template.c,v 1.2 2005/09/07 20:06:06 wntrmute Exp $

	Simple ARM7 stub (sends RTC, TSC, and X/Y data to the ARM 9)

	$Log: template.c,v $
	Revision 1.2  2005/09/07 20:06:06  wntrmute
	updated for latest libnds changes
	
	Revision 1.8  2005/08/03 05:13:16  wntrmute
	corrected sound code


---------------------------------------------------------------------------------*/
#include <nds.h>
#include <nds/registers_alt.h>

#include <nds/bios.h>

#include "../../ipcex.h"
/*
#include "linkreset_arm7.h"

#include "main_boot_gbamp.h"
#include "main_boot_gbarom.h"

extern	void ret_menu7_R4(void);	//====== R4TF was added. by Rudolph (2007/05/23)
extern	void ret_menu7_EZ5(void);	//====== EZ5S was added. by Rudolph (2007/05/25)
extern	void ret_menu7_Gen(void);	//====== General purpose was added. by Rudolph (2007/10/22)
*/
#if 0
static inline void Read_Flash(int address, void *destination, int length) {

	int i;
	u8 *dst;

	dst = (u8*)destination;

	while(REG_SPICNT & 0x80);

	REG_SPICNT = 0x8900;
	REG_SPIDATA = 3;
	while(REG_SPICNT & 0x80);

	REG_SPIDATA = (address >> 16) & 0xFF;
	while(REG_SPICNT & 0x80);

	REG_SPIDATA = (address >> 8) & 0xFF;
	while(REG_SPICNT & 0x80);

	REG_SPIDATA = (address) & 0xFF;
	while(REG_SPICNT & 0x80);

	for(i = 0; i < length; i++) {
		REG_SPIDATA= 0;
		while(REG_SPICNT & 0x80);
		dst[i] = REG_SPIDATA;
	}
	REG_SPICNT = 0;
}

//====== by Rudolph ===================
static inline void load_PersonalData() {
	u32 src = 0, count0 = 0, count1 = 0;

	Read_Flash(0x20, &src, 2);
	src *= 8;

	Read_Flash(src + 0x70, &count0, 1);			//pick recent copy
	Read_Flash(src + 0x170, &count1, 1);
	count0 &= 0x7F;
	count1 &= 0x7F;
	if(count1 == ((count0 + 1) & 0x7F))
		src += 0x100;

	Read_Flash(src, PersonalData, 0x74);
	if(swiCRC16(0xFFFF, PersonalData, 0x70) != ((u16*)PersonalData)[0x72/2])
		Read_Flash(src ^ 0x100, PersonalData, 0x70);	//try the older copy
}
//=====================================

__attribute__((noinline)) static void main_Proc_Reset(ERESET RESET)
{
/*
  switch(RESET){
    case RESET_NULL: return; break;
    case RESET_VRAM: {
      REG_IME = IME_DISABLE;	// Disable interrupts
      REG_IF = REG_IF;	// Acknowledge interrupt
      *((vu32*)0x027FFE34) = (u32)0x06000000;	// Bootloader start address for VRAM
      swiSoftReset();	// Jump to boot loader
    } break;
    case RESET_GBAMP: boot_GBAMP(); break;
    case RESET_GBAROM: boot_GBAROM(); break;
    case RESET_MENU_DSLink: LinkReset_ARM7(); break;
    case RESET_MENU_MPCF: break;
    case RESET_MENU_M3CF: break;
    case RESET_MENU_M3SD: break;
    case RESET_MENU_SCCF: break;
    case RESET_MENU_SCSD: break;
    case RESET_MENU_EZSD: break;
//====== R4TF was added.
//    case RESET_MENU_R4TF: ret_menu7_R4(); break;
//====== EZ5S was added.
//    case RESET_MENU_EZ5S: ret_menu7_EZ5(); break;
//====== by Rudolph (2007/05/25)

//======General purpose was added.
    case RESET_MENU_GEN: ret_menu7_Gen(); break;
//====== by Rudolph (2007/10/22)
  }
*/

  ret_menu7_Gen();
  *(vu16*)(0x04000208) = 0;       //REG_IME = IME_DISABLE;
  *((vu32*)0x027FFE34) = *((vu32*)0x027FFFF8);
  asm("swi 0x00");                //swiSoftReset();
  asm("bx lr");
}


static void VblankHandler(void) {
}

//---------------------------------------------------------------------------------
int main(int argc, char ** argv) {
//---------------------------------------------------------------------------------

  irqInit();
	IPCEX->RESET = RESET_NULL;	//====== IPC RESET Clear by Rudolph (2007/06/13)
	IPCEX->BootAddress=0;
  irqSet(IRQ_VBLANK, VblankHandler);

  irqEnable(IRQ_VBLANK);

  //(*(u32*)0x06000000)=0;
//	load_PersonalData();		//====== by Rudolph

//	writePowerManagement(0, readPowerManagement(0) | PM_BACKLIGHT_BOTTOM | PM_BACKLIGHT_TOP); //====== by Rudolph (2007/11/03)

  
  while (1){
    swiIntrWait(1,IRQ_VBLANK); // vblank

    if(IPCEX->RESET!=RESET_NULL){
      //main_Proc_Reset(IPCEX->RESET);
		u32 i;
		u8 settings1, settings2;
		while(!IPCEX->BootAddress);

		REG_IME = 0;
		for (i=0x04000400; i<0x04000500; i+=4) {
			*((u32*)i)=0;
		}
		SOUND_CR = 0;
		for(i=0x040000B0;i<(0x040000B0+0x30);i+=4){
			*((vu32*)i)=0;
		}
		for(i=0x04000100;i<0x04000110;i+=2){
			*((u16*)i)=0;
		}
		{ //switch to user mode
			u32 r0=0;
			//__asm {
				asm("mov r0, #0x1F");
				asm("msr cpsr, r0");
			//}
		}

		REG_IE = 0;
		REG_IF = ~0;
		(*(vu32*)(0x04000000-4)) = 0;  //IRQ_HANDLER ARM7 version
		(*(vu32*)(0x04000000-8)) = ~0; //VBLANK_INTR_WAIT_FLAGS, ARM7 version
		REG_POWERCNT = 1;  //turn off power to stuffs
	
		// Reload DS Firmware settings
		Read_Flash((u32)0x03FE70,&settings1,1);
		Read_Flash((u32)0x03FF70,&settings2,1);
	
		if (settings1 > settings2) {
			Read_Flash((u32)0x03FE00,(u8*)0x027FFC80,0x70);
		} else {
			Read_Flash((u32)0x03FF00,(u8*)0x027FFC80,0x70);
		}

		*((vu32*)0x027FFE34) = IPCEX->BootAddress;
		swiSoftReset();

      while(1);
    }
    
  }
}

#endif

#define FW_READ        0x03

__attribute__((noinline)) static void _readFirmware(uint32 address, uint32 size, uint8 * buffer) {
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

/*-------------------------------------------------------------------------
resetMemory_ARM7
Clears all of the NDS's RAM that is visible to the ARM7
Written by Darkain.
Modified by Chishm:
 * Added STMIA clear mem loop
--------------------------------------------------------------------------*/
__attribute__((noinline)) static void resetMemory_ARM7 (bool ClearEWRAM)
{
	u32 i;
	u8 settings1, settings2;
	
	REG_IME = 0;

/*
	for (i=0; i<16; i++) {
		SCHANNEL_CR(i) = 0;
		SCHANNEL_TIMER(i) = 0;
		SCHANNEL_SOURCE(i) = 0;
		SCHANNEL_LENGTH(i) = 0;
	}
*/
	for (i=0x04000400; i<0x04000500; i+=4) {
	  *((u32*)i)=0;
	}
	SOUND_CR = 0;

	//clear out ARM7 DMA channels and timers
/*
	for (i=0; i<4; i++) {
		DMA_CR(i) = 0;
		DMA_SRC(i) = 0;
		DMA_DEST(i) = 0;
		TIMER_CR(i) = 0;
		TIMER_DATA(i) = 0;
	}
*/
  for(i=0x040000B0;i<(0x040000B0+0x30);i+=4){
    *((vu32*)i)=0;
  }
  for(i=0x04000100;i<0x04000110;i+=2){
    *((u16*)i)=0;
  }

  { //switch to user mode
    u32 r0=0;
    //__asm {
      asm("mov r0, #0x1F");
      asm("msr cpsr, r0");
    //}
  }

#if 0
  __asm volatile (
	// clear exclusive IWRAM
	// 0380:0000 to 0380:FFFF, total 64KiB
	"mov r0, #0 				\n"	
	"mov r1, #0 				\n"
	"mov r2, #0 				\n"
	"mov r3, #0 				\n"
	"mov r4, #0 				\n"
	"mov r5, #0 				\n"
	"mov r6, #0 				\n"
	"mov r7, #0 				\n"
	"mov r8, #0x03800000		\n"	// Start address 
	"mov r9, #0x03800000		\n" // End address part 1
	"orr r9, r9, #0x10000		\n" // End address part 2
	"clear_EIWRAM_loop:			\n"
	"stmia r8!, {r0, r1, r2, r3, r4, r5, r6, r7} \n"
	"cmp r8, r9					\n"
	"blt clear_EIWRAM_loop		\n"
	:
	:
	: "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9"
	);
#endif

 
	REG_IE = 0;
	REG_IF = ~0;
	(*(vu32*)(0x04000000-4)) = 0;  //IRQ_HANDLER ARM7 version
	(*(vu32*)(0x04000000-8)) = ~0; //VBLANK_INTR_WAIT_FLAGS, ARM7 version
	POWER_CR = 1;  //turn off power to stuffs
	
	// Reload DS Firmware settings
	_readFirmware((u32)0x03FE70, 0x1, &settings1);
	_readFirmware((u32)0x03FF70, 0x1, &settings2);
	
	if (settings1 > settings2) {
		_readFirmware((u32)0x03FE00, 0x70, (u8*)0x027FFC80);
	} else {
		_readFirmware((u32)0x03FF00, 0x70, (u8*)0x027FFC80);
	}
}

static void InterruptHandler_VBlank(void)
{

}

int main(void)
{
  //_consolePrint("ARM7 Start\n");
  REG_IME = 0;
  irqInit();
  //irqSet_u32(IRQ_TIMER1,(u32)InterruptHandler_Timer_Null);
  irqSet(IRQ_VBLANK,(u32)InterruptHandler_VBlank);
  REG_IME = 1;
  
  // Keep the ARM7 out of main RAM
  while (1){
    swiWaitForVBlank();
    
    if(IPCEX->RESET!=RESET_NULL){
	REG_IME = IME_DISABLE;	// Disable interrupts
	REG_IF = REG_IF;	// Acknowledge interrupt
		
	while(IPCEX->BootAddress==0){
		vu32 w;
		for(w=0;w<0x100;w++);
	}
	resetMemory_ARM7(false);
	*((vu32*)0x027FFE34) = IPCEX->BootAddress;	// Bootloader start address	
	swiSoftReset();	// Jump to boot loader
	while(1);
    }
  }
  return 0;
}

