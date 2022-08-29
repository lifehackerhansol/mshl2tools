#include "xenofile.h"
#include "zlib/zlib.h"
#include "libnsbmp.h"
#define min(a,b) ((a)<(b)?(a):(b))

// width  = 256/6 = 42
// height = 192/6 = 32

//filelist
fileinfo ftop;
int contentcount;
int hidehidden;

static char getfilelist_tmp[768];
void getfilelist(char *dir,int filter){
	fileinfo *p=&ftop;
	struct stat st;
	contentcount=0;
	DIR_ITER *dp=diropen(dir);
	if(!dp){_consoleClear();_consolePrintf("cannot diropen %s\n",dir);die();}//return;
	for(;!dirnext(dp,getfilelist_tmp,&st);){
		if(!strcmp(getfilelist_tmp,"."))continue;
		if(p->next==NULL){p->next=(fileinfo*)malloc(sizeof(fileinfo)),memset(p->next,0,sizeof(fileinfo));}
		if(!p->next){_consoleClear();_consolePrintf("cannot alloc memory. halt. contentcount==%d\n",contentcount);die();}
		if(hidehidden){
			if(st.st_spare1&ATTRIB_HID||st.st_spare1&ATTRIB_SYS){/*_consolePrintf2("%s st_mode %02x\n",getfilelist_tmp,st.st_spare1);*/continue;}
		}
		if(st.st_spare1 & ATTRIB_DIR){
			p=p->next;
			p->isDir=1,strcat(getfilelist_tmp,"/");
			strcpy(p->name,getfilelist_tmp);
			strncpy(p->nameshow,getfilelist_tmp,38);
		}else{
			if(filter){
				int max=(filter==1?1:EXTMAX);
				int i=0;
				for(;i<max;i++)
					if(strlen(ext[i])&&strlen(getfilelist_tmp)>=strlen(ext[i])&&!strcasecmp(getfilelist_tmp+strlen(getfilelist_tmp)-strlen(ext[i]),ext[i]))
						goto ok;
				continue;
			}
ok:
			p=p->next;
			p->isDir=0;
			strcpy(p->name,getfilelist_tmp);
			strncpy(p->nameshow,getfilelist_tmp,41);
			//p->nameshow[43]=0;
		}
		contentcount++;
	}
	dirclose(dp);
	if(contentcount<2)return;
	if(contentcount>4096)_consolePrint2("contentcount>4096. sort skipped.\n");
	{
		fileinfo *fI;
		int i,j;
		for(i=0;i<contentcount;i++)
			for(j=0,fI=&ftop;j<contentcount-i-1;j++,fI=fI->next){ //bubble
				if(
					(!fI->next->isDir&&fI->next->next->isDir)||
					(fI->next->isDir==fI->next->next->isDir&&strcasecmp(fI->next->name,fI->next->next->name)>0)
				){ //swap
					fileinfo *nextnextnext=fI->next->next->next;
					fI->next->next->next=fI->next;
					fI->next=fI->next->next;
					fI->next->next->next=nextnextnext;
				}
			}
	}
}

void destroyfilelist(){
	fileinfo *p=ftop.next,*next;
	if(!ftop.next)return;
	while(next=p->next){
		free(p);
		p=next;
	}
	free(p);
	ftop.next=NULL;
}

//show
const int paging=32-2;
const int scroll=10;
int top=0;
void showfilelist(int cursor, char *dir){
	char *dirshow=strlen(dir)>42?dir+strlen(dir)-42:dir;
	int i=0;
	fileinfo *p=&ftop;
	for(;i<top;i++)p=p->next;
	_consoleClear();
	_consolePrintf("%s\n------------------------------------------",dirshow);
	for(;i<top+min(paging,contentcount);i++){
		p=p->next;
		_consolePrint(i==cursor?"\n*":"\n ");
		_consolePrintf(p->isDir?"[%s]":"%s",p->nameshow);
	}
}

//commercial
void runCommercial(char *file,char *loader){
	char target[768];
	u8 head[0x200];
	int size;
	FILE *f;

	if(!(f=fopen(file,"rb"))){_consolePrintf("cannot open %s\n",file);die();}
	{struct stat st;fstat(fileno(f),&st);size=st.st_size;}
	if(size<0x200){fclose(f);_consolePrintf("too short %s\n",file);die();}
	fread(head,1,0x200,f);
	if(!isHomebrew(head)){
		strcpy(target,file);fclose(f);goto target_set;
	}else if(!strcmp((char*)head+0x1e0,"mshl2wrap link")){
		unsigned int s=(head[0x1f0]<<24)+(head[0x1f1]<<16)+(head[0x1f2]<<8)+head[0x1f3];
		_consolePrint("Detected mshl2wrap link.\n");
		if(size<s+256*3){fclose(f);_consolePrintf("mshl2wrap link broken %s\n",file);die();}
		fseek(f,s,SEEK_SET);fread(target,1,256*3,f);fclose(f);goto target_set;
	}
	fclose(f);
	return; //not commercial.
      	target_set:
	_consolePrint("Configuring extlink... ");
	if(!writeFrontend(FRONTEND_EXTLINK,loader,target)){_consolePrint("Failed.\n");die();}
	_consolePrint("Done.\n");

	_consolePrint("Rebooting...\n");
	BootLibrary(loader);
	die();
}

bool runTextEdit(char *file){
	FILE *f;
	//TExtLinkBody extlink;
	char *loader="/moonshl2/extlink/_te.TextEdit.nds";

	_consolePrint("Configuring extlink... ");
	if(!writeFrontend(FRONTEND_EXTLINK,loader,file)){_consolePrint("Failed.\n");die();}
	_consolePrint("Done.\n");

	_consolePrint("Rebooting...\n");
	return BootLibrary(loader);
}

bool runExtLink(char *file,char *ext){
	FILE *f;
	//TExtLinkBody extlink;
	char loader[768];
	strcpy(loader,"/moonshl2/extlink/");
	char *name=loader+strlen(loader);

	DIR_ITER *dp=diropen("/moonshl2/extlink/");
	if(!dp)return false;
	for(;!dirnext(dp,name,NULL);){
		if(!strcasecmp(getextname(name),".nds")&&!memcmp(ext+1,name,strlen(ext+1))&&name[strlen(ext+1)]=='.'){dirclose(dp);goto exec;}
	}
	dirclose(dp);
	_consolePrint("Cannot find extlink. Very weird.\n");return false;
exec:
	_consolePrint("Configuring extlink... ");
	if(!writeFrontend(FRONTEND_EXTLINK,loader,file)){_consolePrint("Failed.\n");die();}
	_consolePrint("Done.\n");

	_consolePrint("Rebooting...\n");
	return BootLibrary(loader);
}

bool runRPGLink(const char *file){
	strcpy((char*)0x023fda00,"fat1:");
	strcpy((char*)0x023fda05,file);
	return ret_menu9_Gen("/__rpg/rpglink.nds");
}

int iterateExtLink(int start){
	char name[768];
	strcpy(name,"/moonshl2/extlink/");
	char *fname=name+strlen(name);
	DIR_ITER *dp=diropen(name);
	if(!dp)return start;
	for(;!dirnext(dp,fname,NULL);){
		if(!strcasecmp(getextname(fname),".nds")){
			char* x=fname;
			for(;*x!='.';x++);
			*x=0;
			if(strlen(fname)&&strcasecmp(fname,"nds")&&fname[0]!='_'){
				if(start>=EXTMAX){_consolePrintf2("sorry but ext limit %d exceeded.\n",EXTMAX);die();}
				strcpy(ext[start],".");
				strncpy(ext[start]+1,fname,x-fname);
				_consolePrintf("Regist EXT: %s\n",name);
				start++;
			}
		}
	}
	dirclose(dp);
	return start;
}

#define MSPMAX 128
char reg_msp[48][MSPMAX];
int reg_msp_count=0;

int iterateMSP(int start){
	char name[768];
	strcpy(name,"/moonshl/plugin/");
	char *fname=name+strlen(name);
	memset(reg_msp,0,sizeof(reg_msp));
	DIR_ITER *dp=diropen("/moonshl/plugin/");
	if(!dp)return start;
	for(;!dirnext(dp,fname,NULL);){
		if(!strcasecmp(getextname(fname),".msp")){
			//check file content
			TPluginHeader PH;
			FILE *f=fopen(name,"rb");
			if(!f)continue; //impossible
			fread(&PH,1,sizeof(PH),f);
			fclose(f);
			if(PH.ID!=0x0050534D||(PH.PluginType!=1&&PH.PluginType!=2))continue;
			//OK, let's register it
			int i=0;
			for(;i<4;i++){
				if(!PH.ext[i])break;
				if(PH.ext[i]==0x00706d62)continue; //deny BMP
				if(reg_msp_count>=MSPMAX){_consolePrintf2("sorry but msp limit %d exceeded.\n",MSPMAX);die();}
				if(start>=EXTMAX){_consolePrintf2("sorry but ext limit %d exceeded.\n",EXTMAX);die();}
				strcpy(reg_msp[reg_msp_count],name);//getfilename(name));//fname);
				reg_msp_count++;
				strcpy(ext[start],".");
				write32(ext[start]+1,PH.ext[i]);
				start++;
			}
			_consolePrintf("Regist MSP: %s\n",name);
		}
	}
	dirclose(dp);
	return start;
}

TPluginBody* getPluginByIndex(int idx){
	//char name[768];
	//sprintf(name,"/moonshl/plugin/%s",reg_msp[idx]);
	//return DLLList_LoadPlugin(name);
	return DLLList_LoadPlugin(reg_msp[idx]);
}	

//pref
u8 dldibuf[32768];
int selectpref(char *title,int argc, char *argv[]){
	int cursor=0,key,keycount=0,keycountmax=KEYCOUNTMAX_UNREPEATED;
	for(;;){
		swiWaitForVBlank();
		int i=0;
		_consoleClear2();
		_consolePrintf2("%s\n------------------------------------------",title);
		for(;i<argc;i++){
			_consolePrint2(i==cursor?"\n*":"\n ");
			_consolePrint2(argv[i]);
		}
		for(;;swiWaitForVBlank()){
			if(!IPCZ->keysheld)continue;
			if(key=IPCZ->keysdown){keycountmax=KEYCOUNTMAX_UNREPEATED;break;}
			keycount++;
			if(keycount==keycountmax){key=IPCZ->keysheld&0xf0;keycountmax=KEYCOUNTMAX_REPEATED;break;}
		}
		keycount=0;

		if(key&KEY_UP){
			if(cursor==0){
				cursor=argc-1;
			}else{
				cursor--;
			}
		}
		if(key&KEY_DOWN){
			if(cursor==argc-1){
				cursor=0;
			}else{
				cursor++;
			}
		}
#if 0
		if(cursor!=oldcursor&&argc>paging){
			oldcursor=cursor;
			if(cursor<5){top=0;continue;}
			if(cursor>argc-7){top=argc-paging;continue;}
			if(cursor-5<top/*&&top<argc-7*/){top=cursor-5;continue;}
			if(/*5<top&&*/top<cursor-paging+6){top=cursor-paging+6;continue;}
		}
#endif
		if(key&0xf0)continue;

		if(key&KEY_A)return cursor;
		if(key&KEY_B)return -1;
	}
}

const int DitherTable4Data8[16]={
	0,4,1,5,
	6,2,7,3,
	1,5,0,4,
	7,3,6,2
};

unsigned char DitherTable4(int c,int x,int y){
	int sx=x&3,sy=y&3;
	c+=DitherTable4Data8[sx*4+sy];
	if(c>0xff)c=0xff;
	return c&~7;
}

void *bitmap_create(int width, int height, unsigned int state){return calloc(width*height,4);}

void bitmap_set_suspendable(void *bitmap, void *private_word,
			     void (*invalidate)(void *bitmap, void *private_word)){}

//void invalidate(void *bitmap, void *private_word){}

unsigned char *bitmap_get_buffer(void *bitmap){return bitmap;}

size_t bitmap_get_bpp(void *bitmap){return 4;}

void bitmap_destroy(void *bitmap){if(bitmap)free(bitmap);}

int bmpdecode_gbaborder(char *name){
	struct stat st;
	int x,y;
	FILE *in=fopen(name,"rb");
	if(!in)return -1;
	fstat(fileno(in),&st);
	//if(!strcasecmp(name+strlen(name)-4,".bmp")){
		u8 *imgbuf=malloc(st.st_size);
		if(!imgbuf){fclose(in);return -2;}
		fread(imgbuf,1,st.st_size,in);
		fclose(in);
		bmp_image bmp;
		bmp_bitmap_callback_vt vt={
			bitmap_create,
			bitmap_destroy,
			bitmap_set_suspendable,
			bitmap_get_buffer,
			bitmap_get_bpp
		};

		bmp_create(&bmp,&vt);
		if(bmp_analyse(&bmp, st.st_size, imgbuf)||bmp.width!=256||bmp.height!=192||bmp_decode(&bmp)){bmp_finalise(&bmp);free(imgbuf);return 1;}
		free(imgbuf);
		for(y=0;y<192;y++)
			for(x=0;x<256;x++){
				u32 coor=y*256+x;
				u8 r=((((u32*)bmp.bitmap)[coor]&0xff0000)>>16)&0xff;
				u8 g=((((u32*)bmp.bitmap)[coor]&0x00ff00)>>8)&0xff;
				u8 b=((((u32*)bmp.bitmap)[coor]&0x0000ff)>>0)&0xff;
				//dither always
				if(1){ //dither){
					r=DitherTable4(r,x,y);g=DitherTable4(g,x,y);b=DitherTable4(b,x,y);
				}
				((u16*)0x06000000)[coor]=((u16*)0x06020000)[coor]=(1<<15)|((r>>3)<<10)|((g>>3)<<5)|((b>>3)<<0);
			}
		bmp_finalise(&bmp);
	//}
	return 0;
}

int bmpdecode_show(char *name){
	struct stat st;
	int x,y;
	FILE *in=fopen(name,"rb");
	if(!in)return -1;
	fstat(fileno(in),&st);
	//if(!strcasecmp(name+strlen(name)-4,".bmp")){
		u8 *imgbuf=malloc(st.st_size);
		if(!imgbuf){fclose(in);return -2;}
		fread(imgbuf,1,st.st_size,in);
		fclose(in);
		bmp_image bmp;
		bmp_bitmap_callback_vt vt={
			bitmap_create,
			bitmap_destroy,
			bitmap_set_suspendable,
			bitmap_get_buffer,
			bitmap_get_bpp
		};

		bmp_create(&bmp,&vt);
		if(bmp_analyse(&bmp, st.st_size, imgbuf)||bmp.width==0||bmp.height==0||bmp_decode(&bmp)){bmp_finalise(&bmp);free(imgbuf);return 1;}
		free(imgbuf);
		for(y=0;y<min(192,bmp.height);y++)
			for(x=0;x<min(256,bmp.width);x++){
				u32 coorfrom=y*bmp.width+x;
				u32 coorto=y*256+x;
				u8 r=((((u32*)bmp.bitmap)[coorfrom]&0xff0000)>>16)&0xff;
				u8 g=((((u32*)bmp.bitmap)[coorfrom]&0x00ff00)>>8)&0xff;
				u8 b=((((u32*)bmp.bitmap)[coorfrom]&0x0000ff)>>0)&0xff;
				//dither always
				if(1){ //dither){
					r=DitherTable4(r,x,y);g=DitherTable4(g,x,y);b=DitherTable4(b,x,y);
				}
				b15ptrMain[coorto]=(1<<15)|((r>>3)<<10)|((g>>3)<<5)|((b>>3)<<0);
			}
		_consolePrintf2(
			"%dx%d %dpixel %dbyte\n"
			"%dbpp %dcolors encoding=%s\n",
			bmp.width,bmp.height,bmp.width*bmp.height,st.st_size,bmp.bpp,bmp.colours,
			bmp.encoding==0?"RGB":bmp.encoding==1?"RLE8":bmp.encoding==2?"RLE4":bmp.encoding==0?"BitFields":"Other"
		);
		bmp_finalise(&bmp);
	//}
	return 0;
}

int icodecode_show(char *name){
	struct stat st;
	int x,y;
	FILE *in=fopen(name,"rb");
	if(!in)return -1;
	fstat(fileno(in),&st);
	//if(!strcasecmp(name+strlen(name)-4,".ico")){
		u8 *imgbuf=malloc(st.st_size);
		if(!imgbuf){fclose(in);return -2;}
		fread(imgbuf,1,st.st_size,in);
		fclose(in);
		bmp_image *bmp;
		bmp_bitmap_callback_vt vt={
			bitmap_create,
			bitmap_destroy,
			bitmap_set_suspendable,
			bitmap_get_buffer,
			bitmap_get_bpp
		};
		ico_collection ico;

		ico_collection_create(&ico,&vt);
		if(ico_analyse(&ico, st.st_size, imgbuf)||!(bmp=ico_find(&ico,0,0))||bmp->width==0||bmp->height==0||bmp_decode(bmp)){ico_finalise(&ico);free(imgbuf);return 1;}
		free(imgbuf);
		for(y=0;y<min(192,bmp->height);y++)
			for(x=0;x<min(256,bmp->width);x++){
				u32 coorfrom=y*bmp->width+x;
				u32 coorto=y*256+x;
				u8 r=((((u32*)bmp->bitmap)[coorfrom]&0xff0000)>>16)&0xff;
				u8 g=((((u32*)bmp->bitmap)[coorfrom]&0x00ff00)>>8)&0xff;
				u8 b=((((u32*)bmp->bitmap)[coorfrom]&0x0000ff)>>0)&0xff;
				//dither always
				if(1){ //dither){
					r=DitherTable4(r,x,y);g=DitherTable4(g,x,y);b=DitherTable4(b,x,y);
				}
				b15ptrMain[coorto]=(1<<15)|((r>>3)<<10)|((g>>3)<<5)|((b>>3)<<0);
			}
		_consolePrintf2(
			"%dx%d %dpixel %dbyte\n"
			"%dbpp %dcolors encoding=%s\n",
			bmp->width,bmp->height,bmp->width*bmp->height,st.st_size,bmp->bpp,bmp->colours,
			bmp->encoding==0?"RGB":bmp->encoding==1?"RLE8":bmp->encoding==2?"RLE4":bmp->encoding==0?"BitFields":"Other"
		);
		ico_finalise(&ico);

	//}
	return 0;
}

