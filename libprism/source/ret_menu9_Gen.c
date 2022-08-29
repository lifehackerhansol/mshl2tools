/***********************************************************
	Arm9 Soft rest for General purpose

		by Rudolph (çcíÈ)
***************************************************************/

#include <nds.h>
//#include <nds/registers_alt.h>	// devkitPror20
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

	*(vu32*)0x027FFDF4=(u32)ldrBuf;
	if(ret_menu9_callback)ret_menu9_callback(ldrBuf);

	installargv(ldrBuf,(char*)0x023ff400,menu_nam);
	_consolePrintf("applying dldi...\n"); //Actually ldrBuf isn't a NDS itself, dldi patching works.
	dldi2(ldrBuf,siz,bypassYSMenu,dumpname);

	_consolePrintf("Rebooting...\n");
	DC_FlushAll();
	IPCZ->cmd=ResetRudolph;
	ret_menu9_GENs();
	_consolePrintf("Failed.\n");die();
}

bool ret_menu9_Gen(const char *menu_nam){return ret_menu9_Gen2(menu_nam,0,NULL);}
