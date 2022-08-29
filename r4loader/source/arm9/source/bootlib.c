//no longer GPL only
//#ifdef GPL
#include <string.h>
#include <stdio.h>
#include <nds.h>
#include <sys/stat.h>
//#include "chishm_bin.h"
#include "_console.h"
#include "dldi.h"

#include "../../../../arm7/ipcz.h"

#define printf _consolePrintf

#define STORED_FILE_CLUSTER_OFFSET 4
#define INIT_DISC_OFFSET 8
#define WANT_TO_PATCH_DLDI_OFFSET 12

typedef signed int addr_t;
typedef unsigned char data_t;

extern const unsigned char ndshead[512];

inline addr_t readAddr(data_t *mem){
	return *((addr_t*)mem);
}

inline void writeAddr(data_t *mem,addr_t value){
	*((addr_t*)mem) = value;
}

inline void vramcpy(void* dst, const void* src, int len){
	u16* dst16 = (u16*)dst;
	u16* src16 = (u16*)src;
	for ( ; len > 0; len -= 2)*dst16++ = *src16++;
}

const data_t dldiMagicLoaderString[] = "\xEE\xA5\x8D\xBF Chishm";	// Different to a normal DLDI file

#define torelative(n) (readAddr(pA+n)-pAdata)
inline int dldiloader(byte *nds,const int ndslen){
#if defined(ARM9) || defined(ARM7)
	byte *pD=NULL;
	int dldilen;
	const byte *DLDIDATA=io_dldi_data;
	//const byte *DLDIDATA=((u32*)(&_io_dldi))-24;
#endif

	byte *pA=NULL,id[5],space;
	u32 reloc,pAdata,pDdata,pDbssEnd,fix;
	int i,ittr;

	for(i=0;i<ndslen-0x80;i+=4){
		if(!memcmp(nds+i,dldiMagicLoaderString,12)&&(readAddr(nds+i+dldiVersion)&0xe0f0e0ff)==1){pA=nds+i;break;}
	}
	if(!pA){printf("not found valid dldi section\n");return 1;}

#if defined(ARM9) || defined(ARM7)
	//Now we have to tune in the dldi...
	pD=(byte*)DLDIDATA;
	memcpy(id,pD+ioType,4);id[4]=0;
/*
	{
		int idx=0;
		for(;idx<32*1024/4;idx++)
			if(pD[idx]!=0)dldilen=(idx+1)*4; //BackupDLDIBody() in MoonShell 2.00beta5
	}
*/
	dldilen=*((u32*)(pD+bssStart))-*((u32*)(pD+dataStart)); //DLDITool 0.32.4

	//if(memcmp(*(void**)(pD+dldiStartup),"\x01\x00\xa0\xe3\x1e\xff\x2f\xe1",8)) //z=="mov r0,#1;bx lr"
	//	goto done;
#endif

	if(*((u32*)(pD+bssEnd))-*((u32*)(pD+dataStart)) > 1<<pA[allocatedSpace])
		{printf("not enough space. available %d bytes, need %d bytes\n",1<<pA[allocatedSpace],*((u32*)(pD+bssEnd))-*((u32*)(pD+dataStart)));return 2;}
	space=pA[allocatedSpace];

	pAdata=readAddr(pA+dataStart);if(!pAdata)pAdata=readAddr(pA+dldiStartup)-dldiData;
	memcpy(id,pA+ioType,4);id[4]=0;
	printf("Old ID=%s, Interface=0x%08x,\nName=%s\n",id,pAdata,pA+friendlyName);
	memcpy(id,pD+ioType,4);id[4]=0;
	printf("New ID=%s, Interface=0x%08x,\nName=%s\n",id,pDdata=readAddr(pD+dataStart),pD+friendlyName);
	printf("Relocation=0x%08x, Fix=0x%02x\n",reloc=pAdata-pDdata,fix=pD[fixSections]); //pAdata=pDdata+reloc
	printf("dldiFileSize=0x%04x, dldiMemSize=0x%04x\n",dldilen,*((u32*)(pD+bssEnd))-*((u32*)(pD+dataStart)));

	vramcpy(pA,pD,dldilen);pA[allocatedSpace]=space;
	for(ittr=dataStart;ittr<ioType;ittr+=4)writeAddr(pA+ittr,readAddr(pA+ittr)+reloc);
	for(ittr=dldiStartup;ittr<dldiData;ittr+=4)writeAddr(pA+ittr,readAddr(pA+ittr)+reloc);
	pAdata=readAddr(pA+dataStart);pDbssEnd=readAddr(pD+bssEnd);

	if(fix&fixAll)
		for(ittr=torelative(dataStart);ittr<torelative(dataEnd);ittr+=4)
			if(pDdata<=readAddr(pA+ittr)&&readAddr(pA+ittr)<pDbssEnd)
				printf("All  0x%04x: 0x%08x -> 0x%08x\n",ittr,readAddr(pA+ittr),readAddr(pA+ittr)+reloc),
				writeAddr(pA+ittr,readAddr(pA+ittr)+reloc);
	if(fix&fixGlue)
		for(ittr=torelative(glueStart);ittr<torelative(glueEnd);ittr+=4)
			if(pDdata<=readAddr(pA+ittr)&&readAddr(pA+ittr)<pDbssEnd)
				printf("Glue 0x%04x: 0x%08x -> 0x%08x\n",ittr,readAddr(pA+ittr),readAddr(pA+ittr)+reloc),
				writeAddr(pA+ittr,readAddr(pA+ittr)+reloc);
	if(fix&fixGot)
		for(ittr=torelative(gotStart);ittr<torelative(gotEnd);ittr+=4)
			if(pDdata<=readAddr(pA+ittr)&&readAddr(pA+ittr)<pDbssEnd)
				printf("Got  0x%04x: 0x%08x -> 0x%08x\n",ittr,readAddr(pA+ittr),readAddr(pA+ittr)+reloc),
				writeAddr(pA+ittr,readAddr(pA+ittr)+reloc);
	if(fix&fixBss)
		memset(pA+torelative(bssStart),0,pDbssEnd-readAddr(pD+bssStart));

	printf("Patched successfully\n");
	return 0;
}


void runNds (/*const void* loader, u32 loaderSize,*/ u32 cluster, bool initDisc, bool dldiPatchNds/*, bool useDSBooter*/)
{
	FILE *f;
	struct stat st,st2;
	char file[256*3]="/boot.bin\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" //will be modified externally
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
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"; //752 paddings
	char stub[256*3]="/bootstub.bin\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" //will be modified externally
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
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"; //748 paddings

	_consolePrintf("Preparing...\n");
	IPCZ->cmd=ResetBootlib;
	irqDisable(IRQ_ALL);
	// Direct CPU access to VRAM bank C
	VRAM_C_CR = VRAM_ENABLE | VRAM_C_LCD;
	// Clear VRAM
	memset (VRAM_C, 0x00, 128 * 1024);

	// Load the loader/patcher into the correct address
	//vramcpy (VRAM_C, loader, loaderSize);
	f=fopen(file,"rb");
	if(!f){_consolePrintf("Cannot stat /boot.bin.\n");die();}
	fstat(fileno(f),&st);
	fread(VRAM_C,2,st.st_size/2+1,f);
	fclose(f);
	dldiloader((data_t*)VRAM_C, st.st_size); //loaderSize);

	_consolePrintf("Installing bootstub...\n");
	f=fopen(stub,"rb");
	if(f){
		extern u8 *fake_heap_end;
		fstat(fileno(f),&st2);
		fread(fake_heap_end,1,st2.st_size,f);
		fclose(f);
		memcpy(fake_heap_end+st2.st_size,VRAM_C,st.st_size);
		writeAddr(fake_heap_end+0x08,readAddr(fake_heap_end+0x08)+fake_heap_end);
		writeAddr(fake_heap_end+0x0c,readAddr(fake_heap_end+0x0c)+fake_heap_end);
		writeAddr(fake_heap_end+0x10,st.st_size);
	}

	// Set the parameters for the loader
	// STORED_FILE_CLUSTER = cluster;
	writeAddr((data_t*) VRAM_C+0x04, cluster);
	// INIT_DISC = initDisc;
	writeAddr((data_t*) VRAM_C+0x08, initDisc);
	// WANT_TO_PATCH_DLDI = dldiPatchNds;
	writeAddr((data_t*) VRAM_C+0x0c, dldiPatchNds);
/*
	if(useDSBooter){
		int i=0;
		for(;i<st.st_size;i+=4)
			if(readAddr((data_t*)VRAM_C+i)==0x54425344)break;
		if(i>=st.st_size)
			{_consolePrintf("This boot.bin doesn't support DSBooter extention.\n");die();}
		writeAddr((data_t*)VRAM_C+i+4, 1);
		//if(!readAddr((data_t*)VRAM_C+0x18))writeAddr((data_t*)VRAM_C+0x18,useDSBooter);
		vramcpy((data_t*)VRAM_C+i+8,ndshead,0x170);
		//printf("dbg:%08x\n",*(u32*)0x0685ff70);die();
	{
	FILE *f=fopen("/bootlib.tmp","wb");
	fwrite((data_t*)VRAM_C+i+8,1,0x170,f);
	fclose(f);
	}
	}
*/
	DC_FlushAll();

	// Give the VRAM to the ARM7
	VRAM_C_CR = VRAM_ENABLE | VRAM_C_ARM7_0x06000000;

	// Reset into a passme loop
	REG_EXMEMCNT |= ARM7_OWNS_ROM | ARM7_OWNS_CARD;
	_consolePrintf("Go!!!\n");
	*((vu32*)0x027FFFFC) = 0;
	*((vu32*)0x027FFE04) = (u32)0xE59FF018;
	*((vu32*)0x027FFE24) = (u32)0x027FFE04;
	swiSoftReset();
	_consolePrintf("Failed.\n");
	die();
}

void runNdsFile (const char* filename){
	struct stat st;
	if (stat (filename, &st) < 0) {
		_consolePrintf("Cannot stat %s\n",filename);die();
	}
	runNds (/*chishm_bin, chishm_bin_size,*/ st.st_ino, true, true);//, false);
}
#if 0
void runDSBooter (const char* filename){
	u8 head[512],dec[512];
	struct stat st;
	u8 *pA=NULL,*p7=NULL,*p9=NULL;
	u32 l7,l9,a7,a9,pad7,pad9;

	unsigned char *p=(unsigned char*)ndshead; //break const rule
	FILE *f=fopen(filename,"rb");

	if(!f){
		_consolePrintf("Cannot stat %s\n",filename);die();
	}
	fstat(fileno(f),&st);
	fread(head,1,512,f);
	fclose(f);
	_consolePrintf("Decrypting... ");
	{
		int i=0,j;
		for(;i<0x100;i++){
			for(j=0xa0;j<0xa8;j++)
				dec[j]=head[j]^i;
			if(!memcmp(dec+0xa0,"DSBooter",8))
				{_consolePrintf("key = 0x%02x\n",i);break;}
		}
		if(i==0x100){printf("Cannot decode or not DSBooter\n");die();}
		for(j=0x000;j<0x200;j++)
			head[j]=head[j]^i;
	}
	pA=head+0xc8;
	_consolePrintf("p9=0x%08x\n",(p9=pA+read32(pA+0x00))-head);
	_consolePrintf("l9=0x%08x\n",l9=read32(pA+0x08));
	_consolePrintf("a9=0x%08x\n",a9=read32(pA+0x04));
	_consolePrintf("p7=0x%08x\n",(p7=pA+read32(pA+0x0c)+0x0c)-head);
	_consolePrintf("l7=0x%08x\n",l7=read32(pA+0x14));
	_consolePrintf("a7=0x%08x\n",a7=read32(pA+0x10));
	pad9=0x100-(l9&0xff);
	pad7=0x100-(l7&0xff);

	//writing loader to memory...
	write32(p+0x20,p9-head);
	write32(p+0x24,a9);
	write32(p+0x28,a9);
	write32(p+0x2c,l9/*+pad9*/);
	write32(p+0x30,p7-head);
	write32(p+0x34,a7);
	write32(p+0x38,a7);
	write32(p+0x3c,l7);
	write32(p+0x80,/*0x200+l9+pad9+l7+pad7*/p7-head+l7);
	//memcpy(p+0x170,"DSBT",4);

	runNds (/*chishm_bin, chishm_bin_size,*/ st.st_ino, true, true, true);
}
#endif
//#endif
