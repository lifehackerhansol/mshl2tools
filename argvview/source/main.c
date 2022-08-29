#include "../../libprism/libprism.h"
const u16 bgcolor=RGB15(0,15,5);
const int useARM7Bios=0;

void Main(){
	int i=0;

	_consolePrintf(
		"ARGV Viewer\n"
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
	_consolePrint("Done.\n\n");

	//getcwd(libprism_buf,768);
	_consolePrintf("getcwd = %s\n",mydrive);

	_consolePrintf("argc = %d\n",argc);
	for(;i<argc;i++)_consolePrint(argv[i]),_consolePrintChar('\n');

	//makeargv("xxx xxx xxx");_consolePrintf("%d %s\n",argvToInstallSize,argvToInstall);
	_consolePrintChar('\n');die();

	// ^^^^^^^^^^^^ add 2008.03.30 kzat3
}
