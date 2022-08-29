#include "../../libprism/libprism.h"
const u16 bgcolor=RGB15(0,0,2);
const int useARM7Bios=0;

void Main(){
	//FILE *f;
	//TExtLinkBody extlink;
	char target[768];
	char dldiid[5];

	_consolePrintf(
		"MoonShell2 VRAM bootlib wrapper (externalized)\n"
		"VRAM bootlib by Chishm\n"
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

	// vvvvvvvvvvv add 2008.03.30 kzat3
	//_consolePrintf("allocating %s...\n",target);
//#ifdef GPL
	//if (!strcmp(dldiid,"EZ5H")||!strcmp(dldiid,"EDGE")||!strcmp(dldiid,"SCDS")
	//||!ret_menu9_Gen(target)){
		_consolePrint("falling back to Chishm VRAM bootlib.\n");
		//fifoSendValue32(FIFO_USER_07,2);
#ifdef LIBFAT
		runNdsFile(target);
		return;
#else
		runNdsFileViaStub(target);
		return;
#endif
	//}else{
	//	_consolePrint("allocate done.\n");
	//}
/*
#else
	if(!strcmp(dldiid,"EZ5H")||!strcmp(dldiid,"EDGE")||!strcmp(dldiid,"SCDS"))
		_consolePrint("this card not supported by ret_menu*_Gen() but we just try to use it since Chishm VRAM bootlib is not linked.\n");
	if(ret_menu9_Gen(target)){
		_consolePrint("allocate done.\n");
	}else{
		_consolePrint("allocate failed (Chishm VRAM bootlib is not linked so big nds code cannot load).\n");die();
	}
#endif
*/
	//RESET=RESET_MENU_GEN;
	//IPCEX->RESET=RESET;
	//IPCZ->cmd=ResetRudolph;
       //fifoSendValue32(FIFO_USER_07,1);
	//_consolePrint("rebooting... \n");
	//ret_menu9_GENs();
	//_consolePrint("failed.\n");
	//die();
	// ^^^^^^^^^^^^ add 2008.03.30 kzat3
}
