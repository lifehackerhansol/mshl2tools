#include "../../libprism/libprism.h"
const u16 bgcolor=RGB15(4,0,12);
const int useARM7Bios=0;

void decodehead(u8* buf){
	_consolePrint("Decoding header(1)...\n");
	if(memcmp(buf+12,"####",4)){
		int i=0,j=0;
		u8 *p=(u8*)buf;
		u8 *t=(u8*)"CuNt"; /////
		for(;i<64;i++)
		if(p[i])
			p[i]=p[i]!=t[j]?p[i]^t[j]:p[i],j++,j%=strlen((char*)t); //fixed in V2
	}
}

void decodebody(u8* buf){
	_consolePrint("Decoding header(2)...\n");
	if(memcmp(buf+12,"####",4)){
		int i=0,j=0;
		u8 *p=(u8*)buf;
		u8 *t=(u8*)"CuNt"; /////
		for(;i<512;i++)
		if(p[i])
			p[i]=p[i]!=t[j]?p[i]^t[j]:p[i],j++,j%=strlen((char*)t); //fixed in V2
	}

	u32 *hed=(u32*)buf;
	_consolePrint("Decoding body...\n");
	u8 *ldrBuf=buf+512;
	if((ldrBuf[3]&0xf0)!=0xe0){
		int i=0,j;
		u8 *k1=(u8*)"DoNt hAx"; /////
		for(j=0;i<hed[11];i++){
			ldrBuf[i]=ldrBuf[i]^k1[j++],j%=strlen((char*)k1);
		}
	}

	ldrBuf=buf+512+hed[11];
	if((ldrBuf[3]&0xf0)!=0xe0){
		int i=0,j;
		u8 *k2=(u8*)"My SHitz"; /////
		for(j=0;i<hed[15];i++){
			ldrBuf[i]=ldrBuf[i]^k2[j++],j%=strlen((char*)k2);
		}
	}
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
			char *p=(char*)o;
			if(oflag)fprintf(f,"%c%s\n",oflag?'@':'#',p);
			//p=p+strlen(p)+1;
			//if(*p)fprintf(f,";@Cheat Note: %s\n",p);
			o=skipnote(o);
			u32 cheatlen=*o;
			//fprintf(f,";@Data Length: %d\n",cheatlen);
			//fputs(";------------------\n",f);
			o++;

			u32 j=0;
			if(oflag)for(;j<cheatlen;j++)fprintf(f,"%08X%c",o[j],(j&1)?'\n':' ');
			if(oflag)if(cheatlen&1)fputs("\n",f);

			i++;
			o=onext;
			//fputs("\n",f);
		}
		//if(folderflag)fputs(";@@EndOfFolder\n\n",f);
	}
	return ret;
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
	//TExtLinkBody extlink;
	FILE *f;

	_consolePrintf(
		"ak2loader extlink\n"
		"reset_mse_06b_for_ak2 by Moonlight, Rudolph, kzat3\n"
		"dldipatch aka dlditool public domain under CC0.\n"
		"%s\n%s\n\n",
		ROMDATE,ROMENV
	);

	{
		unsigned char dldiid[5];
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
	u8 head[512];
	*(vu8*)0x023fdbff=0x10;

	strcpy((char*)0x023fda00,"fat1:");
	getsfnlfn(utf8,(char*)0x023fda05,NULL);
	strcpy((char*)0x023fdc00,"fat1:");
	strcpy(utf8+strlen(utf8)-3,"sav");
	if(!stat(utf8,NULL))getsfnlfn(utf8,(char*)0x023fdc05,NULL);

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
			if(buf[2]&1)*(vu8*)0x023fdbff|=8; //DMA
			if(buf[2]&2)*(vu8*)0x023fdbff|=4; //reset
			goto set_reset_done;
		}

		//default, need to get from YSMenu.ini.
		if(!strcpy_safe(usrcheat,findpath(6,(char*[]){"/YSMenu/","/_SYSTEM_/","/TTMenu/","/__ak2/","/__rpg/","/"},"YSMenu.ini")))goto set_reset_done; //disable.
//set_reset_ini:
		ini_gets("YSMenu","DEFAULT_RESET","false",buf,6,usrcheat);
		if(!strcmp(buf,"true"))*(vu8*)0x023fdbff|=4;
		ini_gets("YSMenu","DEFAULT_DMA","true",buf,6,usrcheat);
		if(!strcmp(buf,"true"))*(vu8*)0x023fdbff|=8;
	}

set_reset_done:
	_consolePrint("Looking for usrcheat.dat... ");
	if(!strcpy_safe(usrcheat,findpath(4,(char*[]){"/YSMenu/","/_SYSTEM_/","/TTMenu/","/__rpg/"},"usrcheat.dat")))goto finalize;

//parsecheat:
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
								FILE *g=fopen("/YSMenu/akloader.cc","w");
								if(g){
									//fprintf(g,";@@@@@ Game Title: %s\n",(char*)p);
									//fprintf(g,";@@@@@ Game Code: ");fwrite(head+12,1,4,g);
									//fprintf(g,"\n;@@@@@ CRC32: 0x%08X\n",CRC32);
									//fprintf(g,";@@@@@ Number of Cheats: %d\n",count&0x0fffffff);
									//fputs(";----- Generated by libprism\n\n",g);
									int ret=writecc(p,g);

									fclose(g);
									if(ret){
										*(vu8*)0x023fdbff|=2; //enable cheating.
										//ret_menu9_callback=patchusrcheat;
										strcpy((char*)0x023fde00,"fat1:/YSMENU/AKLOADER.CC");
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
	ret_menu9_callbackpre=decodehead;
	ret_menu9_callback=decodebody;

	_consolePrintf("Rebooting to %s...\n",file);
	ret_menu9_Gen(file);
}
