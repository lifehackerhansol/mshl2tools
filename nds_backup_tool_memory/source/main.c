#include "../../libprism/libprism.h"
#include "libcarddump.h"
const u16 bgcolor=RGB15(4,0,12);

u8	key_tbl[0x1078];
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
	_consolePrintf("Insert target DS card.\r");
	for(swiWaitForVBlank();;swiWaitForVBlank()){
		if(!(IPCZ->keysdown))continue;
		if(IPCZ->keysdown&KEY_A)break;
		if(IPCZ->keysdown&KEY_B)IPCZ->cmd=Shutdown;
	}
	dump.romID = Card_Open(key_tbl);
	while(dump.romID == 0xFFFFFFFF){
		_consolePrintf("Cannot be recognized. Insert again.\r");
		for(swiWaitForVBlank();;swiWaitForVBlank()){
			if(!(IPCZ->keysdown))continue;
			if(IPCZ->keysdown&KEY_A)break;
			if(IPCZ->keysdown&KEY_B)IPCZ->cmd=Shutdown;
		}
		dump.romID = Card_Retry();
	}
	_consolePrintf("                                        \r");
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
	for(;dump.dumpoffset<dumpto;){
		if(!Card_Read(dump.dumpoffset, data+(dump.dumpoffset-dump.writeoffset))){
			_consolePrintf("                                        \r");
			_consolePrintf("Dump failed...\n");free(data);die();
		}
		dump.dumpoffset+=512;
		_consolePrintf("Dumping %7d / %7d\r",dump.dumpoffset-dump.writeoffset,dumpto_relative);
	}
	_consolePrintf("                                        \r");
	Card_Close();
}

void dumpnext(){
	_consolePrintf("Insert target DS card.\r");
	for(swiWaitForVBlank();;swiWaitForVBlank()){
		if(!(IPCZ->keysdown))continue;
		if(IPCZ->keysdown&KEY_A)break;
		if(IPCZ->keysdown&KEY_B)IPCZ->cmd=Shutdown;
	}
	u32 romID = Card_Open(key_tbl);
	while(romID != dump.romID){
		if(romID==0xFFFFFFFF){
			_consolePrintf("Cannot be recognized. Insert again.\r");
		}else{
			_consolePrintf("Different card. Insert again.\r");
		}
		for(swiWaitForVBlank();;swiWaitForVBlank()){
			if(!(IPCZ->keysdown))continue;
			if(IPCZ->keysdown&KEY_A)break;
			if(IPCZ->keysdown&KEY_B)IPCZ->cmd=Shutdown;
		}
		romID = Card_Retry();
	}
	_consolePrintf("                                        \r");
	u32 dumpto=min(dump.writeoffset+2*1024*1024,dump.size);
	u32 dumpto_relative=dumpto-dump.writeoffset;
	for(;dump.dumpoffset<dumpto;){
		if(!Card_Read(dump.dumpoffset, data+(dump.dumpoffset-dump.writeoffset))){
			_consolePrintf("                                        \r");
			_consolePrintf("Dump failed...\n");free(data);die();
		}
		dump.dumpoffset+=512;
		_consolePrintf("Dumping %7d / %7d\r",dump.dumpoffset-dump.writeoffset,dumpto_relative);
	}
	_consolePrintf("                                        \r");
	Card_Close();
}

void writestart(){
	FILE *f;
	u32 ptr=0;
	_consolePrintf("Insert storage flashcart.\r");
	for(swiWaitForVBlank();;swiWaitForVBlank()){
		if(!(IPCZ->keysdown))continue;
		if(IPCZ->keysdown&KEY_A)break;
		if(IPCZ->keysdown&KEY_B)IPCZ->cmd=Shutdown;
	}
	u32 romID = Card_Open(key_tbl);
	bool fatinited=false;
	while(romID == 0xFFFFFFFF && !(fatinited=fatInitDefault())){
		_consolePrintf("Cannot be recognized. Insert again.\r");
		for(swiWaitForVBlank();;swiWaitForVBlank()){
			if(!(IPCZ->keysdown))continue;
			if(IPCZ->keysdown&KEY_A)break;
			if(IPCZ->keysdown&KEY_B)IPCZ->cmd=Shutdown;
		}
		romID = Card_Retry();
	}
	if(romID!=0xFFFFFFFF)Card_Close();
	_consolePrintf("                                        \r");
	_consolePrintf("Checking libfat (phase 2)... ");
	if(!fatinited&&!fatInitDefault()){
		_consolePrintf(
			"failed.\n"
			"Some flashcarts cannot initialize DLDI after ejected...\n"
			"In short nds_backup_tool_memory doesn't work on this flashcart.\n"
		);
		free(data);die();
	}
	_consolePrintf("done.\n");
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
	for(;ptr<dump.dumpoffset-dump.writeoffset;){
		u32 writebyte=min(dump.dumpoffset-dump.writeoffset-ptr,65536);
		fwrite(data+ptr,1,writebyte,f);
		ptr+=writebyte;
		_consolePrintf("Writing %7d / %7d\r",ptr,dump.dumpoffset-dump.writeoffset);
	}
	_consolePrintf("                                        \r");
	dump.writeoffset=dump.dumpoffset;
	fclose(f);
#ifdef _LIBNDS_MAJOR_
	fatUnmount("fat:/");
#else
	fatUnmount(0);
#endif
}

void writenext(){
	FILE *f;
	u32 ptr=0;
	_consolePrintf("Insert storage flashcart.\r");
	for(swiWaitForVBlank();;swiWaitForVBlank()){
		if(!(IPCZ->keysdown))continue;
		if(IPCZ->keysdown&KEY_A)break;
		if(IPCZ->keysdown&KEY_B)IPCZ->cmd=Shutdown;
	}
	u32 romID = Card_Open(key_tbl);
	bool fatinited=false;
	while(romID == 0xFFFFFFFF && !(fatinited=fatInitDefault())){
		_consolePrintf("Cannot be recognized. Insert again.\r");
		for(swiWaitForVBlank();;swiWaitForVBlank()){
			if(!(IPCZ->keysdown))continue;
			if(IPCZ->keysdown&KEY_A)break;
			if(IPCZ->keysdown&KEY_B)IPCZ->cmd=Shutdown;
		}
		romID = Card_Retry();
	}
	if(romID!=0xFFFFFFFF)Card_Close();
	_consolePrintf("                                        \r");
	if(!fatinited&&!fatInitDefault()){
		_consolePrintf(
			"fatInitDefault failed.\n"
		);
		free(data);die();
	}
	if(stat(dump.name,&st)){
		_consolePrintf("cannot stat %s...\n",dump.name);free(data);die();
	}
	if(st.st_size!=dump.writeoffset){
		_consolePrintf("written size wrong...\n");free(data);die();
	}
	if(!(f=fopen(dump.name,"r+b"))){_consolePrintf("cannot open %s...\n",dump.name);free(data);die();}
	fseek(f,st.st_size,SEEK_SET);
	for(;ptr<dump.dumpoffset-dump.writeoffset;){
		u32 writebyte=min(dump.dumpoffset-dump.writeoffset-ptr,65536);
		fwrite(data+ptr,1,writebyte,f);
		ptr+=writebyte;
		_consolePrintf("Writing %7d / %7d\r",ptr,dump.dumpoffset-dump.writeoffset);
	}
	_consolePrintf("                                        \r");
	dump.writeoffset=dump.dumpoffset;
	fclose(f);
#ifdef _LIBNDS_MAJOR_
	fatUnmount("fat:/");
#else
	fatUnmount(0);
#endif
}

void Main(){
	u32 dumpcount=1,dumprequired=0;

	IPCZ->cmd=0;
	_consolePrintf(
		"nds_backup_tool_memory\n"
		"*** very easy libcarddump frontend ***\n"
		"libcarddump (in nds_backup_tool_ftpd) by Rudolph\n"
		"%s\n%s\n\n",
		ROMDATE,ROMENV
	);
	_consolePrintf("While swaping DS card, press A to proceed, press B to shutdown.\n\n");

	{
		unsigned char dldiid[5];
		unsigned char *dldiFileData=DLDIDATA;
		memcpy(dldiid,(char*)dldiFileData+ioType,4);
		dldiid[4]=0;
		_consolePrintf("DLDI ID: %s\n",dldiid);
		_consolePrintf("DLDI Name: %s\n\n",(char*)dldiFileData+friendlyName);
	}

	_consolePrintf("Allocating internal buffer... ");
	data=(char*)malloc(2*1024*1024); //2MB
	if(!data){_consolePrintf("failed.\n");die();}
	_consolePrintf("done.\n");

	_consolePrintf("Checking libfat (phase 1)... ");

	if(!fatInitDefault()){_consolePrintf("failed.\n");free(data);die();}
	_consolePrintf("done.\n");
#ifdef _LIBNDS_MAJOR_
	fatUnmount("fat:/");
#else
	fatUnmount(0);
#endif

	IPCZ->arm7bios_addr=key_tbl;
	IPCZ->arm7bios_bufsize=0x1078;
	IPCZ->cmd=GetARM7Bios;
	while(IPCZ->cmd)swiWaitForVBlank();

	dumpstart();
	writestart();
	dumprequired=(dump.size+2*1024*1024-1)/(2*1024*1024);
	for(;dumpcount<dumprequired;dumpcount++){
		_consolePrintf2("Swaps remained: %d   \r",dumprequired-dumpcount);
		dumpnext();
		writenext();
	}
	_consolePrintf2("Swaps remained: 0   \r");
	_consolePrintf("Congraturations, dump completed!\n");
	fatInitDefault();free(data);die();
}
