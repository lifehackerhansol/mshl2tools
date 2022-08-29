#include "../../libprism/libprism.h"
const u16 bgcolor=RGB15(0,15,5);
const int useARM7Bios=0;

void Main(){
	FILE *f;
	//int i=0;

	_consolePrintf(
		"ARGV Loader Extlink\n"
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

	if(!readFrontend(libprism_cbuf)){_consolePrint("cannot read frontend.\n");die();}
	f=fopen(libprism_cbuf,"rb");
	if(!f){_consolePrintf("cannot open %s.\n",libprism_buf);die();}
	fread(libprism_cbuf,1,filelength(fileno(f)),f);
	fclose(f);
	//makeargv(libprism_buf);
	ret_menu9_Gen(makeargv(libprism_cbuf));

	_consolePrint("Failed.\n");
	die();
}
