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
  irqSet(IRQ_VBLANK, VblankHandler);

  irqEnable(IRQ_VBLANK);

  (*(u32*)0x06000000)=0;
	load_PersonalData();		//====== by Rudolph

	writePowerManagement(0, readPowerManagement(0) | PM_BACKLIGHT_BOTTOM | PM_BACKLIGHT_TOP); //====== by Rudolph (2007/11/03)

  
  while (1){
    swiIntrWait(1,IRQ_VBLANK); // vblank

    if(IPCEX->RESET!=RESET_NULL){
      main_Proc_Reset(IPCEX->RESET);
      while(1);
    }
    
  }
}


