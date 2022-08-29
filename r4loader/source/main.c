#include "../../libprism/libprism.h"
const u16 bgcolor=RGB15(4,0,12);
const int useARM7Bios=0;

char usrcheat[27];
void patchusrcheat(u8* buf){
	_consolePrint("Patching usrcheat path...\n");
	char *arm9=(char*)buf+512;
	u32 size=read32(buf+0x2c),i=0;
	for(;i<size;i++)
		if(!memcmp(arm9+i,"/__rpg/cheats/usrcheat.dat",26))
			memcpy(arm9+i,usrcheat,26);
}

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

static char file[256*3]="MoonShellExecute\0\0\0\0\0\0\0\0\0\0\0" //will be modified externally
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
//static TExtLinkBody extlink;

void Main(){
	FILE *f;

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

	//_consolePrint("Waiting... ");
	//sleep(1);
	//_consolePrint("Done.\n");

	_consolePrint("initializing FAT... ");
	if(!disc_mount()){_consolePrint("failed.\n");die();}
	_consolePrint("done.\n");

	_consolePrint("Opening frontend... ");
	char utf8[768];
	if(!readFrontend(utf8)){_consolePrint("Failed.\n");die();}
	_consolePrint("Done.\n");

	//magic
	_consolePrint("Setting filename... ");
	*(vu32*)0x023fd900=0x10|(14<<8);
	strcpy((char*)0x023fda00,"fat0:");
	strcpy((char*)0x023fda05,utf8);
	strcpy((char*)0x023fdd00,"fat0:");
	strcpy(utf8+strlen(utf8)-3,"sav");
	if(!stat(utf8,NULL))strcpy((char*)0x023fdd05,utf8);
	_consolePrint("Done.\n");

	_consolePrint("Setting reset/DMA state... ");
	{//set reset/DMA
		FILE *fsave=fopen((char*)0x023fdd05,"rb");
		char buf[7];
		if(!fsave){_consolePrint("Cannot open save.\n");die();}
		fseek(fsave,filelength(fileno(fsave))-4,SEEK_SET);
		fread(buf,1,8,fsave);
		fclose(fsave);
		libprism_touch((char*)0x023fdd05);
		if(!memcmp(buf+4,"NMSY",4)&&(buf[2]&0xfc)==0x0c){
			if(buf[2]&1)*(vu32*)0x023fd900|=8; //DMA
			if(buf[2]&2)*(vu32*)0x023fd900|=4; //reset
			goto set_reset_done;
		}

		//default, need to get from YSMenu.ini.
		if(!strcpy_safe(usrcheat,findpath(6,(char*[]){"/YSMenu/","/_SYSTEM_/","/TTMenu/","/__ak2/","/__rpg/","/"},"YSMenu.ini")))goto set_reset_done; //disable.
//set_reset_ini:
		ini_gets("YSMenu","DEFAULT_RESET","false",buf,6,usrcheat);
		if(!strcmp(buf,"true"))*(vu32*)0x023fd900|=4;
		ini_gets("YSMenu","DEFAULT_DMA","true",buf,6,usrcheat);
		if(!strcmp(buf,"true"))*(vu32*)0x023fd900|=8;
	}

set_reset_done:
	_consolePrint("Looking for usrcheat.dat... ");
	if(!strcpy_safe(usrcheat,findpath(4,(char*[]){"/YSMenu/","/_SYSTEM_/","/TTMenu/","/__rpg/"},"usrcheat.dat")))goto finalize;

//parsecheat:
	_consolePrint("Parsing usrcheat.dat... ");
	{
		u8 head[512];
		if(!(f=fopen((char*)0x023fda05,"rb"))){_consolePrint("Cannot open ROM.\n");die();}
		fread(head,1,512,f);
		u32 gamecode=read32(head+12),CRC32=crc32(0xffffffff,head,512);
		fclose(f);
		if(f=fopen(usrcheat,"rb")){
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
						u8 *p=(u8*)malloc(*(vu32*)0x023fd908);
						if(p){
							fseek(f,*(vu32*)0x023fd904,SEEK_SET);
							fread(p,1,*(vu32*)0x023fd908,f);
							u32 count=*(u32*)( p+align4(strlen((char*)p)+1) );
							if(count&0xf0000000){ //enable it
								*(vu32*)0x023fd900|=2; //enable cheating.
								ret_menu9_callback=patchusrcheat;
							}
							free(p);
						}
						//*(vu32*)0x023fd900|=2; //enable cheating.
						_consolePrint("Done.\n");
						break;
					}
					if(!next.offset){
						_consolePrint("Not found.\n");
						break;
					}
				}
			}
			fclose(f);
		}
	}

finalize:
	ini_gets("WoodMod",(char*)dldiid,file,file,768,"/__rpg/woodload.ini");

	_consolePrintf("Rebooting to %s...\n",file);
	ret_menu9_Gen(file);
	// ^^^^^^^^^^^^ add 2008.03.30 kzat3
}
