#include "libprism.h"

///// BootNDSROMex_final / BootNDSROMex2 should be placed in VRAM for large NDS... /////

extern attrnoinline attrnoreturn void resetMemory2load_ARM9_NoBIOS();

static inline void _dmaFillWords(const void* src, void* dest, uint32 size) {
	DMA_SRC(3)  = (uint32)src;
	DMA_DEST(3) = (uint32)dest;
	DMA_CR(3)   = DMA_COPY_WORDS | DMA_SRC_FIX | (size>>2);
	while(DMA_CR(3) & DMA_BUSY);
}

static attrnoreturn void BootNDSROMex_final(){
	u32 i=0;
	//vu32 w;
	vu32 *MoonShellResetFlag=memUncachedAddr(0x02fffffc);
	*MoonShellResetFlag=0;
	IPCZ->cmd=ResetMoonShell2;
/*
	IPCZ->MoonShellReset=1;
	_consolePrint("ARM7Wait: goto reboot function.");
	while(IPCZ->MoonShellReset!=2){
		for(vu32 w=0;w<0x100;w++);
	}
*/
	IC_InvalidateAll();
	DC_FlushAll();
	DC_InvalidateAll();

	_consolePrint("ARM7Wait... ");
	while(*MoonShellResetFlag!=4){
		// ARM7Wait: Copy EWRAM to ARM7InternalMemory. and, Reset memory.
		//for(w=0;w<0x100;w++);
	}
	_consolePrint("OK.\n");
	for (i=0; i<4; i++) {
		DMA_CR(i) = 0;
		DMA_SRC(i) = 0;
		DMA_DEST(i) = 0;
		TIMER_CR(i) = 0;
		TIMER_DATA(i) = 0;
	}
///
	VRAM_CR = 0x80808080;
	(*(vu32*)0x02fFFE04) = 0;   // temporary variable
	//BG_PALETTE[0] = 0xFFFF;
	_dmaFillWords((void*)0x02fFFE04, BG_PALETTE, (2*1024));
	_dmaFillWords((void*)0x02fFFE04, BG_PALETTE_SUB, (2*1024));
	_dmaFillWords((void*)0x02fFFE04, OAM,     2*1024);
	_dmaFillWords((void*)0x02fFFE04, OAM_SUB,     2*1024);
	_dmaFillWords((void*)0x02fFFE04, (void*)0x04000000, 0x56);  //clear main display registers
	_dmaFillWords((void*)0x02fFFE04, (void*)0x04001000, 0x56);  //clear sub  display registers
	_dmaFillWords((void*)0x02fFFE04, VRAM,  656*1024);
	
	REG_DISPSTAT=0;
	videoSetMode(0);
	videoSetModeSub(0);
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
	REG_POWERCNT = 0x820F;
	//REG_EXMEMCNT = 0xe880;

	//set shared ram to ARM7
	WRAM_CR = 0x03;
///
	//_consolePrint("resetMemory2load_ARM9_NoBIOS\n");

	REG_IME = 0;
	REG_IE = 0;
	REG_IF = ~0;

	type_void _menu9_Gen;
	u32 *adr = (u32*)memUncachedAddr(0x02FFE800);
	u32 *buf = (u32*)resetMemory2load_ARM9_NoBIOS;
	for(i = 0; i < 0x100/4; i++) {
		*adr = *buf;
		adr++;
		buf++;
	}
	_menu9_Gen = (type_void)memUncachedAddr(0x02FFE800);
	_menu9_Gen();
	while(1);
}

bool BootNDSROMex2(const char *pFilename,const int bypassYSMenu,const char* dumpname){
	struct stat st;
	u32 FileSize;
	FILE *FileHandle=fopen(pFilename,"rb");
  
	if(FileHandle==NULL)
		{_consolePrintf("Can not open NDS file %s.\n",pFilename);return false;}

	fstat(fileno(FileHandle),&st);
	FileSize=st.st_size;
  
	if(FileSize<512)
		{_consolePrintf("Can not open NDS file %s.\n",pFilename);return false;}

	_consolePrint("MoonShell2 Loader preparing.\n");
	_consolePrint("Trying to load file\n");

	//modified by X: Use 0x02fffe00 rather than IPC TARMInfo.
	u32 *header=memUncachedAddr(0x02fffe00);
	{
		//u32 header[0x40/4];
		fread(header,1,0x200,FileHandle);

		//volatile TARMInfo *pai=&IPCZ->ARMInfo7;
		//pai->pCopyFrom=(void*)header[0x30/4];
		//pai->pCopyTo=(void*)header[0x38/4];
		//pai->CopySize=header[0x3c/4];
		//pai->ExecAddr=header[0x34/4];
		//_consolePrintf("ARM7 CopyFrom=0x%08x, CopyTo=0x%08x, CopySize=%dbyte, ExecAddr=0x%08x.¥n",pai->pCopyFrom,pai->pCopyTo,pai->CopySize,pai->ExecAddr);
		fseek(FileHandle,header[0x30/4],SEEK_SET);
		header[0x30/4]=(u32*)malloc(header[0x3c/4]);
		if(header[0x30/4]==0){_consolePrintf("Large ARM7 binary size. %dbyte.\n",header[0x3c/4]);die();}
		fread((u32*)header[0x30/4],1,header[0x3c/4],FileHandle);
		_consolePrintf("ARM7: ptr=%08x.\n",header[0x30/4]);

		//pai=&IPCZ->ARMInfo9;
		//pai->pCopyFrom=(void*)header[0x20/4];
		///// It seems that moonshell2load.arm.c requires uncached memory address although I did DC_InvalidateAll(). Cached address handling is worse than RVDS? /////
		header[0x28/4] = memUncachedAddr(header[0x28/4]); //pai->pCopyTo=memUncached( (void*)header[0x28/4] );
		//pai->CopySize=header[0x2c/4];
		//pai->ExecAddr=header[0x24/4];
		//_consolePrintf("ARM9 CopyFrom=0x%08x, CopyTo=0x%08x, CopySize=%dbyte, ExecAddr=0x%08x.¥n",pai->pCopyFrom,pai->pCopyTo,pai->CopySize,pai->ExecAddr);
		fseek(FileHandle,header[0x20/4],SEEK_SET);
		header[0x20/4]=malloc(header[0x2c/4]);
		if(header[0x20/4]==0){_consolePrintf("Large ARM9 binary size. %dbyte.\n",header[0x2c/4]);die();}
		fread((u32*)header[0x20/4],1,header[0x2c/4],FileHandle);
		_consolePrintf("ARM9: ptr=%08x, to=%08x.\n",header[0x20/4],header[0x28/4]);

#if 0
		{
			//volatile TARMInfo *pai7=&IPCZ->ARMInfo7;
			//volatile TARMInfo *pai9=&IPCZ->ARMInfo9;
			//IPCZ->RequestClearMemory=false;
/*
			u32 ARM7CopyTo=header[0x38/4];
			u32 ARM9CopyTo=header[0x28/4];
			if((ARM7CopyTo<0x037f8000)||(0x0380f400<=ARM7CopyTo)) IPCZ->RequestClearMemory=false;
			if((ARM9CopyTo<0x02000000)||(0x023ff000<=ARM9CopyTo)) IPCZ->RequestClearMemory=false;
			if(IPCZ->RequestClearMemory==true) _consolePrintf("Enable memory clean-up function.\n");
*/
		}
#endif
	}
	fclose(FileHandle);

	_consolePrint("Applying DLDI...\n");
	{
		//volatile TARMInfo *pai=&IPCZ->ARMInfo9;
		dldi2((u8*)header[0x20/4],header[0x2c/4],bypassYSMenu,dumpname);
	}
	disc_unmount();
	_consolePrint("Rebooting...\n");
	installargv((u8*)header,(char*)0x02fff400,pFilename);

	//actual reset
	BootNDSROMex_final();
	return true;
}

bool BootNDSROMex(const char *pFilename){return BootNDSROMex2(pFilename,0,NULL);}

