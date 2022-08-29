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

static void VblankHandler(void) {
}

//---------------------------------------------------------------------------------
int main(int argc, char ** argv) {
//---------------------------------------------------------------------------------

  irqInit();
	//IPCEX->RESET = RESET_NULL;	//====== IPC RESET Clear by Rudolph (2007/06/13)
  irqSet(IRQ_VBLANK, VblankHandler);

  irqEnable(IRQ_VBLANK);

  (*(u32*)0x06000000)=0;
	load_PersonalData();		//====== by Rudolph

	writePowerManagement(0, readPowerManagement(0) | PM_BACKLIGHT_BOTTOM | PM_BACKLIGHT_TOP); //====== by Rudolph (2007/11/03)

  
  while (1){
    swiIntrWait(1,IRQ_VBLANK); // vblank
  }
}


