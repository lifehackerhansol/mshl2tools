#include <nds.h>
#include <fat.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/dir.h>

#include "_console.h"
#include "maindef.h"
#include "_const.h"
#include "../../../../arm7/ipcz.h"
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
    BG_PALETTE_SUB[(0*16)+1] = RGB15(10,10,0) | BIT(15); // BG color
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

char* myfgets(char *buf,int n,FILE *fp){ //accepts LF/CRLF
	char *ret=fgets(buf,n,fp);
	if(!ret)return NULL;
	if(strlen(buf)&&buf[strlen(buf)-1]=='\n')buf[strlen(buf)-1]=0;
	if(strlen(buf)&&buf[strlen(buf)-1]=='\r')buf[strlen(buf)-1]=0;
	return ret;
}

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

char source[768],target[768];
void recursive(u8 *p, int offset, int size/*, char *source, char *target*/);
void SoftReset(void)
{
#if 0	// change 2008.03.30 kzat3
  //const char *pname=MSEINFO.AdapterName;
  //_consolePrintf("go to farmware menu. [%s]\n",pname);
#else
  //_consolePrintf("dldi version\n");
#endif

	//ERESET RESET=RESET_NULL;
	FILE *f;
	char dldiid[5];
	unsigned char *dldiFileData=io_dldi_data;

	IPCZ->cmd=0;

	_consolePrintf(
		"NDSLink on DS\n"
		//"reset_mse_06b_for_ak2 by Moonlight, Rudolph, kzat3\n"
		//"dldipatch aka dlditool public domain under CC0.\n"
		"%s\n%s\n\n",
		ROMDATE,ROMENV
	);

	{
		memcpy(dldiid,(char*)dldiFileData+ioType,4);
		dldiid[4]=0;
		_consolePrintf("DLDI ID: %s\n",dldiid);
		_consolePrintf("DLDI Name: %s\n\n",(char*)dldiFileData+friendlyName);
	}

	_consolePrintf("Initializing libfat... ");
	if(!fatInitDefault()){_consolePrintf("Failed.\n");die();}
	_consolePrintf("Done.\n");

	_consolePrintf("Configuring... ");
{
	u8 *p;
	int size,offset;
	
	ini_gets("ndslink","source","",source,768,"/NDSLINK.INI");
	ini_gets("ndslink","target","",target,768,"/NDSLINK.INI");
	if(!*source||!*target){_consolePrintf("As this removes old files, ndslink.ini configuration is required.\n");die();}
	if(source[strlen(source)-1]!='/')strcat(source,"/");
	if(target[strlen(target)-1]!='/')strcat(target,"/");
	mkpath(target);
	rm_rf(target);
	size=getmshl2wrap(&p);
	if(strcmp(p+0x1e0,"mshl2wrap link"))
		{puts("template not mshl2wrap link.");die();}
	offset=(p[0x1f0]<<24)+(p[0x1f1]<<16)+(p[0x1f2]<<8)+p[0x1f3];
	if(size<offset+256*3){puts("template too small or offset invalid.");die();}
	_consolePrintf("Done.\n");

	_consolePrintf("Linking...\n");
	recursive(p,offset,size/*,source,target*/);
}
	_consolePrintf("Done.\n");die();
/*
	// vvvvvvvvvvv add 2008.03.30 kzat3
#ifdef GPL
	//if (!strcmp(dldiid,"EZ5H")||!strcmp(dldiid,"EDGE")||!strcmp(dldiid,"SCDS")){
		_consolePrintf("falling back to Chishm VRAM bootlib.\n");
		IPCZ->cmd=ResetBootlib;
		//fifoSendValue32(FIFO_USER_07,2);
		runNdsFile(ysmenu);
	//}
#endif
	//RESET=RESET_MENU_GEN;
	//IPCEX->RESET=RESET;
       //fifoSendValue32(FIFO_USER_07,1);
	//IPCZ->cmd=ResetRudolph;
	_consolePrintf("Rebooting... \n");
	BootNDSROM(ysmenu);
	//ret_menu9_GENs();
	_consolePrintf("Failed.\n");
	die();
	// ^^^^^^^^^^^^ add 2008.03.30 kzat3
*/
}

