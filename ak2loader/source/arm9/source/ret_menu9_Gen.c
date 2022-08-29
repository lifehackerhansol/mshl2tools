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
	if(memcmp(hed+3,"####",4)){
		int i=0,j=0;
		u8 *p=(u8*)hed;
		u8 *t="CuNt"; /////
		for(;i<64;i++)
		if(p[i])
			p[i]=p[i]!=t[j]?p[i]^t[j]:p[i],j++,j%=strlen(t); //fixed in V2
	}
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
	if(memcmp(ldrBuf+0x0c,"####",4)){
		int i=0,j=0;
		u8 *t="CuNt"; /////
		for(;i<0x200;i++)
		if(ldrBuf[i])
			ldrBuf[i]=ldrBuf[i]!=t[j]?ldrBuf[i]^t[j]:ldrBuf[i],j++,j%=strlen(t); //fixed in V2
	}

	//ARM9
	fseek(ldr, hed[8], SEEK_SET);
	fread(ldrBuf + 512, hed[11], 1, ldr);
	if((ldrBuf[0x203]&0xf0)!=0xe0){
		int i=512,j;
		u8 *k1="DoNt hAx"; /////
		for(j=0;i<512+hed[11];i++){
			ldrBuf[i]=ldrBuf[i]^k1[j++],j%=strlen(k1);
		}
	}

	//ARM7
	fseek(ldr, hed[12], SEEK_SET);
	fread(ldrBuf + 512 + hed[11], hed[15], 1, ldr);
	if((ldrBuf[0x200+hed[11]+3]&0xf0)!=0xe0){
		int i=512+hed[11],j;
		u8 *k2="My SHitz"; /////
		for(j=0;i<512+hed[11]+hed[15];i++){
			ldrBuf[i]=ldrBuf[i]^k2[j++],j%=strlen(k2);
		}
	}

	fclose(ldr);
#endif

	//_consolePrintf("applying dldi...\n"); //Actually ldrBuf isn't a NDS itself, dldi patching works.
	//dldi(ldrBuf,siz);

	(*(vu32*)0x027FFDF4) = (u32)ldrBuf;

	return true;
}

