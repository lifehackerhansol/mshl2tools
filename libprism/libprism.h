#ifndef LIBPRISM_H
#define LIBPRISM_H

/*
	libprism - a fundamental framework for NDS development

	reset_mse_06b_for_ak2 by Moonlight, Rudolph, kzat3
	MoonShell Simply NDS Loader (MainRam Loader X) by Moonlight and X
	VRAM bootlib (external) by Chishm (boot.bin is distributed under GPL, but please note it is externalized)
	Rudolph/Moonlight hybrid Loader by Moonlight and Rudolph
	MoonShell2 Loader by Moonlight and X

	*** Portion Notice ***
	dldipatch aka dlditool public domain under CC0
	Console Library by Moonlight
	cart.c / disc.c / util.c under CC0
	fatx - libfat/libelm helper library under CC0
	memory stream library under CC0
	minIni (C) ITB CompuPhase under Apache License 2.0 (BSD like, compatible with GPLv3+)
	libfat (C) Chishm under BSD License
	libelm (C) ChaN / ywg under BSD License
	DS(lite) firmware decoder based on DeSmuME
	NDS RAM API (C) Rick "Lick" Wong under MIT License

	[XenoFile]
	rpglink (C) Acekard.com and X under MIT license
	libmshlsplash under CC0
	Secure Encryption by DarkFader
	Keyboard Library by HeadSoft
	MoonShell Plugin routine by Moonlight
	zlib (C) Jean-loup Gailly / Mark Adler under zlib/libpng license
	libnsbmp (C) Richard Wilson / Sean Fox under MIT license
	md5.c (C) RSA Data Security, Inc.
	Sound Library by MeRAMAN
	MaxMod (C) Mukunda Johnson under ISC license
	LZMA SDK by Igor Pavlov
	Save Converter based on Unique Geeks Media Offline NDS Save Converter
	***

	You may use libprism in both open source and proprietary software without restriction.
	libprism is distributed AS-IS without WARRANTY.

	Please note example (mshl2tools) are licensed under 2-clause BSD License, rather than CC0,
	which will prevent useless distro using mshl2tools.
	Algorithm isn't copyrighted, so please refer to mshl2tools code, not copy it.

	Moonlight has permitted to use his code freely,
	in http://mdxonline.dyndns.org/index_html/bbslog/log_post502.htm#post582
*/

#include <nds.h>
#define MOONSHELL "/moonshl2/extlink/_hn.HugeNDSLoader.nds"

///// Configure precisely. Wrong config might mess up TF. /////
//Choose one
//#define LIBFAT 1
//#define LIBELM 1
#include "fatdriver.h"

#if defined(LIBFAT) && defined(_LIBNDS_MAJOR_)
//Yes or No
#define USE_LIBFAT109
#endif
///// Config end /////

///// include and version info /////
#define ROMVERSION "0.91g.160319 Phase:Neige"
#define ROMDATE ""__DATE__" "__TIME__" GMT+09:00"

#if defined(LIBFAT) && defined(LIBELM)
#error both LIBFAT and LIBELM are defined
#elif defined(LIBFAT)
#include <fat.h>
#ifdef _LIBNDS_MAJOR_
#define ROMENV "DevKitARMr45 + libnds 1.5.12 +\nlibfat 1.0.14(modified)"
#else
#define ROMENV "DevKitARMr23b + libnds-20071023/i +\nlibfat-20080530less(modified) [legacy]"
#endif
#elif defined(LIBELM)
#include "../libelm/include/elm.h"
#ifdef _LIBNDS_MAJOR_
#define ROMENV "DevKitARMr45 + libnds 1.5.12 +\nlibelm R0.09(modified)"
#else
#define ROMENV "DevKitARMr23b + libnds-20071023/i +\nlibelm R0.09(modified) [legacy]"
#endif
#else
#error define one of LIBFAT / LIBELM
#endif
///// end /////

//all in one
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h> //validateTM, struct tm
#include <sys/iosupport.h> //DIR_ITER
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h> //low level API

#include "include/minIni.h"
#include "include/dldi.h"
#include "../arm7/ipcz.h"

#ifdef __cplusplus
extern "C" {
#endif

/* definition and struct */

#define ScreenWidth (256)
#define ScreenHeight (192)

#define attrinline   static __attribute__((always_inline))
#define attrnoinline __attribute__((noinline))
#define attrnoreturn __attribute__((noreturn))

#define BIT0 (1<<0)
#define BIT1 (1<<1)
#define BIT2 (1<<2)
#define BIT3 (1<<3)
#define BIT4 (1<<4)
#define BIT5 (1<<5)
#define BIT6 (1<<6)
#define BIT7 (1<<7)
#define BIT8 (1<<8)
#define BIT9 (1<<9)
#define BIT10 (1<<10)
#define BIT11 (1<<11)
#define BIT12 (1<<12)
#define BIT13 (1<<13)
#define BIT14 (1<<14)
#define BIT15 (1<<15)

#define align2(i) (((i)+1)&~1)
#define align4(i) (((i)+3)&~3)
#define align8(i) (((i)+7)&~7)
#define align256(i) (((i)+255)&~255)
#define align512(i) (((i)+511)&~511)

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

#define arraysize(a) (sizeof(a)/sizeof(*a))

enum FrontendType{
	FRONTEND_ARGV=1,
	FRONTEND_LOADFILE=2,
	FRONTEND_PLGARGS=4,
	FRONTEND_EXTLINK=8,
};

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

enum{
	ATTRIB_ARCH	= 0x20,	// Archive
	ATTRIB_DIR	= 0x10,	// Directory
	ATTRIB_LFN	= 0x0F,	// Long file name
	ATTRIB_VOL	= 0x08,	// Volume
	ATTRIB_SYS	= 0x04,	// System
	ATTRIB_HID	= 0x02,	// Hidden
	ATTRIB_RO	= 0x01,	// Read only
};

enum FWType{
	FW_iQue	=0x10000,
	FW_Korean	=0x20000,
};

/* libprism API */
#define BUFLEN 65536
extern u8 libprism_buf[BUFLEN];
#define libprism_cbuf ((char*)libprism_buf)
extern char myname[768],mypath[768],argname[768],argpath[768],libprism_name[768];
extern bool fpassarg;
extern u16 *b15ptrMain,*b15ptrSub; //256*192*2=96KB usable
extern char console_strbuf[2048];

extern const unsigned char key_tbl[0x4000];
extern const unsigned char *encr_data;
extern vu16 *extmem;

extern char **argv;
extern int argc;
extern char *argvToInstall;
extern int argvToInstallSize;

extern char mydrive[12];

extern const int useARM7Bios; //need to on if you en/decrypt secure area or dump bios.
extern int nocashMessageMain;
extern int nocashMessageSub;

typedef void (*type_printf)(const char*, ...);
extern type_printf PrintfToDie;

typedef void (*type_charp)(const char*);
extern type_charp consolePrint_callback;
extern type_charp consolePrint2_callback;
extern type_charp consolePrintOnce_callback;
extern type_charp consolePrintOnce2_callback;

typedef void (*type_charp_u32_u32)(const char*,u32,u32);
extern type_charp_u32_u32 consolePrintProgress_callback;
extern type_charp_u32_u32 consolePrintProgress2_callback;

typedef void (*type_void)();
extern type_void consoleClear_callback;
extern type_void consoleClear2_callback;
extern type_void consolePrintOnceEnd_callback;
extern type_void consolePrintOnceEnd2_callback;
extern type_void consoleStartProgress_callback;
extern type_void consoleStartProgress2_callback;
extern type_void consoleEndProgress_callback;
extern type_void consoleEndProgress2_callback;

typedef void (*type_u8p)(u8*);
extern type_u8p ret_menu9_callbackpre;
extern type_u8p ret_menu9_callback;

typedef void (*type_u32p)(u32*);

// _console
void _consoleInit(u16* font, u16* charBase, u16 numCharacters, u8 charStart, u16* map, u8 pal, u8 bitDepth);
void _consoleInitDefault(u16* map, u16* charBase);
void _consolePrint(const char* s);
void _consolePrintf(const char* format, ...);
//void _consolePrintSet(int x, int y);
void _consolePrintChar(char c);
void _consoleClear();
void _consolePrintOnce(const char* s);
void _consolePrintfOnce(const char* format, ...);
void _consolePrintOnceEnd();
void _consolePrintProgress(const char* s, u32 v1, u32 v2);
void _consoleStartProgress();
void _consoleEndProgress();

// _console2
void _consoleInit2(u16* font, u16* charBase, u16 numCharacters, u8 charStart, u16* map, u8 pal, u8 bitDepth);
void _consoleInitDefault2(u16* map, u16* charBase);
void _consolePrint2(const char* s);
void _consolePrintf2(const char* format, ...);
//void _consolePrintSet2(int x, int y);
void _consolePrintChar2(char c);
void _consoleClear2();
void _consolePrintOnce2(const char* s);
void _consolePrintfOnce2(const char* format, ...);
void _consolePrintOnceEnd2();
void _consolePrintProgress2(const char* s, u32 v1, u32 v2);
void _consoleStartProgress2();
void _consoleEndProgress2();

// loader SDK
// bootlib
bool runNdsFile(const char* filename);
bool runNdsFileViaStub(const char* filename);
bool bootlibAvailable();
bool bootstubAvailable();

// MoonShell Simply Loader
bool BootNDSROM2(const char *pFilename,const int bypassYSMenu,const char* dumpname);
bool BootNDSROM(const char *pFilename);
bool BootDSBooter(const char *pFilename);
//bool BootDSBooterRaw(const char *pFilename);
bool BootR4Menu(const char *pFilename);

// MoonShell2 Loader
bool BootNDSROMex2(const char *pFilename,const int bypassYSMenu,const char* dumpname);
bool BootNDSROMex(const char *pFilename);

// Rudolph Loader
bool ret_menu9_Gen2(const char *menu_nam,const int bypassYSMenu,const char* dumpname);
bool ret_menu9_Gen(const char *menu_nam);

// Rudolph / MoonShell2 hybrid Loader
bool ret_menu9_GenM2(const char *menu_nam,const int bypassYSMenu,const char* dumpname);
bool ret_menu9_GenM(const char *menu_nam);

//shouldn't be called without appropriate purpose.
attrnoreturn void bootMoonlight(u32 BootAddress);
attrnoreturn void runNds(const char* filename, u32 cluster/*, bool initDisc, bool dldiPatchNds*//*, bool useDSBooter*/);
void ret_menu9_GENs();

// fatx
void getsfnlfn(const char *path,char *sfn,u16 *lfn);
u32 getFatDataPointer();
u64 fgetFATEntryAddress(int fd);
u64 getFATEntryAddress(const char *path);
u32 fgetSector(int fd);
u32 getSector(const char *path);
int fgetFragments(int fd);
int getFragments(const char *path);
//removed. use st.st_spare1
//int fgetAttributes(int fd);
//int getAttributes(const char *path);
//u8* getDirEntFromDirIter(DIR_ITER *dp);
int libprism_touch(const char *path);
int libprism_fchattr(int fd, u8 attr);
int libprism_chattr(const char *path, u8 attr);
u32 getSectors();

bool disc_mount(); //use this instead of fatInitDefault()
void disc_unmount();

//make sure char[768] or u16[256] is allocated.
void mbstoucs2(u16* dst, const char* src);
void ucs2tombs(char* dst, const u16* src);
u32  getPartitionHandle();
u32  nextCluster(u32 handle, u32 cluster);

int writePartitionInfo(type_printf writer);

// actime/modtime:
// 0: change to current time
// 1: no change
int libprism_futime(int fd, time_t actime, time_t modtime);
int libprism_utime(const char *path, time_t actime, time_t modtime);

// cart
//void cardWaitReady(u32 flags);
//void _cardPolledTransfer(u32 flags, u32 *destination, u32 length);
//void bytecardPolledTransfer(u32 flags, u32 *destination, u32 length);

u32 R4_ReadCardInfo();
void R4_SendMap(u32 address);
void R4_ReadSave(u32 address, u32 *destination, u32 length);
void R4_ReadRom(u32 address, u32 *destination, u32 length);
void R4_ReadMenu(u32 address, u32 *destination, u32 length);
void R4_LogicCardRead(u32 address, u32 *destination, u32 length);
void R4_LogicCardWrite(u32 address, u32 *source, u32 length);

u32 M3_ReadCardRegion();
void SCDS_SetSDHCModeForDSTT();

// disc
bool disc_startup();
bool disc_isInserted();
bool disc_readSectors(u32 sector, u32 numSectors, void* buffer);
bool disc_writeSectors(u32 sector, u32 numSectors, void* buffer);
bool disc_clearStatus();
bool disc_shutdown();

//memstream
typedef struct{
	u8  *p;
	u32 current;
	u32 size;
} memstream;

memstream *mopen(void *p, const u32 size, memstream *s);
int mclose(memstream *s);
int mgetc(memstream *s);
int mputc(const int c, memstream *s);
int mrewind(memstream *s);
int mavail(memstream *s);
int mtell(memstream *s);
int mlength(memstream *s);
int mread(void *buf, const u32 size, memstream *s);
int mwrite(void *buf, const u32 size, memstream *s);
int mcopy(memstream *to, const u32 size, memstream *s);
int mseek(memstream *s, const int offset, const int whence);
unsigned int mread32(memstream *s);
unsigned short mread16(memstream *s);
unsigned char mread8(memstream *s);
int mwrite32(const unsigned int n, memstream *s);
int mwrite16(const unsigned short n, memstream *s);
int mwrite8(const unsigned char n, memstream *s);

// util
void EnableB15Main();
void EnableB15Sub();
void DisableB15Main();
void DisableB15Sub();
attrnoreturn void die();
char* myfgets(char *buf,int n,FILE *fp);
void rm_rf(char *target);
void mkpath(char *path);

void vramcpy(void* dst, const void* src, int len);
void vramset(void* dst, const u16 n, int len);

//normal version (1 byte i/o)
unsigned int read32(const void *p);
unsigned int read24(const void *p);
unsigned short read16(const void *p);
unsigned char read8(const void *p);
unsigned long long int read64(const void *p);
void write32(void *p, const unsigned int n);
void write24(void *p, const unsigned int n);
void write16(void *p, const unsigned short n);
void write8(void *p, const unsigned char n);
void write64(void *p, const unsigned long long int n);

//direct version (4 bytes i/o) *** cannot be used on big-endian machine
unsigned int readAddr(void *mem);
unsigned int readAddr24(void *mem);
unsigned short readAddr16(void *mem);
unsigned char readAddr8(void *mem);
unsigned long long int readAddr64(void *mem);
void writeAddr(void *mem, const unsigned int value);
//void writeAddr24(void *mem, const unsigned int value);
void writeAddr16(void *mem, const unsigned short value);
void writeAddr8(void *mem, const unsigned char value);
void writeAddr64(void *mem, const unsigned long long int value);

void Unicode_Copy(UnicodeChar *tag,const UnicodeChar *src);
void SplitItemFromFullPathAlias(const char *pFullPathAlias,char *pPathAlias,char *pFilenameAlias);
void SplitItemFromFullPathUnicode(const UnicodeChar *pFullPathUnicode,UnicodeChar *pPathUnicode,UnicodeChar *pFilenameUnicode);

int filelength(int fd);
int copy(const char *old, const char *_new);
char *findpath(int argc, char **argv, const char *name);
char *parseargv(const char *str);
char *makeargv(const char *str);
void installargv(u8 *top, void *store); //use top=0x02fffe00 for normal purpose
char *processlinker(const char *name);
char *strcpy_safe(char *s1, const char *s2);
char *getextname(char *s);
char *getfilename(char *s);
void changefileext(char *fn, const char *ext);

int UTCToDateTime(time_t epochTime, u16 *date, u16 *time);
int validateTM(struct tm *timeParts);

bool slot2nds();
bool slot2gba(int screen);
bool jumpBootStub();
#define jumpAddress(addr) (( (type_void)(memUncached(addr)) )())
#define jumpAddressPtr(addr) (( *(type_void*)(memUncached(addr)) )())

bool isHomebrew(u8 *head);
int GetFirmwareVersion();
int fexists(const char *path);

void NotifyARM7(u32 c);
void CallARM7(u32 c);

int strchrindex(const char *s, const int c, const int idx);
int strstrindex(const char *s, const char *c, const int idx);

int GetRunningMode();
bool readFrontend(char *target);
bool writeFrontend(const int frontend_type, const char *exe, const char *target);
void clearExtlink();
int getExtlinkWrapperHBMode();
void getExtlinkWrapperLoaderName(char *loader);

void* memUncached(void *address); //in CRT0.
#define memUncachedAddr(addr)   ( (vu32*)memUncached((void*)(addr)) )
#define memUncachedAddr32(addr) ( (vu32*)memUncached((void*)(addr)) )
#define memUncachedAddr16(addr) ( (vu16*)memUncached((void*)(addr)) )
#define memUncachedAddr8(addr)  ( (vu8*)memUncached((void*)(addr)) )

void nocashMessageSafe(const char *s);

//encryption
int returnDSMenu();
void EnDecryptSecureArea(const char *ndsfilename, char endecrypt_option);
void EncryptSecureArea(const char *ndsfilename);
void DecryptSecureArea(const char *ndsfilename);
void InitializeKeyTable();

//ram
vu16* ram_init();
u32 ram_type();
const char* ram_type_string();
u32 ram_size();
vu16* ram_unlock();
void ram_lock();

void extmem_Init();
void extmem_Free();
bool extmem_ExistMemory();
u32 extmem_GetMemSize();

void extmem_SetCount(u32 Count);
bool extmem_Exists(u32 SlotIndex);
bool extmem_Alloc(u32 SlotIndex,u32 Size);
bool extmem_Write(u32 SlotIndex,void *pData,u32 DataSize);
bool extmem_Read(u32 SlotIndex,void *pData,u32 DataSize);

//memtool
void DC_FlushRangeOverrun(const void *v,u32 size);

void MemCopy8CPU(const void *src,void *dst,u32 len);
void MemCopy16CPU(const void *src,void *dst,u32 len);
void MemCopy32CPU(const void *src,void *dst,u32 len);
void MemSet8CPU(const vu8 v,void *dst,u32 len);
void MemSet16CPU(const vu16 v,void *dst,u32 len);
void MemSet32CPU(const u32 v,void *dst,u32 len);
void MemCopy16DMA3(const void *src,void *dst,u32 len);
void MemCopy32DMA3(const void *src,void *dst,u32 len);
void MemSet16DMA3(const vu16 v,void *dst,u32 len);
void MemSet32DMA3(const u32 v,void *dst,u32 len);
void MemSet8DMA3(const u8 v,void *dst,u32 len);

void MemCopy16DMA2(const void *src,void *dst,u32 len);
void MemSet16DMA2(const u16 v,void *dst,u32 len);

void MemCopy32swi256bit(const void *src,void *dst,u32 len);

void *safemalloc(const int size);
void safefree(const void *ptr);
bool testmalloc(int size);
u32 PrintFreeMem(void);

//diropen
DIR_ITER * mydiropen (const char *path);
int mydirreset (DIR_ITER *dirState);
int mydirnext (DIR_ITER *dirState, char *filename, struct stat *filestat);
int mydirclose (DIR_ITER *dirState);

#ifdef __cplusplus
}
#endif
#endif //included
