//no longer GPL only
//#ifdef GPL
#include "libprism.h"

#define printf _consolePrintf

#define STORED_FILE_CLUSTER_OFFSET 4
#define INIT_DISC_OFFSET 8
#define WANT_TO_PATCH_DLDI_OFFSET 12

typedef signed int addr_t;
typedef unsigned char data_t;

extern const unsigned char ndshead[512];

extern char bootlibname[256*3];
extern char bootstubname[256*3];

int dldiloader(byte *nds,const int ndslen);

void runNdsViaStub(const char* filename, u32 cluster/*, bool initDisc, bool dldiPatchNds*//*, bool useDSBooter*/)
{
	FILE *f;
	struct stat st,st2;
	u8 *bootstub=(u8*)0x02ff4000;
	_consolePrint("Preparing...\n");

	// Load the loader/patcher into the correct address
	//vramcpy (VRAM_C, loader, loaderSize);

	f=fopen(bootstubname,"rb");
	fstat(fileno(f),&st2);
	fread(bootstub,1,st2.st_size,f);
	fclose(f);
	f=fopen(bootlibname,"rb");
	fstat(fileno(f),&st);
	fread(bootstub+st2.st_size,1,st.st_size,f);
	fclose(f);
	disc_unmount();

	write32(bootstub+0x08,read32(bootstub+0x08)+(u32)bootstub);
	write32(bootstub+0x0c,read32(bootstub+0x0c)+(u32)bootstub);
	write32(bootstub+0x10,align4(st.st_size));

	u8 *P=bootstub+st2.st_size;
	dldiloader((data_t*)P, st.st_size); //loaderSize);

	// Set the parameters for the loader
	// STORED_FILE_CLUSTER = cluster;
	write32((data_t*)P+0x04, cluster);
	// INIT_DISC = initDisc;
	write32((data_t*)P+0x08, 1);//initDisc);
	// WANT_TO_PATCH_DLDI = dldiPatchNds;
	write32((data_t*)P+0x0c, DLDIToBoot!=NULL);//dldiPatchNds);

if(*(vu16*)P>=5){ //bootlib v2 with ARGV
	//set argv
	u32 arg=align4( (u32)P+read32((data_t*)P+0x10) );
	if(!argvToInstall)makeargv(filename);
	installargv(NULL,(char*)arg);
	writeAddr((data_t*)P+0x10, arg-(u32)P);
	writeAddr((data_t*)P+0x14, argvToInstallSize);
	writeAddr(bootstub+0x10,align4((char*)arg+argvToInstallSize-(char*)P));//align4(st.st_size)+align4(strlen(filename)+5));
#if 0
	strcpy((char*)arg,"fat:");
	strcpy(((char*)arg)+4,filename);
	write32((data_t*)P+0x10, arg-(u32)P);
	write32((data_t*)P+0x14, strlen(filename)+5);
	write32(bootstub+0x10,align4((char*)arg+strlen(filename)+5-(char*)P));//align4(st.st_size)+align4(strlen(filename)+5));
	if(fpassarg){
		char *p=(char*)arg+strlen(filename)+5;
		strcpy(p,"fat:");
		strcpy(p+4,argname);
		write32((data_t*)P+0x14, strlen(filename)+5+strlen(argname)+5);
		write32(bootstub+0x10,align4((char*)arg+strlen(filename)+5+strlen(argname)+5-(char*)P));//align4(st.st_size)+align4(strlen(filename)+5));
	}
#endif
}

if(*(vu16*)P>=6){ //bootlib v3 with DSi SD
	writeAddr((data_t*)P+0x1c, !strcmp(mydrive,"fat:/")?0:1);
}

	//make sure loader is written to memory
	IC_InvalidateAll();
	DC_FlushAll();
	DC_InvalidateAll();

	// Give the VRAM to the ARM7
	//VRAM_C_CR = VRAM_ENABLE | VRAM_C_ARM7_0x06000000;

	// Reset into a passme loop
	REG_EXMEMCNT |= ARM7_OWNS_ROM | ARM7_OWNS_CARD;
	_consolePrint("Jumping to BootStub...\n");
	(*(type_void*)(bootstub+0x08))();
	_consolePrint("Failed.\n");while(1);
}

bool runNdsFileViaStub(const char* filename){
	struct stat st;
	if (stat (filename, &st) < 0) {
		_consolePrintf("Cannot stat %s\n",filename);return false;
	}
	if(!bootlibAvailable()||!bootstubAvailable()){
		_consolePrint("bootlib/bootstub isn't available\n");return false;
	}
	runNdsViaStub(filename,st.st_ino);//, true, true);//, false);
	return true;
}

//#endif
