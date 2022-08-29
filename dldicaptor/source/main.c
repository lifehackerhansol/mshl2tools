#include "../../libprism/libprism.h"
const u16 bgcolor=RGB15(0,15,5);

void Main(){
	_consolePrintf(
		"DLDI Captor\n"
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

	_consolePrintf("Initializing libfat... ");
	if(!fatInitDefault()){_consolePrintf("Failed.\n");die();}
	_consolePrintf("Done.\n");

	// vvvvvvvvvvv add 2008.03.30 kzat3
	_consolePrintf("Capturing DLDI...\n");
{
	char dldiname[768];
	strcpy(dldiname,"/");
	memcpy(dldiname+1,(char*)DLDIDATA+ioType,4);
	dldiname[5]='_';
	char *pname=dldiname+6,*friendlyname=(char*)DLDIDATA+friendlyName;
	int i=0;
	for(;i<strlen(friendlyname);i++)
		if(0x20<=friendlyname[i] && friendlyname[i]<0x7f && !strchr("\\/:*?\"<>|",friendlyname[i]))
			*pname++=friendlyname[i];
	strcpy(pname,".dldi");
	dldi2(NULL,0,0,dldiname);
}
	_consolePrintf("DLDI Extraction proceeded (see log for result).\n");die();

	// ^^^^^^^^^^^^ add 2008.03.30 kzat3
}
