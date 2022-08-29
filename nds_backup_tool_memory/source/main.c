#include "../../libprism/libprism.h"
#include "libcarddump.h"
const u16 bgcolor=RGB15(4,0,12);

char	*data;

typedef struct{
	char name[23]; // "/XXXX_ZZZZZZZZZZZZ.nds, 1+4+1+12+4+1
	u32 romID;
	u32 size;
	u32 writeoffset;
	u32 dumpoffset;
}dumphandle;

dumphandle dump;

struct stat st;

void dumpstart(){
	_consolePrintOnce("Insert target DS card.");
	for(swiWaitForVBlank();;swiWaitForVBlank()){
		if(!(IPCZ->keysdown))continue;
		if(IPCZ->keysdown&KEY_A)break;
		if(IPCZ->keysdown&KEY_B)IPCZ->cmd=Shutdown;
	}
	dump.romID = Card_Open(key_tbl);
	while(dump.romID == 0xFFFFFFFF){
		_consolePrintOnce("Cannot be recognized. Insert again.");
		for(swiWaitForVBlank();;swiWaitForVBlank()){
			if(!(IPCZ->keysdown))continue;
			if(IPCZ->keysdown&KEY_A)break;
			if(IPCZ->keysdown&KEY_B)IPCZ->cmd=Shutdown;
		}
		dump.romID = Card_Retry();
	}
	_consolePrintOnceEnd();
	dump.writeoffset=dump.dumpoffset=0;
	Card_Read(dump.dumpoffset, data);	// Read Header Area (0x0000-0x3FFF)
	sprintf(dump.name,"/%c%c%c%c_",
		data[12], data[13], data[14], data[15]
	);
	{
		int i=0,j=6;
		for(;i<12;i++){
			if(!data[i])continue;
			dump.name[j++]=strchr("\\/:*?\"<>|",data[i])?'_':data[i];
		}
		dump.name[j]=0;
	}
	strcat(dump.name,".nds");
	dump.size=(128 << data[20])*1024;
	_consolePrintf2("Target: %s\nSize:   %d\nromID:  0x%08x\n\n",dump.name,dump.size,dump.romID);

	dump.dumpoffset=512;
	u32 dumpto=min(dump.writeoffset+2*1024*1024,dump.size);
	u32 dumpto_relative=dumpto-dump.writeoffset;
	_consoleStartProgress();
	for(;dump.dumpoffset<dumpto;){
		if(!Card_Read(dump.dumpoffset, data+(dump.dumpoffset-dump.writeoffset))){
			_consoleEndProgress();
			_consolePrint("Dump failed...\n");free(data);die();
		}
		dump.dumpoffset+=512;
		_consolePrintProgress("Dumping",dump.dumpoffset-dump.writeoffset,dumpto_relative);
	}
	_consoleEndProgress();
	Card_Close();
}

void dumpnext(){
	_consolePrintOnce("Insert target DS card.");
	for(swiWaitForVBlank();;swiWaitForVBlank()){
		if(!(IPCZ->keysdown))continue;
		if(IPCZ->keysdown&KEY_A)break;
		if(IPCZ->keysdown&KEY_B)IPCZ->cmd=Shutdown;
	}
	u32 romID = Card_Open(key_tbl);
	while(romID != dump.romID){
		if(romID==0xFFFFFFFF){
			_consolePrintOnce("Cannot be recognized. Insert again.");
		}else{
			_consolePrintOnce("Different card. Insert again.");
		}
		for(swiWaitForVBlank();;swiWaitForVBlank()){
			if(!(IPCZ->keysdown))continue;
			if(IPCZ->keysdown&KEY_A)break;
			if(IPCZ->keysdown&KEY_B)IPCZ->cmd=Shutdown;
		}
		romID = Card_Retry();
	}
	_consolePrintOnceEnd();
	u32 dumpto=min(dump.writeoffset+2*1024*1024,dump.size);
	u32 dumpto_relative=dumpto-dump.writeoffset;
	_consoleStartProgress();
	for(;dump.dumpoffset<dumpto;){
		if(!Card_Read(dump.dumpoffset, data+(dump.dumpoffset-dump.writeoffset))){
			_consoleEndProgress();
			_consolePrint("Dump failed...\n");free(data);die();
		}
		dump.dumpoffset+=512;
		_consolePrintProgress("Dumping",dump.dumpoffset-dump.writeoffset,dumpto_relative);
	}
	_consoleEndProgress();
	Card_Close();
}

void writestart(){
	FILE *f;
	u32 ptr=0;
	_consolePrintOnce("Insert storage flashcart.");
	for(swiWaitForVBlank();;swiWaitForVBlank()){
		if(!(IPCZ->keysdown))continue;
		if(IPCZ->keysdown&KEY_A)break;
		if(IPCZ->keysdown&KEY_B)IPCZ->cmd=Shutdown;
	}
	u32 romID = Card_Open(key_tbl);
	bool fatinited=false;
	while(romID == 0xFFFFFFFF && !(fatinited=disc_mount())){
		_consolePrintOnce("Cannot be recognized. Insert again.");
		for(swiWaitForVBlank();;swiWaitForVBlank()){
			if(!(IPCZ->keysdown))continue;
			if(IPCZ->keysdown&KEY_A)break;
			if(IPCZ->keysdown&KEY_B)IPCZ->cmd=Shutdown;
		}
		romID = Card_Retry();
	}
	if(romID!=0xFFFFFFFF)Card_Close();
	_consolePrintOnceEnd();
	_consolePrint("Checking FAT (Phase 2)... ");
	if(!fatinited&&!disc_mount()){
		_consolePrint(
			"failed.\n"
			"Some flashcarts cannot initialize DLDI after ejected...\n"
			"In short nds_backup_tool_memory doesn't work on this flashcart.\n"
		);
		free(data);die();
	}
	_consolePrint("done.\n");
	if(!stat(dump.name,&st)){
		if(st.st_mode&S_IFDIR){
			_consolePrintf("%s is a directory...\n",dump.name);free(data);die();
		}
		_consolePrintf("%s already exists.\nA to continue, B to shutdown.\n\n",dump.name);
		for(swiWaitForVBlank();;swiWaitForVBlank()){
			if(!(IPCZ->keysdown))continue;
			if(IPCZ->keysdown&KEY_A)break;
			if(IPCZ->keysdown&KEY_B)IPCZ->cmd=Shutdown;
		}
	}
	if(!(f=fopen(dump.name,"wb"))){_consolePrintf("cannot open %s...\n",dump.name);free(data);die();}
	_consoleStartProgress();
	for(;ptr<dump.dumpoffset-dump.writeoffset;){
		u32 writebyte=min(dump.dumpoffset-dump.writeoffset-ptr,65536);
		fwrite(data+ptr,1,writebyte,f);
		ptr+=writebyte;
		_consolePrintProgress("Writing",ptr,dump.dumpoffset-dump.writeoffset);
	}
	_consoleEndProgress();
	dump.writeoffset=dump.dumpoffset;
	fclose(f);
	disc_unmount();
}

void writenext(){
	FILE *f;
	u32 ptr=0;
	_consolePrintOnce("Insert storage flashcart.");
	for(swiWaitForVBlank();;swiWaitForVBlank()){
		if(!(IPCZ->keysdown))continue;
		if(IPCZ->keysdown&KEY_A)break;
		if(IPCZ->keysdown&KEY_B)IPCZ->cmd=Shutdown;
	}
	u32 romID = Card_Open(key_tbl);
	bool fatinited=false;
	while(romID == 0xFFFFFFFF && !(fatinited=disc_mount())){
		_consolePrintOnce("Cannot be recognized. Insert again.");
		for(swiWaitForVBlank();;swiWaitForVBlank()){
			if(!(IPCZ->keysdown))continue;
			if(IPCZ->keysdown&KEY_A)break;
			if(IPCZ->keysdown&KEY_B)IPCZ->cmd=Shutdown;
		}
		romID = Card_Retry();
	}
	if(romID!=0xFFFFFFFF)Card_Close();
	_consolePrintOnceEnd();
	if(!fatinited&&!disc_mount()){
		_consolePrint(
			"disc_mount failed.\n"
		);
		free(data);die();
	}
	if(stat(dump.name,&st)){
		_consolePrintf("cannot stat %s...\n",dump.name);free(data);die();
	}
	if(st.st_size!=dump.writeoffset){
		_consolePrint("written size wrong...\n");free(data);die();
	}
	if(!(f=fopen(dump.name,"r+b"))){_consolePrintf("cannot open %s...\n",dump.name);free(data);die();}
	fseek(f,st.st_size,SEEK_SET);
	_consoleStartProgress();
	for(;ptr<dump.dumpoffset-dump.writeoffset;){
		u32 writebyte=min(dump.dumpoffset-dump.writeoffset-ptr,65536);
		fwrite(data+ptr,1,writebyte,f);
		ptr+=writebyte;
		_consolePrintProgress("Writing",ptr,dump.dumpoffset-dump.writeoffset);
	}
	_consoleEndProgress();
	dump.writeoffset=dump.dumpoffset;
	fclose(f);
	disc_unmount();
}

void Main(){
	u32 dumpcount=1,dumprequired=0;

	_consolePrintf(
		"nds_backup_tool_memory\n"
		"*** very easy libcarddump frontend ***\n"
		"libcarddump (in nds_backup_tool_ftpd) by Rudolph\n"
		"%s\n%s\n\n",
		ROMDATE,ROMENV
	);
	if(IPCZ->NDSType>=NDSi){_consolePrint("DSi/3DS don't allow swapping DS card.\n");die();}

	_consolePrint("While swaping DS card, press A to proceed, press B to shutdown.\n\n");

	{
		unsigned char dldiid[5];
		unsigned char *dldiFileData=DLDIDATA;
		memcpy(dldiid,(char*)dldiFileData+ioType,4);
		dldiid[4]=0;
		_consolePrintf("DLDI ID: %s\n",dldiid);
		_consolePrintf("DLDI Name: %s\n\n",(char*)dldiFileData+friendlyName);
	}

	_consolePrint("Allocating internal buffer... ");
	data=(char*)malloc(2*1024*1024); //2MB
	if(!data){_consolePrint("failed.\n");die();}
	_consolePrint("done.\n");

	_consolePrint("Checking FAT (Phase 1)... ");

	if(!disc_mount()){_consolePrint("failed.\n");free(data);die();}
	_consolePrint("done.\n");
	disc_unmount();

	dumpstart();
	writestart();
	dumprequired=(dump.size+2*1024*1024-1)/(2*1024*1024);
	for(;dumpcount<dumprequired;dumpcount++){
		_consolePrintfOnce2("Swaps remained: %d   ",dumprequired-dumpcount);
		dumpnext();
		writenext();
	}
	//_consolePrint2("Swaps remained: 0   ");
	_consolePrintOnceEnd2();
	_consolePrint("Congraturations, dump completed!\n");
	disc_mount();free(data);die();
}
