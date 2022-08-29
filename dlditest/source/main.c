#include "../../libprism/libprism.h"
const u16 bgcolor=RGB15(0,15,5);

void Main(){
	u8 buf[512];
	_consolePrintf(
		"DLDI Test\n"
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

	_consolePrint("Initializing DLDI... ");
	if(!disc_startup()){_consolePrint("Failed.\n");die();}
	_consolePrint("Done.\n");

	_consolePrint("Reading sector 0... ");
	if(!disc_readSectors(0,1,buf)){_consolePrint("Failed.\n");die();}
	_consolePrint("Done.\n");

	_consolePrint("Shutting down DLDI... ");
	if(!disc_shutdown()){_consolePrint("Failed.\n");die();}
	_consolePrint("Done.\n");

	_consolePrintf("BootSig is %02X%02X: Test ",buf[510],buf[511]);
	_consolePrint((buf[510]==0x55&&buf[511]==0xaa)?"Success.\n":"Failed.\n");
	die();
}