/***********************************************************
	Arm9 Soft rest for General purpose

		by Rudolph
***************************************************************/

#include <nds.h>
#include "libprism.h"

static inline void _dmaFillWords(const void* src, void* dest, uint32 size) {
	DMA_SRC(3)  = (uint32)src;
	DMA_DEST(3) = (uint32)dest;
	DMA_CR(3)   = DMA_COPY_WORDS | DMA_SRC_FIX | (size>>2);
	while(DMA_CR(3) & DMA_BUSY);
}

bool ret_menu9_GenM2(const char *menu_nam,const int bypassYSMenu,const char* dumpname){
	u32	hed[16];
	u8	*ldrBuf;
	FILE *ldr;
	u32	siz;

	ldr = fopen(menu_nam, "rb");
	if(ldr == NULL){_consolePrintf("Cannot open %s\n",menu_nam);return false;}

	fread((u8*)hed, 16*4, 1, ldr);
	if(ret_menu9_callbackpre)ret_menu9_callbackpre((u8*)hed);
	siz = 512 + hed[11] + hed[15];
	ldrBuf = (u8*)malloc(siz);
	if(ldrBuf == NULL) {
		fclose(ldr);
		_consolePrintf("Cannot alloc %d bytes\n",siz);
		return false;
	}

	fseek(ldr, 0, SEEK_SET);
	fread(ldrBuf, 512, 1, ldr);

	fseek(ldr, hed[8], SEEK_SET);
	fread(ldrBuf + 512, hed[11], 1, ldr);

	fseek(ldr, hed[12], SEEK_SET);
	fread(ldrBuf + 512 + hed[11], hed[15], 1, ldr);

	fclose(ldr);

	*memUncachedAddr(0x02fFFDF4)=(u32)ldrBuf;
	if(ret_menu9_callback)ret_menu9_callback(ldrBuf);

	if(!argvToInstall)makeargv(menu_nam);
	installargv(ldrBuf,(char*)0x02fff400);
	_consolePrint("applying dldi...\n"); //Actually ldrBuf isn't a NDS itself, dldi patching works.
	dldi2(ldrBuf,siz,bypassYSMenu,dumpname);
	disc_unmount();

	_consolePrint("Rebooting...\n");
	NotifyARM7(ResetRudolphM);
	DC_FlushAll();

	//start
 	register int i;
	//clear out ARM9 DMA channels
	for (i=0; i<4; i++) {
		DMA_CR(i) = 0;
		DMA_SRC(i) = 0;
		DMA_DEST(i) = 0;
		TIMER_CR(i) = 0;
		TIMER_DATA(i) = 0;
	}

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

	REG_IME = 0;
	REG_IE = 0;
	REG_IF = ~0;

	IC_InvalidateAll();
	DC_FlushAll();
	DC_InvalidateAll();

	//r0 and r1 are destroyed
if(GetRunningMode()){
	asm(
     		"ldr	r0, =0x0cfFFDF8\n"
     		"ldr	r1, =0xE51FF004\n"
    	 	"str	r1, [r0, #0x0]\n"			// (0cfffdf8)=E51FF004:ldr r15,[r15, #-0x4]
    	 	"str	r0, [r0, #0x4]\n"			// (0cfffdfC)=0cfFFDF8

		"bx	r0\n"				// JUMP 0cfFFDF8
	);
}else{
	asm(
     		"ldr	r0, =0x02fFFDF8\n"
     		"ldr	r1, =0xE51FF004\n"
    	 	"str	r1, [r0, #0x0]\n"			// (02fffdf8)=E51FF004:ldr r15,[r15, #-0x4]
    	 	"str	r0, [r0, #0x4]\n"			// (02fffdfC)=02fFFDF8

		"bx	r0\n"				// JUMP 02fFFDF8
	);
}
	while(1);
}

bool ret_menu9_GenM(const char *menu_nam){return ret_menu9_GenM2(menu_nam,0,NULL);}
