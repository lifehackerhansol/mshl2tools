#include <nds.h>
#include <fat.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

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
	TExtLinkBody extlink;
	char ysmenu[768];
	char dldiid[5];
	unsigned char *dldiFileData=io_dldi_data;

	IPCZ->cmd=0;

	_consolePrintf(
		"MoonShell2 inilinker\n"
		"reset_mse_06b_for_ak2 by Moonlight, Rudolph, kzat3\n"
		"dldipatch aka dlditool public domain under CC0.\n"
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

	_consolePrintf("Opening extlink... ");
	if(!(f=fopen("/MOONSHL2/EXTLINK.DAT","rb"))){_consolePrintf("Failed.\n");die();}
	fread(&extlink,1,sizeof(TExtLinkBody),f);
	fclose(f);
	if(extlink.ID!=ExtLinkBody_ID){_consolePrintf("Incorrect ID.\n");die();}
	_consolePrintf("Done.\n");

	_consolePrintf("Configuring... ");
{
	char ininame[768];
	int useak2,useeos,usewoodr4;
	
	ini_gets("Config","YSMenu","/YSMENU/YSMENU.NDS",ysmenu,768,"/MOONSHL2/EXTLINK/INILINK.INI");
	ini_gets("Config","YSini","/YSMENU/YSMENU.INI",ininame,768,"/MOONSHL2/EXTLINK/INILINK.INI");
	if(!memcmp(dldiid,"SCDS",4))ini_gets("Config","SCYSMenu",ysmenu,ysmenu,768,"/MOONSHL2/EXTLINK/INILINK.INI");
	useak2=ini_getl("Config","UseAK2",2,"/MOONSHL2/EXTLINK/INILINK.INI");
	if(strstr((char*)dldiFileData+friendlyName,"RPG"))useak2=ini_getl("Config","UseRPG",2,"/MOONSHL2/EXTLINK/INILINK.INI");
	useeos=ini_getl("Config","UseEOS",0,"/MOONSHL2/EXTLINK/INILINK.INI");
	usewoodr4=ini_getl("Config","UseWoodR4",1,"/MOONSHL2/EXTLINK/INILINK.INI");
	if(!memcmp(dldiid,"XXXX",4))memcpy(dldiid,"RPGN",4),useak2=2;
	if(!memcmp(dldiid,"iTDS",4)||!memcmp(dldiid,"R4_I",4))memcpy(dldiid,"M3DS",4);
	if(!memcmp(dldiid,"R4DS",4)||!memcmp(dldiid,"_R4i",4)){
		memcpy(dldiid,"RPGS",4);
		if(!useak2)memcmp((char*)dldiFileData+ioType,"RPGS",4);
	}
	if(!memcmp(dldiid,"DEMO",4))memcpy((char*)dldiFileData+ioType,"TTIO",4);

	if(useak2==3&&!memcmp(dldiid,"RPG",3))ini_gets("Config","WoodRPG","/AKMENU.NDS",ysmenu,768,"/MOONSHL2/EXTLINK/INILINK.INI");
	if(useak2==2&&!memcmp(dldiid,"RPG",3))ini_gets("Config","AKAIO","/AKMENU.NDS",ysmenu,768,"/MOONSHL2/EXTLINK/INILINK.INI");
	if(useak2==1&&!memcmp(dldiid,"RPG",3))ini_gets("Config","AKMENU","/AKMENU.NDS",ysmenu,768,"/MOONSHL2/EXTLINK/INILINK.INI");
	if(useeos==1&&!memcmp(dldiid,"SCDS",4))ini_gets("Config","EOS","/EOS.NDS",ysmenu,768,"/MOONSHL2/EXTLINK/INILINK.INI");
	if(!memcmp(dldiid,"M3DS",4))ini_gets("Config","TouchPod","/_SYSTEM_/TOUCHPOD.NDS",ysmenu,768,"/MOONSHL2/EXTLINK/INILINK.INI");
	if(usewoodr4==1&&!memcmp(dldiid,"R4TF",4))ini_gets("Config","WoodR4","/WOODR4.NDS",ysmenu,768,"/MOONSHL2/EXTLINK/INILINK.INI");
	if(usewoodr4==-1&&!memcmp(dldiid,"R4TF",4))ini_gets("Config","R4i3D","/R4i3D.NDS",ysmenu,768,"/MOONSHL2/EXTLINK/INILINK.INI");
	//autoboot=ini_getl("Config","autoboot",1,"/MOONSHL2/EXTLINK/CONFIG.INI");

	//if(!autoboot){
	//	ini_puts("YSMenu","AUTO_BOOT","",ininame);
	//}else
	{
		char utf8[768];
		_FAT_directory_ucs2tombs(utf8,extlink.DataFullPathFilenameUnicode,768);
		if(!memcmp(dldiid,"M3DS",4)){
			char buf[1024];
			FILE *in,*out;
			in=fopen("/system/boot.ini","rb");
			if(!in){_consolePrintf("Cannot open /system/boot.ini. Halt.");die();}
			out=fopen("/system/boot.in~","wb");
			while(myfgets(buf,1024,in)){
				if(!memcmp(buf,"[DirectRun]",11)){
					strcpy(buf,"[DirectRun]="),strcat(buf,utf8),strcat(buf,"\r\n");
					fputs(buf,out);
				}else{
					strcat(buf,"\r\n");
					fputs(buf,out);
				}
			}
			fclose(out);fclose(in);
			remove("/system/boot.ini");
			rename("/system/boot.in~","/system/boot.ini");
		}else if(useak2==3&&!memcmp(dldiid,"RPG",3)){
			char tmp[776];
			strcpy(tmp,"fat1:");
			strcpy(tmp+5,utf8);
			ini_puts("Save Info","lastLoaded",tmp,"/__rpg/lastsave.ini");
			strcpy(tmp+5,extlink.DataFullPathFilenameAlias);
			ini_puts("Save Info","shortPathNDS",tmp,"/__rpg/lastsave.ini");
			ini_putl("system","autorunWithLastRom",1,"/__rpg/globalsettings.ini");
		}else if(useak2==2&&!memcmp(dldiid,"RPG",3)){
			char tmp[776];
			strcpy(tmp,"fat1:");
			strcpy(tmp+5,utf8);
			ini_puts("Save Info","lastLoaded",tmp,"/__aio/lastsave.ini");
			strcpy(tmp+5,extlink.DataFullPathFilenameAlias);
			ini_puts("Save Info","shortPathNDS",tmp,"/__aio/lastsave.ini");
			ini_putl("system","autorunWithLastRom",1,"/__aio/globalsettings.ini");
		}else if(useak2==1&&!memcmp(dldiid,"RPG",3)){
			char tmp[776];
			strcpy(tmp,"fat1:");
			strcpy(tmp+5,utf8);
			ini_puts("Save Info","lastLoaded",tmp,"/__ak2/lastsave.ini");
			strcpy(tmp+5,extlink.DataFullPathFilenameAlias);
			ini_puts("Save Info","shortPathNDS",tmp,"/__ak2/lastsave.ini");
			ini_putl("system","autorunWithLastRom",1,"/__ak2/globalsettings.ini");
		}else if(usewoodr4==1&&!memcmp(dldiid,"R4TF",4)){
			char tmp[776];
			strcpy(tmp,"fat1:");
			strcpy(tmp+5,utf8);
			ini_puts("Save Info","lastLoaded",tmp,"/__rpg/lastsave.ini");
			strcpy(tmp+5,extlink.DataFullPathFilenameAlias);
			//ini_puts("Save Info","shortPathNDS",tmp,"/__rpg/lastsave.ini");
			ini_putl("system","autorunWithLastRom",1,"/__rpg/globalsettings.ini");
		}else if(usewoodr4==-1&&!memcmp(dldiid,"R4TF",4)){
			char tmp[776];
			strcpy(tmp,"fat1:");
			strcpy(tmp+5,utf8);
			ini_puts("Save Info","lastLoaded",tmp,"/system/lastsave.ini");
			strcpy(tmp+5,extlink.DataFullPathFilenameAlias);
			//ini_puts("Save Info","shortPathNDS",tmp,"/system/lastsave.ini");
			ini_putl("system","autorunWithLastRom",1,"/system/globalsettings.ini");
		}else if(useeos&&!memcmp(dldiid,"SCDS",4)){
			char tmp[776];
			strcpy(tmp,"fat1:");
			//strcpy(tmp+5,utf8);
			//ini_puts("Dir Info","last dir",tmp,"/_dsone/lastsave.ini");
			strcpy(tmp+5,extlink.DataFullPathFilenameAlias);
			ini_puts("Dir Info","last dir",tmp,"/_dsone/lastsave.ini");
			ini_putl("system","enterLastDirWhenBoot",1,"/_dsone/globalsettings.ini");
		}else{
			ini_puts("YSMenu","AUTO_BOOT",utf8,ininame);
		}
	}
}
	_consolePrintf("Done.\n");

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
}

