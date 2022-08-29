/*
	dldipatch aka dlditool public domain
	under Creative Commons CC0

	According to ndsdis2 -NH9 0x00 dldi_startup_patch.o (from NDS_loader build result):
	:00000040 E3A00001 mov  r0,#0x1 ;r0=1(0x1)
	:00000044 E12FFF1E bx r14 (Jump to addr_00000000?)
	So the corresponding memory value is "\x01\x00\xa0\xe3\x1e\xff\x2f\xe1" (8 bytes).

*/

#include "libprism.h"

//ARM7 not officially supported
#define printf _consolePrintf2

const byte *dldimagic=(byte*)"\xed\xa5\x8d\xbf Chishm";

//OK now we are prepared. DLDI routine follows.

int tunedldi(const char *name, const char *id, int *size, byte **p, int checkstart){
	FILE *f;
	byte *x;
	struct stat st;
	if(!(f=fopen(name,"rb")))return 1;
	fstat(fileno(f),&st);
	if(st.st_size<0x80||!(x=malloc(st.st_size))){fclose(f);return 2;}
	fread(x,1,st.st_size,f);
	fclose(f);
	if(memcmp(x+ioType,id,4)){free(x);return 3;}
	if(checkstart&&!memcmp(x+(*(u32*)(x+dldiStartup)-*(u32*)(x+dataStart)),"\x01\x00\xa0\xe3\x1e\xff\x2f\xe1",8)){free(x);return 4;}
	*p=x;*size=*((u32*)(x+bssStart))-*((u32*)(x+dataStart));return 0;
}

#define torelative(n) (read32(pA+n)-pAdata)

int dldi2(byte *nds,const int ndslen,const int bypassYSMenu,const char* dumpname){
	byte *pD=DLDIToBoot;
	int dldilen;

	byte *pA=NULL,id[5],space;
	u32 reloc,pAdata,pDdata,pDbssEnd,fix;
	int i,ittr;

	if(!pD)return 0; //don't patch

if(nds){
	for(i=0;i<ndslen-0x80;i+=4){
		if(!memcmp(nds+i,dldimagic,12)&&(read32(nds+i+dldiVersion)&0xe0f0e0ff)==1){
			pA=nds+i;
			printf("Section 0x%08x: ",i);
			if(*((u32*)(pD+bssEnd))-*((u32*)(pD+dataStart)) > 1<<pA[allocatedSpace]){
				printf("Available %dB, need %dB. ",1<<pA[allocatedSpace],*((u32*)(pD+bssEnd))-*((u32*)(pD+dataStart)));
				//if(ignoresize){printf("searching interrupted.\n");break;}
				printf("continue searching.\n");pA=NULL;continue;
			}
			printf("searching done.\n");
			break;
		}
	}
	if(!pA){printf("not found valid dldi section\n");return 1;}
}

	//Now we have to tune in the dldi...
	memcpy(id,pD+ioType,4);id[4]=0;
	dldilen=*((u32*)(pD+bssStart))-*((u32*)(pD+dataStart)); //DLDITool 0.32.4
	if(pD!=DLDIDATA)goto done;

if(bypassYSMenu){
	if(!memcmp(id,"RPGS",4)&&!memcmp(pD+friendlyName,"Acekard AK2",11)){
		pD=NULL;
		printf(
			"Oh dear. I'm patched with akaio DLDI.\n"
			"YSMenu will get upset and burry me.\n"
			"I have to beg a pardon\n"
			"by feeding akmenu DLDI.\n"
		);
		if(tunedldi("/__AK2/AK2_SD.DLDI",(char*)id,&dldilen,&pD,0))
			{printf("Cannot load /__AK2/AK2_SD.DLDI.\n");goto akaiofail;}
		if(!memcmp(pD+friendlyName,"Acekard AK2",11))
			{printf("/__AK2/AK2_SD.DLDI is also akaio DLDI. What a shame!\n");goto akaiofail;}
		goto done;

akaiofail:
		free(pD);printf("I cannot do anything. I have to run away!\n\nNobody knows where he is now...\n");die();
	}
	if(!memcmp(pD+ioType,"DEMO",4))memcpy(pD+ioType,"TTIO",4);
	if(!memcmp(pD+ioType,"R4DS",4))memcpy(pD+ioType,"RPGS",4);
}

	if(memcmp(*(void**)(pD+dldiStartup),"\x01\x00\xa0\xe3\x1e\xff\x2f\xe1",8)) //z=="mov r0,#1;bx lr"
		goto done;

	printf("Startup is nullified. Cannot be used for patching. Trying to fall back to MoonShell2.\n");
	if(memcmp(pD+(*(u32*)(pD+dldiStartup)-*(u32*)(pD+dataStart)),"\x01\x00\xa0\xe3\x1e\xff\x2f\xe1",8))
		{printf("Startup is not nullified by alternative calculation. Something is strange. Halted.\n");die();}
	tunedldi("/MOONSHL2/DLDIBODY.BIN",(char*)id,&dldilen,&pD,1);
	printf("Tuned. Now we selected dldi file to patch with.\n");
done:

if(dumpname){
	FILE *f;
	PrintfToDie("Writing to %s\n",dumpname);
	if(!(f=fopen(dumpname,"wb"))){
		PrintfToDie("Cannot open %s\n",dumpname);
		if(!nds)return -1;
	}else{
		fwrite(pD,1,dldilen,f);
		fclose(f);
	}
}

	if(!nds)return 0;
/*
	if(*((u32*)(pD+bssEnd))-*((u32*)(pD+dataStart)) > 1<<pA[allocatedSpace])
		{printf("not enough space. available %d bytes, need %d bytes\n",1<<pA[allocatedSpace],*((u32*)(pD+bssEnd))-*((u32*)(pD+dataStart)));return 2;}
*/
	space=pA[allocatedSpace];

	pAdata=read32(pA+dataStart);if(!pAdata)pAdata=read32(pA+dldiStartup)-dldiData;
	memcpy(id,pA+ioType,4);id[4]=0;
	printf("Old ID=%s, Interface=0x%08x,\nName=%s\n",id,pAdata,pA+friendlyName);
	memcpy(id,pD+ioType,4);id[4]=0;
	printf("New ID=%s, Interface=0x%08x,\nName=%s\n",id,pDdata=read32(pD+dataStart),pD+friendlyName);
	printf("Relocation=0x%08x, Fix=0x%02x\n",reloc=pAdata-pDdata,fix=pD[fixSections]); //pAdata=pDdata+reloc
	printf("dldiFileSize=0x%04x, dldiMemSize=0x%04x\n",dldilen,*((u32*)(pD+bssEnd))-*((u32*)(pD+dataStart)));

	memcpy(pA,pD,dldilen);pA[allocatedSpace]=space;
	for(ittr=dataStart;ittr<ioType;ittr+=4)write32(pA+ittr,read32(pA+ittr)+reloc);
	for(ittr=dldiStartup;ittr<dldiData;ittr+=4)write32(pA+ittr,read32(pA+ittr)+reloc);
	pAdata=read32(pA+dataStart);pDbssEnd=read32(pD+bssEnd);

	if(fix&fixAll)
		for(ittr=torelative(dataStart);ittr<torelative(dataEnd);ittr+=4)
			if(pDdata<=read32(pA+ittr)&&read32(pA+ittr)<pDbssEnd)
				printf("All  0x%04x: 0x%08x -> 0x%08x\n",ittr,read32(pA+ittr),read32(pA+ittr)+reloc),
				write32(pA+ittr,read32(pA+ittr)+reloc);
	if(fix&fixGlue)
		for(ittr=torelative(glueStart);ittr<torelative(glueEnd);ittr+=4)
			if(pDdata<=read32(pA+ittr)&&read32(pA+ittr)<pDbssEnd)
				printf("Glue 0x%04x: 0x%08x -> 0x%08x\n",ittr,read32(pA+ittr),read32(pA+ittr)+reloc),
				write32(pA+ittr,read32(pA+ittr)+reloc);
	if(fix&fixGot)
		for(ittr=torelative(gotStart);ittr<torelative(gotEnd);ittr+=4)
			if(pDdata<=read32(pA+ittr)&&read32(pA+ittr)<pDbssEnd)
				printf("Got  0x%04x: 0x%08x -> 0x%08x\n",ittr,read32(pA+ittr),read32(pA+ittr)+reloc),
				write32(pA+ittr,read32(pA+ittr)+reloc);
	if(fix&fixBss)
		memset(pA+torelative(bssStart),0,pDbssEnd-read32(pD+bssStart));

	if(pD&&pD!=DLDIDATA&&pD!=DLDIToBoot)free(pD);

	printf("Patched successfully\n");
	return 0;
}

int dldi(byte *nds,const int ndslen){return dldi2(nds,ndslen,0,NULL);}
