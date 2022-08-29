#include <nds.h>
#include <string.h>
#include <stdlib.h>
#include "../ipcz.h"

#define __IRQ__ (IRQ_VBLANK | IRQ_VCOUNT | IRQ_NETWORK)

extern void ARM7_Bios(u8 *addr,u32 size);

static void Read_Flash(int address, void *destination, int length){
	int i=0;
	u8 *dst = (u8*)destination;
	SerialWaitBusy();

	REG_SPICNT = SPI_ENABLE | SPI_BAUD_4MHz | SPI_DEVICE_NVRAM | SPI_CONTINUOUS;
	REG_SPIDATA = 0x3;
	SerialWaitBusy();

	REG_SPIDATA = (address >> 16) & 0xFF;
	SerialWaitBusy();

	REG_SPIDATA = (address >> 8) & 0xFF;
	SerialWaitBusy();

	REG_SPIDATA = (address) & 0xFF;
	SerialWaitBusy();

	for(; i < length; i++) {
		REG_SPIDATA= 0;
		SerialWaitBusy();
		dst[i] = REG_SPIDATA;
	}
	REG_SPICNT = 0;
}

static u32 fwsize;

static inline void DumpFirmware(){
	u32 size=fwsize,dumpsize;

	//Read_Flash(0x20, &size, 2);
	//size *= 8;
	//size += 512;
	dumpsize=size<IPCZ->firmware_bufsize?size:IPCZ->firmware_bufsize;

	Read_Flash(0, IPCZ->firmware_addr, dumpsize);

	//IPCZ->firmware_bufsize=size;
}

static inline  void load_PersonalData(){
	u32 src = fwsize-512, count0 = 0, count1 = 0;

	//Read_Flash(0x20, &src, 2);
	//src *= 8;

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

static inline  void PCMstartSound(u8 ch, int freq, void* top, u32 size, u8 vol, u8 pan, u8 format){
	SCHANNEL_TIMER(ch)  		= SOUND_FREQ(freq);
	SCHANNEL_SOURCE(ch)		= (u32)top;
	SCHANNEL_LENGTH(ch)		= size >> 2 ;
	SCHANNEL_REPEAT_POINT(ch)	= 0;
	SCHANNEL_CR(ch)   		= SCHANNEL_ENABLE | SOUND_REPEAT |
						  SOUND_VOL(vol)  | SOUND_PAN(pan) |
						  ((format==8) ? SOUND_FORMAT_8BIT : SOUND_FORMAT_16BIT);
}

#define PCMstopSound(ch) (SCHANNEL_CR(ch) &= ~SCHANNEL_ENABLE)

static u16 keysold=0;
static touchPosition touch;
static u8 keyxy=0;
static u8 lid=0,lidold=0,lidopen=0,lidclose=0;
static int temperature1,temperature2;
static void VblankHandler(){
	IPCZ->blanks++; //pseudo timer
	keyxy=(~REG_KEYXY) & 0xff;
	IPCZ->keysheld   = ((~REG_KEYINPUT)&0x3ff) | ((keyxy&0x3)<<10) | ((keyxy&0x40/*0xc0*/)<<6);
	IPCZ->keysdown   = ~keysold&IPCZ->keysheld;
	IPCZ->keysup     = keysold&~IPCZ->keysheld;
	IPCZ->keysrepeat = keysold&IPCZ->keysheld;
	keysold          = IPCZ->keysheld;

	lid              = keyxy&0x80;  //true if opened (different from button/touch)
	lidopen          = ~lidold&lid;
	lidclose         = lidold&~lid;
	lidold           = lid;

#ifdef _LIBNDS_MAJOR_
	touchReadXY(&touch);
#else
	touch=touchReadXY();
#endif
	IPCZ->touchX = touch.px;
	IPCZ->touchY = touch.py;

	IPCZ->temperature = touchReadTemperature(&temperature1, &temperature2);
}

static void _menu7_Gen_s(){
	u32	*adr;
	u32	*bufh, *buf7, *buf9;
	u32	siz;
	u32	i;
	u32	*arm9s, *arm9e;
	u32	*arm7s, *arm7e;

//relocation start
	bufh = (u32*)(*(vu32*)0x027FFDF4); //allocated in ret_menu9_Gen()

	adr = (u32*)0x027FFE00;
	for(i = 0; i < 512/4; i++) {		// Header
		*adr = *bufh;
		adr++;
		bufh++;
	}

	buf9 = bufh;
	buf7 = buf9 + ((*(vu32*)0x027FFE2C) / 4);

	adr = (u32*)(*(vu32*)0x027FFE38);
	siz = (*(vu32*)0x027FFE3C);
	for(i = 0; i < siz/4; i++) {		// ARM7
		*adr = *buf7;
		adr++;
		buf7++;
	}
	arm7e = adr;

	adr = (u32*)(*(vu32*)0x027FFE28);
	siz = (*(vu32*)0x027FFE2C);
	if(adr < buf9) {			// ARM9
		for(i = 0; i < siz/4; i++) {
			*adr = *buf9;
			adr++;
			buf9++;
		}
		arm9e = adr;
	} else { //if ldrBuf is before adr, we have to copy from back side.
		adr += (siz/4 - 1);
		buf9 += (siz/4 - 1);
		arm9e = adr + 1;
		for(i = 0; i < siz/4; i++) {
			*adr = *buf9;
			adr--;
			buf9--;
		}
	}
//relocation end

//clear main memory
	adr = (u32*)0x02000000;
	arm9s = (u32*)(*(vu32*)0x027FFE28);
	while(adr < arm9s) {
		*adr = 0x00000000;
		adr++;
	}

	arm7s = (u32*)(*(vu32*)0x027FFE38);

	//We will stop at 0x023F4000 rather than 0x023FF800
	if(arm7s > (u32*)0x023F4000)
		arm7s = (u32*)0x023F4000;
	while(arm9e < arm7s) {
		*arm9e = 0x00000000;
		arm9e++;
	}

	while(arm7e < (u32*)0x023F4000) {
		*arm7e = 0x00000000;
		arm7e++;
	}

	*(vu32*)0x027FFDFC = *(vu32*)0x027FFE24;

//jump to "copy address"
	asm("swi 0x00");			// JUMP 0x027FFE34
	while(1);
}

int main(){ //int argc, char **argv){
	rtcReset();
	irqInit();
	initClockIRQ();
#ifdef _LIBNDS_MAJOR_
	fifoInit();
	installSystemFIFO();
#endif

	memset((TransferRegionZ*)IPCZ,0,sizeof(TransferRegionZ));
	(*(u32*)0x06000000)=0;

	Read_Flash(0x20, &fwsize, 2);
	fwsize *= 8;
	fwsize += 512;
	IPCZ->fwsize=fwsize;
	load_PersonalData();
	Read_Flash(0x36, (u8*)IPCZ->MAC, 6);

	writePowerManagement(0, readPowerManagement(0) | PM_BACKLIGHT_BOTTOM | PM_BACKLIGHT_TOP | PM_SOUND_AMP);
#ifdef _LIBNDS_MAJOR_
	REG_SOUNDCNT = SOUND_ENABLE | SOUND_VOL(0x7F);
#else
	SOUND_CR = SOUND_ENABLE | SOUND_VOL(0x7F);
#endif
	swiChangeSoundBias(1,0x400);

	u32 dstt_sdhc=*(vu32*)0x27ffc24;

	//because some libnds doesn't have "struct _user_data" thing. Universal source among devkitARM r23/r31.
	unsigned char userdata=((u8*)(&PersonalData->calY2px))[1];

	{
		u32 myself = readPowerManagement(4); //(PM_BACKLIGHT_LEVEL);
		if(myself & (1<<6))
			IPCZ->NDSType=(myself==readPowerManagement(5))?NDSLite:NDSi;
	}

	{
		u16 flashme;
		Read_Flash(0x6, &IPCZ->fwchksum, 2);
		Read_Flash(0x17c, &flashme, 2);
		if(flashme!=0xffff){
			IPCZ->flashmeversion=(u8)flashme;
			if(flashme>=2)Read_Flash(0x3f7fc, (u8*)&IPCZ->flashmeversion, 1);
		}
	}

	irqSet(IRQ_VBLANK, VblankHandler); //set irq just before mainloop...
	irqEnable(__IRQ__);

	while(1){
		swiWaitForVBlank();
		if(lidclose){
			writePowerManagement(0,readPowerManagement(0)&~(PM_BACKLIGHT_BOTTOM | PM_BACKLIGHT_TOP));
			writePowerManagement(0, readPowerManagement(0) | PM_LED_SLEEP);
		}
		if(lidopen){
			writePowerManagement(0,readPowerManagement(0) | PM_BACKLIGHT_BOTTOM | PM_BACKLIGHT_TOP);
			writePowerManagement(0, readPowerManagement(0)&~(PM_LED_SLEEP));
		}
		if(IPCZ->cmd){
			if(IPCZ->cmd==ResetRudolph){
				#define ARM7_PROG (0x03810000 - 0xA00)
				void (*_menu7_Gen)();
				u32	*adr;
				u32	*buf;
				u32	i;

				IPCZ->cmd=0;
				*(vu32*)0x27ffc24=dstt_sdhc;
				//writePowerManagement(0, readPowerManagement(0) | PM_LED_BLINK);
				REG_IME = 0;
				REG_IE = 0;
				REG_IF = REG_IF;

				REG_IPC_SYNC = 0;
				DMA0_CR = 0;
				DMA1_CR = 0;
				DMA2_CR = 0;
				DMA3_CR = 0;

				//copy final loader to private RAM
				adr = (u32*)ARM7_PROG;
				buf = (u32*)_menu7_Gen_s;
				for(i = 0; i < 0x200/4; i++) {
					*adr = *buf;
					adr++;
					buf++;
				}

				while((*(vu32*)0x027FFDFC) != 0x027FFDF8);	// Timing adjustment with ARM9
				_menu7_Gen = (void(*)())ARM7_PROG;
				_menu7_Gen();
				while(1);
			}else if(IPCZ->cmd==ResetBootlib){
				IPCZ->cmd=0;
				*(vu32*)0x27ffc24=dstt_sdhc;
				//writePowerManagement(0, readPowerManagement(0) | PM_LED_BLINK);
				while(*((vu32*)0x027FFE24) != (u32)0x027FFE04)swiWaitForVBlank(); 
				irqDisable(IRQ_ALL);
				*((vu32*)0x027FFE34) = (u32)0x06000000; //BG_BMP_RAM(0)
				swiSoftReset();
				while(1);
			}else if(IPCZ->cmd==ResetMoonlight){
				u32 i;
				IPCZ->cmd=0;
				*(vu32*)0x27ffc24=dstt_sdhc;
				//writePowerManagement(0, readPowerManagement(0) | PM_LED_BLINK);
				REG_IME = IME_DISABLE;	// Disable interrupts
				REG_IF = REG_IF;	// Acknowledge interrupt
		
				while(!IPCZ->bootaddress)swiDelay(4);
					//for(w=0;w<0x100;w++);
				REG_IME = 0;

				for(i=0x04000400; i<0x04000500; i+=4)
					*((u32*)i)=0;
#ifdef _LIBNDS_MAJOR_
				REG_SOUNDCNT = 0;
#else
				SOUND_CR = 0;
#endif

				//clear out ARM7 DMA channels and timers
				for(i=0x040000B0;i<(0x040000B0+0x30);i+=4)
					*((vu32*)i)=0;
				for(i=0x04000100;i<0x04000110;i+=2)
					*((u16*)i)=0;
				{ //switch to user mode
					//u32 r0=0;
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
			}else if(IPCZ->cmd==ResetRudolphM){
				//#define ARM7_PROG (0x03810000 - 0xA00)
				void (*_menu7_Gen)();
				u32 i;
				IPCZ->cmd=0;
				*(vu32*)0x27ffc24=dstt_sdhc;
				//writePowerManagement(0, readPowerManagement(0) | PM_LED_BLINK);
				REG_IME = IME_DISABLE;	// Disable interrupts
				REG_IF = REG_IF;	// Acknowledge interrupt

				REG_IME = 0;

				for(i=0x04000400; i<0x04000500; i+=4)
					*((u32*)i)=0;
#ifdef _LIBNDS_MAJOR_
				REG_SOUNDCNT = 0;
#else
				SOUND_CR = 0;
#endif

				//clear out ARM7 DMA channels and timers
				for(i=0x040000B0;i<(0x040000B0+0x30);i+=4)
					*((vu32*)i)=0;
				for(i=0x04000100;i<0x04000110;i+=2)
					*((u16*)i)=0;
				{ //switch to user mode
					//u32 r0=0;
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

				while((*(vu32*)0x027FFDFC) != 0x027FFDF8);	// Timing adjustment with ARM9
				_menu7_Gen = (void(*)())ARM7_PROG;
				_menu7_Gen();
				while(1);
			}else if(IPCZ->cmd==Shutdown){
				IPCZ->cmd=0;
				writePowerManagement(0, PM_SYSTEM_PWR);
			}else if(IPCZ->cmd==ReturnBootstub){
				IPCZ->cmd=0;
				exit(0);
			//}else if(IPCZ->cmd==Slot2NDS){
			//	//Use bootMoonlight(0x080000c0)
			}else if(IPCZ->cmd==Slot2GBA){
				IPCZ->cmd=0;
				writePowerManagement(0, PM_SOUND_AMP | ((userdata>>3)&1?PM_BACKLIGHT_BOTTOM:PM_BACKLIGHT_TOP));
				swiChangeSoundBias(0,0x400);
				swiSwitchToGBAMode();
				while(1);
			//}else if(IPCZ->cmd==ReturnDSMenu){
			//	//GetFirmware then decompress it
			}else if(IPCZ->cmd==ReturnDSiMenu){
				IPCZ->cmd=0;
				// http://devkitpro.org/viewtopic.php?f=13&t=2140#p6013
				writePowerManagement(0x10, 1);
			}else if(IPCZ->cmd==GetARM7Bios){
				ARM7_Bios(IPCZ->arm7bios_addr,IPCZ->arm7bios_bufsize-1);
				IPCZ->cmd=0;
			}else if(IPCZ->cmd==GetFirmware){
				irqDisable(__IRQ__);
				DumpFirmware();
				irqEnable(__IRQ__);
				IPCZ->cmd=0;
			}else if(IPCZ->cmd==RequestBatteryLevel){
				IPCZ->battery = (u16)(readPowerManagement(PM_BATTERY_REG) & 1);
				//IPCZ->battery2 = (u16)(readPowerManagement(PM_BATTERY_REG) & 1);
				u32 dslite = readPowerManagement(4); //(PM_BACKLIGHT_LEVEL);
				if(dslite & (1<<6))IPCZ->battery += (dslite & (1<<3))<<12; // libnds 1.4.7 fixed with my report :)
				IPCZ->cmd=0;
			}else if(IPCZ->cmd==EnableDSTTSDHCFlag){
				dstt_sdhc=1;
				IPCZ->cmd=0;
			}else if(IPCZ->cmd==DisableDSTTSDHCFlag){
				dstt_sdhc=0;
				IPCZ->cmd=0;
			}else if(IPCZ->cmd==PlaySound){
				PCMstartSound(0, IPCZ->PCM_freq, IPCZ->PCM_L, IPCZ->PCM_size, 100,   0, IPCZ->PCM_bits);
				PCMstartSound(1, IPCZ->PCM_freq, IPCZ->PCM_R, IPCZ->PCM_size, 100, 127, IPCZ->PCM_bits);
				IPCZ->cmd=0;
			}else if(IPCZ->cmd==StopSound){
				PCMstopSound(0);
				PCMstopSound(1);
				IPCZ->cmd=0;
/*
			}else if(IPCZ->cmd==CPTest){
				memcpy(IPCZBuf,0x0601fe00,0x200);
				IPCZ->cmd=0;
*/
			}
		}
	}
}
