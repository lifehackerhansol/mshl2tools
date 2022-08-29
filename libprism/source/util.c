#include "libprism.h"

#if !defined(_LIBNDS_MAJOR_)
#include <nds/registers_alt.h>  //to maintain r23
#endif

extern void Main();
extern const u16 bgcolor;

u8 libprism_buf[BUFLEN];

char myname[768],mypath[768];
char argname[768],argpath[768];
char libprism_name[768];
bool fpassarg;

byte *DLDIToBoot;
type_printf PrintfToDie;
type_u8p ret_menu9_callbackpre;
type_u8p ret_menu9_callback;

u16 *b15ptrMain=(u16*)0x06008000;
u16 *b15ptrSub=(u16*)0x06208000;

char __sig_mode[8]="\xFE""cMode\x01";

char **argv;
int argc;
char *argvToInstall;
int argvToInstallSize;
char mydrive[12];
int nocashMessageMain;
int nocashMessageSub;

const byte DLDINull[]={
	0x00,0xA5,0x8D,0xBF,0x20,0x43,0x68,0x69,0x73,0x68,0x6D,0x00,0x01,0x0F,0x00,0x0F,
	0x44,0x65,0x66,0x61,0x75,0x6C,0x74,0x20,0x28,0x4E,0x6F,0x20,0x69,0x6E,0x74,0x65,
	0x72,0x66,0x61,0x63,0x65,0x29,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x80,0xbf,0x88,0x00,0x80,0xbf,0x88,0x00,0x80,0xbf,0x88,0x00,0x80,0xbf,
	0x88,0x00,0x80,0xbf,0x88,0x00,0x80,0xbf,0x88,0x00,0x80,0xbf,0x88,0x00,0x80,0xbf,
	0x44,0x4c,0x44,0x49,0x23,0x00,0x00,0x00,0x80,0x00,0x80,0xbf,0x80,0x00,0x80,0xbf,
	0x80,0x00,0x80,0xbf,0x80,0x00,0x80,0xbf,0x80,0x00,0x80,0xbf,0x80,0x00,0x80,0xbf,
	0x00,0x00,0xA0,0xE3,0x1E,0xFF,0x2F,0xE1,
};

inline void SetARM9_REG_WaitCR(){
	u16 bw=REG_EXMEMCNT;

	bw&=BIT8 | BIT9 | BIT10 | BIT12 | BIT13;

	// mp2 def.0x6800
	// loader def.0x6000

	bw|=2 << 0; // 0-1  RAM-region access cycle control   0..3=10,8,6,18 cycles def.0
	bw|=0 << 2; // 2-3  ROM 1st access cycle control   0..3=10,8,6,18 cycles def.0
	bw|=0 << 4; // 4    ROM 2nd access cycle control   0..1=6,4 cycles def.0
	bw|=0 << 5; // 5-6  PHI-terminal output control   0..3=Lowlevel, 4.19MHz, 8.38MHZ, 16.76MHz clock output def.0
	bw|=0 << 7; // 7    Cartridge access right   0=ARM9, 1=ARM7 def.0
	bw|=0 << 11; // 11   Card access right   0=ARM9, 1=ARM7 def.1
	bw|=1 << 14; // 14   Main Memory Interface mode   0=Asychronous (prohibited!), 1=Synchronous def.1
	bw|=1 << 15; // 15   Main Memory priority   0=ARM9 priority, 1=ARM7 priority def.0
  
	REG_EXMEMCNT=bw;
}
#if 0
inline void fillMemory( void * addr, u32 count, u32 value ){
	swiFastCopy( (void*)(&value), addr, (count>>2) | COPY_MODE_WORD | COPY_MODE_FILL);
}
inline void zeroMemory( void * addr, u32 count ){
	fillMemory( addr, count, 0 );
}

void resetARM9Video()
{
	//REG_IME=0;
	// DMA
	u8 i=0;
	for(; i<4; i++){
		DMA_CR(i) = 0;
		DMA_SRC(i) = 0;
		DMA_DEST(i) = 0;
		TIMER_CR(i) = 0;
		TIMER_DATA(i) = 0;
	}

	// VIDEO
	// trun on vram banks for clear
	VRAM_CR = 0x80808080;
	VRAM_E_CR = 0x80;
	VRAM_F_CR = 0x80;
	VRAM_G_CR = 0x80;
	VRAM_H_CR = 0x80;
	VRAM_I_CR = 0x80;
	// clear vram
	zeroMemory( VRAM, 656 * 1024 );
	// clear video palette
	zeroMemory( BG_PALETTE, 2048 );//PALETTE[0] = RGB15(1,1,1);
	zeroMemory( BG_PALETTE_SUB, 2048 );
	// clear video object attribution memory
	zeroMemory( OAM, 2048 );
	zeroMemory( OAM_SUB, 2048 );
	// clear video object data memory
	zeroMemory( SPRITE_GFX, 128 * 1024 );
	zeroMemory( SPRITE_GFX_SUB, 128 * 1024 );
	// clear main display registers
	zeroMemory( (void*)0x04000008, 0x4e );
	// clear sub display registers
	zeroMemory( (void*)0x04001008, 0x4e );
	
	// clear video registers
	//REG_DISPSTAT = DISP_IN_VBLANK;
	SetYtrigger(80);

	REG_DISPCNT = 0;
	REG_DISPCNT_SUB = 0;
	VRAM_A_CR = 0;
	VRAM_B_CR = 0;
	VRAM_C_CR = 0;
	VRAM_D_CR = 0;
	VRAM_E_CR = 0;
	VRAM_F_CR = 0;
	VRAM_G_CR = 0;
	VRAM_H_CR = 0;
	VRAM_I_CR = 0;
	VRAM_CR   = 0x00000000;
	//REG_IME=1;
}
#endif
void EnableB15Main(){
	videoSetMode(MODE_4_2D | DISPLAY_BG2_ACTIVE | DISPLAY_BG3_ACTIVE);
}

void EnableB15Sub(){
	videoSetModeSub(MODE_4_2D | DISPLAY_BG2_ACTIVE | DISPLAY_BG3_ACTIVE);
}

void DisableB15Main(){
	videoSetMode(MODE_4_2D | DISPLAY_BG2_ACTIVE);
}

void DisableB15Sub(){
	videoSetModeSub(MODE_4_2D | DISPLAY_BG2_ACTIVE);
}

TransferRegionZ volatile *IPCZ;
int main(int __argc, char **__argv){
	IPCZ=(*(vu32*)0x04004000)?IPCZ_DSiMode:IPCZ_DSMode;
	//IPCZ=IPCZ_DSMode;

	//SetARM9_REG_WaitCR();
	sysSetBusOwners( BUS_OWNER_ARM9, BUS_OWNER_ARM9 );
	//resetARM9Video();

/// I never know about side-effects.
#ifdef _LIBNDS_MAJOR_
	REG_MASTER_BRIGHT_SUB = REG_MASTER_BRIGHT = 0;
#else
	SUB_BRIGHTNESS = BRIGHTNESS = 0;
#endif
	//GFX_CONTROL = 0;
	//GFX_STATUS = 0x06000000;
	//memset((void*)0x04000640,0,0x70);
	//REG_IF = 0x00080000;
///

	REG_POWERCNT = (u16)POWER_ALL_2D;
	REG_POWERCNT |= POWER_SWAP_LCDS;

#ifdef _LIBNDS_MAJOR_
	//fifoInit();
#else
	irqInit();
	irqEnable(IRQ_VBLANK);
#endif
	//REG_IME=0;

	argv=__argv;
	argc=__argc;
	argvToInstall=NULL;
	argvToInstallSize=0;
	memset(mydrive,0,sizeof(mydrive));

	DLDIToBoot=DLDIDATA;
	*(byte*)DLDINull=0xed;
	PrintfToDie=_consolePrintf;
	ret_menu9_callbackpre=NULL;
	ret_menu9_callback=NULL;

	consolePrint_callback=NULL;
	consolePrint2_callback=NULL;
	consolePrintOnce_callback=NULL;
	consolePrintOnce2_callback=NULL;

	consolePrintProgress_callback=NULL;
	consolePrintProgress2_callback=NULL;

	consoleClear_callback=NULL;
	consoleClear2_callback=NULL;
	consolePrintOnceEnd_callback=NULL;
	consolePrintOnceEnd2_callback=NULL;
	consoleStartProgress_callback=NULL;
	consoleStartProgress2_callback=NULL;
	consoleEndProgress_callback=NULL;
	consoleEndProgress2_callback=NULL;

	fpassarg=false;
	nocashMessageMain=1;
	nocashMessageSub=1;

	//InitVRAM
	//From 0.51m, you can use BG_BMP_RAM(2)/BG_BMP_RAM_SUB(2) (u16 [256*192]) to display image.
	EnableB15Main();
	EnableB15Sub();

#ifdef _LIBNDS_MAJOR_
	vramSetPrimaryBanks
#else
	vramSetMainBanks
#endif
	(
		VRAM_A_MAIN_BG_0x06000000,
		0x82, //VRAM_B_LCD, //MAIN_BG_0x06020000,
		//VRAM_B_MAIN_SPRITE_0x06400000,
		VRAM_C_SUB_BG_0x06200000,
		//VRAM_C_MAIN_BG_0x06020000,
		VRAM_D_SUB_SPRITE
	);
	vramSetBankE(VRAM_E_LCD);
	vramSetBankF(VRAM_F_LCD);
	vramSetBankG(VRAM_G_LCD);
	vramSetBankH(VRAM_H_LCD);
	vramSetBankI(VRAM_I_LCD);

	vramset(b15ptrMain,0,256*192);
	vramset(b15ptrSub,0,256*192);

	REG_BG0CNT = 0;
	REG_BG0CNT_SUB = 0;
	REG_BG1CNT = 0;
	REG_BG1CNT_SUB = 0;

	{
		REG_BG2CNT = BG_COLOR_256 | BG_RS_64x64 | BG_MAP_BASE(8) | BG_TILE_BASE(0) | BG_PRIORITY_1; // Tile16kb Map2kb(64x32)

		BG_PALETTE[(0*16)+0] = RGB15(0,0,0); // unuse (transparent)
		BG_PALETTE[(0*16)+1] = BG_PALETTE[(0*16)+2] = bgcolor | BIT(15); // BG color
		//BG_PALETTE[(0*16)+2] = RGB15(0,0,0) | BIT(15); // Shadow color
		BG_PALETTE[(0*16)+3] = RGB15(31,31,31) | BIT(15); // Text color

		u16 XDX=341; //(u16)((8.0/6)*0x100);
		u16 YDY=341; //(u16)((8.0/6)*0x100);

		REG_BG2PA = XDX;
		REG_BG2PB = 0;
		REG_BG2PC = 0;
		REG_BG2PD = YDY;

		REG_BG2X=-1;
		REG_BG2Y=-1;

		//consoleInit() is a lot more flexible but this gets you up and running quick
		_consoleInitDefault((u16*)(SCREEN_BASE_BLOCK(8)), (u16*)(CHAR_BASE_BLOCK(0)));
		_consoleClear();
		_consolePrint("Main screen init OK.\n\n");
	}

	{
		REG_BG3CNT = BG_BMP16_256x256 | BG_BMP_BASE(2) | BG_PRIORITY_0;

		REG_BG3PA = 1 << 8;
		REG_BG3PB = 0;
		REG_BG3PC = 0;
		REG_BG3PD = 1 << 8;
		REG_BG3X = 0;
		REG_BG3Y = 0;
	}
  
	{
		REG_BG2CNT_SUB = BG_COLOR_256 | BG_RS_64x64 | BG_MAP_BASE(8) | BG_TILE_BASE(0) | BG_PRIORITY_1; // Tile16kb Map2kb(64x32)

		if(__sig_mode[6]==1){
			BG_PALETTE_SUB[(0*16)+0] = RGB15(0,0,0); // unuse (transparent)
			BG_PALETTE_SUB[(0*16)+1] = BG_PALETTE_SUB[(0*16)+2] = bgcolor | BIT(15); // BG color
			//BG_PALETTE_SUB[(0*16)+2] = RGB15(0,0,0) | BIT(15); // Shadow color
			BG_PALETTE_SUB[(0*16)+3] = RGB15(31,31,31) | BIT(15); // Text color
		}else if(__sig_mode[6]==2){
			BG_PALETTE_SUB[(0*16)+0] = RGB15(0,0,0); // unuse (transparent)
			BG_PALETTE_SUB[(0*16)+1] = BG_PALETTE_SUB[(0*16)+2] = RGB15(31,31,31) | BIT(15); // BG color
			//BG_PALETTE_SUB[(0*16)+2] = RGB15(0,0,0) | BIT(15); // Shadow color
			BG_PALETTE_SUB[(0*16)+3] = RGB15(0,0,0) | BIT(15); // Text color
		}else{
			_consolePrint("\\xFEcMode? signature has to be \\x01 or \\x02.\n");die();
		}

		u16 XDX=341; //(u16)((8.0/6)*0x100);
		u16 YDY=341; //(u16)((8.0/6)*0x100);

		REG_BG2PA_SUB = XDX;
		REG_BG2PB_SUB = 0;
		REG_BG2PC_SUB = 0;
		REG_BG2PD_SUB = YDY;

		REG_BG2X_SUB=-1;
		REG_BG2Y_SUB=-1;

		//consoleInit() is a lot more flexible but this gets you up and running quick
		_consoleInitDefault2((u16*)(SCREEN_BASE_BLOCK_SUB(8)), (u16*)(CHAR_BASE_BLOCK_SUB(0)));
		_consoleClear2();
		_consolePrint2("Sub screen init OK.\n\n");
	}

	{
		REG_BG3CNT_SUB = BG_BMP16_256x256 | BG_BMP_BASE(2) | BG_PRIORITY_0;

		REG_BG3PA_SUB = 1 << 8;
		REG_BG3PB_SUB = 0;
		REG_BG3PC_SUB = 0;
		REG_BG3PD_SUB = 1 << 8;
		REG_BG3X_SUB = 0;
		REG_BG3Y_SUB = 0;
	}

	DisableB15Main();
	DisableB15Sub();

	if(sizeof(TransferRegionZ)>256){
		_consolePrint("TransferRegionZ 256bytes limit exceeded. Compilation was bad.\n");die();
	}

	*myname=0;
	strcpy(mypath,"/");
	*argname=0;
	strcpy(argpath,"/");

	_consolePrint2("Checking ARGV...\n");
	if(*(vu32*)0x02fffe70==0x5f617267){
		*(vu32*)0x02fffe70=0; //hide it from libfat to avoid "chdir to myself"
		char *arg=argv[0];
		while(*arg&&*arg!='/')arg++;
		if(!*arg){
			_consolePrint2("warn: no '/' in argv[0], mypath is discarded.\n"); //perhaps that's memory garbage. need to ignore.
		}else{
			strcpy(myname,arg);
			strcpy(mypath,arg);
			int i=strlen(mypath)+1;
			for(;i>0;i--)if(mypath[i-1]=='/'){mypath[i]=0;break;}

			if(argc>1){
				char *arg=argv[1];
				while(*arg&&*arg!='/')arg++;
				if(!*arg){
					_consolePrint2("warn: no '/' in argv[1], argpath is discarded.\n"); //perhaps that's memory garbage. need to ignore.
				}else{
					strcpy(argname,arg);
					strcpy(argpath,arg);
					int i=strlen(argpath)+1;
					for(;i>0;i--)if(argpath[i-1]=='/'){argpath[i]=0;break;}
				}
			}
		}

/*
		_consolePrintf2("argv: 0x%08x\n%s\n",*(vu32*)0x02fffe74,*(char**)0x02fffe74);
		char *arg=*(char**)0x02fffe74;
		while(*arg&&*arg!='/')arg++;
		if(!*arg){
			_consolePrint2("warn: no '/' in argv[0], mypath is discarded.\n");
		}else{
			strcpy(myname,arg);
			strcpy(mypath,arg);
			int i=strlen(mypath)+1;
			for(;i>0;i--)if(mypath[i-1]=='/'){mypath[i]=0;break;}

			if(strlen(*(char**)0x02fffe74)+1<*(vu32*)0x02fffe78){
				arg=arg+strlen(arg)+1;
				while(*arg&&*arg!='/')arg++;
				if(!*arg){
					//_consolePrint2("warn: no '/' in argv[0], mypath is discarded.\n");
				}else{
					strcpy(argname,arg);
					strcpy(argpath,arg);
					int i=strlen(argpath)+1;
					for(;i>0;i--)if(argpath[i-1]=='/'){argpath[i]=0;break;}
					_consolePrintf2("%s\n",argname);
				}
			}
		}
		_consolePrint2("\n");
*/
	}
#if 0 //unneeded with modified ds_arm9_crt0.s
#ndef _LIBNDS_MAJOR_
	if(!mypath[1]&&*(vu32*)0x027ffe70==0x5f617267){
		_consolePrint2("Falling back to 0x027xxxxx...\n");
		*(vu32*)0x027ffe70=0; //hide it from libfat to avoid "chdir to myself"
/*
		_consolePrintf2("argv: 0x%08x\n%s\n",*(vu32*)0x027ffe74,*(char**)0x027ffe74);
		char *arg=*(char**)0x027ffe74;
		while(*arg&&*arg!='/')arg++;
		if(!*arg){
			_consolePrint2("warn: no '/' in argv[0], mypath is discarded.\n");
		}else{
			strcpy(myname,arg);
			strcpy(mypath,arg);
			int i=strlen(mypath)+1;
			for(;i>0;i--)if(mypath[i-1]=='/'){mypath[i]=0;break;}

			if(strlen(*(char**)0x027ffe74)+1<*(vu32*)0x027ffe78){
				arg=arg+strlen(arg)+1;
				while(*arg&&*arg!='/')arg++;
				if(!*arg){
					//_consolePrint2("warn: no '/' in argv[0], mypath is discarded.\n");
				}else{
					strcpy(argname,arg);
					strcpy(argpath,arg);
					int i=strlen(argpath)+1;
					for(;i>0;i--)if(argpath[i-1]=='/'){argpath[i]=0;break;}
					_consolePrintf2("%s\n",argname);
				}
			}
		}
		_consolePrint2("\n");
*/
	}
#endif


	_consolePrint2("Waiting for ARM7... ");
	for(;IPCZ->cmd;); //Wait ARM7 initialization
	_consolePrint2("Start.\n\n");
	SCDS_SetSDHCModeForDSTT();
	InitializeKeyTable();
	//fifoSendValue32(FIFO_PM, PM_REQ_SLEEP_DISABLE);
	Main();

	_consolePrint("Program real Main() end. Finalization seems to have failed.\n");
	die();
	return 0; //unreachable
}

void vramcpy(void* dst, const void* src, int len){ //use align2 macro then divide by 2
	u16* dst16 = (u16*)dst;
	u16* src16 = (u16*)src;
	for(;len>0;len--)*dst16++=*src16++;
}

void vramset(void* dst, const u16 n, int len){ //use align2 macro then divide by 2
	u16* dst16 = (u16*)dst;
	for(;len>0;len--)*dst16++=n;
}

unsigned int readAddr(void *mem){
	return *((unsigned int*)mem);
}

unsigned int readAddr24(void *mem){
	return (*((unsigned int*)mem))&0xffffff;
}

unsigned short readAddr16(void *mem){
	return (*((unsigned short*)mem));
}

unsigned char readAddr8(void *mem){
	return (*((unsigned char*)mem));
}

unsigned long long int readAddr64(void *mem){
	return (*((unsigned long long int*)mem));
}

void writeAddr(void *mem, const unsigned int value){
	*((unsigned int*)mem) = value;
}
/*
void writeAddr24(void *mem, const unsigned int value){
	*((unsigned int*)mem) = value&0xffffff;
}
*/

void writeAddr16(void *mem, const unsigned short value){
	*((unsigned short*)mem) = value;
}

void writeAddr8(void *mem, const unsigned char value){
	*((unsigned char*)mem) = value;
}

void writeAddr64(void *mem, const unsigned long long int value){
	*((unsigned long long int*)mem) = value;
}

unsigned int read32(const void *p){
	const unsigned char *x=(const unsigned char*)p;
	return x[0]|(x[1]<<8)|(x[2]<<16)|(x[3]<<24);
}

unsigned int read24(const void *p){
	const unsigned char *x=(const unsigned char*)p;
	return x[0]|(x[1]<<8)|(x[2]<<16);
}

unsigned short read16(const void *p){
	const unsigned char *x=(const unsigned char*)p;
	return x[0]|(x[1]<<8);
}

unsigned char read8(const void *p){
	const unsigned char *x=(const unsigned char*)p;
	return x[0];
}

unsigned long long int read64(const void *p){
	const unsigned char *x=(const unsigned char*)p;
	return x[0]|(x[1]<<8)|(x[2]<<16)|(x[3]<<24)|( (unsigned long long int)(x[4]|(x[5]<<8)|(x[6]<<16)|(x[7]<<24)) <<32);
}

void write32(void *p, const unsigned int n){
	unsigned char *x=(unsigned char*)p;
	x[0]=n&0xff,x[1]=(n>>8)&0xff,x[2]=(n>>16)&0xff,x[3]=(n>>24)&0xff;
}

void write24(void *p, const unsigned int n){
	unsigned char *x=(unsigned char*)p;
	x[0]=n&0xff,x[1]=(n>>8)&0xff,x[2]=(n>>16)&0xff;
}

void write16(void *p, const unsigned short n){
	unsigned char *x=(unsigned char*)p;
	x[0]=n&0xff,x[1]=(n>>8)&0xff;
}

void write8(void *p, const unsigned char n){
	unsigned char *x=(unsigned char*)p;
	x[0]=n&0xff;
}

void write64(void *p, const unsigned long long int n){
	unsigned char *x=(unsigned char*)p;
	x[0]=n&0xff,x[1]=(n>>8)&0xff,x[2]=(n>>16)&0xff,x[3]=(n>>24)&0xff,
	x[4]=(n>>32)&0xff,x[5]=(n>>40)&0xff,x[6]=(n>>48)&0xff,x[7]=(n>>56)&0xff;
}

void die(){
	int f=0,g=0,h=1;
	struct stat st;
	char *file="/moonshl2/resetmse/xxxx.nds";
// #ifdef _LIBNDS_MAJOR_
	u8 *bootstub=(u8*)0x02ff4000;
	//extern u8 *fake_heap_end; //DKPr28:0x023f4000 DKPr23:0x023ff000 I should definitely kill support in r23.
// #endif

	ret_menu9_callback=NULL;
	memcpy(file+19,(char*)DLDIDATA+ioType,4);
	if(getPartitionHandle()&&!stat(file,&st)&&!(st.st_mode&S_IFDIR)&&st.st_size>0)f=1;
	if(*(u64*)bootstub==0x62757473746F6F62ULL)g=1;

retry:
	for(swiWaitForVBlank();;swiWaitForVBlank())
		if(!IPCZ->keysheld)break;
	PrintfToDie("Press A to shutdown.\n");
	if(h)PrintfToDie("Press B for NDS firmware.\n");
	if(f)PrintfToDie("Press X for %s.\n",file);
	if(g)PrintfToDie("Press Y for libnds bootstub.\n");
	for(swiWaitForVBlank();;swiWaitForVBlank()){
		if(KEY_A&IPCZ->keysheld)disc_unmount(),IPCZ->cmd=Shutdown;
		if(h&&(KEY_B&IPCZ->keysheld))
			{returnDSMenu();h=0;goto retry;}
		if(f&&(KEY_X&IPCZ->keysheld))
			if(!BootNDSROM(file)){f=0;goto retry;}
		if(g&&(KEY_Y&IPCZ->keysheld))
			disc_unmount(),(*(type_void*)(bootstub+0x08))();
	}
}

//routines from moonshell
void Unicode_Copy(UnicodeChar *tag,const UnicodeChar *src){
  while(*src!=0){*tag=*src;tag++; src++;}
  *tag=(UnicodeChar)0;
}
/*
int Unicode_Length(const UnicodeChar *src){
  int ret=0;
  while(*src!=0){ret++;src++;}
  return ret;
}
*/

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

	if(pPathAlias){
		if(SplitPos<=1){
			pPathAlias[0]='/';
			pPathAlias[1]=0;
		}else{
			u32 idx=0;
			for(;idx<SplitPos-1;idx++){
				pPathAlias[idx]=pFullPathAlias[idx];
			}
			pPathAlias[SplitPos-1]=0;
		}
	}
	if(pFilenameAlias)strcpy(pFilenameAlias,&pFullPathAlias[SplitPos]);
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

	if(pPathUnicode){
		if(SplitPos<=1){
			pPathUnicode[0]=(UnicodeChar)'/';
			pPathUnicode[1]=0;
		}else{
			u32 idx=0;
			for(;idx<SplitPos-1;idx++){
				pPathUnicode[idx]=pFullPathUnicode[idx];
			}
			pPathUnicode[SplitPos-1]=0;
		}
	}
	if(pFilenameUnicode)Unicode_Copy(pFilenameUnicode,&pFullPathUnicode[SplitPos]);
}

char* myfgets(char *buf,int n,FILE *fp){ //accepts LF/CRLF
	char *ret=fgets(buf,n,fp);
	if(!ret)return NULL;
	if(strlen(buf)&&buf[strlen(buf)-1]=='\n')buf[strlen(buf)-1]=0;
	if(strlen(buf)&&buf[strlen(buf)-1]=='\r')buf[strlen(buf)-1]=0;
	return ret;
}

void rm_rf(char *target){ // last byte has to be '/' *** target itself won't be erased
	DIR_ITER *dp;
	if(!target)return;
	char *targetfile=target+strlen(target);
	struct stat st;
	dp=mydiropen(target);
	while(!mydirnext(dp,targetfile,&st)){
		if(!strcmp(targetfile,".")||!strcmp(targetfile,".."))continue;
		if(st.st_mode&S_IFDIR){
			strcat(targetfile,"/");
			rm_rf(target);
			targetfile[strlen(targetfile)-1]=0;
			unlink(target);
		}else unlink(target);
	}
	mydirclose(dp);
	*targetfile=0; //genjo fukki!
}

void mkpath(char *path){ // last byte has to be '/' (after final '/' will be ignored)
	int i=2;
	char dir[768];
	if(!path||!*path||!path[1])return;
	for(memset(dir,0,768);i<strlen(path);i++)
		if(path[i]=='/')strncpy(dir,path,i),mkdir(dir,0777),i++;
}

int filelength(int fd){ //Windows compatible
	struct stat st;
	if(fstat(fd,&st))return 0;
	return st.st_size;
}

int copy(const char *old, const char *_new){
	FILE *in=fopen(old,"rb");
	if(!in)return 1;
	int size=filelength(fileno(in));
	FILE *out=fopen(_new,"r+b");
	if(out){
		int osize=filelength(fileno(out));
		if(osize==size)goto copy_progress; //use r+b handle to avoid cluster reallocation.
		fclose(out);
	}
	out=fopen(_new,"wb");
	if(!out){fclose(in);return 2;}
copy_progress:;
	int readlen,cur=0;
	_consoleStartProgress2();
	while((readlen=fread(libprism_buf,1,BUFLEN,in))>0){
		cur+=readlen;
		fwrite(libprism_buf,1,readlen,out);
		_consolePrintProgress2("Copying",cur,size);
	}
	_consoleEndProgress2();
	fclose(out);
	fclose(in);
	return 0;
}

char *findpath(int argc, char **argv, const char *name){
	int i=0;
	for(;i<argc;i++){
		strcpy(libprism_name,argv[i]);
		if(libprism_name[strlen(libprism_name)-1]!='/')strcat(libprism_name,"/");
		strcat(libprism_name,name);
		if(!access(libprism_name,0))return libprism_name;
	}
	libprism_name[0]=0;
	return NULL;
}

void installargv(u8 *top, void *store){
	if(top)*(vu32*)(top+0x70)=0x5f617267;
	if(top)*(vu32*)(top+0x74)=(u32)store;
	//vramcpy(store,nds,strlen(nds)+1);
	vramcpy((u16*)store,argvToInstall,align2(argvToInstallSize)/2);
	if(top)*(vu32*)(top+0x78)=argvToInstallSize;
#if 0
	strcpy((char*)store,"fat:");
	strcpy((char*)store+4,nds);
	*(vu32*)(top+0x78)=strlen(nds)+5;
	if(fpassarg){
		char *p=(char*)store+strlen(nds)+5;
		strcpy(p,"fat:");
		strcpy(p+4,argname);
		*(vu32*)(top+0x78)+=strlen(argname)+5;
	}
#endif
}

#if 0
struct __argv {
	int argvMagic;		//!< argv magic number, set to 0x5f617267 ('_arg') if valid 
	char *commandLine;	//!< base address of command line, set of null terminated strings ///argvToInstall///
	int length;		//!< total length of command line ///argvToInstallSize///
	int argc;		//!< internal use, number of arguments
	char **argv;		//!< internal use, argv pointer
};
#endif

char *parseargv(const char *str){ //str is memory obtained by fread argv file.
	if(argvToInstall)free(argvToInstall);
	argvToInstallSize=0;
	if(!str&&!*str)return NULL; //this will cause program to crash, be careful bah
	int i,j;
	char *seps="\n\r\t #";

	//char c;
	//1st: get size
	int quote=0;
	for(i=0;i<strlen(str)&&strchr(seps,str[i]);i++) //skip continuous separater
		if(str[i]=='#')
			for(;i<strlen(str)&&str[i]!='\n';i++);

	int start=i;
	for(i=start;i<strlen(str);i++){
		if(strchr(seps,str[i])&&(str[i]!=' '||!quote)){ //malformed argv file will cause bug...
			for(;i<strlen(str)&&strchr(seps,str[i]);i++) //skip continuous separater
				if(str[i]=='#')
					for(;i<strlen(str)&&str[i]!='\n';i++);
			argvToInstallSize++; //counted as one separator.
		}
		if(str[i]=='\"'){quote^=1;continue;}
		argvToInstallSize++;
	}
	if(!strchr(seps,str[strlen(str)-1]))argvToInstallSize++;

	if(!argvToInstallSize)return NULL;
	argvToInstall=(char*)malloc(argvToInstallSize+4); //4==strlen("fat:")
	if(!argvToInstall){argvToInstallSize=0;return NULL;}

	//2nd: make
	quote=0;
	for(i=start,j=0;i<strlen(str);i++){
		if(strchr(seps,str[i])&&(str[i]!=' '||!quote)){ //malformed argv file will cause bug...
			for(;i<strlen(str)&&strchr(seps,str[i]);i++) //skip continuous separater
				if(str[i]=='#')
					for(;i<strlen(str)&&str[i]!='\n';i++);
			argvToInstall[j++]=0; //counted as one separator.
		}else{
			if(!j && str[i]=='/'){ //force to add fat: to argv[0]
				strcpy(argvToInstall,mydrive);
				argvToInstall[strlen(argvToInstall)-1]=0; // "fat:/(/)" -> "fat:(/)"
				argvToInstallSize+=strlen(argvToInstall);
				j=strlen(argvToInstall);
			}
		}
		if(str[i]=='\"'){quote^=1;continue;}
		argvToInstall[j++]=str[i];
	}
	if(!strchr(seps,str[strlen(str)-1]))argvToInstall[j++]=0;
	return argvToInstall;
}

char *makeargv(const char *str){ //make it in faster way...
	if(argvToInstall)free(argvToInstall);
	argvToInstallSize=0;
	if(!str&&!*str)return NULL; //this will cause program to crash, be careful bah
	char *seps="\n\r\t";

	argvToInstallSize=strlen(str);
	if(!strchr(seps,str[strlen(str)-1]))argvToInstallSize++;

	if(!argvToInstallSize)return NULL;
	argvToInstall=(char*)malloc(argvToInstallSize+4); //4==strlen("fat:")
	if(!argvToInstall){argvToInstallSize=0;return NULL;}

	int i=0,j=0;
	if(str[0]=='/'){
		strcpy(argvToInstall,mydrive);
		argvToInstall[strlen(argvToInstall)-1]=0; // "fat:/(/)" -> "fat:(/)"
		argvToInstallSize+=strlen(argvToInstall);
		j=strlen(argvToInstall);
	}
	strcpy(argvToInstall+j,str);
	for(;i<strlen(str);i++)
		if(strchr(seps,argvToInstall[j+i]))argvToInstall[j+i]=0;
	if(!strchr(seps,str[strlen(str)-1]))argvToInstall[j+i]=0;
	return argvToInstall;
}

char *processlinker(const char *name){
	char buf[768],key[768];
	if(!name)return NULL;
	if(name[strlen(name-1)]!='=')return (char*)name;
	strcpy(key,name),key[strlen(name)-1]=0;
	if(!strcpy_safe(buf,findpath(6,(char*[]){"/","/_dstwoplug/","/ismartplug/","/_iMenu/_ini/","/_plugin_/",mypath},"linkpath.ini")))return NULL;
	ini_gets("linkpath",key,"",libprism_name,768,buf);
	return libprism_name[0]?libprism_name:NULL;
}

char *strcpy_safe(char *s1, const char *s2){
	if(!s1||!s2)return NULL;
	return strcpy(s1,s2);
}

char *getextname(char *s){
	int i;
	if(!s)return NULL;
	if(!*s)return "";
	for(i=strlen(s)-1;i>0;i--){
		if(s[i]=='/'){i++;break;}
		if(s[i]=='.')break;
	}
	return s+i;
}

char *getfilename(char *s){
	int i;
	if(!s)return NULL;
	if(!*s)return "";
	for(i=strlen(s);i>0;i--){
		if(s[i-1]=='/'){break;}
		//if(s[i]=='.')break;
	}
	return s+i;
}

#define MAX_HOUR 23
#define MAX_MINUTE 59
#define MAX_SECOND 60

#define MAX_MONTH 11
#define MIN_MONTH 0
#define MAX_DAY 31
#define MIN_DAY 1

int UTCToDateTime(time_t epochTime, u16 *date, u16 *time){
	struct tm timeParts;

	localtime_r(&epochTime, &timeParts);

	if(validateTM(&timeParts))return -1;
	
	if(date)*date=
		(((timeParts.tm_year - 80) & 0x7F) <<9) |	// Adjust for MS-FAT base year (1980 vs 1900 for tm_year)
		(((timeParts.tm_mon + 1) & 0xF) << 5) |
		(timeParts.tm_mday & 0x1F);

	if(time)*time=
		((timeParts.tm_hour & 0x1F) << 11) |
		((timeParts.tm_min & 0x3F) << 5) |
		((timeParts.tm_sec >> 1) & 0x1F);

	return 0;
}

int validateTM(struct tm *timeParts){
	if(!timeParts)return -1;
	if ((timeParts->tm_mon < MIN_MONTH) || (timeParts->tm_mon > MAX_MONTH)) return -1;
	if ((timeParts->tm_mday < MIN_DAY) || (timeParts->tm_mday > MAX_DAY)) return -1;
	if ((timeParts->tm_hour < 0) || (timeParts->tm_hour > MAX_HOUR))	return -1;
	if ((timeParts->tm_min < 0) || (timeParts->tm_min > MAX_MINUTE)) return -1;
	if ((timeParts->tm_sec < 0) || (timeParts->tm_sec > MAX_SECOND)) return -1;
	return 0;
}

bool slot2nds(){
	if(IPCZ->NDSType<NDSi){
		ram_lock();
		disc_unmount();
		sysSetCartOwner(BUS_OWNER_ARM7);
		bootMoonlight(0x080000c0);
		while(1);
	}
	return false;
}

bool slot2gba(int screen){
	if(IPCZ->NDSType<NDSi){
		disc_unmount();
		sysSetBusOwners(BUS_OWNER_ARM7,BUS_OWNER_ARM7);
		REG_POWERCNT=screen?1:(POWER_SWAP_LCDS|1);
		NotifyARM7(Slot2GBA);
		while(1);
	}
	return false;
}

bool jumpBootStub(){
// #ifdef _LIBNDS_MAJOR_ //r23 modified crt0 removes this limitation.
	u8 *bootstub=(u8*)0x02ff4000;
	if(*(u64*)bootstub==0x62757473746F6F62ULL){
		disc_unmount();
		(*(type_void*)(bootstub+0x08))();
	}
// #endif
	return false;
}

bool isHomebrew(u8 *head){ //at least 0xc0 bytes has to be allocated.
	char ID[5];
	ID[4]=0;
	ID[0]=head[0x0c],ID[1]=head[0x0d],ID[2]=head[0x0e],ID[3]=head[0x0f];
	if(
		!strcmp(ID,"####")||!strcmp(ID,"PASS")||	//usual homebrew
		!strcmp(ID,"ENG0")||!strcmp(ID,"JPN0")||	//SuperCard
		!strcmp(ID,"0000")||				//PPSEDS
		!strcmp(ID,"GBAL")				//GBALdr
	)return true;
	if(ID[0]==0x3d&&ID[1]==0x84&&ID[2]==0x82&&ID[3]==0x0a)return true;

	ID[0]=head[0xac],ID[1]=head[0xad],ID[2]=head[0xae],ID[3]=head[0xaf];
	if(!strcmp(ID,"PASS"))return true;

	if(read32(head+0x20)<0x4000)return true;		//if ARM9_offset < 0x4000
	if(read32(head+0x34)>=0x037f8000)return true;	//if ARM7_exec >= 0x037f8000
	return false;
}

int GetFirmwareVersion(){
	switch(IPCZ->fwchksum){
		case 0x2c7a: return 1;
		case 0xe0ce: return 2;
		case 0xbfba: return 3;
		case 0xdfc7: return 4;
		case 0x73b3: return 5;
		case 0xe843: return 6;
		case 0x0f1f: return 7;

		case 0xf96d: return FW_iQue;
		case 0x74f0: return FW_Korean;
	}
	return 0;
}

int fexists(const char *path){
	struct stat st;
	if(stat(path,&st))return 0;
	return (st.st_mode&S_IFDIR)?2:1;
}

void NotifyARM7(u32 c){IPCZ->cmd=c;}
void CallARM7(u32 c){for(IPCZ->cmd=c;IPCZ->cmd;)swiWaitForVBlank();}

#if 0
void NotifyARM7(u32 argc, ...){
	va_list list;
	va_start(list,argc);
	for(i=0;i<argc;i++){
		IPCZ->args[i]=va_arg(list, u32);
	}
	va_end(list);
	IPCZ->argc=argc;
}
void CallARM7(u32 argc, ...){
	va_list list;
	va_start(list,argc);
	for(i=0;i<argc;i++){
		IPCZ->args[i]=va_arg(list, u32);
	}
	va_end(list);
	IPCZ->argc=argc;
	for(;IPCZ->argc;)swiWaitForVBlank();
}
#endif

int strchrindex(const char *s, const int c, const int idx){
	const char *ret=strchr(s+idx,c);
	if(!ret)return -1;
	return ret-s;
}

int strstrindex(const char *s, const char *c, const int idx){
	const char *ret=strstr(s+idx,c);
	if(!ret)return -1;
	return ret-s;
}

void changefileext(char *fn, const char *ext){
	if(!fn||!ext||!*ext)return;
	strcpy(getextname(fn),ext);
}

int GetRunningMode(){
#if 0
//ndef _LIBNDS_MAJOR_
	return 0;
#endif
	//if(*(vu32*)0x023ffe2c==*(vu32*)0x027ffe2c)return 0;
	if(!*(vu32*)0x4004000)return 0;
	return 1;

	//3DS?
	//return 2;
}

bool readFrontend(char *target){
	if(*argname){strcpy(target,argname);return true;}

	//strcpy(target,mydrive);target[strlen(target)-1]=0;
	strcpy(target,"");
	FILE *f=fopen("/loadfile.dat","rb");
	if(f){
		int i=0;
		myfgets((char*)libprism_buf,768,f);
		fclose(f);
		unlink("/loadfile.dat");
		if(!memcmp((char*)libprism_buf,"//",2))i+=1;
		if(!memcmp((char*)libprism_buf,"/./",3))i+=2; //menudo dir handling is buggy?
		strcat(target,(char*)libprism_buf+i);
		return true;
	}
	f=fopen("/plgargs.dat","rb");
	if(f){
		//int i=0;
		myfgets((char*)libprism_buf,768,f);
		myfgets((char*)libprism_buf,768,f); //second line
		fclose(f);
		unlink("/plgargs.dat");
		//if(!memcmp((char*)libprism_buf,"//",2))i+=1;
		//if(!memcmp((char*)libprism_buf,"/./",3))i+=2;
		strcat(target,(char*)libprism_buf); //+i);
		return true;
	}
	f=fopen("/moonshl2/extlink.dat\0in\0","r+b"); //make it modifiable externally
	if(f){
		TExtLinkBody extlink;
		memset(&extlink,0,sizeof(TExtLinkBody));
		fread(&extlink,1,sizeof(TExtLinkBody),f);
		if(extlink.ID!=ExtLinkBody_ID){fclose(f);return false;}
		ucs2tombs(target+strlen(target),extlink.DataFullPathFilenameUnicode);
		rewind(f);
		fwrite("____",1,4,f);
		fclose(f);
		return true;
	}
	return false; //it is your choice to boot GUI or to halt.
}

bool writeFrontend(const int frontend_type, const char *exe, const char *target){ //kinda void...
#if 0
	if(frontend_type==FRONTEND_ARGV){
		strcpy(argname,target);
		fpassarg=true;
		return true;
	}
#endif
	if(frontend_type==FRONTEND_LOADFILE){
		FILE *f=fopen("/loadfile.dat","wb");
		if(f){
			char sfn[768];
			getsfnlfn(target,sfn,NULL);
			fputs(strchr(target,'/'),f);
			fputs("\n",f);
			fputs(sfn,f);
			fputs("\n",f);
			fclose(f);
			return true;
		}
	}
	if(frontend_type==FRONTEND_PLGARGS){
		FILE *f=fopen("/plgargs.dat","wb");
		if(f){
			fputs(exe,f);
			fputs("\n",f);
			fputs(strchr(target,'/'),f);
			fputs("\n",f);
			fclose(f);
			return true;
		}
	}
	if(frontend_type==FRONTEND_EXTLINK){
		FILE *f=fopen("/moonshl2/extlink.dat\0out\0","wb");
		if(f){
			TExtLinkBody extlink;
			memset(&extlink,0,sizeof(TExtLinkBody));
			extlink.ID=ExtLinkBody_ID;
			getsfnlfn(target,extlink.DataFullPathFilenameAlias,extlink.DataFullPathFilenameUnicode);
			SplitItemFromFullPathAlias(extlink.DataFullPathFilenameAlias,extlink.DataPathAlias,extlink.DataFilenameAlias);
			SplitItemFromFullPathUnicode(extlink.DataFullPathFilenameUnicode,extlink.DataPathUnicode,extlink.DataFilenameUnicode);

			getsfnlfn(exe,extlink.NDSFullPathFilenameAlias,extlink.NDSFullPathFilenameUnicode);
			SplitItemFromFullPathAlias(extlink.NDSFullPathFilenameAlias,extlink.NDSPathAlias,extlink.NDSFilenameAlias);
			SplitItemFromFullPathUnicode(extlink.NDSFullPathFilenameUnicode,extlink.NDSPathUnicode,extlink.NDSFilenameUnicode);
			fwrite(&extlink,1,sizeof(TExtLinkBody),f);
			fclose(f);
			return true;
		}
	}
	return false;
}

void clearExtlink(){ //altloader should call this...
	FILE *f=fopen("/moonshl2/extlink.dat\0in\0","r+b"); //no truncate
	if(f){
		fwrite("____",1,4,f);
		fclose(f);
	}
}

int getExtlinkWrapperHBMode(){
	return ini_getl("mshl2wrap","hbmode",0,"/moonshl2/extlink/mshl2wrap.ini");
}

void getExtlinkWrapperLoaderName(char *loader){
	char dldiid[5];
	memcpy(dldiid,DLDIDATA+ioType,4);
	dldiid[4]=0;
	ini_gets("mshl2wrap",dldiid,MOONSHELL,loader,256*3,"/moonshl2/extlink/mshl2wrap.ini");
}

void nocashMessageSafe(const char *s){
#ifdef _LIBNDS_MINOR_
	const int LENGTH=112;
	int i=0,c;
	for(;i+LENGTH<strlen(s);i+=LENGTH){
		c=s[i+LENGTH];
		((char*)s)[i+LENGTH]=0;
		nocashMessage(s+i);
		((char*)s)[i+LENGTH]=c;
	}
	nocashMessage(s+i);
#endif
}
