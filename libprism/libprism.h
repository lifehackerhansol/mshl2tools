#ifndef LIBPRISM_H
#define LIBPRISM_H

#include <nds.h>
#include <fat.h>

//all in one
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h> //validateTM, struct tm
#include <sys/dir.h> //getDirEntFromDirIter, DIR_ITER
#include <sys/stat.h>
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

#define attrinline   __attribute__ ((always_inline))
#define attrnoreturn __attribute__ ((noreturn))

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

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

#define arraysize(a) (sizeof(a)/sizeof(*a))

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
extern u8 libprism_buf[65536];
extern char myname[768],mypath[768],libprism_name[768];
extern u16 keysdown,keysrepeat;
extern u16 *b15ptrMain,*b15ptrSub; //256*192*2=96KB usable

typedef void (*type_printf)(const char*, ...);
extern type_printf PrintfToDie;

typedef void (*type_void)();
typedef void (*type_u8p)(u8*);
extern type_u8p ret_menu9_callbackpre;
extern type_u8p ret_menu9_callback;

// _console
void _consoleInit(u16* font, u16* charBase, u16 numCharacters, u8 charStart, u16* map, u8 pal, u8 bitDepth);
void _consoleInitDefault(u16* map, u16* charBase);
void _consolePrint(const char* s);
void _consolePrintf(const char* format, ...);
void _consolePrintSet(int x, int y);
int  _consoleGetPrintSetY();
void _consolePrintChar(char c);
void _consoleClear();
void _consolePrintOne(char *str,u32 v);

// _console2
void _consoleInit2(u16* font, u16* charBase, u16 numCharacters, u8 charStart, u16* map, u8 pal, u8 bitDepth);
void _consoleInitDefault2(u16* map, u16* charBase);
void _consolePrint2(const char* s);
void _consolePrintf2(const char* format, ...);
void _consolePrintSet2(int x, int y);
int  _consoleGetPrintSetY2();
void _consolePrintChar2(char c);
void _consoleClear2();
void _consolePrintOne2(char *str,u32 v);

// bootlib / moonshellreset / ret_menu9_Gen
bool runNdsFile(const char* filename);
bool bootlibAvailable();

bool BootNDSROM2(const char *pFilename,const int bypassYSMenu,const char* dumpname);
bool BootNDSROM(const char *pFilename);
bool BootDSBooter(const char *pFilename);
//bool BootDSBooterRaw(const char *pFilename);
bool BootR4Menu(const char *pFilename);

bool ret_menu9_Gen2(const char *menu_nam,const int bypassYSMenu,const char* dumpname);
bool ret_menu9_Gen(const char *menu_nam);
bool ret_menu9_GenM2(const char *menu_nam,const int bypassYSMenu,const char* dumpname);
bool ret_menu9_GenM(const char *menu_nam);

//shouldn't be called without appropriate reason.
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
int fgetAttributes(int fd);
int getAttributes(const char *path);
u8* getDirEntFromDirIter(DIR_ITER *dp);
int libprism_touch(const char *path);
int libprism_fchattr(int fd, u8 attr);
int libprism_chattr(const char *path, u8 attr);
u32 getSectors();

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

memstream *mopen(void *p, u32 size, memstream *s);
int mclose(memstream *s);
int mgetc(memstream *s);
int mputc(int c, memstream *s);
int mrewind(memstream *s);
int mavail(memstream *s);
int mtell(memstream *s);
int mlength(memstream *s);
int mread(memstream *s, void *buf, u32 size);
int mwrite(memstream *s, void *buf, u32 size);
int mseek(memstream *s, int offset, int whence);
int mread32(memstream *s);
int mread16(memstream *s);
int mwrite32(const unsigned int n, memstream *s);
int mwrite16(const unsigned int n, memstream *s);

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
void write32(void *p, const unsigned int n);
void write24(void *p, const unsigned int n);
void write16(void *p, const unsigned int n);
void write8(void *p, const unsigned int n);

//direct version (4 bytes i/o) *** cannot be used on big-endian machine
unsigned int readAddr(void *mem);
unsigned int readAddr24(void *mem);
unsigned short readAddr16(void *mem);
unsigned char readAddr8(void *mem);
void writeAddr(void *mem, const unsigned int value);
void writeAddr24(void *mem, const unsigned int value);
void writeAddr16(void *mem, const unsigned int value);
void writeAddr8(void *mem, const unsigned int value);

void Unicode_Copy(UnicodeChar *tag,const UnicodeChar *src);
void SplitItemFromFullPathAlias(const char *pFullPathAlias,char *pPathAlias,char *pFilenameAlias);
void SplitItemFromFullPathUnicode(const UnicodeChar *pFullPathUnicode,UnicodeChar *pPathUnicode,UnicodeChar *pFilenameUnicode);

int filelength(int fd);
int copy(const char *old, const char *_new);
char *findpath(int argc, char **argv, const char *name);
void installargv(u8 *top, void *store, const char *nds); //use top=0x02fffe00 for normal purpose
char *processlinker(const char *name);
char *strcpy_safe(char *s1, const char *s2);
char *getextname(char *s);
char *getfilename(char *s);

int UTCToDateTime(time_t epochTime, u16 *date, u16 *time);
int validateTM(struct tm *timeParts);

attrnoreturn void slot2nds();
attrnoreturn void slot2gba(int screen);
bool isHomebrew(u8 *head);
int GetFirmwareVersion();
int fexists(const char *path);

//libfat mod UTF8 <=> UTF16
size_t _FAT_directory_mbstoucs2 (u16* dst, const char* src, size_t len);
size_t _FAT_directory_ucs2tombs (char* dst, const u16* src, size_t len);
#ifndef MSHL2TOOLS_FATX
void* _FAT_partition_getPartitionFromPath (const char* path);
uint32_t _FAT_fat_nextCluster(void* partition, uint32_t cluster);
#endif

#ifdef __cplusplus
}
#endif
#endif //included
