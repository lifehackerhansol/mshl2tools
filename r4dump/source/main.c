#include "../../libprism/libprism.h"
const u16 bgcolor=RGB15(4,0,12);

void Main(){
	_consolePrintf(
		"R4 Dumper\n"
		"reset_mse_06b_for_ak2 by Moonlight, Rudolph, kzat3\n"
		"dldipatch aka dlditool public domain under CC0.\n"
		"%s\n%s\n\n",
		ROMDATE,ROMENV
	);

	{
		unsigned char dldiid[5];
		unsigned char *dldiFileData=DLDIDATA;
		_consolePrintf("DLDI Interface: 0x%08x\n",dldiFileData);
		memcpy(dldiid,(char*)dldiFileData+ioType,4);
		dldiid[4]=0;
		_consolePrintf("DLDI ID: %s\n",dldiid);
		_consolePrintf("DLDI Name: %s\n\n",(char*)dldiFileData+friendlyName);
	}

	//_consolePrint("Waiting... ");
	//sleep(1);
	//_consolePrint("Done.\n");

	_consolePrint("initializing FAT... ");
	if(!disc_mount()){_consolePrint("failed.\n");die();}
	_consolePrint("done.\n");

	_consolePrint("Dumping...\n");
	{
		FILE *f;
		u8 buf[512];

		struct stat st;
		if(stat("/_DS_MENU.DAT",&st)){_consolePrintf("Can not stat R4Menu.\n");return;}
		u32 addr=(u32)getFATEntryAddress("/_DS_MENU.DAT");
		if(!addr){_consolePrintf("Error occurred in getting sector.\n");return;}
		R4_ReadCardInfo();
		R4_SendMap(addr&0xfffffffe);
		//R4_00();
		R4_ReadCardInfo();

		int pos;
		f=fopen("/_DS_MENU.nds","wb");
		for(pos=0;pos<st.st_size;pos+=512){
			R4_ReadMenu(pos,buf,128);
			fwrite(buf,1,512,f);
		}
		fclose(f);
	}
	_consolePrint("Completed.\n");
	die();
}
