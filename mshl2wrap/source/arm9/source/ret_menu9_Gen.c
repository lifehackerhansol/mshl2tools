/***********************************************************
	Arm9 Soft rest for General purpose

		by Rudolph (çcíÈ)
***************************************************************/

#include <nds.h>
//#include <nds/registers_alt.h>	// devkitPror20
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if 0	// change 2008.03.30 kzat3
#include "disc_io.h"
#include "gba_nds_fat.h"
#else
#include <fat.h>
#endif

#define FAT_FT_END	(0)
#define FAT_FT_FILE	(1)
#define FAT_FT_DIR	(2)

#if 0	// change 2008.03.30 kzat3
bool ret_menu9_Gen()
#else
bool ret_menu9_Gen(char *menu_nam)
#endif
{
	u32	hed[16];
	u8	*ldrBuf;
#if 0	// change 2008.03.30 kzat3
	FAT_FILE *ldr;
#else
	FILE *ldr;
#endif
	u32	siz;

#if 0	// change 2008.03.30 kzat3
	ldr = FAT_fopen(menu_nam, "rb");
#else
	//if(fatInitDefault() == false) {
	//	return false;
	//}
	ldr = fopen(menu_nam, "rb");
#endif
	if(ldr == NULL)	return false;

#if 0	// change 2008.03.30 kzat3
	FAT_fread((u8*)hed, 16*4, 1, ldr);
#else
	fread((u8*)hed, 16*4, 1, ldr);
#endif
	siz = 512 + hed[11] + hed[15];
	ldrBuf = (u8*)malloc(siz);
	if(ldrBuf == NULL) {
#if 0	// change 2008.03.30 kzat3
		FAT_fclose(ldr);
#else
		fclose(ldr);
#endif
		return false;
	}

#if 0	// change 2008.03.30 kzat3
	FAT_fseek(ldr, 0, SEEK_SET);
	FAT_fread(ldrBuf, 512, 1, ldr);

	FAT_fseek(ldr, hed[8], SEEK_SET);
	FAT_fread(ldrBuf + 512, hed[11], 1, ldr);

	FAT_fseek(ldr, hed[12], SEEK_SET);
	FAT_fread(ldrBuf + 512 + hed[11], hed[15], 1, ldr);

	FAT_fclose(ldr);
#else
	fseek(ldr, 0, SEEK_SET);
	fread(ldrBuf, 512, 1, ldr);

	fseek(ldr, hed[8], SEEK_SET);
	fread(ldrBuf + 512, hed[11], 1, ldr);

	fseek(ldr, hed[12], SEEK_SET);
	fread(ldrBuf + 512 + hed[11], hed[15], 1, ldr);

	fclose(ldr);
#endif

	_consolePrintf("Applying dldi...\n"); //Actually ldrBuf isn't a NDS itself, dldi patching works.
	dldi(ldrBuf,siz);//while(1);

	(*(vu32*)0x027FFDF4) = (u32)ldrBuf;

	return true;
}

