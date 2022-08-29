#include "../../libprism/libprism.h"
const u16 bgcolor=RGB15(4,0,12);
const int useARM7Bios=0;

static char usrcheat[27];
static void patchusrcheat(u8* buf){
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
		"YSMenu Wood Loader\n"
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

	_consolePrint("initializing FAT... ");
	if(!disc_mount()){_consolePrint("failed.\n");die();}
	_consolePrint("done.\n");

	//magic
	u8 head[512];
	*(vu32*)0x023fd900=0x10|(14<<8)|( ((*(u8*)0x023fdbff)&8)?8:0 )|( ((*(u8*)0x023fdbff)&4)?4:0 ); //reset/DMA
	*(char*)0x023fda03='0';
	*(char*)0x023fdc03='0';
	memcpy(head,(char*)0x023fdc00,512);
	memcpy((char*)0x023fdd00,head,512);

	_consolePrint("Looking for usrcheat.dat... ");
	struct stat st;
	if(!stat("/YSMenu/usrcheat.dat",&st)){
		strcpy(usrcheat,"/YSMenu/usrcheat.dat");
		goto parsecheat;
	}
	if(!stat("/_SYSTEM_/usrcheat.dat",&st)){
		strcpy(usrcheat,"/_SYSTEM_/usrcheat.dat");
		goto parsecheat;
	}
	if(!stat("/TTMenu/usrcheat.dat",&st)){
		strcpy(usrcheat,"/TTMenu/usrcheat.dat");
		goto parsecheat;
	}
	if(!stat("/__rpg/usrcheat.dat",&st)){
		strcpy(usrcheat,"/__rpg/usrcheat.dat");
		goto parsecheat;
	}
	goto finalize;

parsecheat:
	_consolePrint("Parsing usrcheat.dat... ");
	{

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
						*(vu32*)0x023fd904=cur.offset; //offset
						*(vu32*)0x023fd908=(next.offset?next.offset:fsize)-cur.offset; //size
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
