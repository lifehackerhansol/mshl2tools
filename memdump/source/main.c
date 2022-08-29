#include "../../libprism/libprism.h"
const u16 bgcolor=RGB15(0,15,5);

void Main(){
	FILE *f;

	_consolePrintf(
		"Memory Dumper\n"
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
	_consolePrint("Dumping memory...\n");
	{
		int i=0,j;
		f=fopen("/MEMDUMP.BIN","wb");
		_consoleStartProgress2();
		for(;i<4;i++)
			for(j=0;j<1024*1024;){
				fwrite((u8*)0x02000000+i*1024*1024+j,65536,1,f);
				j+=65536;
				_consolePrintProgress2("Dumping",i*1024*1024+j,4*1024*1024);
			}
		_consoleEndProgress2();
		fclose(f);
	}
	_consolePrint("Dump proceeded.\n");die();

	// ^^^^^^^^^^^^ add 2008.03.30 kzat3
}
