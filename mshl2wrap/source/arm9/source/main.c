#define MOONSHELL "/MOONSHL2/EXTLINK/_hn.HugeNDSLoader.nds"

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

#include "fatx.h"
#include "minIni.h"

//static bool MSEINFO_Readed=false;
//static TMSEINFO MSEINFO;
char *template="*mshl2wrap link template";

#if 0		// change 2008.03.30 kzat3
void ret_menu9_R4(void);	//====== R4TF was added. by Rudolph (2007/05/23)
void ret_menu9_EZ5(void);	//====== EZ5S was added. by Rudolph (2007/05/25)
bool ret_menu_chk(const char *name);	//====== Added by Rudolph (2007/10/22)
bool ret_menu9_Gen(void);			//====== Added by Rudolph (2007/10/22)
#else
bool ret_menu9_Gen(char *);			//====== Added by Rudolph (2007/10/22)
#endif
void ret_menu9_GENs(void);			//====== Added by Rudolph (2007/10/22)

//extlink
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

#define MaxFilenameLength ExtLinkBody_MaxLength

//routines from moonshell
void Unicode_Copy(UnicodeChar *tag,const UnicodeChar *src){
  while(*src!=0){*tag=*src;tag++; src++;}
  *tag=(UnicodeChar)0;
}

void SplitItemFromFullPathAlias(const char *pFullPathAlias,char *pPathAlias,char *pFilenameAlias){
  u32 SplitPos=0;
  {
    u32 idx=0;
    while(1){
      char uc=pFullPathAlias[idx];
      if(uc==0) break;
      if(uc=='/') SplitPos=idx+1;
      idx++;
    }
  }
  if(SplitPos<=1){
    pPathAlias[0]='/';
    pPathAlias[1]=0;
    }else{u32 idx=0;
    for(;idx<SplitPos-1;idx++){
      pPathAlias[idx]=pFullPathAlias[idx];
    }
    pPathAlias[SplitPos-1]=0;
  }
  strcpy(pFilenameAlias,&pFullPathAlias[SplitPos]);
}

void SplitItemFromFullPathUnicode(const UnicodeChar *pFullPathUnicode,UnicodeChar *pPathUnicode,UnicodeChar *pFilenameUnicode){
  u32 SplitPos=0;
  {
    u32 idx=0;
    while(1){
      UnicodeChar uc=pFullPathUnicode[idx];
      if(uc==0) break;
      if(uc==(UnicodeChar)'/') SplitPos=idx+1;
      idx++;
    }
  }

  if(SplitPos<=1){
    pPathUnicode[0]=(UnicodeChar)'/';
    pPathUnicode[1]=0;
    }else{u32 idx=0;
    for(;idx<SplitPos-1;idx++){
      pPathUnicode[idx]=pFullPathUnicode[idx];
    }
    pPathUnicode[SplitPos-1]=0;
  }
  Unicode_Copy(pFilenameUnicode,&pFullPathUnicode[SplitPos]);
}

//---------------------------------------------------------------------------------
#include "setarm9_reg_waitcr.h"

int main(void) {
  //MSEINFO_Readed=MSE_GetMSEINFO(&MSEINFO);
//VRAM_C_CR = 0;
//VRAM_CR   = 0x03000000;
  
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
    BG_PALETTE_SUB[(0*16)+1] = RGB15(0,8,8) | BIT(15); // BG color
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

char ext[32][768];

void SoftReset(void)
{
#if 0	// change 2008.03.30 kzat3
  //const char *pname=MSEINFO.AdapterName;
  //_consolePrintf("go to farmware menu. [%s]\n",pname);
#else
  //_consolePrintf("dldi version\n");
#endif

	FILE *f;
	TExtLinkBody extlink;
	int size,hbmode=0;
	//ERESET RESET=RESET_NULL;
	int flag=0;

	char target[256*3]=
				//"GUIDDIUG\0\0\0\0\0\0\0\0\0\0\0\0"
				"mshl2wrap link template\0\0\0\0" //will be modified from external link maker
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"; //744 paddings

	char loader[256*3];
	char utf8[768];
	char dldiid[5];
	unsigned char head[0x200];

	IPCZ->cmd=0;
	if(strcmp(target,template+1))flag=1;

	_consolePrintf(
		"MoonShell2 %s\nVersion %s\n"
		"reset_mse_06b_for_ak2 by Moonlight, Rudolph, kzat3\n"
		"dldipatch aka dlditool public domain under CC0.\n"
		"%s\n%s\n\n",
		flag?"Link Launcher":"Extlink Wrapper",ROMVERSION,ROMDATE,ROMENV
	);

	{
		unsigned char *dldiFileData=io_dldi_data;
		memcpy(dldiid,(unsigned char*)dldiFileData+ioType,4);
		dldiid[4]=0;
		_consolePrintf("DLDI ID: %s\n",dldiid);
		_consolePrintf("DLDI Name: %s\n\n",(char*)dldiFileData+friendlyName);
	}

	_consolePrintf("Initializing libfat... ");
	if(!fatInitDefault())goto fail;
	_consolePrintf("Done.\n");

	_consolePrintf("Checking condition of /moonshl2/extlink/... ");
	{
		char tmp1[768],tmp2[768];
		u8 head[512];
		int n=0,l=-1,i=0;
		int l1,l2;
		DIR_ITER *dir=diropen("/moonshl2/extlink/");
		strcpy(tmp1,"/moonshl2/extlink/");l1=strlen(tmp1);
		strcpy(tmp2,"/moonshl2/extlink/__link_rearrange/");l2=strlen(tmp2);

		if(dir){
			while(!dirnext(dir,ext[n],NULL)){
				if(strlen(ext[n])>6&&!memcmp(ext[n],"nds.",4)&&!memcmp(ext[n]+strlen(ext[n])-4,".nds",4)){
					if(n==32){dirclose(dir);_consolePrintf("too many extlink. Halt.\n");die();}
					strcpy(tmp1+l1,ext[n]);
					if(!(f=fopen(tmp1,"rb"))){dirclose(dir);_consolePrintf("Cannot open %s. Possibly bug... Halt.\n",tmp1);die();}
					fread(head,1,512,f);
					fclose(f);
					if(!strcmp(head+0x1e0,"mshl2wrap link")){
						if(l>=0){dirclose(dir);_consolePrintf("multiple mshl2wrap. Halt.\n");die();}
						l=n;
					}
					n++;
				}
			}
			dirclose(dir);
			if(l<0&&n>0){_consolePrintf("mshl2wrap has to be in /moonshl2/extlink/ if you put nds.*.nds there. Halt.\n");die();}
			if(l>0){
				_consolePrintf("Need rearranging.\n");
				mkdir("/moonshl2/extlink/__link_rearrange",0777);
				for(i=0;i<n;i++){
					strcpy(tmp1+l1,ext[i]);
					strcpy(tmp2+l2,ext[i]);
					rename(tmp1,tmp2);
				}
					strcpy(tmp1+l1,ext[l]);
					strcpy(tmp2+l2,ext[l]);
					rename(tmp2,tmp1);
				for(i=0;i<n;i++){
					strcpy(tmp1+l1,ext[i]);
					strcpy(tmp2+l2,ext[i]);
					rename(tmp2,tmp1);
				}
				unlink("/moonshl2/extlink/__link_rearrange"); //in libnds unlink is used for dir
			}
		}
	}
	_consolePrintf("Done.\n");
	//die();

	ini_gets("mshl2wrap",dldiid/*"loader"*/,MOONSHELL,loader,256*3,"/MOONSHL2/EXTLINK/mshl2wrap.ini");
/*	//This might be used on 2.06...
	if(!strcmp(loader,MOONSHELL)){
		_consolePrintf(
			"Loader not set for cardtype %s.\n"
			"To force to use MoonShell HugeNDSLoader, press A\n"
			To return to the firmware, press B\n"
		,dldiid);
	}
*/

if(!flag){
	_consolePrintf("Opening moonshl2/extlink.dat... ");
	if(!(f=fopen("/MOONSHL2/EXTLINK.DAT","rb")))goto fail;
	_consolePrintf("Done.\n");
	_consolePrintf("Reading moonshl2/extlink.dat... ");
	memset(&extlink,0,sizeof(TExtLinkBody));
	fread(&extlink,1,sizeof(TExtLinkBody),f);
	fclose(f);

	//if(!(f=fopen("/EXTLINK.BAK","wb")))goto fail;
	//fwrite(&extlink,1,sizeof(TExtLinkBody),f);
	//fclose(f);

	_consolePrintf("0x%08x\n",extlink.ID);
	//extlink.ID=ExtLinkBody_ID;
	if(extlink.ID!=ExtLinkBody_ID)goto fail;
	_consolePrintf("Target NDS is:\n%s\n",extlink.DataFullPathFilenameAlias);
	_consolePrintf("My name is:\n%s\n\n",extlink.NDSFullPathFilenameAlias);
}

	_consolePrintf("Setting desired filenames to moonshl2/extlink.dat...\n");


if(!flag){
	//_consolePrintf("Linked NDS is:\n%s\n",extlink.DataFullPathFilenameAlias);
	_FAT_directory_ucs2tombs(utf8,extlink.DataFullPathFilenameUnicode,768);
	if(!(f=fopen(utf8,"rb")))goto fail;
	{struct stat st;fstat(fileno(f),&st);size=st.st_size;}
	if(size<0x200){fclose(f);goto fail;}
	fread(head,1,0x200,f);
	if(memcmp(head+0x0c,"####",4)&&memcmp(head+0x0c,"PASS",4)&&memcmp(head+0x0c,"ENG0",4)){
		_FAT_directory_ucs2tombs(target,extlink.DataFullPathFilenameUnicode,768);goto target_set;
	}else if(!strcmp(head+0x1e0,"mshl2wrap link")){
		unsigned int s=(head[0x1f0]<<24)+(head[0x1f1]<<16)+(head[0x1f2]<<8)+head[0x1f3];
		_consolePrintf("Detected mshl2wrap link.\n");
		if(size<s+256*3){fclose(f);goto fail;}
		fseek(f,s,SEEK_SET);fread(target,1,256*3,f);goto target_set;
	}
	_FAT_directory_ucs2tombs(target,extlink.DataFullPathFilenameUnicode,768);
	hbmode=ini_getl("mshl2wrap","hbmode",0,"/MOONSHL2/EXTLINK/mshl2wrap.ini");
	if(!hbmode&&!strcmp(dldiid,"M3DS"))hbmode=1;
	if(hbmode==1&&read32(head+0x24)!=0x02000000)hbmode=2; //hn loader has some issue...
	if(hbmode){
		if(hbmode==1)strcpy(loader,MOONSHELL);
		else _FAT_directory_ucs2tombs(loader,extlink.NDSFullPathFilenameUnicode,768); //dummy
       }
	target_set:
	fclose(f);
}else{ //applying target/loader manually.
	hbmode=ini_getl("mshl2wrap","hbmode",0,"/MOONSHL2/EXTLINK/mshl2wrap.ini");
	if(!hbmode&&!strcmp(dldiid,"M3DS"))hbmode=1;
	//Now confirm target.
	_consolePrintf("Linked NDS is:\n%s\n",target);
	if(!(f=fopen(target,"rb")))goto fail;
	{struct stat st;fstat(fileno(f),&st);size=st.st_size;}
	if(size<0x200){fclose(f);goto fail;}
	fread(head,1,0x200,f);
	fclose(f);
	if(memcmp(head+0x0c,"####",4)&&memcmp(head+0x0c,"PASS",4)&&memcmp(head+0x0c,"ENG0",4))hbmode=0;
	if(hbmode==1&&read32(head+0x24)!=0x02000000)hbmode=2; //hn loader has some issue...
	if(hbmode==1)strcpy(loader,MOONSHELL);
}

	memset(&extlink,0,sizeof(TExtLinkBody));
	extlink.ID=ExtLinkBody_ID;
	getsfnlfn(target,extlink.DataFullPathFilenameAlias,extlink.DataFullPathFilenameUnicode);
	SplitItemFromFullPathAlias(extlink.DataFullPathFilenameAlias,extlink.DataPathAlias,extlink.DataFilenameAlias);
	SplitItemFromFullPathUnicode(extlink.DataFullPathFilenameUnicode,extlink.DataPathUnicode,extlink.DataFilenameUnicode);

	getsfnlfn(loader,extlink.NDSFullPathFilenameAlias,extlink.NDSFullPathFilenameUnicode);
	SplitItemFromFullPathAlias(extlink.NDSFullPathFilenameAlias,extlink.NDSPathAlias,extlink.NDSFilenameAlias);
	SplitItemFromFullPathUnicode(extlink.NDSFullPathFilenameUnicode,extlink.NDSPathUnicode,extlink.NDSFilenameUnicode);

	_consolePrintf("Target NDS is:\n%s\n",extlink.DataFullPathFilenameAlias);
	_consolePrintf("Loader name is:\n%s\n",extlink.NDSFullPathFilenameAlias);

	//if(strstr(extlink.DataFullPathFilenameAlias,"/MOONSHL2/EXTLINK/NDS"))
	//	{_consolePrintf("You must not set loader to /moonshl2/extlink/nds*. Read warning_about_mshl2wrap_configuration.txt again. Halted.\n");die();}

if(hbmode<2){
	if(!(f=fopen("/MOONSHL2/EXTLINK.DAT","wb")))goto fail;
	fwrite(&extlink,1,sizeof(TExtLinkBody),f);
	fclose(f);
	//if(!(f=fopen("/EXTLINK.LOG","wb")))goto fail;
	//fwrite(&extlink,1,sizeof(TExtLinkBody),f);
	//fclose(f);
	_consolePrintf("Done.\n\n");
}

	//if(hbmode<0 || 2<hbmode){_consolePrintf("hbmode has to 0, 1 or 2.\nLaunch ");goto fail;}

	// vvvvvvvvvvv add 2008.03.30 kzat3
	_FAT_directory_ucs2tombs(utf8,hbmode<2?extlink.NDSFullPathFilenameUnicode:extlink.DataFullPathFilenameUnicode,768);

	BootNDSROM(utf8);
/*
	_consolePrintf("Allocating actual loader...\n");
	if(!ret_menu9_Gen(utf8))goto fail;
	_consolePrintf("Done.\n");

	_consolePrintf("Rebooting... ");
#if 1
	//RESET=RESET_MENU_GEN;
	//IPCEX->RESET=RESET;
	IPCZ->cmd=ResetRudolph;
       //fifoSendValue32(FIFO_USER_07,1);
	ret_menu9_GENs();
#endif
*/
fail:
	_consolePrintf("Failed.\nProcess cannot continue. Accept your fate.\n");
	die();
	// ^^^^^^^^^^^^ add 2008.03.30 kzat3
}

