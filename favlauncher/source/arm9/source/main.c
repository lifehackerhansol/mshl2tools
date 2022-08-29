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

#if 0
u32 keys=0,keyrecv=0;
static void FIFORECV(){
	while(REG_IPC_FIFO_CR&IPC_FIFO_RECV_EMPTY);
	keys=REG_IPC_FIFO_RX;keyrecv=1;
}
#endif

#if 0
u32 keys=0,keysrecv=0;
static void fifoHandler(u32 value, void* u){
	//_consolePrintf("Trying fifo ~REG_KEYXY... ");
	//_consolePrintf("0x%04x, ",value);
	if(value&KEY_A)keys|=KEY_X;
	if(value&KEY_B)keys|=KEY_Y;
	keysrecv=1;
}
#endif

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

  //irqSet(IRQ_FIFO_NOT_EMPTY, FIFORECV);
  //irqEnable(IRQ_FIFO_NOT_EMPTY);
  //fifoSetValue32Handler(FIFO_USER_08, fifoHandler, 0);

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
#if 0	// change 2008.03.30 kzat3
  //const char *pname=MSEINFO.AdapterName;
  //_consolePrintf("go to farmware menu. [%s]\n",pname);
#else
  //_consolePrintf("dldi version\n");
#endif

	//ERESET RESET=RESET_NULL;
	char *file;
	int c=0;
	u32 keys=0;
	char dldiid[5];

	char menu_a[768],menu_b[768],menu_x[768],menu_y[768],menu_st[768],menu_se[768],menu_l[768],menu_r[768];
	char menu_touch[768],menu_crossu[768],menu_crossd[768],menu_crossl[768],menu_crossr[768];
	char menu_none[768];
	int usenone=0;

	IPCZ->cmd=0;
	_consolePrintf(
		"FavLauncher\n"
		"reset_mse_06b_for_ak2 by Moonlight, Rudolph, kzat3\n"
		"dldipatch aka dlditool public domain under CC0.\n"
		"%s\n%s\n\n",
		ROMDATE,ROMENV
	);

	{
		unsigned char *dldiFileData=io_dldi_data;
		memcpy(dldiid,(char*)dldiFileData+ioType,4);
		dldiid[4]=0;
		_consolePrintf("DLDI ID: %s\n",dldiid);
		_consolePrintf("DLDI Name: %s\n\n",(char*)dldiFileData+friendlyName);
	}

	_consolePrintf("Initializing libfat... ");
	if(!fatInitDefault()){_consolePrintf("Failed.\n");die();}
	_consolePrintf("Done.\n");

	//_consolePrintf("Reading /favlauncher.ini Phase1 ");
	_consolePrintf("Reading /favlauncher.ini...");
	ini_gets("General","A","/_menu_a.nds",menu_a,768,"/favlauncher.ini");
	ini_gets("General","B","/_menu_b.nds",menu_b,768,"/favlauncher.ini");
	ini_gets("General","X","/_menu_x.nds",menu_x,768,"/favlauncher.ini");
	ini_gets("General","Y","/_menu_y.nds",menu_y,768,"/favlauncher.ini");
	ini_gets("General","Start","/_menu_st.nds",menu_st,768,"/favlauncher.ini");
	ini_gets("General","Select","/_menu_se.nds",menu_se,768,"/favlauncher.ini");
	ini_gets("General","L","/_menu_l.nds",menu_l,768,"/favlauncher.ini");
	ini_gets("General","R","/_menu_r.nds",menu_r,768,"/favlauncher.ini");
	ini_gets("General","Touch","/_menu_touch.nds",menu_touch,768,"/favlauncher.ini");
	ini_gets("General","Up","/_menu_crossu.nds",menu_crossu,768,"/favlauncher.ini");
	ini_gets("General","Down","/_menu_crossd.nds",menu_crossd,768,"/favlauncher.ini");
	ini_gets("General","Left","/_menu_crossl.nds",menu_crossl,768,"/favlauncher.ini");
	ini_gets("General","Right","/_menu_crossr.nds",menu_crossr,768,"/favlauncher.ini");
	ini_gets("General","None","/_menu_.nds",menu_none,768,"/favlauncher.ini");
	usenone=ini_getl("General","UseNone",0,"/favlauncher.ini");

	//_consolePrintf("Phase2");
	ini_gets(dldiid,"A",menu_a,menu_a,768,"/favlauncher.ini");
	ini_gets(dldiid,"B",menu_b,menu_b,768,"/favlauncher.ini");
	ini_gets(dldiid,"X",menu_x,menu_x,768,"/favlauncher.ini");
	ini_gets(dldiid,"Y",menu_y,menu_y,768,"/favlauncher.ini");
	ini_gets(dldiid,"Start",menu_st,menu_st,768,"/favlauncher.ini");
	ini_gets(dldiid,"Select",menu_se,menu_se,768,"/favlauncher.ini");
	ini_gets(dldiid,"L",menu_l,menu_l,768,"/favlauncher.ini");
	ini_gets(dldiid,"R",menu_r,menu_r,768,"/favlauncher.ini");
	ini_gets(dldiid,"Touch",menu_touch,menu_touch,768,"/favlauncher.ini");
	ini_gets(dldiid,"Up",menu_crossu,menu_crossu,768,"/favlauncher.ini");
	ini_gets(dldiid,"Down",menu_crossd,menu_crossd,768,"/favlauncher.ini");
	ini_gets(dldiid,"Left",menu_crossl,menu_crossl,768,"/favlauncher.ini");
	ini_gets(dldiid,"Right",menu_crossr,menu_crossr,768,"/favlauncher.ini");
	ini_gets(dldiid,"None",menu_none,menu_none,768,"/favlauncher.ini");
	usenone=ini_getl(dldiid,"UseNone",usenone,"/favlauncher.ini");

if(!usenone){
	_consolePrintf("\n     A ");
	_consolePrintf(menu_a);
	_consolePrintf("\n     B ");
	_consolePrintf(menu_b);
	_consolePrintf("\n     X ");
	_consolePrintf(menu_x);
	_consolePrintf("\n     Y ");
	_consolePrintf(menu_y);
	_consolePrintf("\n Start ");
	_consolePrintf(menu_st);
	_consolePrintf("\nSelect ");
	_consolePrintf(menu_se);
	_consolePrintf("\n     L ");
	_consolePrintf(menu_l);
	_consolePrintf("\n     R ");
	_consolePrintf(menu_r);
	_consolePrintf("\n Touch ");
	_consolePrintf(menu_touch);
	_consolePrintf("\n    Up ");
	_consolePrintf(menu_crossu);
	_consolePrintf("\n  Down ");
	_consolePrintf(menu_crossd);
	_consolePrintf("\n  Left ");
	_consolePrintf(menu_crossl);
	_consolePrintf("\n Right ");
	_consolePrintf(menu_crossr);
	//_consolePrintf("\n  None ");
	//_consolePrintf(menu_none);
}
	_consolePrintf("\n");

	//_consolePrintf("Now start select procedure.\n");

	//scanKeys();
	for(;;c++){
		keys=0;
		if(!c)_consolePrintf("\nPress desired key: ");
		//_consolePrintf("\r");
		keys = ((~REG_KEYINPUT)&0x3ff) | ((IPCZ->keyxy&0x3)<<10) | ((IPCZ->keyxy&0x40/*0xc0*/)<<6);
		if(keys/*&0x1fff*/||usenone)break;
	}

			if(keys&KEY_A){_consolePrintf("A");file=menu_a;}
		else	if(keys&KEY_B){_consolePrintf("B");file=menu_b;}
		else	if(keys&KEY_X){_consolePrintf("X");file=menu_x;}
		else	if(keys&KEY_Y){_consolePrintf("Y");file=menu_y;}
		else	if(keys&KEY_L){_consolePrintf("L");file=menu_l;}
		else	if(keys&KEY_R){_consolePrintf("R");file=menu_r;}
		else	if(keys&KEY_START){_consolePrintf("Start");file=menu_st;}
		else	if(keys&KEY_SELECT){_consolePrintf("Select");file=menu_se;}
		else	if(keys&KEY_TOUCH){_consolePrintf("Touch");file=menu_touch;}
		else	if(keys&KEY_UP){_consolePrintf("Up");file=menu_crossu;}
		else	if(keys&KEY_DOWN){_consolePrintf("Down");file=menu_crossd;}
		else	if(keys&KEY_LEFT){_consolePrintf("Left");file=menu_crossl;}
		else	if(keys&KEY_RIGHT){_consolePrintf("Right");file=menu_crossr;}
		else	{_consolePrintf("None");file=menu_none;}

	_consolePrintf(".\n");

	for(c=0;;c++){
		keys=0;
		if(!c)_consolePrintf("Now release all keys to ensure to boot the NDS cleanly... ");
		//_consolePrintf("\r");
		keys = ((~REG_KEYINPUT)&0x3ff) | ((IPCZ->keyxy&0x3)<<10) | ((IPCZ->keyxy&0x40/*0xc0*/)<<6);
		if(!keys)break;
	}
	_consolePrintf("OK.\n\n");

	//_consolePrintf("Waiting... ");
	//sleep(1);
	//_consolePrintf("Done.\n");

	// vvvvvvvvvvv add 2008.03.30 kzat3
	_consolePrintf("Allocating %s...\n",file);
	if (ret_menu9_Gen(file) == true) {
		_consolePrintf("Allocate done.\n");
	} else {
		_consolePrintf("Allocate failed.\n");
		die();
	}
	//RESET=RESET_MENU_GEN;
	//IPCEX->RESET=RESET;
	IPCZ->cmd=ResetRudolph;
       //fifoSendValue32(FIFO_USER_07,1);
	_consolePrintf("Rebooting... \n");
	ret_menu9_GENs();
	_consolePrintf("Failed.\n");
	die();
	// ^^^^^^^^^^^^ add 2008.03.30 kzat3
}

