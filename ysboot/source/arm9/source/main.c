#include <nds.h>
#include <fat.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "_console.h"
#include "maindef.h"
#include "_const.h"
#include "../../ipcex.h"
//#include "linkreset_arm9.h"

#include "dldi.h"

#define POWER_CR       (*(vuint16*)0x04000304)	// add 2008.03.30 kzat3

#if 0		// change 2008.03.30 kzat3
void ret_menu9_R4(void);	//====== R4TF was added. by Rudolph (2007/05/23)
void ret_menu9_EZ5(void);	//====== EZ5S was added. by Rudolph (2007/05/25)
bool ret_menu_chk(const char *name);	//====== Added by Rudolph (2007/10/22)
bool ret_menu9_Gen(void);			//====== Added by Rudolph (2007/10/22)
#else
bool ret_menu9_Gen(char *);			//====== Added by Rudolph (2007/10/22)
#endif
void ret_menu9_GENs(void);			//====== Added by Rudolph (2007/10/22)

//---------------------------------------------------------------------------------
#include "setarm9_reg_waitcr.h"

int main(void) {
  //MSEINFO_Readed=MSE_GetMSEINFO(&MSEINFO);
  
  REG_IME=0;
  
  POWER_CR = POWER_ALL_2D;
//  POWER_CR &= ~POWER_SWAP_LCDS;
  POWER_CR |= POWER_SWAP_LCDS;
  
  SetARM9_REG_WaitCR();
  
  irqInit();
  
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
  
  while(1);
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
    BG_PALETTE_SUB[(0*16)+1] = RGB15(10,0,10) | BIT(15); // BG color
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
void SoftReset(void)
{
#if 0	// change 2008.03.30 kzat3
  //const char *pname=MSEINFO.AdapterName;
  //_consolePrintf("go to farmware menu. [%s]\n",pname);
#else
  //_consolePrintf("dldi version\n");
#endif

	ERESET RESET=RESET_NULL;
	FILE *f;
	char ysmenu[768];

	_consolePrintf(
		"YSMenu Boot\n"
		"reset_mse_06b_for_ak2 by Moonlight, Rudolph, kzat3\n"
		"dldipatch aka dlditool public domain under CC0.\n"
		"%s\n%s\n\n",
		ROMDATE,ROMENV
	);

	{
		unsigned char dldiid[5];
		unsigned char *dldiFileData=((u32*)(&_io_dldi))-24;
		memcpy(dldiid,(char*)dldiFileData+ioType,4);
		dldiid[4]=0;
		_consolePrintf("DLDI ID: %s\n",dldiid);
		_consolePrintf("DLDI Name: %s\n\n",(char*)dldiFileData+friendlyName);
	}

	_consolePrintf("Initializing libfat... ");
	if(!fatInitDefault()){_consolePrintf("Failed.\n");while(1);}
	_consolePrintf("Done.\n");

	_consolePrintf("Configuring AUTO_BOOT... ");
{
	char bootini[30],ininame[768],defaultdir[768];
	ini_gets("YSBoot","YSMenu","",ysmenu,768,"/YSMENU/YSBOOT.INI");
	if(*ysmenu){strcpy(bootini,"/YSMENU/YSBOOT.INI");goto done;}
	ini_gets("YSBoot","YSMenu","",ysmenu,768,"/_SYSTEM_/YSBOOT.INI");
	if(*ysmenu){strcpy(bootini,"/_SYSTEM_/YSBOOT.INI");goto done;}
	ini_gets("YSBoot","YSMenu","",ysmenu,768,"/TTMENU/YSBOOT.INI");
	if(*ysmenu){strcpy(bootini,"/TTMENU/YSBOOT.INI");goto done;}
	ini_gets("YSBoot","YSMenu","",ysmenu,768,"/__AK2/YSBOOT.INI");
	if(*ysmenu){strcpy(bootini,"/__AK2/YSBOOT.INI");goto done;}
	ini_gets("YSBoot","YSMenu","",ysmenu,768,"/__RPG/YSBOOT.INI");
	if(*ysmenu){strcpy(bootini,"/__RPG/YSBOOT.INI");goto done;}
	ini_gets("YSBoot","YSMenu","",ysmenu,768,"/YSBOOT.INI");
	if(*ysmenu){strcpy(bootini,"/YSBOOT.INI");goto done;}

	done:
	if(*bootini){
		ini_gets("YSBoot","YSini","",ininame,768,bootini);
		ini_gets("YSBoot","DefaultDir","",defaultdir,768,bootini);
		ini_puts("YSMenu","AUTO_BOOT",defaultdir,ininame);
	}else{ //fallback
		strcpy(ysmenu,"/YSMENU/YSMENU.NDS");
		//ininame is dummy here
		ini_gets("YSMenu","AUTO_BOOT","",ininame,768,"/YSMENU/YSMENU.INI");
		if(*ininame)ini_puts("YSMenu","AUTO_BOOT","","/YSMENU/YSMENU.INI");
		ini_gets("YSMenu","AUTO_BOOT","",ininame,768,"/_SYSTEM_/YSMENU.INI");
		if(*ininame)ini_puts("YSMenu","AUTO_BOOT","","/_SYSTEM_/YSMENU.INI");
		ini_gets("YSMenu","AUTO_BOOT","",ininame,768,"/TTMENU/YSMENU.INI");
		if(*ininame)ini_puts("YSMenu","AUTO_BOOT","","/TTMENU/YSMENU.INI");
		ini_gets("YSMenu","AUTO_BOOT","",ininame,768,"/__AK2/YSMENU.INI");
		if(*ininame)ini_puts("YSMenu","AUTO_BOOT","","/__AK2/YSMENU.INI");
		ini_gets("YSMenu","AUTO_BOOT","",ininame,768,"/__RPG/YSMENU.INI");
		if(*ininame)ini_puts("YSMenu","AUTO_BOOT","","/__RPG/YSMENU.INI");
		ini_gets("YSMenu","AUTO_BOOT","",ininame,768,"/YSMENU.INI");
		if(*ininame)ini_puts("YSMenu","AUTO_BOOT","","/YSMENU.INI");
	}
}
	_consolePrintf("Done.\n");

	// vvvvvvvvvvv add 2008.03.30 kzat3
	_consolePrintf("Allocating YSMenu...\n");
	if (ret_menu9_Gen(ysmenu) == true) {
		_consolePrintf("Allocate done.\n");
	} else {
		_consolePrintf("Allocate failed.\n");
		while(1);
	}
	RESET=RESET_MENU_GEN;
	IPCEX->RESET=RESET;
	_consolePrintf("Rebooting... \n");
	ret_menu9_GENs();
	_consolePrintf("Failed.\n");
	while(1);
	// ^^^^^^^^^^^^ add 2008.03.30 kzat3
}

