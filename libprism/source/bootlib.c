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

char bootlibname[256*3]="/boot.bin\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" //will be modified externally
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

//#ifdef _LIBNDS_MAJOR_
char bootstubname[256*3]="/bootstub.bin\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" //will be modified externally
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
//#endif

bool bootlibAvailable(){return !access(bootlibname,0);}
bool bootstubAvailable(){return !access(bootstubname,0);}

const data_t dldiMagicLoaderString[] = "\xEE\xA5\x8D\xBF Chishm";	// Different to a normal DLDI file

#define torelative(n) (readAddr(pA+n)-pAdata)
int dldiloader(byte *nds,const int ndslen){
	byte *pD=DLDIToBoot!=NULL?DLDIToBoot:DLDIDATA;
	int dldilen;

	byte *pA=NULL,id[5],space;
	u32 reloc,pAdata,pDdata,pDbssEnd,fix;
	int i,ittr;

	for(i=0;i<ndslen-0x80;i+=4){
		if(!memcmp(nds+i,dldiMagicLoaderString,12)&&(readAddr(nds+i+dldiVersion)&0xe0f0e0ff)==1){pA=nds+i;break;}
	}
	if(!pA){printf("not found valid dldi section\n");return 1;}

	//Now we have to tune in the dldi...
	memcpy(id,pD+ioType,4);id[4]=0;
	dldilen=*((u32*)(pD+bssStart))-*((u32*)(pD+dataStart)); //DLDITool 0.32.4

	//if(memcmp(*(void**)(pD+dldiStartup),"\x01\x00\xa0\xe3\x1e\xff\x2f\xe1",8)) //z=="mov r0,#1;bx lr"
	//	goto done;

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


void runNds(const char* filename, u32 cluster/*, bool initDisc, bool dldiPatchNds*//*, bool useDSBooter*/)
{
	u8 *bootstub=(u8*)0x02ff4000;
	FILE *f;
	struct stat st,st2;
	_consolePrint("Preparing...\n");
	
	// Load the loader/patcher into the correct address
	//vramcpy (VRAM_C, loader, loaderSize);
	f=fopen(bootlibname,"rb");
	if(!f){_consolePrint("Cannot open bootlib. Halt.\n");while(1);}
	fstat(fileno(f),&st);

	//using _consolePrintf2() from here is strictly prohibited.
	irqDisable(IRQ_ALL);
	// Direct CPU access to VRAM bank C
	VRAM_C_CR = VRAM_ENABLE | VRAM_C_LCD;
	// Clear VRAM
	vramset(VRAM_C, 0, 64*1024);
	fread(VRAM_C,2,align2(st.st_size)/2,f);
	fclose(f);
	dldiloader((data_t*)VRAM_C, st.st_size); //loaderSize);

// #ifdef _LIBNDS_MAJOR_ //r23 modified crt0 removes this limitation.
	_consolePrint("Installing bootstub...\n");
	f=fopen(bootstubname,"rb");
	if(f){
		fstat(fileno(f),&st2);
		fread(bootstub,1,st2.st_size,f);
		fclose(f);
		memcpy(bootstub+st2.st_size,VRAM_C,st.st_size);
		writeAddr(bootstub+0x08,readAddr(bootstub+0x08)+(u32)bootstub);
		writeAddr(bootstub+0x0c,readAddr(bootstub+0x0c)+(u32)bootstub);
		writeAddr(bootstub+0x10,st.st_size);
	}
// #else
//	_consolePrint("devkitARM r23b w/o bootstub support.\n");
// #endif
	disc_unmount();

	// Set the parameters for the loader
	// STORED_FILE_CLUSTER = cluster;
	writeAddr((data_t*)VRAM_C+0x04, cluster);
	// INIT_DISC = initDisc;
	writeAddr((data_t*)VRAM_C+0x08, 1);//initDisc);
	// WANT_TO_PATCH_DLDI = dldiPatchNds;
	writeAddr((data_t*)VRAM_C+0x0c, DLDIToBoot!=NULL);//dldiPatchNds);

if(*(vu16*)VRAM_C>=5){ //bootlib v2 with ARGV
	//set argv
	u32 arg=align4( (u32)VRAM_C+readAddr((data_t*)VRAM_C+0x10) );
	vramcpy((u16*)arg,"fat:",2);
	vramcpy(((u16*)arg)+2,filename,align2(strlen(filename))/2);
	writeAddr((data_t*)VRAM_C+0x10, arg-(u32)VRAM_C);
	writeAddr((data_t*)VRAM_C+0x14, strlen(filename)+5);
	//if(fpassarg){}
}

if(*(vu16*)VRAM_C>=6){ //bootlib v3 with DSi SD
	writeAddr((data_t*)VRAM_C+0x1c, 0); // sorry but sd:/ not supported currently.
}

/*
	if(useDSBooter){
		int i=0;
		for(;i<st.st_size;i+=4)
			if(readAddr((data_t*)VRAM_C+i)==0x54425344)break;
		if(i>=st.st_size)
			{_consolePrint("This boot.bin doesn't support DSBooter extention. Halt.\n");while(1);}
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
	//(*(type_void*)(bootstub+0x08))();
#ifndef _LIBNDS_MAJOR_
	NotifyARM7(ResetBootlib);
#endif
	irqDisable(IRQ_ALL);
#ifndef _LIBNDS_MAJOR_
	IC_InvalidateAll();
	DC_FlushAll();
	DC_InvalidateAll();
#endif

	// Give the VRAM to the ARM7
	VRAM_C_CR = VRAM_ENABLE | VRAM_C_ARM7_0x06000000;

	// Reset into a passme loop
	REG_EXMEMCNT |= ARM7_OWNS_ROM | ARM7_OWNS_CARD;
	_consolePrint("Go!!!\n");
	//*((vu32*)0x02fFFFFC) = 0;
	//*((vu32*)0x02fFFE04) = (u32)0xE59FF018;
	//*((vu32*)0x02fFFE24) = (u32)0x02fFFE04;
	*memUncachedAddr(0x02fFFFFC)=0;
	*memUncachedAddr(0x02fFFE04)=0xE59FF018;
	*memUncachedAddr(0x02fFFE24)=(u32)memUncachedAddr(0x02fFFE04);
#ifdef _LIBNDS_MAJOR_
	resetARM7(0x06000000);
#endif
	swiSoftReset();
	_consolePrint("Failed.\n");while(1);
}

bool runNdsFile(const char* filename){
	struct stat st;
	if (stat (filename, &st) < 0) {
		_consolePrintf("Cannot stat %s\n",filename);return false;
	}
	if(!bootlibAvailable()){
		_consolePrint("bootlib isn't available\n");return false;
	}
	runNds(filename,st.st_ino);//, true, true);//, false);
	return true;
}
#if 0
bool runDSBooter (const char* filename){
	u8 head[512],dec[512];
	struct stat st;
	u8 *pA=NULL,*p7=NULL,*p9=NULL;
	u32 l7,l9,a7,a9,pad7,pad9;

	unsigned char *p=(unsigned char*)ndshead; //break const rule
	FILE *f=fopen(filename,"rb");

	if(!f){
		_consolePrintf("Cannot stat %s\n",filename);return false;
	}
	fstat(fileno(f),&st);
	fread(head,1,512,f);
	fclose(f);
	_consolePrint("Decrypting... ");
	{
		int i=0,j;
		for(;i<0x100;i++){
			for(j=0xa0;j<0xa8;j++)
				dec[j]=head[j]^i;
			if(!memcmp(dec+0xa0,"DSBooter",8))
				{_consolePrintf("key = 0x%02x\n",i);break;}
		}
		if(i==0x100){_consolePrint("Cannot decode or not DSBooter\n");return false;}
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

	return runNds (filename, st.st_ino);//, true, true, true);
}
#endif
//#endif
