#include "../../libprism/libprism.h"
const u16 bgcolor=RGB15(4,0,12);
const int useARM7Bios=0;

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

#define align4(i) (((i)+3)&~3)

unsigned int *skipstring(unsigned int *o){
	char *p=(char*)o;
	return (unsigned int*)( align4((unsigned int)p+strlen(p)+1) );
}

unsigned int *skipnote(unsigned int *o){
	char *p=(char*)o;
	p=p+strlen(p)+1; //skip title
	return (unsigned int*)( align4((unsigned int)p+strlen(p)+1) ); //skip additional note
}

int writecc(u32 *o, FILE *f){ //from updatecheat --;
	int ret=0;
	u32 e1=0xcf000000,e2=0;

	//read all o
	o=skipstring(o);
	unsigned int ocount=*o&0x0fffffff;
	//unsigned int oenable=*o&0xf0000000; //already checked
	o+=9;
	u32 i=0;
	for(;i<ocount;){
		unsigned int foldercount=1;
		//int folderflag=0;
		if(*o&0x10000000){//folder
			//folderflag=1;
			//fprintf(f,";@@Folder Type: %s\n",(*o&0x01000000)?"one":"multi"); //folder-choice
			foldercount=*o&0x00ffffff;
			//fprintf(f,";@@Folder Items: %d\n",foldercount);
			o++;

			//char *p=(char*)o;
			//if(*p)fprintf(f,";@@Folder Name: %s\n",p);
			//p=p+strlen(p)+1;
			//if(*p)fprintf(f,";@@Folder Note: %s\n",p);
			//fputs("\n",f);

			o=skipnote(o);
			i++;
		}
		for(;foldercount;foldercount--){
			unsigned int oflag=*o&0xff000000; //fixme
			unsigned int *onext=o+1+(*o&0x00ffffff);
			if(oflag)ret++;
			o++;
			//char *p=(char*)o;
			//fprintf(f,"%c%s\n",oflag?'@':'#',p);
			//p=p+strlen(p)+1;
			//if(*p)fprintf(f,";@Cheat Note: %s\n",p);
			o=skipnote(o);
			u32 cheatlen=*o;
			//fprintf(f,";@Data Length: %d\n",cheatlen);
			//fputs(";------------------\n",f);
			o++;

			u32 j=0;
			//for(;j<cheatlen;j++)fprintf(f,"%08X%c",o[j],(j&1)?'\n':' ');
			if(oflag)for(;j<cheatlen;j++)fwrite(o+j,4,1,f);
			//if(cheatlen&1)fputs("\n",f);

			i++;
			o=onext;
			//fputs("\n",f);
		}
		//if(folderflag)fputs(";@@EndOfFolder\n\n",f);
	}
	fwrite(&e1,4,1,f);fwrite(&e2,4,1,f);
	return ret;
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
		"MoonShell2 AKAIO Linker\n"
		"reset_mse_06b_for_ak2 by Moonlight, Rudolph, kzat3\n"
		//"dldipatch aka dlditool public domain under CC0.\n"
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

	_consolePrint("Opening frontend... ");
	char utf8[768];
	if(!readFrontend(utf8)){_consolePrint("Failed.\n");die();}
	_consolePrint("Done.\n");

	strcpy((char*)0x023fda00,"fat1:");
	getsfnlfn(utf8,(char*)0x023fda05,NULL);
	strcpy((char*)0x023fdc00,"fat1:");
	strcpy(utf8+strlen(utf8)-3,"sav");
	if(!stat(utf8,NULL))getsfnlfn(utf8,(char*)0x023fdc05,NULL);

	//magic
	u8 head[512];
	u8 arg[8]={0,0,0,0,0,0,0,0};
	u8 reset=1;//((*(u8*)0x023fdbff)&4)?2:1;
	u8 cheat=1;
	//*(vu8*)0x023fdbff=0x10|( ((*(u8*)0x023fdbff)&8)?8:0 )|( ((*(u8*)0x023fdbff)&4)?4:0 ); //reset/DMA
	//*(char*)0x023fda03='0';
	//*(char*)0x023fdc03='0';
	//memcpy(head,(char*)0x023fdc00,512);
	//memcpy((char*)0x023fdd00,head,512);

	if(!(f=fopen((char*)0x023fda05,"rb"))){_consolePrint("Cannot open ROM.\n");die();}
	fread(head,1,512,f);
	u32 gamecode=read32(head+12),CRC32=crc32(0xffffffff,head,512); //from here head is const
	fclose(f);
	if(isHomebrew(head))ret_menu9_Gen((char*)0x023fda05);

	char usrcheat[27]; //recycling var
	_consolePrint("Setting reset/DMA state... ");
	{//set reset/DMA
		FILE *fsave=fopen((char*)0x023fdc05,"rb");
		char buf[7];
		if(!fsave){_consolePrint("Cannot open save.\n");die();}
		fseek(fsave,filelength(fileno(fsave))-4,SEEK_SET);
		fread(buf,1,8,fsave);
		fclose(fsave);
		libprism_touch((char*)0x023fdc05);
		if(!memcmp(buf+4,"NMSY",4)&&(buf[2]&0xfc)==0x0c){
			//if(buf[2]&1)*(vu8*)0x023fdbff|=8; //DMA
			if(buf[2]&2)reset=2;//*(vu8*)0x023fdbff|=4; //reset
			goto set_reset_done;
		}

		//default, need to get from YSMenu.ini.
		if(!strcpy_safe(usrcheat,findpath(6,(char*[]){"/YSMenu/","/_SYSTEM_/","/TTMenu/","/__ak2/","/__rpg/","/"},"YSMenu.ini")))goto set_reset_done; //disable.
//set_reset_ini:
		ini_gets("YSMenu","DEFAULT_RESET","false",buf,6,usrcheat);
		if(!strcmp(buf,"true"))reset=2;//*(vu8*)0x023fdbff|=4;
		//ini_gets("YSMenu","DEFAULT_DMA","true",buf,6,usrcheat);
		//if(!strcmp(buf,"true"))*(vu8*)0x023fdbff|=8;
	}

set_reset_done:
	_consolePrint("Looking for usrcheat.dat... ");
	if(!strcpy_safe(usrcheat,findpath(4,(char*[]){"/YSMenu/","/_SYSTEM_/","/TTMenu/","/__rpg/"},"usrcheat.dat")))goto finalize;

//parsecheat:
	_consolePrint("Parsing usrcheat.dat... ");
	{
			if(f=fopen(usrcheat,"rb")){
			u8 usrcheathead[12];
			fread(usrcheathead,1,12,f);
			if(!memcmp(usrcheathead,"R4 CheatCode",12)){
				u32 fsize=filelength(fileno(f));
				fseek(f,0x100,SEEK_SET);
				cheatindex cur,next;
				fread(&next,1,sizeof(next),f);
				for(;;){
					memcpy(&cur,&next,sizeof(cur));
					fread(&next,1,sizeof(next),f);
					if(gamecode==cur.gamecode&&CRC32==cur.crc32){
						u32 size=(next.offset?next.offset:fsize)-cur.offset;
						//*(vu32*)0x023fd904=cur.offset; //offset
						//*(vu32*)0x023fd908=(next.offset?next.offset:fsize)-cur.offset; //size
						u32 *p=(u32*)malloc(size);
						if(p){
							fseek(f,cur.offset,SEEK_SET);
							fread(p,1,size,f);
							u32 count=*skipstring(p);
							if(count&0xf0000000){ //enable it
								//write .cc
								char fname[50],gamecode[5];
								memcpy(gamecode,head+12,4);gamecode[4]=0;
								mkdir("/__aio/cheats/cc",0777);
								sprintf(fname,"/__aio/cheats/cc/%s%08X.bin",gamecode,CRC32);
								FILE *g=fopen(fname,"wb");
								if(g){
									//fprintf(g,";@@@@@ Game Title: %s\n",(char*)p);
									//fprintf(g,";@@@@@ Game Code: %s\n",gamecode);
									//fprintf(g,";@@@@@ CRC32: 0x%08X\n",CRC32);
									//fprintf(g,";@@@@@ Number of Cheats: %d\n",count&0x0fffffff);
									//fputs(";----- Generated by libprism\n\n",g);
									int ret=writecc(p,g);

									fclose(g);
									if(ret){
										cheat=2;//*(vu8*)0x023fdbff|=2; //enable cheating.
										//ret_menu9_callback=patchusrcheat;
										//strcpy((char*)0x023fde00,"fat1:/YSMENU/AKLOADER.CC");
									}
								}
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
	_consolePrint("Configuring...\n");
	//write optionlist.bin
	f=fopen("/__aio/optionlist.bin","r+b");
	int sflag=1;
	if(f){
		u8 buf[0x1a];
		u32 offset=0;
		for(;fread(buf,1,0x1a,f)==0x1a;){
			if(!memcmp(buf,head,0x10)&&!memcmp(buf+0x10,head+0x15e,2)){memcpy(arg,buf+0x12,8);sflag=0;break;}
			offset+=0x1a;
		}
		if(feof(f))clearerr(f);
		fseek(f,offset,SEEK_SET);
	}else{
		f=fopen("/__aio/optionlist.bin","wb");
	}
	//handle obtained. write 0x1a bytes.
	fwrite(head,1,0x10,f);
	fwrite(head+0x15e,1,2,f);
	//8bytes: DownloadPlay(default), SoftReset, Cheat, Slot(0), 0, AAP(default), 0, 0
	arg[1]=reset,arg[2]=cheat,arg[3]=0;
	fwrite(arg,1,8,f);
	fclose(f);

	if(sflag){
		f=fopen("/__aio/savelistex.bin","r+b");
		if(f)fseek(f,filelength(fileno(f)),SEEK_SET);
		else f=fopen("/__aio/savelistex.bin","wb");
		fwrite(head,1,0x10,f);
		fwrite(head+0x15e,1,2,f);
		fwrite("\0",1,1,f); //use auto size
		fclose(f);
	}

	//write globalsettings
	char tmp[776];
	strcpy(tmp,"fat0:"); //0.66b
	{
		u16 buf[256];
		getsfnlfn((char*)0x023fda05,NULL,buf);
		ucs2tombs(tmp+5,buf);
	}
	ini_puts("Save Info","lastLoaded",tmp,"/__aio/lastsave.ini");
	//strcpy(tmp+5,extlink.DataFullPathFilenameAlias);
	//ini_puts("Save Info","shortPathNDS",tmp,"/__aio/lastsave.ini");
	ini_putl("system","autorunWithLastRom",1,"/__aio/globalsettings.ini");

	ini_gets("Config","AKAIO",file,file,768,"/MOONSHL2/EXTLINK/inilink.ini");
	_consolePrintf("Rebooting to %s...\n",file);
	ret_menu9_Gen(file);
	// ^^^^^^^^^^^^ add 2008.03.30 kzat3
}
