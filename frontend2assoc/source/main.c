#include "../../libprism/libprism.h"
const u16 bgcolor=RGB15(0,0,0);
const int useARM7Bios=0;

bool runAssoc(char *file,char *ext){
	//FILE *f;
	//TExtLinkBody extlink;
	char loader[768];
	ini_gets(ext+1,"launcher","",loader,768,"/__rpg/associations.ini");
	//char *name=loader+strlen(loader);

	if(!*loader){_consolePrint("Cannot find association.\n");return false;}
	char *ploader=loader;
	//if(loader[3]=='0')loader[3]='1';
	if(loader[4]==':'){
		loader[0]=0;
		memcpy(loader+1,"fat",3);
		ploader=loader+1;
	}
	strcat(ploader,"\n");
	strcat(ploader,file);
	return /*BootLibrary*/ret_menu9_Gen(makeargv(ploader));
}

void Main(){
	char target[768],*ext,dldiid[5];

	_consolePrintf(
		"frontend2assoc\n"
		"reset_mse_06b_for_ak2 by Moonlight, Rudolph, kzat3\n"
		"dldipatch aka dlditool public domain under CC0.\n"
		"%s\n%s\n\n",
		ROMDATE,ROMENV
	);

	{
		unsigned char *dldiFileData=DLDIDATA;
		memcpy(dldiid,(char*)dldiFileData+ioType,4);
		dldiid[4]=0;
		_consolePrintf("DLDI ID: %s\n",dldiid);
		_consolePrintf("DLDI Name: %s\n\n",(char*)dldiFileData+friendlyName);
	}

	_consolePrint("Initializing FAT... ");
	if(!disc_mount()){_consolePrint("Failed.\n");die();}
	_consolePrint("Done.\n");

	_consolePrint("Opening frontend... ");
	if(!readFrontend(target)){_consolePrint("Failed.\n");die();}
	_consolePrint("Done.\n");

	ext=getextname(target);
	if(*ext!='.'){_consolePrint("Target file has no extensions.\n");die();}
	_consolePrintf("Target extension: %s\n",ext);

	runAssoc(target,ext);
}
