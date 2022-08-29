#include "../../libprism/libprism.h"
const u16 bgcolor=RGB15(10,10,0);

void Main(){
	char dldiid[5];
	unsigned char *dldiFileData=DLDIDATA;

	_consolePrintf(
		"m3 region check\n"
		//"reset_mse_06b_for_ak2 by Moonlight, Rudolph, kzat3\n"
		//"dldipatch aka dlditool public domain under CC0.\n"
		"%s\n%s\n\n",
		ROMDATE,ROMENV
	);

	u32 type=R4_ReadCardInfo()&0x3ff;
	_consolePrintf("Jumper: 0x%03X (%d%d%d-%d%d%d-%d%d%d)\n",type,
		(type>>8)&1,(type>>7)&1,(type>>6)&1,(type>>5)&1,(type>>4)&1,(type>>3)&1,(type>>2)&1,(type>>1)&1,(type>>0)&1);
	type=(u8)M3_ReadCardRegion()&0xff;
	_consolePrintf("Region: 0x%03X (%d-%d%d%d-%d%d-%d%d)\n",type,
		(type>>7)&1,(type>>6)&1,(type>>5)&1,(type>>4)&1,(type>>3)&1,(type>>2)&1,(type>>1)&1,(type>>0)&1);
	_consolePrint("\n");

	_consolePrint2(
		"Jumper:  flag1 jumper flag2\n"
		"[R4/M3S] 1 1 1        1 0 0\n"
		"[M3Real] 0 1 1        1 0 0\n"
		"[M3]           0 x x\n"
		"[R4]           1 x x\n"
		"[JPN]          x 0 1        [CC/EC]\n"
		"[ENG]          x 1 0        [D4/F4]\n"
		"[GB]           x 1 1        [DC/FC]\n"
		"\n"
		"Region:  type cart  fw  lng\n"
		"[M3Real]    1-0 1 0-0 1\n"
		"[JPN]                   0 0 [A4]\n"
		"[ENG]                   1 1 [A7]\n"
		"[GB]                    0 1 [A5]\n"
		"[iTDS]      1-0 1 1-1 0\n"
		"[JPN]                   0 0 [B8]\n"
		"[ENG/GB]                1 0 [BA]\n"
		"[Sakura]    1-1 0 0-0 1\n"
		"[JPN]                   0 0 [C4]\n"
		"[ENG]                   1 1 [C7]\n"
		"[GB]                    1 0 [C6]\n"
		"[R4iRTS]    1-0 1 1-0 0-1 1 [B3]\n"
		"[M3iZero]   1-0 1 1-0 1\n"
		"[JPN]                   0 0 [B4]\n"
		"[ENG]                   1 1 [B7]\n"
		"[GB]                    1 0 [B6]\n"
		"[M3i0 G003] 0-1 0 1-0 0-1 1 [53] (?)\n"
	);

	{
		memcpy(dldiid,(char*)dldiFileData+ioType,4);
		dldiid[4]=0;
		_consolePrintf("DLDI ID: %s\n",dldiid);
		_consolePrintf("DLDI Name: %s\n\n",(char*)dldiFileData+friendlyName);
	}

	_consolePrint("Initializing FAT...o exit)... ");
	if(!disc_mount()){_consolePrint("Failed.\n");die();}
	_consolePrint("Done.\n");

	die();
}
