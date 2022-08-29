#include "../../../../libprism/libprism.h"
#include "libmshlsplash.h"

#include "msp.h"

#define EXTMAX 256
extern char ext[32][EXTMAX];

// 1/3sec
#define KEYCOUNTMAX_UNREPEATED 20
// 1/20sec
#define KEYCOUNTMAX_REPEATED 3

typedef bool (*type_constpchar)(const char*);
extern type_constpchar BootLibrary;

//filelist
typedef struct FILEINFO{
	char name[768];
	char nameshow[42];
	int isDir;
	struct FILEINFO *next;
}fileinfo;
extern fileinfo ftop;
extern int contentcount;

void getfilelist(char *dir, int filter);

//to avoid memory fragmentation, I don't free memory if new dir has less files than old dir.
//But before doing some actions, you should free filelist!
void destroyfilelist();

//show
extern const int paging;
extern const int scroll;
extern int top;
void showfilelist(int cursor, char *dir);

//extlink
void runCommercial(char *file,char *loader);
bool runTextEdit(char *file);
bool runExtLink(char *file,char *ext);
int iterateExtLink(int start);

bool runRPGLink(const char *file);

//pref
extern u8 dldibuf[32768];
int selectpref(char *title,int argc, char **argv);

//savconv
bool savConvert(char *file);

//keyboard
extern const unsigned char keyboard_Hit[];
extern const unsigned char keyboard_Hit_Shift[];

enum{
	BSP	= 0x8, // Backspace
	CAP	= 0x2, // Caps
	RET	= 0xa, // Enter
	SHF	= 0x4, // Shift
	SPC	= 0x20, // Space
};

//md5
typedef struct{
  u32 state[4];
  u32 count[2];        /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];
} MD5_CTX;

void MD5Init(MD5_CTX *context);
void MD5Update(MD5_CTX *context, unsigned char *input, unsigned int inputLen);
void MD5Final(unsigned char digest[16], MD5_CTX *context);

//bmp
int bmpdecode_gbaborder(char *name);
int bmpdecode_show(char *name);
int icodecode_show(char *name);

//MSP
int iterateMSP(int start);
TPluginBody* getPluginByIndex(int idx);

//encryption
void EncryptSecureArea(char *ndsfilename);
void DecryptSecureArea(char *ndsfilename);

//firmware
int returnDSMenu();
