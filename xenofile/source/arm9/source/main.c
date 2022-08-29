#include "xenofile.h"
#include "zlib/zlib.h" // CRC32
#include "libcarddump.h"
#include "libnsbmp.h"
const u16 bgcolor=RGB15(0,0,6);

#include "keyboard_b15z.h"
#include "keyboard_shift_b15z.h"

extern int hidehidden;

u8	key_tbl[0x4000]; //lol

//u32 sound_status,vflag,playtime,playtime_v,playtime_flag;
//u32 need_read,prev_timer;

void startSound(u32 freq, void *top_L, void *top_R, u32 size, u32 format){
	IPCZ->cmd=StopSound;
	while(IPCZ->cmd)swiWaitForVBlank();

	IPCZ->PCM_freq=freq;
	IPCZ->PCM_L=top_L;
	IPCZ->PCM_R=top_R;
	IPCZ->PCM_size=size;
	IPCZ->PCM_bits=format;
	DC_FlushAll();
	IPCZ->cmd=PlaySound;
	while(IPCZ->cmd)swiWaitForVBlank();
	
	//playtime	= 0;
	//playtime_v	= 0;
	//playtime_flag = 1;

	//sound_status = 6;
	//prev_timer	 = 0;
	//need_read    = 0;

	TIMER1_DATA	= 0;							// For counting samples
	TIMER1_CR	= TIMER_ENABLE | TIMER_DIV_1 | TIMER_CASCADE;
	TIMER0_DATA	= (65536-33554432/(freq)); //TIMER_FREQ(freq);		// Equal to sampling freq.
	TIMER0_CR	= TIMER_ENABLE | TIMER_DIV_1;
}

void stopSound(){
	IPCZ->cmd=StopSound;

	TIMER0_DATA	= 0;
	TIMER0_CR	= 0;
	TIMER1_DATA	= 0;
	TIMER1_CR	= 0;

	//sound_status = 0;
}

void swapcartbeforeexec(u8* buf){
#ifdef _LIBNDS_MAJOR_
	fatUnmount("fat:/");
#else
	fatUnmount(0);
#endif
	_consolePrintf(
		"mSD/flashcart unmounted. Please swap flashcart.\n"
		"Press A to proceed, B to shutdown.\n"
	);

	///
	for(swiWaitForVBlank();;swiWaitForVBlank()){
		if(!(IPCZ->keysdown))continue;
		if(IPCZ->keysdown&KEY_A)break;
		if(IPCZ->keysdown&KEY_B)IPCZ->cmd=Shutdown;
	}
	u32 romID = Card_Open(key_tbl);
	while(romID == 0xFFFFFFFF){
		_consolePrintf("Cannot be recognized. Insert again.\r");
		for(swiWaitForVBlank();;swiWaitForVBlank()){
			if(!(IPCZ->keysdown))continue;
			if(IPCZ->keysdown&KEY_A)break;
			if(IPCZ->keysdown&KEY_B)IPCZ->cmd=Shutdown;
		}
		romID = Card_Retry();
	}
	Card_Close();
	_consolePrintf("done.\n");
}

char ext[32][EXTMAX]={
	".nds", //1
	".c",".cpp",".h",".pl",".py",".rb",".php",".txt",".ini",".log",".cfg",".conf",".htm",".html",".lst", //16
	".sav",".bak",".duc",".dsv",".gds", //21
	".dldi",".b15",".ani",".bmp",".ico",".u8m", //27
};

char *prefs[]={
	"Use Rudolph Loader to boot",
	"Use MoonShell Simply Loader to boot",
	"Use bootlib loader to boot",
	"Use Rudolph/Moonlight hybrid loader",
	"Use rpglink to boot",
	"Use my DLDI to boot",
	"Disable DLDI patching",
};

char *context[]={
	"Copy",
	"Cut",
	"Paste",
	"Delete",
	"Rename",
	"Create new file",
	"Make directory",
	"Change timestamp",
	"Touch",
	"Change attribute",
	"Stat",
	"Calc hash",
	"Fix header",
	"Encrypt secure area",
	"Decrypt secure area",
	"Run as homebrew",
	"Run as hb after swapping cart",
	"Run as DSBooter",
	//"Run as DSBooter  (Raw)",
	"Run as R4 kernel (only on R4)",
	"Manual DLDI patch",
	"Open in text editor",
};

char *systemmenu[]={
	"About",
	"Show partition info",
	"Show system info",
	"Show M3 Region / R4 Jumper",
	"Swap microSD",
	"Return to firmware(moonshl2)",
	"Return to NDS firmware",
	"Shutdown",
	"Dump Bios/Firmware",
	"Test microSD speed",
	"Boot to Slot2 NDS",
	"Boot to Slot2 GBA",
};

enum{
	op_copy=0,
	op_cut,
	op_paste,
	op_delete,
	op_rename,
	op_newfile,
	op_mkdir,
	op_timestamp,
	op_touch,
	op_attr,
	op_stat,
	op_hash,
	op_fixheader,
	op_encryptsecure,
	op_decryptsecure,
	op_hb,
	op_hb_swapcart,
	op_dsb,
	//op_dsbraw,
	op_r4,
	op_dldi,
	op_textedit,
};

enum{
	sys_notice=0,
	sys_partitioninfo,
	sys_systeminfo,
	sys_m3region,
	sys_swapsd,
	sys_return,
	sys_dsmenu,
	sys_shutdown,
	sys_bios,
	sys_testsd,
	sys_slot2nds,
	sys_slot2gba,
};

type_constpchar BootLibrary;

int msp_end,ext_end;
int filter;
void usage(){
	_consoleClear2();
#if 0
	_consolePrintf2(
		"XenoFile - simple and sophisticated filer\n"
		"Powered by libprism "ROMVERSION"\n"
		"reset_mse_06b_for_ak2 by Moonlight, Rudolph, kzat3\n"
		"MoonShell NDS Loader by Moonlight\n"
		"VRAM bootlib by Chishm\n"
		"dldipatch aka dlditool public domain under CC0\n"
		"zlib by Jean-loup Gailly and Mark Adler\n"
		ROMDATE"\n"
		ROMENV"\n\n"
	);
#endif
	_consolePrint2("Supported type:\n");
	int i=0;
	for(;i<ext_end;i++){
		if(i)_consolePrintChar2('/');
		_consolePrint2(ext[i]);
	}

	_consolePrintf2(
		"\n\n"
		" Cross : Cursor (Left/Right: move 10)\n"
		"     A : Chdir/Execute/Open by textedit\n"
		"     B : Chdir to parent\n"
		"     X : Toggle showing hidden/system file\n"
		"     Y : System Menu\n"
		"   L/R : Switch file filter\n"
		" Start : Context Menu\n"
		"Select : Preferences\n\n"
	);
	_consolePrintf2("Current filter: %s\n%s hidden/system\n\n",
		filter==0?"all":filter==1?"nds":"supported",
		hidehidden==0?"Show":"Hide"
	);
}

void notice(){
	_consoleClear2();
	_consolePrintf2(
		"XenoFile - simple and sophisticated filer\n"
		"*** XenoShell Legacy ***\n"
		"(C) Taiju Yamada\n"
#ifdef GPL
		"under 2-clause BSDL / GNU GPL.\n"
#else
		"under 2-clause BSDL.\n"
#endif
		"Powered by libprism "ROMVERSION"\n"
		ROMDATE"\n"
		ROMENV"\n\n"
		//"[loader]\n"
		"reset_mse_06b_for_ak2 by Moonlight, Rudolph, kzat3\n"
		"MoonShell NDS Loader by Moonlight\n"
		"VRAM bootlib (external) by Chishm\n"
		"rpglink (C) Acekard.com and Xenon under MIT license\n"
		"\n"
		//"[library]\n"
		"dldipatch aka dlditool public domain under CC0\n"
		"libmshlsplash under CC0\n"
		"libcarddump by Rudolph\n"
		"Secure Encryption by DarkFader\n"
		"Keyboard Library by HeadSoft\n"
		"MoonShell Plugin routine by Moonlight\n"
		"libfat (C) Chishm under BSD License\n"
		//"minIni (C) ITB CompuPhase under Apache License 2.0\n"
		"zlib (C) Jean-loup Gailly / Mark Adler under zlib/libpng license\n"
		"libnsbmp (C) Richard Wilson / Sean Fox under MIT license\n"
		"md5.c (C) RSA Data Security, Inc.\n"
		"Sound Library by MeRAMAN\n"
		//"\n"
		//"Supported type:\n"
	);
/*
	int i=0;
	for(;i<ext_end;i++){
		if(i)_consolePrintChar2('/');
		_consolePrint2(ext[i]);
	}
*/
	//_consolePrintChar2('\n');
	while(IPCZ->keysheld)swiWaitForVBlank();
	//_consolePrintf2("\n");
	_consolePrintf2("Press any key.\n");
	while(!IPCZ->keysdown)swiWaitForVBlank();
	usage();
}

static u8 keyboard_buf[49158],keyboard_shift_buf[49158];

static char dir[768],file[768],loader[768];
static char clipboard[768];
static char clipboardiscopy=0;

void Main(){
	char dldiid[5];

	clipboard[0]=0;
	IPCZ->cmd=0;
	_consoleClear();
	_consolePrint("Waiting for initialization...\n");
	PrintfToDie=_consolePrintf2;

	_consolePrint2("Initializing XenoFile...\n\n");
	{
		unsigned char *dldiFileData=DLDIDATA;
		memcpy(dldiid,(char*)dldiFileData+ioType,4);
		dldiid[4]=0;
		_consolePrintf2("DLDI ID: %s\n",dldiid);
		_consolePrintf2("DLDI Name: %s\n\n",(char*)dldiFileData+friendlyName);
	}

	_consolePrintf2("Initializing libfat... ");
	if(!fatInitDefault()){_consolePrintf2("Failed.\n");die();}
	_consolePrintf2("Done.\n");

	BootLibrary=bootlibAvailable()?runNdsFile:ret_menu9_Gen;

	ini_gets("mshl2wrap",dldiid,"",loader,256*3,"/MOONSHL2/EXTLINK/mshl2wrap.ini");

	//delay enough
	IPCZ->arm7bios_addr=key_tbl;
	IPCZ->arm7bios_bufsize=0x4000;
	IPCZ->cmd=GetARM7Bios;
	while(IPCZ->cmd)swiWaitForVBlank();

	_consolePrint2("Iterating MoonShell plugin...\n");
	msp_end=iterateMSP(27);
	_consolePrint2("Iterating MoonShell2 extlink...\n");
	ext_end=iterateExtLink(msp_end);

	_consolePrint2("Iterating root dir...\n");
	memset(&ftop,0,sizeof(top));
	strcpy(dir,mypath);

	filter=hidehidden=0;
	int key=0,keycount=0,keycountmax=KEYCOUNTMAX_UNREPEATED;
	getfilelist(dir,filter);
	int cursor=0,oldcursor=0;

	_consolePrint2("Allocating keyboard...\n");
	zlib_decompress(keyboard_b15z,keyboard_b15z_size,keyboard_buf,49158);
	zlib_decompress(keyboard_shift_b15z,keyboard_shift_b15z_size,keyboard_shift_buf,49158);

	//because some libnds doesn't have "struct _user_data" thing. Universal source among devkitARM r23/r31.
	unsigned char userdata=((u8*)(&PersonalData->calY2px))[1];

	usage();

	for(;;){
		swiWaitForVBlank();
		showfilelist(cursor,dir);
		for(;;swiWaitForVBlank()){
			if(!IPCZ->keysheld)continue;
			if(key=IPCZ->keysdown){keycountmax=KEYCOUNTMAX_UNREPEATED;break;}
			keycount++;
			if(keycount==keycountmax){key=IPCZ->keysheld&0xf0;keycountmax=KEYCOUNTMAX_REPEATED;break;}
		}
		keycount=0;

		if(key&KEY_UP){
			if(cursor==0){
				cursor=contentcount-1;
			}else{
				cursor--;
			}
		}
		if(key&KEY_DOWN){
			if(cursor==contentcount-1){
				cursor=0;
			}else{
				cursor++;
			}
		}
		if(key&KEY_LEFT){
			if(cursor==0){
				cursor=contentcount-1;
			}else if(cursor<scroll){
				cursor=0;
			}else{
				cursor-=scroll;
			}
		}
		if(key&KEY_RIGHT){
			if(cursor==contentcount-1){
				cursor=0;
			}else if(cursor>contentcount-1-scroll){
				cursor=contentcount-1;
			}else{
				cursor+=scroll;
			}
		}
		if(cursor!=oldcursor&&contentcount>paging){
			oldcursor=cursor;
			if(cursor<5){top=0;continue;}
			if(cursor>contentcount-7){top=contentcount-paging;continue;}
			if(cursor-5<top/*&&top<contentcount-7*/){top=cursor-5;continue;}
			if(/*5<top&&*/top<cursor-paging+6){top=cursor-paging+6;continue;}
		}
		if(key&0xf0)continue;

		swiWaitForVBlank();
		fileinfo *p=&ftop;
		{int i=0;for(;i<=cursor;i++)p=p->next;}
		if(key&KEY_A){
			if(p->isDir){
				if(!strcmp(p->name,"../")){
					if(dir[1]==0)continue;
					dir[strlen(dir)-1]=0;
					strrchr(dir,'/')[1]=0;
					getfilelist(dir,filter);
					cursor=0,oldcursor=0,top=0;
					continue;
				}
				strcat(dir,p->name);
				getfilelist(dir,filter);
				cursor=0,oldcursor=0,top=0;
				continue;
			}

			//handler_NDS
			if(strlen(p->name)>3&&!strcasecmp(p->name+strlen(p->name)-4,".nds")){
				strcpy(file,dir);
				strcat(file,p->name);
				destroyfilelist();
				_consolePrintf2("Selected %s\n",file);
				_consoleClear();
				if(loader[0])runCommercial(file,loader);
				BootLibrary(file);
				while(IPCZ->keysheld)swiWaitForVBlank();
				_consolePrintf("Failed. Press any key.\n");
				while(!IPCZ->keysdown)swiWaitForVBlank();
				usage();
				getfilelist(dir,filter);
			break;}

			//handler_Text
			{int i;for(i=1;i<16;i++)
			if(strlen(p->name)>=strlen(ext[i])&&!strcasecmp(p->name+strlen(p->name)-strlen(ext[i]),ext[i])){
				strcpy(file,dir);
				strcat(file,p->name);
				destroyfilelist();
				_consolePrintf2("Selected %s\n",file);
				_consoleClear();
				runTextEdit(file);
				while(IPCZ->keysheld)swiWaitForVBlank();
				_consolePrintf("Failed. Press any key.\n");
				while(!IPCZ->keysdown)swiWaitForVBlank();
				usage();
				getfilelist(dir,filter);
			break;}}

			//handler_SavConvert
			{int i;for(i=16;i<21;i++)
			if(strlen(p->name)>=strlen(ext[i])&&!strcasecmp(p->name+strlen(p->name)-strlen(ext[i]),ext[i])){
				strcpy(file,dir);
				strcat(file,p->name);
				destroyfilelist();
				_consoleClear2();
				bool ret=savConvert(file);
				while(IPCZ->keysheld)swiWaitForVBlank();
				_consolePrintf2("Convert %s. Press any key.\n",ret?"Done":"Failed");
				while(!IPCZ->keysdown)swiWaitForVBlank();
				usage();
				getfilelist(dir,filter);
			break;}}

			// handler_DLDI
			{int i;for(i=21;i<22;i++)
			if(strlen(p->name)>=strlen(ext[i])&&!strcasecmp(p->name+strlen(p->name)-strlen(ext[i]),ext[i])){
				struct stat st;
				strcpy(file,dir);
				strcat(file,p->name);
				if(stat(file,&st)||st.st_size<128||st.st_size>32768){
					while(IPCZ->keysheld);
					_consolePrintf2("DLDI open failed or invalid size. Press any key.\n");
					while(!IPCZ->keysdown);
					usage();
					continue;
				}
				FILE *f=fopen(file,"rb");
				fread(dldibuf,1,st.st_size,f);
				fclose(f);
				DLDIToBoot=dldibuf;
				_consolePrintf2("Selected DLDI: %s\n",file);
			break;}}

			// handler_B15
			{int i;for(i=22;i<23;i++)
			if(strlen(ext[i])&&strlen(p->name)>=strlen(ext[i])&&!strcasecmp(p->name+strlen(p->name)-strlen(ext[i]),ext[i])){
				strcpy(file,dir);
				strcat(file,p->name);
				_consolePrintf2("Displaying %s\n",file);
				u16 z[256];
				FILE *f=fopen(file,"rb");
				if(!f){
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf2("cannot open. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();continue;
				}
				fread(z,2,3,f);
				u16 width=read16(z),height=read16(z+1);
				u32 size=filelength(fileno(f));
				_consolePrintf2("%dx%d size=%d\n",width,height,size);
				if(width>256||height>192||size!=width*height*2+6){
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf2("%s. Press any key.\n",(size!=width*height*2+6)?"filesize wrong":"image too big");
					fclose(f);
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();continue;
				}

				vramset(b15ptrMain,0xffff,256*192);
				{int l=0;for(;l<height;l++){
						fread(z,2,width,f);
						vramcpy(b15ptrMain+256*l,z,width);
				}}
				EnableB15Main();
				while(IPCZ->keysheld)swiWaitForVBlank();
				_consolePrintf2("Press any key to end.\n");
				while(!IPCZ->keysdown)swiWaitForVBlank();
				DisableB15Main();
				usage();
			break;}}

			// handler_ANI
			{int i;for(i=23;i<24;i++)
			if(strlen(ext[i])&&strlen(p->name)>=strlen(ext[i])&&!strcasecmp(p->name+strlen(p->name)-strlen(ext[i]),ext[i])){
				strcpy(file,dir);
				strcat(file,p->name);
				_consolePrintf2("Displaying %s\n",file);
				_consolePrintf2("A to slideshow, B to end.\n",file);
				splash* sp=opensplash(file);
				if(!sp){
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf2("invalid file or cannot initialize libmshlsplash. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();continue;
				}
				_consolePrintf2("%s splash.\n",
					sp->version==SPLASH_MOONSHELL1?"MoonShell1":
					sp->version==SPLASH_MOONSHELL2?"MoonShell2":
					sp->version==SPLASH_ISAKUREAL?"iSakuReal":
					sp->version==SPLASH_M3SAKURA?"M3Sakura":
					"");//unreachable
				int oldcursor=0,cursor=0,key,keycount=0,keycountmax=KEYCOUNTMAX_UNREPEATED;
				vramset(b15ptrMain,0xffff,256*192);
				decompressimage(sp,cursor);
				vramcpy(b15ptrMain,sp->decompbuf,getimagesize(sp,cursor)/2);
				_consolePrintf2("Showing %03d / %03d\r",cursor,sp->frame-1);
				EnableB15Main();
				for(swiWaitForVBlank();;swiWaitForVBlank()){
					for(;;swiWaitForVBlank()){
						if(!IPCZ->keysheld)continue;
						if(key=IPCZ->keysdown){keycountmax=KEYCOUNTMAX_UNREPEATED;break;}
						keycount++;
						if(keycount==keycountmax){key=IPCZ->keysheld&0xf0;keycountmax=KEYCOUNTMAX_REPEATED;break;}
					}
					keycount=0;

					if(key&KEY_UP||key&KEY_LEFT){
						if(cursor==0)cursor=sp->frame;
						cursor--;
					}
					if(key&KEY_DOWN||key&KEY_RIGHT){
						if(cursor==sp->frame-1)cursor=-1;
						cursor++;
					}
					if(oldcursor!=cursor){
						decompressimage(sp,cursor);
						vramcpy(b15ptrMain,sp->decompbuf,getimagesize(sp,cursor)/2);
						_consolePrintf2("Showing %03d / %03d\r",cursor,sp->frame-1);
						oldcursor=cursor;
					}
					if(key&0xf0)continue;
					if(key&KEY_B)break;
					if(key&KEY_A){
						for(cursor=0;cursor<sp->frame;cursor++){
							decompressimage(sp,cursor);
							swiWaitForVBlank();
							vramcpy(b15ptrMain,sp->decompbuf,getimagesize(sp,cursor)/2);
							_consolePrintf2("Showing %03d / %03d\r",cursor,sp->frame-1);
						}
						oldcursor=cursor=sp->frame-1;
					}
				}
				DisableB15Main();
				closesplash(sp);
				usage();
			break;}}

			// handler_BMP
			{int i;for(i=24;i<26;i++)
			if(strlen(ext[i])&&strlen(p->name)>=strlen(ext[i])&&!strcasecmp(p->name+strlen(p->name)-strlen(ext[i]),ext[i])){
				strcpy(file,dir);
				strcat(file,p->name);
				_consolePrintf2("Displaying %s\n",file);
				vramset(b15ptrMain,0xffff,256*192);
				if(i==24?bmpdecode_show(file):icodecode_show(file)){_consolePrintf2("Decode failed\n");goto bmp_fail;}
/*
				u16 z[256];
				FILE *f=fopen(file,"rb");
				if(!f){
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf2("cannot open. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();continue;
				}
				fread(z,2,3,f);
				u16 width=read16(z),height=read16(z+1);
				u32 size=filelength(fileno(f));
				_consolePrintf2("%dx%d size=%d\n",width,height,size);
				if(width>256||height>192||size!=width*height*2+6){
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf2("%s. Press any key.\n",(size!=width*height*2+6)?"filesize wrong":"image too big");
					fclose(f);
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();continue;
				}

				vramset(b15ptrMain,0xffff,256*192);
				{int l=0;for(;l<height;l++){
						fread(z,2,width,f);
						vramcpy(b15ptrMain+256*l,z,width);
				}}
*/
				EnableB15Main();
bmp_fail:
				while(IPCZ->keysheld)swiWaitForVBlank();
				_consolePrintf2("Press any key to end.\n");
				while(!IPCZ->keysdown)swiWaitForVBlank();
				DisableB15Main();
				usage();
			break;}}

			// handler_U8M
			{int i;for(i=26;i<27;i++)
			if(strlen(ext[i])&&strlen(p->name)>=strlen(ext[i])&&!strcasecmp(p->name+strlen(p->name)-strlen(ext[i]),ext[i])){
				struct stat st;
				FILE *f;
				strcpy(file,dir);
				strcat(file,p->name);
				_consolePrintf2("Playing %s\n",file);
				f=fopen(file,"rb");
				fstat(fileno(f),&st);
				u8 *p=malloc(st.st_size);
				if(!p){_consolePrintf2("Cannot alloc\n");goto u8m_fail;}
				fread(p,1,st.st_size,f);
				fclose(f);
				int i=0;for(;i<st.st_size;i++)p[i]-=0x80;
				stopSound();
				startSound(22050,p,p,st.st_size,8);

u8m_fail:
				while(IPCZ->keysheld)swiWaitForVBlank();
				_consolePrintf2("Press any key to end.\n");
				while(!IPCZ->keysdown)swiWaitForVBlank();
				stopSound();
				free(p);
				usage();
			break;}}

			// handler_MSP
			{int i;for(i=27;i<msp_end;i++)
			if(strlen(ext[i])&&strlen(p->name)>=strlen(ext[i])&&!strcasecmp(p->name+strlen(p->name)-strlen(ext[i]),ext[i])){
				strcpy(file,dir);
				strcat(file,p->name);
				destroyfilelist();
				//_consoleClear();
				_consoleClear2();

				FILE *f=fopen(file,"rb");
				if(!f){_consolePrintf2("cannot open file.\n");goto msp_fail;}
				TPluginBody* PB=NULL;
				PB=getPluginByIndex(i-27);
				if(!PB){_consolePrintf2("cannot open msp.\n");goto msp_fail;}

				if(PB->PluginHeader.PluginType==EPT_Image){
					_consolePrintf2("Displaying %s\n",file);
					vramset(b15ptrMain,0xffff,256*192);
					_consolePrintf2("Opening...\n");
					if(!PB->pIL->Start((int)f)){_consolePrintf2("cannot open image.\n");goto msp_fail;}
				
					PB->pIL->RefreshScreen(0,0,b15ptrMain,256,256,192,false);
					EnableB15Main();
				}else if(PB->PluginHeader.PluginType==EPT_Sound){
					_consolePrintf2("Playing %s\n",file);
					stopSound();
					_consolePrintf2("Opening...\n");
					if(!PB->pSL->Start((int)f)){_consolePrintf2("cannot open music.\n");goto msp_fail;}
					u16 sampleperframe=PB->pSL->GetSamplePerFrame();
					u16 channel=PB->pSL->GetChannelCount();
					s16 *pL=(s16*)malloc(2*sampleperframe*16);
					s16 *pR=NULL;
					if(channel>1)pR=(s16*)malloc(2*sampleperframe*16);
					if(!pL||(channel>1&&!pR)){
						if(pL)free(pL);
						if(pR)free(pR);
						_consolePrintf2("cannot alloc buffer.\n");goto msp_fail;
					}
					int f=0;
					for(;f<8;f++){
						PB->pSL->Update(pL+sampleperframe*(2*f),channel>1?pR+sampleperframe*(2*f):NULL);
						PB->pSL->Update(pL+sampleperframe*(2*f+1),channel>1?pR+sampleperframe*(2*f+1):NULL);
					}
					startSound(PB->pSL->GetSampleRate(),pL,channel>1?pR:pL,2*sampleperframe*16,16);
					IPCZ->blanks=0;

					//while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf2("\nPress any key to end.\n");
//_consolePrintf2("%08x\n",sampleperframe);
					f=0;
					u16 oldtime=0;
					while(!IPCZ->keysdown){ //mainloop.
						u16 newtime=TIMER1_DATA;
//_consolePrintf("%08x %08x\n",newtime,sampleperframe*2+oldtime);
						while((u16)(newtime-oldtime)>=sampleperframe*2){
							u16 ret=0;
//_consolePrintf("%d %08x %08x\n",f,newtime-oldtime,TIMER1_DATA);

							///in case of delay...
							if((u16)(newtime-oldtime)>=sampleperframe*14){
								ret+=PB->pSL->Update(pL+sampleperframe*(2*f),channel>1?pR+sampleperframe*(2*f):NULL);
								ret+=PB->pSL->Update(pL+sampleperframe*(2*f+1),channel>1?pR+sampleperframe*(2*f+1):NULL);
								f++;
								if(f==8)f=0;
							}
							if((u16)(newtime-oldtime)>=sampleperframe*12){
								ret+=PB->pSL->Update(pL+sampleperframe*(2*f),channel>1?pR+sampleperframe*(2*f):NULL);
								ret+=PB->pSL->Update(pL+sampleperframe*(2*f+1),channel>1?pR+sampleperframe*(2*f+1):NULL);
								f++;
								if(f==8)f=0;
							}
							if((u16)(newtime-oldtime)>=sampleperframe*10){
								ret+=PB->pSL->Update(pL+sampleperframe*(2*f),channel>1?pR+sampleperframe*(2*f):NULL);
								ret+=PB->pSL->Update(pL+sampleperframe*(2*f+1),channel>1?pR+sampleperframe*(2*f+1):NULL);
								f++;
								if(f==8)f=0;
							}
							if((u16)(newtime-oldtime)>=sampleperframe*8){
								ret+=PB->pSL->Update(pL+sampleperframe*(2*f),channel>1?pR+sampleperframe*(2*f):NULL);
								ret+=PB->pSL->Update(pL+sampleperframe*(2*f+1),channel>1?pR+sampleperframe*(2*f+1):NULL);
								f++;
								if(f==8)f=0;
							}
							if((u16)(newtime-oldtime)>=sampleperframe*6){
								ret+=PB->pSL->Update(pL+sampleperframe*(2*f),channel>1?pR+sampleperframe*(2*f):NULL);
								ret+=PB->pSL->Update(pL+sampleperframe*(2*f+1),channel>1?pR+sampleperframe*(2*f+1):NULL);
								f++;
								if(f==8)f=0;
							}
							if((u16)(newtime-oldtime)>=sampleperframe*4){
								ret+=PB->pSL->Update(pL+sampleperframe*(2*f),channel>1?pR+sampleperframe*(2*f):NULL);
								ret+=PB->pSL->Update(pL+sampleperframe*(2*f+1),channel>1?pR+sampleperframe*(2*f+1):NULL);
								f++;
								if(f==8)f=0;
							}
							///

							ret+=PB->pSL->Update(pL+sampleperframe*(2*f),channel>1?pR+sampleperframe*(2*f):NULL);
							ret+=PB->pSL->Update(pL+sampleperframe*(2*f+1),channel>1?pR+sampleperframe*(2*f+1):NULL);
							f++;
							if(f==8)f=0;
							oldtime=newtime;
							DC_FlushAll();

							_consolePrintf2("Playing Time %02d:%02d\r",IPCZ->blanks/3600,IPCZ->blanks/60%60);

							if(ret<sampleperframe*2){
								while(TIMER1_DATA-oldtime<ret);
								goto msp_sound_finalize;
							}
						}
					}
msp_sound_finalize:
					stopSound();
					free(pL);if(pR)free(pR);
					goto msp_sound_end;
				}
msp_fail:
				while(IPCZ->keysheld)swiWaitForVBlank();
				_consolePrintf2("\nPress any key to end.\n");
				while(!IPCZ->keysdown)swiWaitForVBlank();

msp_sound_end:
				stopSound();
				DisableB15Main();

				DLLList_FreePlugin(PB);
				fclose(f);
				getfilelist(dir,filter);
				usage();
			}}

			// handler_ExtLink
			{int i;for(i=msp_end;i<ext_end;i++)
			if(strlen(ext[i])&&strlen(p->name)>=strlen(ext[i])&&!strcasecmp(p->name+strlen(p->name)-strlen(ext[i]),ext[i])){
				strcpy(file,dir);
				strcat(file,p->name);
				destroyfilelist();
				_consolePrintf2("Selected %s\n",file);
				_consoleClear();
				runExtLink(file,ext[i]);
				while(IPCZ->keysheld)swiWaitForVBlank();
				_consolePrintf("Failed. Press any key.\n");
				while(!IPCZ->keysdown)swiWaitForVBlank();
				usage();
				getfilelist(dir,filter);
			}}

			continue;
		}
		if(key&KEY_B){
			if(dir[1]==0)continue;
			dir[strlen(dir)-1]=0;
			strrchr(dir,'/')[1]=0;
			getfilelist(dir,filter);
			cursor=0,oldcursor=0,top=0;
			continue;
		}
		if((key&KEY_L)||(key&KEY_R)){
			if(key&KEY_L)filter=(filter+1)%3;
			if(key&KEY_R)filter=(filter-1+3)%3;
			getfilelist(dir,filter);
			usage();
			cursor=0,oldcursor=0,top=0;
			continue;
		}
		if(key&KEY_START){
			strcpy(file,dir);
			strcat(file,p->name);
			if(p->isDir)file[strlen(file)-1]=0;
			int ret=selectpref("Context Menu",arraysize(context),context);
			usage();
			if(p->isDir){
				if(ret==-1)continue;
				if(
					ret!=op_cut&&
					ret!=op_paste&&
					ret!=op_delete&&
					ret!=op_rename&&
					ret!=op_stat&&
					ret!=op_timestamp&&
					ret!=op_touch&&
					ret!=op_attr&&
					ret!=op_newfile&&
					ret!=op_mkdir
				){
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf2("You cannot do that operation for folder.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					continue;
				}
				if(!strcmp(p->name,"../")&&(ret==op_cut||ret==op_delete||ret==op_rename||ret==op_timestamp||ret==op_touch||ret==op_attr)){
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf2("You cannot do that operation for \"..\".\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					continue;
				}
			}
			switch(ret){
				case -1:break;
				case op_copy:{
					strcpy(clipboard,file);
					clipboardiscopy=1;
					_consolePrintf2("Copied %s\n",file);
				}break;
				case op_cut:{
					strcpy(clipboard,file);
					clipboardiscopy=0;
					_consolePrintf2("Cut %s\n",file);
				}break;
				case op_paste:{
					struct stat st,stto;
					stat(clipboard,&st);
					strcpy(getfilename(file),getfilename(clipboard));
					if(!stat(file,&stto)){
						if(stto.st_mode&S_IFDIR){
							while(IPCZ->keysheld)swiWaitForVBlank();
							_consolePrintf2("%s is a folder; cannot paste. Press any key.\n",file);
							while(!IPCZ->keysdown)swiWaitForVBlank();
							usage();
							break;
						}
						_consolePrintf2("%s already exists (%s)\n",file,(stat(clipboard,&st),stto.st_mtime>st.st_mtime)?"NEWER":"older");
						_consolePrintf2("A to overwrite, other to abort.\n");
						int key;
						for(;!(key=IPCZ->keysdown);)swiWaitForVBlank();
						if(!(key&KEY_A)){_consolePrintf2("Aborted.\n");break;}
					}
					int ret=clipboardiscopy?copy(clipboard,file):rename(clipboard,file);
					if(clipboardiscopy)libprism_utime(file,st.st_atime,st.st_mtime);
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf2("%s. Press any key.\n",!ret?"Pasted":"Failed");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					getfilelist(dir,filter);
					usage();
					cursor=0,oldcursor=0,top=0;
					continue;
				}break;
				case op_delete:{
					struct stat st;
					stat(file,&st);
					if(st.st_mode&S_IFDIR){
						_consolePrintf2("%s is a folder. All files inside will be deleted. If you are sure, press A twice.\n",file);
						int key;
						for(;!(key=IPCZ->keysdown);)swiWaitForVBlank();
						if(!(key&KEY_A)){_consolePrintf2("Aborted.\n");break;}
						for(swiWaitForVBlank();!(key=IPCZ->keysdown);)swiWaitForVBlank();
						if(!(key&KEY_A)){_consolePrintf2("Aborted.\n");break;}
						strcat(file,"/");
						rm_rf(file);
						file[strlen(file)-1]=0;
					}else{
						_consolePrintf2("Press A to confirm to delete %s, other to cancel.\n",file);
						int key;
						for(;!(key=IPCZ->keysdown);)swiWaitForVBlank();
						if(!(key&KEY_A)){_consolePrintf2("Aborted.\n");break;}
					}
					unlink(file);
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf2("Deleted. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					getfilelist(dir,filter);
					usage();
					cursor=0,oldcursor=0,top=0;
				}break;
				case op_rename:{
					char to[768];
					strcpy(to,file);
					_consoleClear2();
rename_retry:
					_consolePrintf2("*** Rename ***\nA or Enter to proceed, B to cancel.\n\n");
					_consolePrintf2("Old:\n%s\nNew:\n%s",file,to);

					vramcpy(b15ptrSub+256*96,keyboard_buf+6,256*96);
					EnableB15Sub();
					int shift=0,caps=0;
					for(swiWaitForVBlank();;swiWaitForVBlank()){
						if(IPCZ->keysdown&KEY_A){_consolePrintChar2('\n');break;}
						if(IPCZ->keysdown&KEY_B){_consolePrintChar2('\n');goto rename_cancel;}
						if(IPCZ->keysdown&KEY_TOUCH &&
							0<=IPCZ->touchX&&IPCZ->touchX<=255 &&
							96<=IPCZ->touchY&&IPCZ->touchY<=191
						){
							u8 key=((caps|shift)?keyboard_Hit_Shift:keyboard_Hit)[((IPCZ->touchY-96)/8)*32+IPCZ->touchX/8];
							if(!key)continue;
							if(key==RET){
								_consolePrintChar2('\n');break;
							}else if(key==SHF){
								if(shift==0)caps=0;
								shift^=1;
								vramcpy(b15ptrSub+256*96,(shift?keyboard_shift_buf:keyboard_buf)+6,256*96);
							}else if(key==CAP){
								if(caps==0)shift=0;
								caps^=1;
								vramcpy(b15ptrSub+256*96,(caps?keyboard_shift_buf:keyboard_buf)+6,256*96);
							}else{
								if(strchr("\\:*?\"<>|",key))continue; //ignore
								if(key!=8||to[0]){
									_consolePrintChar2(key);
									if(key==8)to[strlen(to)-1]=0;
									else{to[strlen(to)+1]=0;to[strlen(to)]=key;}
								}
								shift=0;
								vramcpy(b15ptrSub+256*96,((caps|shift)?keyboard_shift_buf:keyboard_buf)+6,256*96);
							}
						}
					}

					if(!access(to,0)){
						_consoleClear2();
						_consolePrintf2("%s already exists. Please check.\n\n",to);
						goto rename_retry;
					}
					rename(file,to);

					DisableB15Sub();
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf2("\nDone. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					getfilelist(dir,filter);
					usage();
					cursor=0,oldcursor=0,top=0;
					continue;
rename_cancel:
					DisableB15Sub();
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf2("\nCancelled. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
					continue;
				}break;
				case op_newfile:{
					strcpy(getfilename(file),"");
					_consoleClear2();
newfile_retry:
					_consolePrintf2("*** Create new file ***\nA or Enter to proceed, B to cancel.\n\n");
					_consolePrintf2("%s",file);

					vramcpy(b15ptrSub+256*96,keyboard_buf+6,256*96);
					EnableB15Sub();
					int shift=0,caps=0;
					for(swiWaitForVBlank();;swiWaitForVBlank()){
						if(IPCZ->keysdown&KEY_A){_consolePrintChar2('\n');break;}
						if(IPCZ->keysdown&KEY_B){_consolePrintChar2('\n');goto newfile_cancel;}
						if(IPCZ->keysdown&KEY_TOUCH &&
							0<=IPCZ->touchX&&IPCZ->touchX<=255 &&
							96<=IPCZ->touchY&&IPCZ->touchY<=191
						){
							u8 key=((caps|shift)?keyboard_Hit_Shift:keyboard_Hit)[((IPCZ->touchY-96)/8)*32+IPCZ->touchX/8];
							if(!key)continue;
							if(key==RET){
								_consolePrintChar2('\n');break;
							}else if(key==SHF){
								if(shift==0)caps=0;
								shift^=1;
								vramcpy(b15ptrSub+256*96,(shift?keyboard_shift_buf:keyboard_buf)+6,256*96);
							}else if(key==CAP){
								if(caps==0)shift=0;
								caps^=1;
								vramcpy(b15ptrSub+256*96,(caps?keyboard_shift_buf:keyboard_buf)+6,256*96);
							}else{
								if(strchr("\\:*?\"<>|",key))continue; //ignore
								if(key!=8||file[0]){
									_consolePrintChar2(key);
									if(key==8)file[strlen(file)-1]=0;
									else{file[strlen(file)+1]=0;file[strlen(file)]=key;}
								}
								shift=0;
								vramcpy(b15ptrSub+256*96,((caps|shift)?keyboard_shift_buf:keyboard_buf)+6,256*96);
							}
						}
					}

					if(!access(file,0)){
						_consoleClear2();
						_consolePrintf2("%s already exists. Please check.\n\n",file);
						goto newfile_retry;
					}
					FILE *f=fopen(file,"wb");
					fputc(' ',f);
					fclose(f);

					DisableB15Sub();
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf2("\nDone. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					getfilelist(dir,filter);
					usage();
					cursor=0,oldcursor=0,top=0;
					continue;
newfile_cancel:
					DisableB15Sub();
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf2("\nCancelled. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
					continue;
				}break;
				case op_mkdir:{
					strcpy(getfilename(file),"");
					_consoleClear2();
mkdir_retry:
					_consolePrintf2("*** Make directory ***\nA or Enter to proceed, B to cancel.\n\n");
					_consolePrintf2("%s",file);

					vramcpy(b15ptrSub+256*96,keyboard_buf+6,256*96);
					EnableB15Sub();
					int shift=0,caps=0;
					for(swiWaitForVBlank();;swiWaitForVBlank()){
						if(IPCZ->keysdown&KEY_A){_consolePrintChar2('\n');break;}
						if(IPCZ->keysdown&KEY_B){_consolePrintChar2('\n');goto mkdir_cancel;}
						if(IPCZ->keysdown&KEY_TOUCH &&
							0<=IPCZ->touchX&&IPCZ->touchX<=255 &&
							96<=IPCZ->touchY&&IPCZ->touchY<=191
						){
							u8 key=((caps|shift)?keyboard_Hit_Shift:keyboard_Hit)[((IPCZ->touchY-96)/8)*32+IPCZ->touchX/8];
							if(!key)continue;
							if(key==RET){
								_consolePrintChar2('\n');break;
							}else if(key==SHF){
								if(shift==0)caps=0;
								shift^=1;
								vramcpy(b15ptrSub+256*96,(shift?keyboard_shift_buf:keyboard_buf)+6,256*96);
							}else if(key==CAP){
								if(caps==0)shift=0;
								caps^=1;
								vramcpy(b15ptrSub+256*96,(caps?keyboard_shift_buf:keyboard_buf)+6,256*96);
							}else{
								if(strchr("\\:*?\"<>|",key))continue; //ignore
								if(key!=8||file[0]){
									_consolePrintChar2(key);
									if(key==8)file[strlen(file)-1]=0;
									else{file[strlen(file)+1]=0;file[strlen(file)]=key;}
								}
								shift=0;
								vramcpy(b15ptrSub+256*96,((caps|shift)?keyboard_shift_buf:keyboard_buf)+6,256*96);
							}
						}
					}

					if(!access(file,0)){
						_consoleClear2();
						_consolePrintf2("%s already exists. Please check.\n\n",file);
						goto mkdir_retry;
					}
					mkdir(file,0777);

					DisableB15Sub();
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf2("\nDone. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					getfilelist(dir,filter);
					usage();
					cursor=0,oldcursor=0,top=0;
					continue;
mkdir_cancel:
					DisableB15Sub();
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf2("\nCancelled. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
					continue;
				}break;
				case op_timestamp:{
#ifndef _LIBNDS_MAJOR_
					_consolePrintf2("Changing timestamp isn't supported on legacy version.\n");goto timestamp_cancel;

#endif
					char origstamp[16], timestamp[100];
					struct tm t;
					{
						struct stat st;
						stat(file,&st);
						localtime_r(&st.st_mtime, &t);
						sprintf(origstamp,"%04d%02d%02d %02d%02d%02d",
							t.tm_year+1900,t.tm_mon+1,t.tm_mday,
							t.tm_hour,t.tm_min,t.tm_sec);
						strcpy(timestamp,origstamp);
					}
					_consoleClear2();
timestamp_retry:
					_consolePrintf2("*** Change timestamp ***\nA or Enter to proceed, B to cancel.\nFormat is YYYYMMDD hhmmss.\n\n");
					_consolePrintf2("Old: %s\nNew: %s",origstamp,timestamp);

					vramcpy(b15ptrSub+256*96,keyboard_buf+6,256*96);
					EnableB15Sub();
					int shift=0,caps=0;
					for(swiWaitForVBlank();;swiWaitForVBlank()){
						if(IPCZ->keysdown&KEY_A){_consolePrintChar2('\n');break;}
						if(IPCZ->keysdown&KEY_B){_consolePrintChar2('\n');goto timestamp_cancel;}
						if(IPCZ->keysdown&KEY_TOUCH &&
							0<=IPCZ->touchX&&IPCZ->touchX<=255 &&
							96<=IPCZ->touchY&&IPCZ->touchY<=191
						){
							u8 key=((caps|shift)?keyboard_Hit_Shift:keyboard_Hit)[((IPCZ->touchY-96)/8)*32+IPCZ->touchX/8];
							if(!key)continue;
							if(key==RET){
								_consolePrintChar2('\n');break;
#if 0
							}else if(key==SHF){
								if(shift==0)caps=0;
								shift^=1;
								vramcpy(b15ptrSub+256*96,(shift?keyboard_shift_buf:keyboard_buf)+6,256*96);
							}else if(key==CAP){
								if(caps==0)shift=0;
								caps^=1;
								vramcpy(b15ptrSub+256*96,(caps?keyboard_shift_buf:keyboard_buf)+6,256*96);
#endif
							}else{
								if(!strchr("\x08 0123456789",key))continue; //only accept...
								if(key!=8||timestamp[0]){
									_consolePrintChar2(key);
									if(key==8)timestamp[strlen(timestamp)-1]=0;
									else{timestamp[strlen(timestamp)+1]=0;timestamp[strlen(timestamp)]=key;}
								}
								shift=0;
								vramcpy(b15ptrSub+256*96,((caps|shift)?keyboard_shift_buf:keyboard_buf)+6,256*96);
							}
						}
					}

					t.tm_mday=-1;
					if(strlen(timestamp)==15&&timestamp[8]==' '){
						int date,time;
						sscanf(timestamp,"%d %d",&date,&time);
						t.tm_year=date/10000-1900;
						t.tm_mon=date/100%100-1;
						t.tm_mday=date%100;
						t.tm_hour=time/10000;
						t.tm_min=time/100%100;
						t.tm_sec=time%100;
					}
					if(validateTM(&t)){
						_consoleClear2();
						_consolePrintf2("Inavlid format. Input again.\n\n");
						goto timestamp_retry;
					}
					libprism_utime(file,1,mktime(&t));

					DisableB15Sub();
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf2("\nDone. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
					continue;
timestamp_cancel:
					DisableB15Sub();
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf2("\nCancelled. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
					continue;
				}break;
				case op_touch:{
#ifndef _LIBNDS_MAJOR_
					_consolePrintf2("Changing timestamp isn't supported on legacy version.\n");goto touch_cancel;

#endif
					_consoleClear2();
					_consolePrintf2("*** Touch file ***\n\n");
					libprism_touch(file);

					DisableB15Sub();
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf2("\nDone. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
					continue;
#ifndef _LIBNDS_MAJOR_
touch_cancel:
					DisableB15Sub();
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf2("\nCancelled. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
					continue;
#endif
				}break;
				case op_attr:{
#ifndef _LIBNDS_MAJOR_
					_consolePrintf2("Changing attribute isn't supported on legacy version.\n");goto attr_cancel;

#endif
					char attrib[6]="-----";
					int iattrib=getAttributes(file);
					if(iattrib&ATTRIB_RO)attrib[0]='R';
					if(iattrib&ATTRIB_HID)attrib[1]='H';
					if(iattrib&ATTRIB_SYS)attrib[2]='S';
					if(iattrib&ATTRIB_DIR)attrib[3]='D';
					if(iattrib&ATTRIB_ARCH)attrib[4]='A';

attr_again:
					_consoleClear2();
					_consolePrintf2("*** Change attribute ***\nA for Yes, B for No.\n\n");
					_consolePrintf2("Current attr: %s\n",attrib);
					u8 newattr=0;

					_consolePrintf2("Set readonly? ");
					for(swiWaitForVBlank();;swiWaitForVBlank()){
						if(IPCZ->keysdown&KEY_A){_consolePrintf2("Yes.\n");newattr|=ATTRIB_RO;break;}
						if(IPCZ->keysdown&KEY_B){_consolePrintf2("No.\n");break;}
					}
					_consolePrintf2("Set hidden? ");
					for(swiWaitForVBlank();;swiWaitForVBlank()){
						if(IPCZ->keysdown&KEY_A){_consolePrintf2("Yes.\n");newattr|=ATTRIB_HID;break;}
						if(IPCZ->keysdown&KEY_B){_consolePrintf2("No.\n");break;}
					}
					_consolePrintf2("Set system? ");
					for(swiWaitForVBlank();;swiWaitForVBlank()){
						if(IPCZ->keysdown&KEY_A){_consolePrintf2("Yes.\n");newattr|=ATTRIB_SYS;break;}
						if(IPCZ->keysdown&KEY_B){_consolePrintf2("No.\n");break;}
					}

					_consolePrintf2("\n"
						"Press Start to apply.\n"
						"Press Select to cancel.\n"
						"Press B to configure again.\n"
					);

					for(swiWaitForVBlank();;swiWaitForVBlank()){
						if(IPCZ->keysdown&KEY_START){goto attr_apply;}
						if(IPCZ->keysdown&KEY_SELECT){goto attr_cancel;}
						if(IPCZ->keysdown&KEY_B){goto attr_again;}
					}

attr_apply:
					libprism_chattr(file,newattr);

					DisableB15Sub();
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf2("\nDone. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
					continue;
attr_cancel:
					DisableB15Sub();
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf2("\nCancelled. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
					continue;
				}break;
				case op_stat:{
					char sfn[768],lfn[768],stime[30],attrib[6]="-----";
					u16 LFN[256];
					struct stat st;
					stat(file,&st);
					getsfnlfn(file,sfn,LFN);
					_FAT_directory_ucs2tombs(lfn,LFN,768);
					int iattrib=getAttributes(file);
					if(iattrib&ATTRIB_RO)attrib[0]='R';
					if(iattrib&ATTRIB_HID)attrib[1]='H';
					if(iattrib&ATTRIB_SYS)attrib[2]='S';
					if(iattrib&ATTRIB_DIR)attrib[3]='D';
					if(iattrib&ATTRIB_ARCH)attrib[4]='A';

					_consoleClear2();
					_consolePrintf2("SFN: %s\n",sfn);
					_consolePrintf2("LFN: %s\n",lfn);
					_consolePrintf2("Attrib:   %s\n",attrib);
					_consolePrintf2("Size:     %d\n",st.st_size);
					_consolePrintf2("Cluster:  %d\n",st.st_ino);
					_consolePrintf2("Sector:   %d\n",getSector(file));
					_consolePrintf2("Fragment: %d\n",getFragments(file));
					strftime(stime, 30, "%Y-%m-%d (%a)", localtime(&st.st_atime));
					_consolePrintf2("Access:   %s\n",stime);
					strftime(stime, 30, "%Y-%m-%d (%a) %H:%M:%S", localtime(&st.st_mtime));
					_consolePrintf2("Modify:   %s\n",stime);
					strftime(stime, 30, "%Y-%m-%d (%a) %H:%M:%S", localtime(&st.st_ctime));
					_consolePrintf2("Create:   %s\n",stime);

					_consolePrintf2("\n");
					_consolePrintf2("Fragment:0 means the file can be accessed sequentially (WoodR4DLDI save OK on clones).\n");
					_consolePrintf2("Last access time isn't recorded on FAT.\n");
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf2("\nPress any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
				}break;
				case op_hash:{
					_consoleClear2();
					_consolePrintf2("Calculating hashes of %s\n",file);
					FILE *f=fopen(file,"rb");
					struct stat st;
					if(!f||fstat(fileno(f),&st)||!st.st_size){
						if(f)fclose(f);
						while(IPCZ->keysheld)swiWaitForVBlank();
						_consolePrintf2("Cannot open file or filesize==0. Press any key.\n");
						while(!IPCZ->keysdown)swiWaitForVBlank();
					}else{
						u32 read,cur=0;
						u32 _crc32=crc32(0L, Z_NULL, 0),_adler32=adler32(0L, Z_NULL, 0);
						u16 _crc16=0xffff;
						MD5_CTX ctx;
						u8 digest[16];
						MD5Init(&ctx);
						for(;(read=fread(libprism_buf,1,65536,f))>0;_consolePrintf2("Reading %8d / %8d\r",cur,st.st_size)){
							_crc16 = swiCRC16(_crc16, libprism_buf, read);
							_crc32 = crc32(_crc32, libprism_buf, read);
							_adler32 = adler32(_adler32, libprism_buf, read);
							MD5Update(&ctx, libprism_buf, read);
							cur+=read;
						}
						_consolePrint2("                              \r");
						fclose(f);
						MD5Final(digest,&ctx);
						_consolePrintf2("CRC16:   %04x\n",_crc16);
						_consolePrintf2("CRC32:   %08x\n",_crc32);
						_consolePrintf2("Adler32: %08x\n",_adler32);
						_consolePrintf2("MD5:     ");
						{int i=0;for(;i<16;i++)_consolePrintf2("%02x",digest[i]);}
						_consolePrintChar2('\n');
						while(IPCZ->keysheld)swiWaitForVBlank();
						_consolePrintf2("\nPress any key.\n");
						while(!IPCZ->keysdown)swiWaitForVBlank();
					}
					usage();
				}break;
				case op_fixheader:{
					u8 head[0x160];
					FILE *f=fopen(file,"rb");
					struct stat st;
					if(!f||fstat(fileno(f),&st)||st.st_size<0x160){
						if(f)fclose(f);
						while(IPCZ->keysheld)swiWaitForVBlank();
						_consolePrintf2("Cannot open file or filesize too short. Press any key.\n");
						while(!IPCZ->keysdown)swiWaitForVBlank();
					}else{
						fread(head,1,0x15c,f);
						write16(head+0x15c,swiCRC16(0xffff,head+0xc0,0x9c));
						write16(head+0x15e,swiCRC16(0xffff,head,0x15e));
						fseek(f,0,SEEK_CUR); //This is garbage, but Windows Version requires this... I don't know why.
						fwrite(head+0x15c,1,4,f);
						fclose(f);
						while(IPCZ->keysheld)swiWaitForVBlank();
						_consolePrintf2("Done. Press any key.\n");
						while(!IPCZ->keysdown)swiWaitForVBlank();
					}
					usage();
				}break;
				case op_encryptsecure:{
					EncryptSecureArea(file);
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf2("Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
				}break;
				case op_decryptsecure:{
					DecryptSecureArea(file);
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf2("Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
				}break;
				case op_hb:{
					destroyfilelist();
					_consoleClear();
					_consolePrintf2("Selected %s\n",file);
					BootLibrary(file);
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf("Failed. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
					getfilelist(dir,filter);
				}break;
				case op_hb_swapcart:{
					destroyfilelist();
					_consoleClear();
					_consolePrintf2("Selected %s\n",file);
					ret_menu9_callback=swapcartbeforeexec;
					ret_menu9_Gen(file);
					ret_menu9_callback=NULL;
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf("Failed. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
					getfilelist(dir,filter);
				}break;
				case op_dsb:{
					destroyfilelist();
					_consoleClear();
					_consolePrintf2("Selected %s\n",file);
					BootDSBooter(file);
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf("Failed. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
					getfilelist(dir,filter);
				}break;
/*
				case op_dsbraw:{
					destroyfilelist();
					_consoleClear();
					_consolePrintf2("Selected %s\n",file);
					BootDSBooterRaw(file);
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf("Failed. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
					getfilelist(dir,filter);
				}break;
*/
				case op_r4:{
					destroyfilelist();
					_consoleClear();
					_consolePrintf2("Selected %s\n",file);
					BootR4Menu(file);
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf("Failed. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
					getfilelist(dir,filter);
				}break;
				case op_dldi:{
					destroyfilelist();
					_consolePrintf2("Patching %s\n",file);
					FILE *f=fopen(file,"r+b");
					struct stat st;
					if(!f||fstat(fileno(f),&st)||!st.st_size){
						if(f)fclose(f);
						while(IPCZ->keysheld)swiWaitForVBlank();
						_consolePrintf2("Cannot open file filesize==0. Press any key.\n");
						while(!IPCZ->keysdown)swiWaitForVBlank();
					}else{
						u8 *p=(u8*)malloc(st.st_size);
						if(!p){
							fclose(f);
							while(IPCZ->keysheld)swiWaitForVBlank();
							_consolePrintf2("Cannot alloc memory %dbytes. Press any key.\n",st.st_size);
							while(!IPCZ->keysdown)swiWaitForVBlank();
						}else{
							fread(p,1,st.st_size,f);
							rewind(f);
							int ret=dldi(p,st.st_size);
							if(!ret)fwrite(p,1,st.st_size,f);
							fclose(f);
							while(IPCZ->keysheld)swiWaitForVBlank();
							_consolePrintf2("Patching %s. Press any key.\n",!ret?"Done":"Failed");
							while(!IPCZ->keysdown)swiWaitForVBlank();
						}
					}
					usage();
					getfilelist(dir,filter);
				}break;
				case op_textedit:{
					destroyfilelist();
					_consoleClear();
					_consolePrintf2("Selected %s\n",file);
					runTextEdit(file);
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf("Failed. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
					getfilelist(dir,filter);
				}break;
			}
			continue;
		}
		if(key&KEY_Y){
			int ret=selectpref("System Menu",arraysize(systemmenu),systemmenu);
			usage();
			switch(ret){
				case sys_notice:{
					notice();
				}break;
				case sys_partitioninfo:{
					_consoleClear2();
					writePartitionInfo(_consolePrintf2);
					_consolePrintf2("\n");
					_consolePrintf2("If FAT32, bytesPerCluster:32768 means that 512MB ROM \"might\" work on this microSD.\n");
					_consolePrintf2("bytesPerCluster:65536 means that MoonShell2 fast mode doesn't work.\n");
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf2("\nPress any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
				}break;
				case sys_systeminfo:{
					u16 name_utf16[11],message_utf16[27];
					char name[40],message[100],stime[30],fwver[20],temper[20];
					vramcpy(name_utf16,PersonalData->name,PersonalData->nameLen);name_utf16[PersonalData->nameLen]=0;
					vramcpy(message_utf16,PersonalData->message,PersonalData->messageLen);message_utf16[PersonalData->messageLen]=0;
					_FAT_directory_ucs2tombs(name,name_utf16,40);
					_FAT_directory_ucs2tombs(message,message_utf16,100);

					char *theme[]={
						"Gray","Brown","Red","Pink",
						"Orange","Yellow","Green-ish Yellow","Green",
						"Dark Green","Blue-ish Green","Light Blue","Blue",
						"Dark Blue","Dark Purple","Purple","Purple/Red-ish",
					};
					char *lang[]={"Japanese","English","French","German","Italian","Spanish","Chinese","Unknown/Reserved"};
					char *month[]={"","Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

					_consoleClear2();
					IPCZ->cmd=RequestBatteryLevel;
					while(IPCZ->cmd)swiWaitForVBlank();
					time_t ttt=time(NULL);
					strftime(stime, 30, "%Y-%m-%d (%a) %H:%M:%S", localtime(&ttt));
					*fwver=0;
					if(IPCZ->flashmeversion==1)strcpy(fwver,"FlashMe old");
					if(IPCZ->flashmeversion>1)sprintf(fwver,"FlashMe v%d",IPCZ->flashmeversion+3);
					if(!*fwver){
						int iVer=GetFirmwareVersion();
						if(!iVer)sprintf(fwver,"Unknown (%04x)",IPCZ->fwchksum);
						else sprintf(fwver,
							iVer==FW_iQue?"iQue":
							iVer==FW_Korean?"Korean":
							"v%d",iVer);
					}
					sprintf(temper,IPCZ->temperature/0x1000>1000?"Not Available":"%.2f C",(double)IPCZ->temperature/0x1000);

					_consolePrintf2(
						"I'm:         Nintendo DS%s\n"
						"Firmware:    %s (%d bytes)\n"
						"Battery:     %s%s\n"
						"MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n"
						//"DSLite info: 0x%08x\n"
						"Time:        %s\n"
						"Temperature: %s\n"
						"\n"
						//"FW Version:  0x%02x%02x\n"
						"Theme:       %d (%s)\n"
						"Birthday:    %s/%d\n"
						"Name:        %s\n"
						"Message:     %s\n"
						"Alarm:       %d:%02d\n"
						"Calibration: %d,%d - %d,%d\n"
						"Language:    %d (%s)\n"
						"GBA Screen:  %d (%s)\n"
						"Brightness:  %d\n"
						"AutoMode:    %d\n",
						//"rtcOffset:   0x%08x\n",
						IPCZ->NDSType==NDSPhat?"":IPCZ->NDSType==NDSLite?" Lite":"i",
						fwver,IPCZ->fwsize,
						IPCZ->battery&1?"Low":"Good",IPCZ->battery&BIT15?", charging":"",
						IPCZ->MAC[0],IPCZ->MAC[1],IPCZ->MAC[2],IPCZ->MAC[3],IPCZ->MAC[4],IPCZ->MAC[5],
						//IPCZ->dslite,
						stime,
						temper,
						//PersonalData->RESERVED0[1],PersonalData->RESERVED0[0],
						PersonalData->theme,theme[PersonalData->theme],
						month[PersonalData->birthMonth],PersonalData->birthDay,
						name,message,
						PersonalData->alarmHour,PersonalData->alarmMinute,
						PersonalData->calX1px,PersonalData->calY1px,PersonalData->calX2px,PersonalData->calY2px,
						userdata&7,lang[userdata&7],
						(userdata>>3)&1,((userdata>>3)&1)?"Lower":"Upper",
						(userdata>>4)&3,
						(userdata>>6)&1
						//PersonalData->rtcOffset
					);
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf2("\nPress any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
				}break;
				case sys_m3region:{
					_consoleClear();
					_consoleClear2();
					_consolePrintf("*** Show M3 Region / R4 Jumper ***\n\n");

					u32 type=R4_ReadCardInfo()&0x3ff;
					_consolePrintf("Jumper: 0x%03X (%d%d%d-%d%d%d-%d%d%d)\n",type,
						(type>>8)&1,(type>>7)&1,(type>>6)&1,(type>>5)&1,(type>>4)&1,(type>>3)&1,(type>>2)&1,(type>>1)&1,(type>>0)&1);
					type=(u8)M3_ReadCardRegion()&0xff;
					_consolePrintf("Region: 0x%03X (%d-%d%d%d-%d%d-%d%d)\n",type,
						(type>>7)&1,(type>>6)&1,(type>>5)&1,(type>>4)&1,(type>>3)&1,(type>>2)&1,(type>>1)&1,(type>>0)&1);
					_consolePrint("\n");

					_consolePrintf2(
						"Jumper:  flag1 jumper flag2\n"
						"[R4/M3S] 1 1 1        1 0 0\n"
						"[M3Real] 0 1 1        1 0 0\n"
						"[M3]           0 x x\n"
						"[R4]           1 x x\n"
						"[JPN]          x 0 1        [CC/EC]\n"
						"[ENG]          x 1 0        [D4/F4]\n"
						"[GB]           x 1 1        [DC/FC]\n"
						"\n"
						"Region:  type cart  fw  lng\n"
						"[M3Real]    1-0 1 0-0 1\n"
						"[JPN]                   0 0 [A4]\n"
						"[ENG]                   1 1 [A7]\n"
						"[GB]                    0 1 [A5]\n"
						"[iTDS]      1-0 1 1-1 0\n"
						"[JPN]                   0 0 [B8]\n"
						"[ENG/GB]                1 0 [BA]\n"
						"[Sakura]    1-1 0 0-0 1\n"
						"[JPN]                   0 0 [C4]\n"
						"[ENG]                   1 1 [C7]\n"
						"[GB]                    1 0 [C6]\n"
						"[R4iRTS]    1-0 1 1-0 0-1 1 [B3]\n"
						"[M3iZero]   1-0 1 1-0 1\n"
						"[JPN]                   0 0 [B4]\n"
						"[ENG]                   1 1 [B7]\n"
						"[GB]                    1 0 [B6]\n"
						"[M3i0 G003] 0-1 0 1-0 0-1 1 [53] (?)\n"
					);
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf("Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
				}break;
				case sys_swapsd:{
					destroyfilelist();
					_consoleClear();
#ifdef _LIBNDS_MAJOR_
					fatUnmount("fat:/");
#else
					fatUnmount(0);
#endif
					_consolePrintf(
						"microSD unmounted. Please swap microSD.\n"
						"Please note you have to reinsert storage flashcart.\n"
						"Press A to proceed, B to shutdown.\n"
					);

					///
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
					_consolePrintf("Reinitializing... ");
					if(!fatinited&&!fatInitDefault()){
						_consolePrintf(
							"failed.\n"
							"Some flashcarts cannot initialize DLDI after ejected...\n"
							"In short swaping microSD doesn't work on this flashcart.\n"
						);
						die();
					}
					_consolePrintf("done.\n");
					///

					strcpy(dir,"/");
					getfilelist(dir,filter);
					usage();
					cursor=0,oldcursor=0,top=0;
				}break;
				case sys_return:{
					char *file="/moonshl2/resetmse/zzzz.nds";
					memcpy(file+19,DLDIDATA+ioType,4);
					destroyfilelist();
					_consoleClear();
					_consolePrintf2("Selected %s\n",file);
					BootLibrary(file);
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf("Failed. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
					getfilelist(dir,filter);
				}break;
				case sys_dsmenu:{
					destroyfilelist();
					_consoleClear();

					_consolePrintf("*** Return to NDS Firmware ***\n\n");
					if(IPCZ->NDSType==NDSi){
						IPCZ->cmd=ReturnDSiMenu;
					}else{
						switch(returnDSMenu()){
							case -1:_consolePrintf2("non GPL version cannot return to DS Menu.\n");break;
							//case 1:_consolePrintf2("NDSi cannot return to DS Menu.\n");break;
							//case 2:_consolePrintf2("Firmware isn't 256KB.\n");break;
							case 3:_consolePrintf2("Cannot alloc memory.\n");break;
							case 4:_consolePrintf2("Firmware decode error.\n");break;
						}
					}
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf2("Failed. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
					getfilelist(dir,filter);
				}break;
				case sys_shutdown:{
					IPCZ->cmd=Shutdown;
				}break;
				case sys_bios:{
					FILE *f;
					char fwname[20];
					destroyfilelist();
					_consoleClear2();
					_consolePrint2("Dumping ARM7 to /biosnds7.rom... ");
					f=fopen("/biosnds7.rom","wb");
					fwrite(key_tbl,1,0x4000,f);
					fclose(f);
					_consolePrint2("Done.\n");
					_consolePrint2("Dumping ARM9 to /biosnds9.rom... ");
					f=fopen("/biosnds9.rom","wb");
					fwrite((u8*)0xffff0000,1,0x1000,f);
					fclose(f);
					_consolePrint2("Done.\n");
					sprintf(fwname,"/FW%02X%02X%02X%02X%02X%02X.bin",
						IPCZ->MAC[0],IPCZ->MAC[1],IPCZ->MAC[2],IPCZ->MAC[3],IPCZ->MAC[4],IPCZ->MAC[5]
					);
					_consolePrintf2("Dumping Firmware to %s...\n",fwname);
					u8 *p=(u8*)malloc(IPCZ->fwsize);
					if(!p){_consolePrint2("Cannot alloc buffer.\n");goto bios_end;}
					IPCZ->firmware_addr=p;
					IPCZ->firmware_bufsize=IPCZ->fwsize;
					DC_FlushAll();
					IPCZ->cmd=GetFirmware;
					while(IPCZ->cmd)swiWaitForVBlank();
					DC_InvalidateAll();

					f=fopen(fwname,"wb");
					int i=0;
					for(;i<IPCZ->fwsize;i+=min(65536,IPCZ->fwsize-i),_consolePrintf2("Writing %7d / %7d\r",i,IPCZ->fwsize)){
						fwrite(p+i,1,min(65536,IPCZ->fwsize-i),f);
					}
					free(p);
					fclose(f);
					_consolePrint2("\nDone.\n");
bios_end:
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf2("Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
					getfilelist(dir,filter);
				}break;
				case sys_testsd:{
					FILE *f;
					u8 buf[512];
					u32 n=getSectors()/8;
					u32 i;
					double r;

					_consoleClear2();
					_consolePrint2("*** Test microSD speed ***\n\n");
					for(i=0;i<100;i++){
						sprintf(buf,"/TestSD%02d.txt",i);
						if(access(buf,0/*2*/))break;
					}
					if(i==100){
						_consolePrint2("Cannot create log file.\n");goto testsd_end;
					}
					f=fopen(buf,"w");
					_consolePrintf2("Log file: %s\n\n",buf);
					fprintf(f,"*** XenoFile Test microSD Log ***\nDLDI ID: %s\n\n",dldiid); 

					//OK test from here
					_consolePrint2("Sequential 4K: ");
					IPCZ->blanks=0;
					for(i=0;i<8;i++){
						disc_readSectors(i,1,buf);
					}
					r=IPCZ->blanks*1024/61263.0;
					_consolePrintf2("%.3fs (%.3fKB/s)\n",r,4/r);
					fprintf(f,"Sequential 4K: %.3fs (%.3fKB/s)\n",r,4/r);

					_consolePrint2("Sequential 1M: ");
					IPCZ->blanks=0;
					for(i=0;i<1024*2;i++){
						disc_readSectors(i,1,buf);
					}
					r=IPCZ->blanks*1024/61263.0;
					_consolePrintf2("%.3fs (%.3fKB/s)\n",r,1024/r);
					fprintf(f,"Sequential 1M: %.3fs (%.3fKB/s)\n",r,1024/r);

					_consolePrint2("Random 4K: ");
					IPCZ->blanks=0;
					for(i=0;i<8;i++){
						disc_readSectors(i*n,1,buf);
					}
					r=IPCZ->blanks*1024/61263.0;
					_consolePrintf2("%.3fs (%.3fKB/s)\n",r,4/r);
					fprintf(f,"Random 4K: %.3fs (%.3fKB/s)\n",r,4/r);

					_consolePrint2("Random 1M: ");
					IPCZ->blanks=0;
					for(i=0;i<1024*2;i++){
						disc_readSectors((i&7)*n+i/8,1,buf);
					}
					r=IPCZ->blanks*1024/61263.0;
					_consolePrintf2("%.3fs (%.3fKB/s)\n",r,1024/r);
					fprintf(f,"Random 1M: %.3fs (%.3fKB/s)\n",r,1024/r);
					fclose(f);
testsd_end:
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf2("Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
				}break;
				case sys_slot2nds:{
					slot2nds();
				}break;
				case sys_slot2gba:{
					videoSetMode(MODE_5_2D),videoSetModeSub(MODE_5_2D); //just kill it
					int screen=(userdata>>3)&1;

					//transfer image to BG_BMP_BASE(0) and BG_BMP_BASE(8). Firstly reset.
					vramset((u16*)0x06000000,0x8000,256*192);
					vramset((u16*)0x06020000,0x8000,256*192);

					FILE *f=fopen("/xenogba.b15","rb");
					if(f){
						u16 z[256];
						fread(z,2,3,f);
						u16 width=read16(z),height=read16(z+1);
						u32 size=filelength(fileno(f));

						if(width>256||height>192||size!=width*height*2+6){fclose(f);goto gbaborder_bmp;}

						{int l=0;for(;l<height;l++){
							fread(z,2,width,f);
							vramcpy((u16*)0x06000000+256*l,z,width);
							vramcpy((u16*)0x06020000+256*l,z,width);
						}}
						fclose(f);
					}else{
gbaborder_bmp:
						bmpdecode_gbaborder("/gbaframe.bmp");
					}
					slot2gba(screen);
				}break;
			}
			continue;
		}
		if(key&KEY_SELECT){
			int ret=selectpref("Preferences",arraysize(prefs),prefs);
			usage();
			switch(ret){
				case -1:break;
				case 0:BootLibrary=ret_menu9_Gen;_consolePrintf2("Use Rudolph Loader.\n");break;
				case 1:BootLibrary=BootNDSROM;_consolePrintf2("Use MoonShell Simply Loader.\n");break;
				case 2:BootLibrary=runNdsFile;_consolePrintf2("Use bootlib loader.\n");break;
				case 3:BootLibrary=ret_menu9_GenM;_consolePrintf2("Use Rudolph/Moonlight hybrid loader.\n");break;
				case 4:BootLibrary=runRPGLink;_consolePrintf2("Use rpglink.\n");break;
				case 5:DLDIToBoot=DLDIDATA;_consolePrintf2("Use my DLDI.\n");break;
				case 6:DLDIToBoot=NULL;_consolePrintf2("Disabled DLDI patching.\n");break;
			}
			continue;
		}
		if(key&KEY_X){
			hidehidden^=1;
			usage();
			getfilelist(dir,filter);
			cursor=0,oldcursor=0,top=0;
			//_consolePrintf2("%s hidden/system file.\n",hidehidden?"Hide":"Show");
			continue;
		}
	}

	die();
}
