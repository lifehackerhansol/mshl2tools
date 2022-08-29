#include <nds.h>
#include <string.h>
//#include <stdlib.h>
#include "../ipcz.h"

#define myPM_LED_ON    (0<<4)
#define myPM_LED_SLEEP (1<<4)
#define myPM_LED_BLINK (3<<4)

#define __IRQ__ (IRQ_VBLANK | IRQ_VCOUNT | IRQ_NETWORK)

extern void ARM7_Bios(u8 *addr,u32 size);
u8 *bootstub;
typedef void (*type_void)();
type_void bootstub_arm7;

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

//Write Flash from DSOrganize (not copied, as usual...)
static void Write_Flash(int address, void *destination){ //length is always 256
	int i=0;
	u8 *dst = (u8*)destination;
	SerialWaitBusy();

	//Enable write
	REG_SPICNT = SPI_ENABLE|SPI_CONTINUOUS|SPI_DEVICE_NVRAM;
	REG_SPIDATA = 0x6;
	SerialWaitBusy();
	REG_SPICNT = 0;

	//Write start
	REG_SPICNT = SPI_ENABLE|SPI_CONTINUOUS|SPI_DEVICE_NVRAM;
	REG_SPIDATA = 0x5;
	SerialWaitBusy();
	do{
		REG_SPIDATA = 0x0;
		SerialWaitBusy();
	}while(!(REG_SPIDATA&0x02));
	REG_SPICNT = 0;

	REG_SPICNT = SPI_ENABLE|SPI_CONTINUOUS|SPI_DEVICE_NVRAM;
	REG_SPIDATA = 0xa;
	SerialWaitBusy();

	REG_SPIDATA = (address >> 16) & 0xFF;
	SerialWaitBusy();

	REG_SPIDATA = (address >> 8) & 0xFF;
	SerialWaitBusy();

	REG_SPIDATA = (address) & 0xFF;
	SerialWaitBusy();

	for(; i < 256; i++) {
		REG_SPIDATA = dst[i];
		SerialWaitBusy();
	}
	REG_SPICNT = 0;

	//Write end
	REG_SPICNT = SPI_ENABLE|SPI_CONTINUOUS|SPI_DEVICE_NVRAM;
	REG_SPIDATA = 0x5;
	SerialWaitBusy();
	do{
		REG_SPIDATA = 0x0;
		SerialWaitBusy();
	}while(REG_SPIDATA&0x01);
	REG_SPICNT = 0;
}

static inline void WriteToFirmware(){
	Write_Flash((fwsize-0x600)+(IPCZ->firmware_write_index<<8), IPCZ->firmware_write_addr);
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
	IPCZ->keysheld   = ((~REG_KEYINPUT)&0x3ff) | ((keyxy&0x3)<<10);
#if _LIBNDS_MINOR_ > 4
	if(touchPenDown())IPCZ->keysheld |= (0x40<<6);
#else
	IPCZ->keysheld   |= ((keyxy&0x40/*0xc0*/)<<6);
#endif
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

static void sys_exit(){
	if(!bootstub_arm7){
		if(IPCZ->NDSType>=NDSi)writePowerManagement(0x10, 1);
		else writePowerManagement(0, PM_SYSTEM_PWR);
	}
	bootstub_arm7(); //won't return
}

extern __attribute__((noreturn)) void _menu7_Gen_s();
extern __attribute__((noreturn)) void reboot();

#if defined(_LIBNDS_MINOR_)
#if _LIBNDS_MINOR_ > 4
extern bool __dsimode;
#else
static void i2cIRQHandler(){
	int cause = (i2cReadRegister(I2C_PM, 0x10) & 0x3) | (i2cReadRegister(I2C_UNK4, 0x02)<<2);
	if(cause&1)sys_exit();
}
#endif
#endif

//TransferRegionZ volatile *IPCZ;
//#ifndef _LIBNDS_MAJOR_
//TransferRegion volatile *pIPC;
//#endif
int main(){ //int argc, char **argv){
	//IPCZ=/* (*(vu32*)0x04004000)?IPCZ_DSiMode: */
	//	IPCZ_DSMode;
	IPCZ->cmd=0xffffffff;

	{int i=0;for(i=0;i<4;i++){
		DMA_CR(i) = 0;
		DMA_SRC(i) = 0;
		DMA_DEST(i) = 0;
		TIMER_CR(i) = 0;
		TIMER_DATA(i) = 0;
	}}

	rtcReset();
	irqInit();
	initClockIRQ();

	SetYtrigger(80);
#ifdef _LIBNDS_MAJOR_
	fifoInit();
	installSystemFIFO();
#endif

	memset(((char*)IPCZ)+4,0,sizeof(TransferRegionZ)-4);
	//(*(u32*)0x06000000)=0;

	Read_Flash(0x20, &fwsize, 2);
	fwsize <<=3; //*= 8;
	fwsize += 512;
	IPCZ->fwsize=fwsize;
	load_PersonalData();
	Read_Flash(0x36, (u8*)IPCZ->MAC, 6);

	writePowerManagement(0, readPowerManagement(0) | PM_BACKLIGHT_BOTTOM | PM_BACKLIGHT_TOP | PM_SOUND_AMP);
	writePowerManagement(0, readPowerManagement(0)&~(myPM_LED_BLINK));
#ifdef _LIBNDS_MAJOR_
	REG_SOUNDCNT = SOUND_ENABLE | SOUND_VOL(0x7F);
#else
	SOUND_CR = SOUND_ENABLE | SOUND_VOL(0x7F);
#endif
	swiChangeSoundBias(1,0x400);

	u32 dstt_sdhc=*(vu32*)0x2fffc24;

	//because some libnds doesn't have "struct _user_data" thing. Universal source among devkitARM r23/r31.
	unsigned char userdata=((u8*)(&PersonalData->calY2px))[1];

	{
		u32 myself = readPowerManagement(4); //(PM_BACKLIGHT_LEVEL);
		if(myself & (1<<6))
			IPCZ->NDSType=(myself==readPowerManagement(5))?NDSLite:NDSi;
/*
		u32 td1=touchRead(TSC_MEASURE_TEMP1);
		u32 td2=touchRead(TSC_MEASURE_TEMP2);
		if((td1==0x0fff)&&(td2==0x0fff))IPCZ->NDSType=NDSi;
*/
	}

	{
		u16 flashme;
		Read_Flash(0x6, (u16*)&IPCZ->fwchksum, 2); //un-volatilize
		Read_Flash(0x17c, &flashme, 2);
		if(flashme!=0xffff){
			IPCZ->flashmeversion=(u8)flashme;
			if(flashme>=2)Read_Flash(0x3f7fc, (u8*)&IPCZ->flashmeversion, 1);
		}
	}

	irqSet(IRQ_VBLANK, VblankHandler); //set irq just before mainloop...
#ifdef _LIBNDS_MINOR_
#if _LIBNDS_MINOR_ > 4
	setPowerButtonCB(sys_exit);
#else
	irqSetAUX(IRQ_I2C, i2cIRQHandler);
	irqEnableAUX(IRQ_I2C);
#endif
#endif
	irqEnable(__IRQ__);

	bootstub=(u8*)0x02ff4000;
	bootstub_arm7=(*(u64*)bootstub==0x62757473746F6F62ULL)?(*(type_void*)(bootstub+0x0c)):NULL;
	IPCZ->cmd=0;
	while(1){
		swiWaitForVBlank();
#ifdef _LIBNDS_MAJOR_
		if(0 == (REG_KEYINPUT & (KEY_SELECT | KEY_START | KEY_L | KEY_R))){
#else
		if(0 == ((*(vuint16*)0x04000130) & 0x30c)){
#endif
			sys_exit();
		}

		if(lidclose){
			writePowerManagement(0,readPowerManagement(0)&~(PM_BACKLIGHT_BOTTOM | PM_BACKLIGHT_TOP));
			writePowerManagement(0, readPowerManagement(0) | myPM_LED_SLEEP);
		}
		if(lidopen){
			writePowerManagement(0,readPowerManagement(0) | PM_BACKLIGHT_BOTTOM | PM_BACKLIGHT_TOP);
			writePowerManagement(0, readPowerManagement(0)&~(myPM_LED_SLEEP));
		}
		if(IPCZ->cmd){
			if(IPCZ->cmd==ResetRudolph){
				#define ARM7_PROG (0x03810000 - 0xA00)
				void (*_menu7_Gen)();
#ifdef _LIBNDS_MAJOR_
				REG_SOUNDCNT = 0;
#else
				SOUND_CR = 0;
#endif
				u32	*adr;
				u32	*buf;
				u32	i;

				IPCZ->cmd=0;
				*(vu32*)0x2fffc24=dstt_sdhc;
				//writePowerManagement(0, readPowerManagement(0) | myPM_LED_BLINK);
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

				while((*(vu32*)0x02fFFDFC) != 0x02fFFDF8 && (*(vu32*)0x02fFFDFC) != 0x0cfFFDF8);	// Timing adjustment with ARM9
				_menu7_Gen = (void(*)())ARM7_PROG;
				_menu7_Gen();
				while(1);
			}else if(IPCZ->cmd==ResetBootlib){
				IPCZ->cmd=0;
				*(vu32*)0x2fffc24=dstt_sdhc;
				//writePowerManagement(0, readPowerManagement(0) | myPM_LED_BLINK);
				while(*((vu32*)0x02fFFE24) != (u32)0x02fFFE04 && *((vu32*)0x02fFFE24) != (u32)0x0cfFFE04)swiWaitForVBlank(); 
				irqDisable(IRQ_ALL);
				*((vu32*)0x02fFFE34) = (u32)0x06000000; //BG_BMP_RAM(0)
				swiSoftReset();
				while(1);
			}else if(IPCZ->cmd==ResetMoonlight){
				u32 i;
				IPCZ->cmd=0;
				*(vu32*)0x2fffc24=dstt_sdhc;
				//writePowerManagement(0, readPowerManagement(0) | myPM_LED_BLINK);
				REG_IME = IME_DISABLE;	// Disable interrupts
				REG_IF = REG_IF;	// Acknowledge interrupt

				//while(!IPCZ->bootaddress)swiDelay(4);
					//for(w=0;w<0x100;w++);
				//REG_IME = 0;

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
#if 0
				{ //switch to user mode
					//u32 r0=0;
					//__asm {
						asm("mov r0, #0x1F");
						asm("msr cpsr, r0");
					//}
				}
#endif
				SwitchUserMode();

				REG_IE = 0;
				REG_IF = ~0;
				(*(vu32*)(0x04000000-4)) = 0;  //IRQ_HANDLER ARM7 version
				(*(vu32*)(0x04000000-8)) = ~0; //VBLANK_INTR_WAIT_FLAGS, ARM7 version
				REG_POWERCNT = 1;  //turn off power to stuffs

				// Reload DS Firmware settings
				// already done in load_PersonalData()

				while(*((vu32*)0x02fFFE24) != (u32)0x02fFFE04 && *((vu32*)0x02fFFE24) != (u32)0x0cfFFE04);
				*((vu32*)0x02fFFE34) = IPCZ->bootaddress;	// Bootloader start address
				swiSoftReset();
				while(1);
			}else if(IPCZ->cmd==ResetRudolphM){
				#define ARM7_PROG (0x03810000 - 0xA00)
				void (*_menu7_Gen)();
				u32	*adr;
				u32	*buf;
				u32	i;

				IPCZ->cmd=0;
				*(vu32*)0x2fffc24=dstt_sdhc;
				//writePowerManagement(0, readPowerManagement(0) | myPM_LED_BLINK);
				REG_IME = 0;
				REG_IE = 0;
				REG_IF = REG_IF;

				REG_IPC_SYNC = 0;
				DMA0_CR = 0;
				DMA1_CR = 0;
				DMA2_CR = 0;
				DMA3_CR = 0;

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
#if 0
				{ //switch to user mode
					//u32 r0=0;
					//__asm {
						asm("mov r0, #0x1F");
						asm("msr cpsr, r0");
					//}
				}
#endif
				SwitchUserMode();

				REG_IE = 0;
				REG_IF = ~0;
				(*(vu32*)(0x04000000-4)) = 0;  //IRQ_HANDLER ARM7 version
				(*(vu32*)(0x04000000-8)) = ~0; //VBLANK_INTR_WAIT_FLAGS, ARM7 version
				REG_POWERCNT = 1;  //turn off power to stuffs

				//copy final loader to private RAM
				adr = (u32*)ARM7_PROG;
				buf = (u32*)_menu7_Gen_s;
				for(i = 0; i < 0x200/4; i++) {
					*adr = *buf;
					adr++;
					buf++;
				}

				while((*(vu32*)0x02fFFDFC) != 0x02fFFDF8 && (*(vu32*)0x02fFFDFC) != 0x0cfFFDF8);	// Timing adjustment with ARM9
				_menu7_Gen = (void(*)())ARM7_PROG;
				_menu7_Gen();
				while(1);
			}else if(IPCZ->cmd==ResetMoonShell2){
				#define ARM7_PROG (0x03810000 - 0xA00)
				void (*_menu7_Gen)();
				u32	*adr;
				u32	*buf;
				u32	i;

				IPCZ->cmd=0;
				*(vu32*)0x2fffc24=dstt_sdhc;
				//writePowerManagement(0, readPowerManagement(0) | myPM_LED_BLINK);

				REG_IME = 0;
				REG_IE = 0;
				REG_IF = REG_IF;

				REG_IPC_SYNC = 0;
				DMA0_CR = 0;
				DMA1_CR = 0;
				DMA2_CR = 0;
				DMA3_CR = 0;

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

				//copy final loader to private RAM
				adr = (u32*)ARM7_PROG;
				buf = (u32*)reboot;
				for(i = 0; i < 0x200/4; i++) {
					*adr = *buf;
					adr++;
					buf++;
				}

				//while((*(vu32*)0x02fFFDFC) != 0x02fFFDF8 && (*(vu32*)0x02fFFDFC) != 0x0cfFFDF8);	// Timing adjustment with ARM9
				_menu7_Gen = (void(*)())ARM7_PROG;
				_menu7_Gen();
				while(1);
			}else if(IPCZ->cmd==Shutdown){
				IPCZ->cmd=0;
				writePowerManagement(0, PM_SYSTEM_PWR);
			}else if(IPCZ->cmd==ReturnBootstub){
				IPCZ->cmd=0;
				sys_exit();
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
			}else if(IPCZ->cmd==WriteFirmware){
				if(IPCZ->firmware_write_index==0||IPCZ->firmware_write_index==1||IPCZ->firmware_write_index==2)
					*(vu16*)(IPCZ->firmware_write_addr+0xfe)=swiCRC16(0,IPCZ->firmware_write_addr,0xfe);
				else if(IPCZ->firmware_write_index==4||IPCZ->firmware_write_index==5)
					*(vu16*)(IPCZ->firmware_write_addr+0x72)=swiCRC16(0xffff,IPCZ->firmware_write_addr,0x70);
				else goto WriteFirmware_abort;

				irqDisable(__IRQ__);
				WriteToFirmware();
				irqEnable(__IRQ__);
WriteFirmware_abort:
				IPCZ->cmd=0;
			}else if(IPCZ->cmd==RequestBatteryLevel){
#if defined(_LIBNDS_MINOR_) && _LIBNDS_MINOR_ > 4
				if(__dsimode){
					IPCZ->battery = i2cReadRegister(I2C_PM,I2CREGPM_BATTERY);
				}else
#endif
				{
					IPCZ->battery = (u16)(readPowerManagement(PM_BATTERY_REG) & 1)?3:15;
					//IPCZ->battery2 = (u16)(readPowerManagement(PM_BATTERY_REG) & 1);
					u32 dslite = readPowerManagement(4); //(PM_BACKLIGHT_LEVEL);
					if(dslite & (1<<6))IPCZ->battery |= (dslite & (1<<3))<<4; // libnds 1.4.7 fixed with my report :)
				}
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
