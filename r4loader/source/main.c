#include "../../libprism/libprism.h"
const u16 bgcolor=RGB15(4,0,12);

typedef struct{
	u32 gamecode;
	u32 crc32;
	u64 offset;
}cheatindex;

u32 crc32(u32 crc, const u8 *p, u32 size){
	int n,f;
	for(;size;size--){
		crc^=*p++;
		for(n=0;n<8;n++){
			f = crc & 1;
			crc >>= 1 ;
			if(f)crc^=0xedb88320;
		}
	}
	return crc;
} 

void Main(){
	char file[256*3]="MoonShellExecute\0\0\0\0\0\0\0\0\0\0\0" //will be modified externally
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"; //744 paddings
	TExtLinkBody extlink;
	FILE *f;

	IPCZ->cmd=0;
	_consolePrintf(
		"r4loader extlink\n"
		"reset_mse_06b_for_ak2 by Moonlight, Rudolph, kzat3\n"
		"dldipatch aka dlditool public domain under CC0.\n"
		"%s\n%s\n\n",
		ROMDATE,ROMENV
	);

	unsigned char dldiid[5];
	{
		unsigned char *dldiFileData=DLDIDATA;
		memcpy(dldiid,(char*)dldiFileData+ioType,4);
		dldiid[4]=0;
		_consolePrintf("DLDI ID: %s\n",dldiid);
		_consolePrintf("DLDI Name: %s\n\n",(char*)dldiFileData+friendlyName);
	}

	//_consolePrintf("Waiting... ");
	//sleep(1);
	//_consolePrintf("Done.\n");

	_consolePrintf("initializing libfat... ");
	if(!fatInitDefault()){_consolePrintf("failed.\n");die();}
	_consolePrintf("done.\n");

	_consolePrintf("Opening extlink... ");
	if(!(f=fopen("/MOONSHL2/EXTLINK.DAT","rb"))){_consolePrintf("Failed.\n");die();}
	fread(&extlink,1,sizeof(TExtLinkBody),f);
	fclose(f);
	if(extlink.ID!=ExtLinkBody_ID){_consolePrintf("Incorrect ID.\n");die();}
	_consolePrintf("Done.\n");

	//magic
	_consolePrintf("Setting filename... ");
	*(vu32*)0x023fd900=0x1c|(14<<8);
	strcpy((char*)0x023fda00,"fat0:");
	_FAT_directory_ucs2tombs((char*)0x023fda05,extlink.DataFullPathFilenameUnicode,762);
	strcpy((char*)0x023fdd00,"fat0:");
	char utf8[768];
	_FAT_directory_ucs2tombs(utf8,extlink.DataFullPathFilenameUnicode,768);
	strcpy(utf8+strlen(utf8)-3,"sav");
	if(!stat(utf8,NULL))strcpy((char*)0x023fdd05,utf8);
	_consolePrintf("Done.\n");

	_consolePrintf("Parsing /__rpg/cheats/usrcheat.dat... ");
	{
		u8 head[512];
		if(!(f=fopen((char*)0x023fda05,"rb"))){_consolePrintf("Cannot open ROM.\n");die();}
		fread(head,1,512,f);
		u32 gamecode=read32(head+12),CRC32=crc32(0xffffffff,head,512);
		fclose(f);
		if(f=fopen("/__rpg/cheats/usrcheat.dat","rb")){
			fread(head,1,12,f);
			if(!memcmp(head,"R4 CheatCode",12)){
				u32 fsize=filelength(fileno(f));
				fseek(f,0x100,SEEK_SET);
				cheatindex cur,next;
				fread(&next,1,sizeof(next),f);
				for(;;){
					memcpy(&cur,&next,sizeof(cur));
					fread(&next,1,sizeof(next),f);
					if(gamecode==cur.gamecode&&CRC32==cur.crc32){
						*(vu32*)0x023fd904=cur.offset;
						*(vu32*)0x023fd908=(next.offset?next.offset:fsize)-cur.offset;
						*(vu32*)0x023fd900|=2; //enable cheating.
						_consolePrintf("Done.\n");
						break;
					}
					if(!next.offset){
						_consolePrintf("Not found.\n");
						break;
					}
				}
			}
			fclose(f);
		}
	}

	ini_gets("WoodMod",(char*)dldiid,file,file,768,"/__rpg/woodload.ini");

	_consolePrintf("Rebooting to %s...\n",file);
	ret_menu9_Gen(file);
	// ^^^^^^^^^^^^ add 2008.03.30 kzat3
}
