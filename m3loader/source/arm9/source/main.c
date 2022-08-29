#include <nds.h>
#include <fat.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "_console.h"
#include "maindef.h"
#include "_const.h"
#include "../../../../arm7/ipcz.h"
#include "minIni.h"
//#include "linkreset_arm9.h"

#include "dldi.h"

//#define POWER_CR       (*(vuint16*)0x04000304)	// add 2008.03.30 kzat3

#include <nds/registers_alt.h>
#define BG_256_COLOR BG_COLOR_256

#if 0		// change 2008.03.30 kzat3
void ret_menu9_R4(void);	//====== R4TF was added. by Rudolph (2007/05/23)
void ret_menu9_EZ5(void);	//====== EZ5S was added. by Rudolph (2007/05/25)
bool ret_menu_chk(const char *name);	//====== Added by Rudolph (2007/10/22)
bool ret_menu9_Gen(void);			//====== Added by Rudolph (2007/10/22)
#else
bool ret_menu9_Gen(char *);			//====== Added by Rudolph (2007/10/22)
#endif
void ret_menu9_GENs(void);			//====== Added by Rudolph (2007/10/22)

typedef u16 UnicodeChar;
#define ExtLinkBody_MaxLength (256)
#define ExtLinkBody_ID (0x30545845) // EXT0
typedef struct {
  u32 ID,dummy1,dummy2,dummy3; // dummy is ZERO.
  char DataFullPathFilenameAlias[ExtLinkBody_MaxLength];
  char DataPathAlias[ExtLinkBody_MaxLength];
  char DataFilenameAlias[ExtLinkBody_MaxLength];
  char NDSFullPathFilenameAlias[ExtLinkBody_MaxLength];
  char NDSPathAlias[ExtLinkBody_MaxLength];
  char NDSFilenameAlias[ExtLinkBody_MaxLength];
  UnicodeChar DataFullPathFilenameUnicode[ExtLinkBody_MaxLength];
  UnicodeChar DataPathUnicode[ExtLinkBody_MaxLength];
  UnicodeChar DataFilenameUnicode[ExtLinkBody_MaxLength];
  UnicodeChar NDSFullPathFilenameUnicode[ExtLinkBody_MaxLength];
  UnicodeChar NDSPathUnicode[ExtLinkBody_MaxLength];
  UnicodeChar NDSFilenameUnicode[ExtLinkBody_MaxLength];
} TExtLinkBody;

//---------------------------------------------------------------------------------
#include "setarm9_reg_waitcr.h"

u32 sleep(u32 t){
	TIMER2_CR = TIMER_ENABLE|TIMER_DIV_1024;
	TIMER3_CR = TIMER_ENABLE|TIMER_CASCADE;
	while(((TIMER3_DATA*(1<<16))+TIMER2_DATA)/32 < 1024*t);
	return 0;
}

int main(void) {
  //MSEINFO_Readed=MSE_GetMSEINFO(&MSEINFO);
  
  POWER_CR = POWER_ALL_2D;
//  POWER_CR &= ~POWER_SWAP_LCDS;
  POWER_CR |= POWER_SWAP_LCDS;
  
  SetARM9_REG_WaitCR();
  
  irqInit();
  fifoInit();
  //REG_IPC_FIFO_CR |= IPC_FIFO_SEND_CLEAR;
  //REG_IPC_FIFO_CR &= ~IPC_FIFO_SEND_CLEAR;
  REG_IME=0;
  
  {
    void InitVRAM(void);
    InitVRAM();
#if 0	// delete 2008.03.30 kzat3
    void ShowMSEINFO(void);
    ShowMSEINFO();
#endif
    void SoftReset(void);
    SoftReset();
  }
}

void InitVRAM(void)
{
  videoSetMode(MODE_5_2D);
  videoSetModeSub(MODE_2_2D | DISPLAY_BG2_ACTIVE);

#if 0	// change 2008.03.30 kzat3  
  vramSetMainBanks(VRAM_A_MAIN_BG_0x6000000, VRAM_B_MAIN_SPRITE, VRAM_C_MAIN_BG_0x6020000, VRAM_D_SUB_SPRITE);
#else
  vramSetMainBanks(VRAM_A_MAIN_BG_0x06000000, VRAM_B_MAIN_SPRITE, VRAM_C_MAIN_BG_0x06020000, VRAM_D_SUB_SPRITE);
#endif

  vramSetBankH(VRAM_H_SUB_BG);
  vramSetBankI(VRAM_I_LCD);
  
  {
    SUB_BG2_CR = BG_256_COLOR | BG_RS_64x64 | BG_MAP_BASE(8) | BG_TILE_BASE(0) | BG_PRIORITY_3; // Tile16kb Map2kb(64x32)
    
    BG_PALETTE_SUB[(0*16)+0] = RGB15(0,0,0); // unuse (transparent)
    BG_PALETTE_SUB[(0*16)+1] = RGB15(4,0,12) | BIT(15); // BG color
    BG_PALETTE_SUB[(0*16)+2] = RGB15(0,0,0) | BIT(15); // Shadow color
    BG_PALETTE_SUB[(0*16)+3] = RGB15(31,31,31) | BIT(15); // Text color
    
    u16 XDX=(u16)((8.0/6)*0x100);
    u16 YDY=(u16)((8.0/6)*0x100);
    
    SUB_BG2_XDX = XDX;
    SUB_BG2_XDY = 0;
    SUB_BG2_YDX = 0;
    SUB_BG2_YDY = YDY;
    
    SUB_BG2_CX=-1;
    SUB_BG2_CY=-1;
    
    //consoleInit() is a lot more flexible but this gets you up and running quick
    _consoleInitDefault((u16*)(SCREEN_BASE_BLOCK_SUB(8)), (u16*)(CHAR_BASE_BLOCK_SUB(0)));
    _consoleClear();
  }
  
  BG2_CR = BG_BMP16_256x256 | BG_BMP_BASE(0) | BG_PRIORITY_0;
  
  {
    BG2_XDX = 1 << 8;
    BG2_XDY = 0 << 8;
    BG2_YDX = 0 << 8;
    BG2_YDY = 1 << 8;
    BG2_CX = 0;
    BG2_CY = 0;
  }
}

int wait=0;
void timer(){wait=1;}

void die(){
	extern char *fake_heap_end;
	for(swiWaitForVBlank();;swiWaitForVBlank())
		if(!( ((~REG_KEYINPUT)&0x3ff) | ((IPCZ->keyxy&0x3)<<10) | ((IPCZ->keyxy&0x40/*0xc0*/)<<6) ))break;
	if(*(u64*)fake_heap_end==0x62757473746F6F62ULL){
		_consolePrintf("Press A to return to menu.\n");
		for(swiWaitForVBlank();;swiWaitForVBlank())
			if(KEY_A&( ((~REG_KEYINPUT)&0x3ff) | ((IPCZ->keyxy&0x3)<<10) | ((IPCZ->keyxy&0x40/*0xc0*/)<<6) ))
				exit(0);
	}else{
		_consolePrintf("Press A to shutdown.\n");
		for(swiWaitForVBlank();;swiWaitForVBlank())
			if(KEY_A&( ((~REG_KEYINPUT)&0x3ff) | ((IPCZ->keyxy&0x3)<<10) | ((IPCZ->keyxy&0x40/*0xc0*/)<<6) ))
				IPCZ->cmd=Shutdown;
	}
}

void SoftReset(void)
{
	char loader[768],hbloader[768],lang[10],config[768],dir[768],target[768],head[512];
	TExtLinkBody extlink;
	FILE *f;
	int type;

	IPCZ->cmd=0;
	_consolePrintf(
		"m3loader beta\n"
		"reset_mse_06b_for_ak2 by Moonlight, Rudolph, kzat3\n"
		"moonshellreset by Moonlight\n"
		//"dldipatch aka dlditool public domain under CC0.\n"
		"%s\n%s\n\n",
		ROMDATE,ROMENV
	);

	{
		unsigned char dldiid[5];
		unsigned char *dldiFileData=io_dldi_data;
		memcpy(dldiid,(char*)dldiFileData+ioType,4);
		dldiid[4]=0;
		_consolePrintf("DLDI ID: %s\n",dldiid);
		_consolePrintf("DLDI Name: %s\n\n",(char*)dldiFileData+friendlyName);
	}

	//_consolePrintf("Waiting... ");
	//sleep(1);
	//_consolePrintf("Done.\n");

	_consolePrintf("initializing libfat... ");
	if(!fatInitDefault()){_consolePrintf("failed.\n");die();}
	_consolePrintf("done.\n");

	_consolePrintf("Opening extlink... ");
	if(!(f=fopen("/MOONSHL2/EXTLINK.DAT","rb"))){_consolePrintf("Failed.\n");die();}
	fread(&extlink,1,sizeof(TExtLinkBody),f);
	fclose(f);
	if(extlink.ID!=ExtLinkBody_ID){_consolePrintf("Incorrect ID.\n");die();}
	_consolePrintf("Done.\n");

	_consolePrintf("Configuring loader... ");
	type=ini_getl("m3loader","Type",0,"/MOONSHL2/EXTLINK/M3LOADER.INI");
	if(type){
		strcpy(loader,"/_system_/_sys_data/r4_firends.ext");
		strcpy(config,"/_system_/_sys_data/r4_homebrew.ini");
	}else{
		ini_gets("m3loader","TouchPodLang","eng",lang,10,"/MOONSHL2/EXTLINK/M3LOADER.INI");
		strcpy(loader,"/system/minigame.");
		strcat(loader,lang);
		strcpy(config,"/system/minibuff.swp");
	}
	_consolePrintf("Done.\n");

	_consolePrintf("Setting target... ");
	_FAT_directory_ucs2tombs(target,extlink.DataFullPathFilenameUnicode,768);
	if(!(f=fopen(target,"rb"))){_consolePrintf("Failed.\n");die();}
	//{struct stat st;fstat(fileno(f),&st);size=st.st_size;}
	//if(size<0x200){fclose(f);goto fail;}
	fread(head,1,0x200,f);
	fclose(f);
	if(!memcmp(head+0x0c,"####",4)||!memcmp(head+0x0c,"PASS",4)/*||!memcmp(head+0x0c,"ENG0",4)||!memcmp(head+0x0c,"JPN0",4)*/){
		_consolePrintf("Homebrew detected.\n"); //Using internal loader. Allocating %s...\n",target);
		if(!type&&!ini_getl("m3loader","UseR4iRTSForHomebrew",0,"/MOONSHL2/EXTLINK/M3LOADER.INI")){
			_consolePrintf("Falling back to internal loader. Allocating %s...\n",target);
			if (ret_menu9_Gen(target) == true) {
				_consolePrintf("Allocate done.\n");
			} else {
				_consolePrintf("Allocate failed.\n");
				die();
			}

			IPCZ->cmd=ResetRudolph;
			//fifoSendValue32(FIFO_USER_07,1);
			_consolePrintf("rebooting... \n");
			ret_menu9_GENs();
			_consolePrintf("failed.\n");
			die();
		}

		//if(type){
			strcpy(loader,"/_system_/_sys_data/r4_homebrew.ext");
			strcpy(config,"/_system_/_sys_data/r4_homebrew.ini");
			type=1;
		//}else{
		//	strcpy(loader,"/system/homebrew.");
		//	strcat(loader,lang);
		//}

	}

	if(type){
		_FAT_directory_ucs2tombs(dir,extlink.DataPathUnicode,768);
		if(dir[1])strcat(dir,"/");
		if(!(f=fopen(config,"wb"))){_consolePrintf("Failed.\n");die();}
		fwrite(dir,1,512,f);
		fwrite(target,1,512,f);
		fclose(f);
	}else{
		memset(target+255,0,1+36);
		if(!(f=fopen(config,"wb"))){_consolePrintf("Failed.\n");die();}
		fwrite(target,1,292/*256*/,f); //fixme
		fclose(f);
	}

	_consolePrintf("Done.\n");
	BootDSBooter(loader);
	//IPCZ->cmd=ResetBootlib;
	//runDSBooter(loader);
	die(); //Rudolph's loader cannot load r4_firends.ext(white-out)
}

