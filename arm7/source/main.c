#include <nds.h>
#include <nds/bios.h>

#include "../ipcz.h"

static inline void Read_Flash(int address, void *destination, int length){
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

static inline void load_PersonalData(){
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

static void VblankHandler(){
	IPCZ->keyxy=(~REG_KEYXY) & 0xff;
}

int main(int argc, char **argv){
	irqInit();
	fifoInit();
	installSystemFIFO();

	irqSet(IRQ_VBLANK, VblankHandler);
	irqEnable(IRQ_VBLANK);

	(*(u32*)0x06000000)=0;
	load_PersonalData();
	writePowerManagement(0, readPowerManagement(0) | PM_BACKLIGHT_BOTTOM | PM_BACKLIGHT_TOP);
	IPCZ->cmd=0;

	while(1){
		swiWaitForVBlank();
		if(IPCZ->cmd){
			if(IPCZ->cmd==ResetRudolph){
				IPCZ->cmd=0;
				//writePowerManagement(0, readPowerManagement(0) | PM_LED_BLINK);
				ret_menu7_Gen();
				REG_IME = IME_DISABLE;
				*((vu32*)0x027FFE34) = *((vu32*)0x027FFFF8);
				swiSoftReset();
				while(1);
			}else if(IPCZ->cmd==ResetBootlib){
				IPCZ->cmd=0;
				//writePowerManagement(0, readPowerManagement(0) | PM_LED_BLINK);
				while(*((vu32*)0x027FFE24) != (u32)0x027FFE04)swiWaitForVBlank();
				irqDisable(IRQ_ALL);
				*((vu32*)0x027FFE34) = (u32)0x06000000; //BG_BMP_RAM(0)
				swiSoftReset();
				while(1);
			}else if(IPCZ->cmd==ResetMoonlight){
				u32 w,i;
				IPCZ->cmd=0;
				//writePowerManagement(0, readPowerManagement(0) | PM_LED_BLINK);
				REG_IME = IME_DISABLE;	// Disable interrupts
				REG_IF = REG_IF;	// Acknowledge interrupt
		
				while(!IPCZ->bootaddress)
					for(w=0;w<0x100;w++);
				REG_IME = 0;

				for(i=0x04000400; i<0x04000500; i+=4)
					*((u32*)i)=0;
				REG_SOUNDCNT = 0;

				//clear out ARM7 DMA channels and timers
				for(i=0x040000B0;i<(0x040000B0+0x30);i+=4)
					*((vu32*)i)=0;
				for(i=0x04000100;i<0x04000110;i+=2)
					*((u16*)i)=0;
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
				// already done in load_PersonalData()

				*((vu32*)0x027FFE34) = IPCZ->bootaddress;	// Bootloader start address	
				swiSoftReset();
				while(1);
/*
			}else if(IPCZ->cmd==QueryKeyXY){
				IPCZ->cmd=0;
				IPCZ->keyxy=(~REG_KEYXY) & 0xff;
*/
			}else if(IPCZ->cmd==Shutdown){
				IPCZ->cmd=0;
				writePowerManagement(0, PM_SYSTEM_PWR);
			}else if(IPCZ->cmd==ReturnBootstub){
				IPCZ->cmd=0;
				exit(0);
/*
			}else if(IPCZ->cmd==CPTest){
				memcpy(IPCZBuf,0x0601fe00,0x200);
				IPCZ->cmd=0;
*/
			}
		}
	}
}
