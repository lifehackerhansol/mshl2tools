/***********************************************************
	Arm9 Soft rest for General purpose

		by Rudolph
***************************************************************/

#include <nds.h>
#include "libprism.h"

bool ret_menu9_Gen2(const char *menu_nam,const int bypassYSMenu,const char* dumpname){
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
	NotifyARM7(ResetRudolph);
	IC_InvalidateAll();
	DC_FlushAll();
	DC_InvalidateAll();
	ret_menu9_GENs();
	_consolePrint("Failed.\n");die();
}

bool ret_menu9_Gen(const char *menu_nam){return ret_menu9_Gen2(menu_nam,0,NULL);}
