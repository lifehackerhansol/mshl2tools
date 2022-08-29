#include "libprism.h"
#include <nds/registers_alt.h>  //to maintain r23

extern void Main();
extern const u16 bgcolor;

u8 libprism_buf[65536];

char myname[768],mypath[768];
char libprism_name[768];

byte *DLDIToBoot;
type_printf PrintfToDie;
type_u8p ret_menu9_callbackpre;
type_u8p ret_menu9_callback;

u16 *b15ptrMain=(u16*)0x06008000;
u16 *b15ptrSub=(u16*)0x06208000;

char __sig_mode[8]="\xFE""cMode\x01";

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

int main(){
	POWER_CR = (u16)POWER_ALL_2D;
	POWER_CR |= POWER_SWAP_LCDS;

	SetARM9_REG_WaitCR();

#ifdef _LIBNDS_MAJOR_
	//fifoInit();
#else
	irqInit();
	irqEnable(IRQ_VBLANK);
#endif
	REG_IME=0;

	DLDIToBoot=DLDIDATA;
	PrintfToDie=_consolePrintf;
	ret_menu9_callbackpre=NULL;
	ret_menu9_callback=NULL;

	//InitVRAM
	//From 0.51m, you can use BG_BMP_RAM(2)/BG_BMP_RAM_SUB(2) (u16 [256*192]) to display image.
	EnableB15Main();
	EnableB15Sub();
  
	vramSetMainBanks(
		VRAM_A_MAIN_BG_0x06000000,
		VRAM_B_MAIN_BG_0x06020000,
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

	{
		BG2_CR = BG_COLOR_256 | BG_RS_64x64 | BG_MAP_BASE(8) | BG_TILE_BASE(0) | BG_PRIORITY_3; // Tile16kb Map2kb(64x32)

		BG_PALETTE[(0*16)+0] = RGB15(0,0,0); // unuse (transparent)
		BG_PALETTE[(0*16)+1] = BG_PALETTE[(0*16)+2] = bgcolor | BIT(15); // BG color
		//BG_PALETTE[(0*16)+2] = RGB15(0,0,0) | BIT(15); // Shadow color
		BG_PALETTE[(0*16)+3] = RGB15(31,31,31) | BIT(15); // Text color

		u16 XDX=(u16)((8.0/6)*0x100);
		u16 YDY=(u16)((8.0/6)*0x100);

		BG2_XDX = XDX;
		BG2_XDY = 0;
		BG2_YDX = 0;
		BG2_YDY = YDY;

		BG2_CX=-1;
		BG2_CY=-1;

		//consoleInit() is a lot more flexible but this gets you up and running quick
		_consoleInitDefault((u16*)(SCREEN_BASE_BLOCK(8)), (u16*)(CHAR_BASE_BLOCK(0)));
		_consoleClear();
		_consolePrintf("Main screen init OK.\n\n");
	}

	{
		BG3_CR = BG_BMP16_256x256 | BG_BMP_BASE(2) | BG_PRIORITY_1;

		BG3_YDX = 0;
		BG3_XDX = 1 << 8;
		BG3_XDY = 0;
		BG3_YDY = 1 << 8;
		BG3_CX = 0;
		BG3_CY = 0;
	}
  
	{
		SUB_BG2_CR = BG_COLOR_256 | BG_RS_64x64 | BG_MAP_BASE(8) | BG_TILE_BASE(0) | BG_PRIORITY_3; // Tile16kb Map2kb(64x32)

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
			_consolePrintf("\\xFEcMode? signature has to be \\x01 or \\x02.\n");die();
		}

		u16 XDX=(u16)((8.0/6)*0x100);
		u16 YDY=(u16)((8.0/6)*0x100);

		SUB_BG2_XDX = XDX;
		SUB_BG2_XDY = 0;
		SUB_BG2_YDX = 0;
		SUB_BG2_YDY = YDY;

		SUB_BG2_CX=-1;
		SUB_BG2_CY=-1;

		//consoleInit() is a lot more flexible but this gets you up and running quick
		_consoleInitDefault2((u16*)(SCREEN_BASE_BLOCK_SUB(8)), (u16*)(CHAR_BASE_BLOCK_SUB(0)));
		_consoleClear2();
		_consolePrintf2("Sub screen init OK.\n\n");
	}

	{
		SUB_BG3_CR = BG_BMP16_256x256 | BG_BMP_BASE(2) | BG_PRIORITY_1;

		SUB_BG3_YDX = 0;
		SUB_BG3_XDX = 1 << 8;
		SUB_BG3_XDY = 0;
		SUB_BG3_YDY = 1 << 8;
		SUB_BG3_CX = 0;
		SUB_BG3_CY = 0;
	}

	DisableB15Main();
	DisableB15Sub();

	if(sizeof(TransferRegionZ)>256){
		_consolePrintf("TransferRegionZ 256bytes limit exceeded.\n");die();
	}

	*myname=0;
	strcpy(mypath,"/");

	if(*(vu32*)0x023ffe70==0x5f617267){
		*(vu32*)0x023ffe70=0; //hide it from libfat
		_consolePrintf2("argv: 0x%08x\n%s\n",*(vu32*)0x023ffe74,*(char**)0x023ffe74);
		char *arg=*(char**)0x023ffe74;
		while(*arg&&*arg!='/')arg++;
		if(!*arg){
			_consolePrintf2("warn: no '/' in argv[0], mypath is discarded.\n");
		}else{
			strcpy(myname,arg);
			strcpy(mypath,arg);
			int i=strlen(mypath)+1;
			for(;i>0;i--)if(mypath[i-1]=='/'){mypath[i]=0;break;}
		}
		_consolePrint2("\n");
	}

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
	return (*((unsigned int*)mem))&0xffff;
}

unsigned char readAddr8(void *mem){
	return (*((unsigned int*)mem))&0xff;
}

void writeAddr(void *mem, const unsigned int value){
	*((unsigned int*)mem) = value;
}

void writeAddr24(void *mem, const unsigned int value){
	*((unsigned int*)mem) = value&0xffffff;
}

void writeAddr16(void *mem, const unsigned int value){
	*((unsigned int*)mem) = value&0xffff;
}

void writeAddr8(void *mem, const unsigned int value){
	*((unsigned int*)mem) = value&0xff;
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

void write32(void *p, const unsigned int n){
	unsigned char *x=(unsigned char*)p;
	x[0]=n&0xff,x[1]=(n>>8)&0xff,x[2]=(n>>16)&0xff,x[3]=(n>>24)&0xff;
}

void write24(void *p, const unsigned int n){
	unsigned char *x=(unsigned char*)p;
	x[0]=n&0xff,x[1]=(n>>8)&0xff,x[2]=(n>>16)&0xff;
}

void write16(void *p, const unsigned int n){
	unsigned char *x=(unsigned char*)p;
	x[0]=n&0xff,x[1]=(n>>8)&0xff;
}

void write8(void *p, const unsigned int n){
	unsigned char *x=(unsigned char*)p;
	x[0]=n&0xff;
}

void die(){
	int f=0;
	struct stat st;
	char *file="/moonshl2/resetmse/xxxx.nds";
#ifdef _LIBNDS_MAJOR_
	extern char *fake_heap_end; //DKPr28:0x023f4000 DKPr23:0x023ff000 I should definitely kill support in r23.
#endif

	ret_menu9_callback=NULL;
	memcpy(file+19,(char*)DLDIDATA+ioType,4);
	if(_FAT_partition_getPartitionFromPath("fat:/")&&!stat(file,&st)&&!(st.st_mode&S_IFDIR)&&st.st_size>0)f=1;
retry:
	for(swiWaitForVBlank();;swiWaitForVBlank())
		if(!IPCZ->keysheld)break;
#ifdef _LIBNDS_MAJOR_
	if(*(u64*)fake_heap_end==0x62757473746F6F62ULL){
		PrintfToDie("Press A to return to menu.\n");
		if(f)PrintfToDie("Press B for %s.\n",file);
		for(swiWaitForVBlank();;swiWaitForVBlank()){
			if(KEY_A&IPCZ->keysheld)exit(0);
			if(f&&KEY_B&IPCZ->keysheld)
				if(!BootNDSROM(file)){f=0;goto retry;}
		}
	}else
#endif
	{
		PrintfToDie("Press A to shutdown.\n");
		if(f)PrintfToDie("Press B for %s.\n",file);
		for(swiWaitForVBlank();;swiWaitForVBlank()){
			if(KEY_A&IPCZ->keysheld)IPCZ->cmd=Shutdown;
			if(f&&KEY_B&IPCZ->keysheld)
				if(!BootNDSROM(file)){f=0;goto retry;}
		}
	}
}

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
	dp=diropen(target);
	while(!dirnext(dp,targetfile,&st)){
		if(!strcmp(targetfile,".")||!strcmp(targetfile,".."))continue;
		if(st.st_mode&S_IFDIR){
			strcat(targetfile,"/");
			rm_rf(target);
			targetfile[strlen(targetfile)-1]=0;
			unlink(target);
		}else unlink(target);
	}
	dirclose(dp);
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

int copy(const char *old, const char *new){
	FILE *in=fopen(old,"rb");
	if(!in)return 1;
	FILE *out=fopen(new,"wb");
	if(!out){fclose(in);return 2;}
	int size=filelength(fileno(in));
	int read,cur=0;
	while((read=fread(libprism_buf,1,65536,in))>0){
		cur+=read;
		fwrite(libprism_buf,1,read,out);
		_consolePrintf2("Copying %8d / %8d\r",cur,size); //99MB is enough I think...
	}
	_consolePrint2("                           \r");
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

void installargv(u8 *top, void *store, const char *nds){
	*(vu32*)(top+0x70)=0x5f617267;
	*(vu32*)(top+0x74)=(u32)store;
	//vramcpy(store,nds,strlen(nds)+1);
	strcpy((char*)store,"fat:");
	strcpy((char*)store+4,nds);
	*(vu32*)(top+0x78)=strlen(nds)+5;
}

char *processlinker(const char *name){
	char buf[768],key[768];
	if(!name)return NULL;
	if(name[strlen(name-1)]!='=')return (char*)name;
	strcpy(key,name),key[strlen(name)-1]=0;
	if(!strcpy_safe(buf,findpath(3,(char*[]){"/","/_dstwoplug/",mypath},"linkpath.ini")))return NULL;
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
	for(i=strlen(s)-1;i>0;i--){
		if(s[i]=='/'){i++;break;}
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

void slot2nds(){
	sysSetCartOwner(BUS_OWNER_ARM7);
	bootMoonlight(0x080000c0);
	while(1);
}

void slot2gba(int screen){
	sysSetBusOwners(BUS_OWNER_ARM7,BUS_OWNER_ARM7);
	REG_POWERCNT=screen?1:(POWER_SWAP_LCDS|1);
	IPCZ->cmd=Slot2GBA;
	while(1);
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

