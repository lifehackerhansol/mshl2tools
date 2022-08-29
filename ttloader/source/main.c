#include "../../libprism/libprism.h"
const u16 bgcolor=RGB15(4,0,12);
const int useARM7Bios=0;

void Main(){
	char file[256*3]="";

	char utf8[768];
	char name[0x1001];
	//TExtLinkBody extlink;
	FILE *f;

	_consolePrintf(
		"ttloader technology preview\n"
		"reset_mse_06b_for_ak2 by Moonlight, Rudolph, kzat3\n"
		"dldipatch aka dlditool public domain under CC0.\n"
		"%s\n%s\n\n",
		ROMDATE,ROMENV
	);

	char dldiid[5];
	{
		unsigned char *dldiFileData=DLDIDATA;
		memcpy(dldiid,(char*)dldiFileData+ioType,4);
		dldiid[4]=0;
		_consolePrintf("DLDI ID: %s\n",dldiid);
		_consolePrintf("DLDI Name: %s\n\n",(char*)dldiFileData+friendlyName);
	}
	DLDIToBoot=NULL; //kill dldipatch

	//_consolePrint("Waiting... ");
	//sleep(1);
	//_consolePrint("Done.\n");

	_consolePrint("Initializing FAT... ");
	if(!disc_mount()){_consolePrint("Failed.\n");die();}
	_consolePrint("Done.\n");

	_consolePrint("Opening frontend... ");
	if(!readFrontend(utf8)){_consolePrint("Failed.\n");die();}
	_consolePrint("Done.\n");

	_consolePrint("Setting loader... ");
	if(!strcmp(dldiid,"TTIO")){
		if(!strcpy_safe(file,findpath(3,(char*[]){"/YSMenu/","/_SYSTEM_/","/TTMenu/"},"ttpatch.dat")))goto loader_fail;
	}else if(!strcmp(dldiid,"R4TF")){
		if(!strcpy_safe(file,findpath(3,(char*[]){"/YSMenu/","/_SYSTEM_/","/TTMenu/"},"r4patch.dat")))goto loader_fail;
	}else if(!strcmp(dldiid,"M3DS")||!strcmp(dldiid,"iTDS")||!strcmp(dldiid,"R4_I")){
		if(!strcpy_safe(file,findpath(3,(char*[]){"/YSMenu/","/_SYSTEM_/","/TTMenu/"},"m3patch.dat")))goto loader_fail;
	}else{
loader_fail:
		_consolePrint("Failed.\n");die();
	}
	_consolePrint("Done.\n");
	

	if(!(f=fopen("/ttmenu.sys","r+b"))){_consolePrint("Failed.\n");die();}
	fwrite("ttds",1,4,f);

	fseek(f,0x100,SEEK_SET);
	memset(name,0,0x1001);
	getsfnlfn(utf8,name,NULL);
	fwrite(name+1,1,0x1000,f);

	memset(name,0,0x1001);
	strcpy(utf8+strlen(utf8)-3,"sav");
	getsfnlfn(utf8,name,NULL);
	libprism_touch(utf8);
	fwrite(name+1,1,0x1000,f);

	memset(name,0,0x1001);
	//strcpy(name,***ARP***);
	fwrite(name+1,1,0x1000,f);
	fclose(f);

	BootNDSROMex(file);
#if 0

	// vvvvvvvvvvv add 2008.03.30 kzat3
	_consolePrintf("allocating %s...\n",file);
	if (ret_menu9_Gen(file) == true) {
		_consolePrint("allocate done.\n");
	} else {
		_consolePrint("allocate failed.\n");
		die();
	}

	//magic
/*
	strcpy(0x023fda00,"fat1:");
	strcpy(0x023fda05,extlink.DataFullPathFilenameAlias);
	strcpy(0x023fdc00,"fat1:");
	strcpy(0x023fdc05,extlink.DataFullPathFilenameAlias);
	strcpy(0x023fdc05+strlen(extlink.DataFullPathFilenameAlias)-3,"SAV");
	strcpy(0x023fde00,"fat1:/");
*/

       IPCZ->cmd=ResetRudolph;
	//fifoSendValue32(FIFO_USER_07,1);
	_consolePrint("rebooting... \n");
	ret_menu9_GENs();
	_consolePrint("failed.\n");
#endif
	die();
	// ^^^^^^^^^^^^ add 2008.03.30 kzat3
}
