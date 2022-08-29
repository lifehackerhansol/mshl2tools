#include "../../libprism/libprism.h"
const u16 bgcolor=RGB15(0,0,0);
const int useARM7Bios=0;

void Main(){
	FILE *f;

	_consolePrintf(
		"GBA Dumper\n"
		"%s\n%s\n\n",
		ROMDATE,ROMENV
	);

	{
		unsigned char dldiid[5];
		unsigned char *dldiFileData=DLDIDATA;
		memcpy(dldiid,(char*)dldiFileData+ioType,4);
		dldiid[4]=0;
		_consolePrintf("DLDI ID: %s\n",dldiid);
		_consolePrintf("DLDI Name: %s\n\n",(char*)dldiFileData+friendlyName);
	}

	_consolePrint("Initializing FAT... ");
	if(!disc_mount()){_consolePrint("Failed.\n");die();}
	_consolePrint("Done.\n");

	// vvvvvvvvvvv add 2008.03.30 kzat3
	_consolePrint("Dumping slot2...\n");
	{
		int i=0,j;
		f=fopen("/GBADUMP.GBA","wb");
		_consoleStartProgress2();
		for(;i<32;i++)
			for(j=0;j<1024*1024;){
				fwrite((u8*)0x08000000+i*1024*1024+j,65536,1,f);
				j+=65536;
				_consolePrintProgress2("Dumping",i*1024*1024+j,32*1024*1024);
			}
		_consoleEndProgress2();
		fclose(f);
	}

	_consolePrint("Dump processed.\n");die();

	// ^^^^^^^^^^^^ add 2008.03.30 kzat3
}
