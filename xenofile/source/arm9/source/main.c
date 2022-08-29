#include "xenofile.h"
#include "zlib/zlib.h" // CRC32
#include "libcarddump.h"
#include "libnsbmp.h"
const u16 bgcolor=RGB15(0,0,6);

#include "keyboard_b15z.h"
#include "keyboard_shift_b15z.h"

extern int hidehidden;

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
	NotifyARM7(PlaySound);
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
	NotifyARM7(StopSound);

	TIMER0_DATA	= 0;
	TIMER0_CR	= 0;
	TIMER1_DATA	= 0;
	TIMER1_CR	= 0;

	//sound_status = 0;
}

void swapcartbeforeexec(u8* buf){
	disc_unmount();
	_consolePrint(
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
		_consolePrintOnce("Cannot be recognized. Insert again.");
		for(swiWaitForVBlank();;swiWaitForVBlank()){
			if(!(IPCZ->keysdown))continue;
			if(IPCZ->keysdown&KEY_A)break;
			if(IPCZ->keysdown&KEY_B)IPCZ->cmd=Shutdown;
		}
		romID = Card_Retry();
	}
	_consolePrintOnceEnd();
	Card_Close();
	_consolePrint("done.\n");
}

char ext[EXTMAX][32]={
	".nds",
	".c",".cpp",".cc",".h",".pl",".py",".rb",".php",".txt",".ini",".log",".cfg",".conf",".htm",".html",".xml",".lst",".lua",".gm",
	".sav",".bak",".duc",".dsv",".gds",".cfs",".ngs",
	".dldi",".b15",".ani",".bmp",".ico",".u8m",
};

#define TEXT_START 1
#define SAV_START  (TEXT_START+19)
#define DLDI_START (SAV_START+7)
#define B15_START  (DLDI_START+1)
#define ANI_START  (B15_START+1)
#define BMP_START  (ANI_START+1)
#define U8M_START  (BMP_START+2)
#define MSP_START  (U8M_START+1)

char *__dummy[]={ //never know the reason, but without this libelm edition freezes
	"foo","bar",
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
	"Trim NDS",
	"Open Patch",
	"Encrypt secure area",
	"Decrypt secure area",
	"Decrypt R4 kernel (only on R4)",
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
	"Show DS card info",
	"Show M3 Region / R4 Jumper",
	"Swap microSD",
	"Change DLDI",
	"Dump Bios/Firmware",
	"Test microSD speed",
	"Fix FSInfo sector",
	"Return to firmware(moonshl2)",
	"Return to NDS firmware",
	"Return to BootStub",
	"Shutdown",
	"Boot to Slot2 NDS",
	"Boot to Slot2 GBA",
};

char *preferences[]={
	"[Loader] Rudolph Loader",
	"[Loader] MoonShell Simply Loader",
	"[Loader] MoonShell2 Loader",
	"[Loader] bootlib",
	"[Loader] bootlib+stub",
	"[Loader] Rudolph/Moonlight hybrid Loader",
	"[Loader] rpglink",
	"[DLDI]   Use mine",
	"[DLDI]   Disable patching",
	"[DLDI]   Patch null (to kill prepatched)",
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
	op_trim,
	op_openpatch,
	op_encryptsecure,
	op_decryptsecure,
	op_decryptr4,
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
	sys_dscardinfo,
	sys_m3region,
	sys_swapsd,
	sys_changedldi,
	sys_bios,
	sys_testsd,
	sys_fixfsinfo,
	sys_return,
	sys_dsmenu,
	sys_bootstub,
	sys_shutdown,
	sys_slot2nds,
	sys_slot2gba,
};

enum{
	pref_rudolphloader=0,
	pref_mshlsimplyloader,
	pref_mshl2loader,
	pref_bootlibloader,
	pref_bootlibstubloader,
	pref_rudolphmshlloader,
	pref_rpglinkloader,
	pref_mydldi,
	pref_disabledldi,
	pref_nulldldi,
};

type_constpchar BootLibrary;

int msp_end,ext_end;
int filter;
void usage(){
	_consoleClear2();
#if 0
	_consolePrint2(
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
		if(i)_consolePrint2("/");
		_consolePrint2(ext[i]);
	}

	_consolePrint2(
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
	_consolePrint2(
		"XenoFile - simple and sophisticated filer\n"
		"*** XenoShell Legacy ***\n"
		"(C) Ciel de Rive\n"
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
		"rpglink (C) Acekard.com and X under MIT license\n"
		"\n"
		//"[library]\n"
#if defined(LIBFAT)
		"libfat (C) Chishm under BSD License\n"
#elif defined(LIBELM)
		"libelm (C) Chan / ywg under BSD License\n"
#endif
		"dldipatch aka dlditool public domain under CC0\n"
		"libmshlsplash under CC0\n"
		"libcarddump by Rudolph\n"
		"Secure Encryption by DarkFader\n"
		"Keyboard Library by HeadSoft\n"
		"MoonShell Plugin routine by Moonlight\n"
		//"minIni (C) CompuPhase under Apache License 2.0\n"
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
		if(i)_consolePrint2("/");
		_consolePrint2(ext[i]);
	}
*/
	//_consolePrint2("\n");
	while(IPCZ->keysheld)swiWaitForVBlank();
	//_consolePrintf2("\n");
	_consolePrint2("Press any key.\n");
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
	_consoleClear();
	_consolePrint("Waiting for initialization...\n");
	PrintfToDie=_consolePrintf2;

	_consolePrint2(
		"XenoFile - simple and sophisticated filer\n"
		"Powered by libprism "ROMVERSION"\n"
		ROMDATE"\n"
		ROMENV"\n\n"
	);
	{
		unsigned char *dldiFileData=DLDIDATA;
		memcpy(dldiid,(char*)dldiFileData+ioType,4);
		dldiid[4]=0;
		_consolePrintf2("DLDI ID: %s\n",dldiid);
		_consolePrintf2("DLDI Name: %s\n\n",(char*)dldiFileData+friendlyName);
	}

	_consolePrint2("Initializing FAT... ");
	if(!disc_mount()){_consolePrint2("Failed.\n");die();}
	_consolePrint2("Done.\n");

#ifdef LIBFAT
	BootLibrary=bootlibAvailable()?runNdsFile:ret_menu9_Gen;
#else
	BootLibrary=(bootlibAvailable()&&bootstubAvailable())?runNdsFileViaStub:ret_menu9_Gen;
#endif

	getExtlinkWrapperLoaderName(loader);

	_consolePrint2("Iterating MoonShell plugin...\n");
	msp_end=iterateMSP(MSP_START);
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

	extmem_Init();

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
		if(key&KEY_SELECT){
			int ret=selectpref("Preferences",arraysize(preferences),preferences);
			usage();
			switch(ret){
				case -1:break;
				case pref_rudolphloader:BootLibrary=ret_menu9_Gen;_consolePrint2("Use Rudolph Loader.\n");break;
				case pref_mshlsimplyloader:BootLibrary=BootNDSROM;_consolePrint2("Use MoonShell Simply Loader.\n");break;
				case pref_mshl2loader:BootLibrary=BootNDSROMex;_consolePrint2("Use MoonShell2 Loader.\n");break;
				case pref_bootlibloader:BootLibrary=runNdsFile;_consolePrint2("Use bootlib Loader.\n");break;
				case pref_bootlibstubloader:BootLibrary=runNdsFileViaStub;_consolePrint2("Use bootlib+stub Loader.\n");break;
				case pref_rudolphmshlloader:BootLibrary=ret_menu9_GenM;_consolePrint2("Use Rudolph/Moonlight hybrid Loader.\n");break;
				case pref_rpglinkloader:BootLibrary=runRPGLink;_consolePrint2("Use rpglink.\n");break;
				case pref_mydldi:DLDIToBoot=DLDIDATA;_consolePrint2("Use my DLDI.\n");break;
				case pref_disabledldi:DLDIToBoot=NULL;_consolePrint2("Disabled DLDI patching.\n");break;
				case pref_nulldldi:DLDIToBoot=DLDINull;_consolePrint2("Patch with null DLDI.\nPlease note bootlib isn't compatible.\n");break; // It will go to DSi SD in next boot.\n
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
				_consolePrint("Failed. Press any key.\n");
				while(!IPCZ->keysdown)swiWaitForVBlank();
				usage();
				getfilelist(dir,filter);
			break;}

			//handler_Text
			{int i;for(i=TEXT_START;i<SAV_START;i++)
			if(strlen(p->name)>=strlen(ext[i])&&!strcasecmp(p->name+strlen(p->name)-strlen(ext[i]),ext[i])){
				strcpy(file,dir);
				strcat(file,p->name);
				destroyfilelist();
				_consolePrintf2("Selected %s\n",file);
				_consoleClear();
				runTextEdit(file);
				while(IPCZ->keysheld)swiWaitForVBlank();
				_consolePrint("Failed. Press any key.\n");
				while(!IPCZ->keysdown)swiWaitForVBlank();
				usage();
				getfilelist(dir,filter);
			break;}}

			//handler_SavConvert
			{int i;for(i=SAV_START;i<DLDI_START;i++)
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
			{int i;for(i=DLDI_START;i<B15_START;i++)
			if(strlen(p->name)>=strlen(ext[i])&&!strcasecmp(p->name+strlen(p->name)-strlen(ext[i]),ext[i])){
				struct stat st;
				strcpy(file,dir);
				strcat(file,p->name);
				if(stat(file,&st)||st.st_size<128||st.st_size>32768){
					while(IPCZ->keysheld);
					_consolePrint2("DLDI open failed or invalid size. Press any key.\n");
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
			{int i;for(i=B15_START;i<ANI_START;i++)
			if(strlen(ext[i])&&strlen(p->name)>=strlen(ext[i])&&!strcasecmp(p->name+strlen(p->name)-strlen(ext[i]),ext[i])){
				strcpy(file,dir);
				strcat(file,p->name);
				_consolePrintf2("Displaying %s\n",file);
				u16 z[256];
				FILE *f=fopen(file,"rb");
				if(!f){
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrint2("cannot open. Press any key.\n");
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
				_consolePrint2("Press any key to end.\n");
				while(!IPCZ->keysdown)swiWaitForVBlank();
				DisableB15Main();
				usage();
			break;}}

			// handler_ANI
			{int i;for(i=ANI_START;i<BMP_START;i++)
			if(strlen(ext[i])&&strlen(p->name)>=strlen(ext[i])&&!strcasecmp(p->name+strlen(p->name)-strlen(ext[i]),ext[i])){
				strcpy(file,dir);
				strcat(file,p->name);
				_consolePrintf2("Displaying %s\n",file);
				_consolePrint2("A to slideshow, B to end.\n");
				splash* sp=opensplash(file);
				if(!sp){
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrint2("invalid file or cannot initialize libmshlsplash. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();break;
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
				_consoleStartProgress2();
				_consolePrintProgress2("Showing",cursor,sp->frame-1);
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
						_consolePrintProgress2("Showing",cursor,sp->frame-1);
						oldcursor=cursor;
					}
					if(key&0xf0)continue;
					if(key&KEY_B)break;
					if(key&KEY_A){
						for(cursor=0;cursor<sp->frame;cursor++){
							decompressimage(sp,cursor);
							swiWaitForVBlank();
							vramcpy(b15ptrMain,sp->decompbuf,getimagesize(sp,cursor)/2);
							_consolePrintProgress2("Showing",cursor,sp->frame-1);
						}
						oldcursor=cursor=sp->frame-1;
					}
				}
				DisableB15Main();
				_consoleEndProgress2();
				closesplash(sp);
				usage();
			break;}}

			// handler_BMP
			{int i;for(i=BMP_START;i<U8M_START;i++)
			if(strlen(ext[i])&&strlen(p->name)>=strlen(ext[i])&&!strcasecmp(p->name+strlen(p->name)-strlen(ext[i]),ext[i])){
				strcpy(file,dir);
				strcat(file,p->name);
				_consolePrintf2("Displaying %s\n",file);
				vramset(b15ptrMain,0xffff,256*192);
				if(i==BMP_START?bmpdecode_show(file):icodecode_show(file)){_consolePrint2("Decode failed\n");goto bmp_fail;}
/*
				u16 z[256];
				FILE *f=fopen(file,"rb");
				if(!f){
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrint2("cannot open. Press any key.\n");
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
				_consolePrint2("Press any key to end.\n");
				while(!IPCZ->keysdown)swiWaitForVBlank();
				DisableB15Main();
				usage();
			break;}}

			// handler_U8M
			{int i;for(i=U8M_START;i<MSP_START;i++)
			if(strlen(ext[i])&&strlen(p->name)>=strlen(ext[i])&&!strcasecmp(p->name+strlen(p->name)-strlen(ext[i]),ext[i])){
				struct stat st;
				FILE *f;
				strcpy(file,dir);
				strcat(file,p->name);
				_consolePrintf2("Playing %s\n",file);
				f=fopen(file,"rb");
				fstat(fileno(f),&st);
				u8 *p=malloc(st.st_size);
				if(!p){_consolePrint2("Cannot alloc\n");goto u8m_fail;}
				fread(p,1,st.st_size,f);
				fclose(f);
				int i=0;for(;i<st.st_size;i++)p[i]-=0x80;
				stopSound();
				startSound(22050,p,p,st.st_size,8);

u8m_fail:
				while(IPCZ->keysheld)swiWaitForVBlank();
				_consolePrint2("Press any key to end.\n");
				while(!IPCZ->keysdown)swiWaitForVBlank();
				stopSound();
				free(p);
				usage();
			break;}}

			// handler_MSP
			{int i;for(i=MSP_START;i<msp_end;i++)
			if(strlen(ext[i])&&strlen(p->name)>=strlen(ext[i])&&!strcasecmp(p->name+strlen(p->name)-strlen(ext[i]),ext[i])){
				strcpy(file,dir);
				strcat(file,p->name);
				destroyfilelist();
				//_consoleClear();
				_consoleClear2();

				FILE *f=fopen(file,"rb");
				if(!f){_consolePrint2("cannot open file.\n");goto msp_fail;}
				TPluginBody* PB=NULL;
				PB=getPluginByIndex(i-MSP_START);
				if(!PB){_consolePrint2("cannot open msp.\n");goto msp_fail;}

				if(PB->PluginHeader.PluginType==EPT_Image){
					_consolePrintf2("Displaying %s\n",file);
					vramset(b15ptrMain,0xffff,256*192);
					_consolePrint2("Opening...\n");
					if(!PB->pIL->Start((int)f)){_consolePrint2("cannot open image.\n");goto msp_fail;}
				
					PB->pIL->RefreshScreen(0,0,b15ptrMain,256,256,192,false);
					EnableB15Main();
				}else if(PB->PluginHeader.PluginType==EPT_Sound){
					_consolePrintf2("Playing %s\n",file);
					stopSound();
					_consolePrint2("Opening...\n");
					if(!PB->pSL->Start((int)f)){_consolePrint2("cannot open music.\n");goto msp_fail;}
					u16 sampleperframe=PB->pSL->GetSamplePerFrame();
					u16 channel=PB->pSL->GetChannelCount();
					s16 *pL=(s16*)malloc(2*sampleperframe*16);
					s16 *pR=NULL;
					if(channel>1)pR=(s16*)malloc(2*sampleperframe*16);
					if(!pL||(channel>1&&!pR)){
						if(pL)free(pL);
						if(pR)free(pR);
						_consolePrint2("cannot alloc buffer.\n");goto msp_fail;
					}
					int f=0;
					for(;f<8;f++){
						PB->pSL->Update(pL+sampleperframe*(2*f),channel>1?pR+sampleperframe*(2*f):NULL);
						PB->pSL->Update(pL+sampleperframe*(2*f+1),channel>1?pR+sampleperframe*(2*f+1):NULL);
					}
					startSound(PB->pSL->GetSampleRate(),pL,channel>1?pR:pL,2*sampleperframe*16,16);
					IPCZ->blanks=0;

					//while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrint2("\nPress any key to end.\n");
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

							_consolePrintfOnce2("Playing Time %02d:%02d",IPCZ->blanks/3600,IPCZ->blanks/60%60);

							if(ret<sampleperframe*2){
								while(TIMER1_DATA-oldtime<ret);
								goto msp_sound_finalize;
							}
						}
					}
msp_sound_finalize:
					_consolePrintOnceEnd2();
					stopSound();
					free(pL);if(pR)free(pR);
					goto msp_sound_end;
				}
msp_fail:
				while(IPCZ->keysheld)swiWaitForVBlank();
				_consolePrint2("\nPress any key to end.\n");
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
				_consolePrint("Failed. Press any key.\n");
				while(!IPCZ->keysdown)swiWaitForVBlank();
				usage();
				getfilelist(dir,filter);
			}}

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
					_consolePrint2("You cannot do that operation for folder.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					continue;
				}
				if(!strcmp(p->name,"../")&&(ret==op_cut||ret==op_delete||ret==op_rename||ret==op_timestamp||ret==op_touch||ret==op_attr)){
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrint2("You cannot do that operation for \"..\".\n");
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
						_consolePrint2("A to overwrite, other to abort.\n");
						int key;
						for(;!(key=IPCZ->keysdown);)swiWaitForVBlank();
						if(!(key&KEY_A)){_consolePrint2("Aborted.\n");break;}
					}
					if(!clipboardiscopy)unlink(file);
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
						if(!(key&KEY_A)){_consolePrint2("Aborted.\n");break;}
						for(swiWaitForVBlank();!(key=IPCZ->keysdown);)swiWaitForVBlank();
						if(!(key&KEY_A)){_consolePrint2("Aborted.\n");break;}
						strcat(file,"/");
						rm_rf(file);
						file[strlen(file)-1]=0;
					}else{
						_consolePrintf2("Press A to confirm to delete %s, other to cancel.\n",file);
						int key;
						for(;!(key=IPCZ->keysdown);)swiWaitForVBlank();
						if(!(key&KEY_A)){_consolePrint2("Aborted.\n");break;}
					}
					unlink(file);
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrint2("Deleted. Press any key.\n");
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
					_consolePrint2("*** Rename ***\nA or Enter to proceed, B to cancel.\n\n");
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
					_consolePrint2("\nDone. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					getfilelist(dir,filter);
					usage();
					cursor=0,oldcursor=0,top=0;
					continue;
rename_cancel:
					DisableB15Sub();
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrint2("\nCancelled. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
					continue;
				}break;
				case op_newfile:{
					strcpy(getfilename(file),"");
					_consoleClear2();
newfile_retry:
					_consolePrint2("*** Create new file ***\nA or Enter to proceed, B to cancel.\n\n");
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
					_consolePrint2("\nDone. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					getfilelist(dir,filter);
					usage();
					cursor=0,oldcursor=0,top=0;
					continue;
newfile_cancel:
					DisableB15Sub();
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrint2("\nCancelled. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
					continue;
				}break;
				case op_mkdir:{
					strcpy(getfilename(file),"");
					_consoleClear2();
mkdir_retry:
					_consolePrint2("*** Make directory ***\nA or Enter to proceed, B to cancel.\n\n");
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
					_consolePrint2("\nDone. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					getfilelist(dir,filter);
					usage();
					cursor=0,oldcursor=0,top=0;
					continue;
mkdir_cancel:
					DisableB15Sub();
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrint2("\nCancelled. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
					continue;
				}break;
				case op_timestamp:{
#ifndef _LIBNDS_MAJOR_
					_consolePrint2("Changing timestamp isn't supported on legacy version.\n");goto timestamp_cancel;

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
					_consolePrint2("*** Change timestamp ***\nA or Enter to proceed, B to cancel.\nFormat is YYYYMMDD hhmmss.\n\n");
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
						_consolePrint2("Invalid format. Input again.\n\n");
						goto timestamp_retry;
					}
					libprism_utime(file,1,mktime(&t));

					DisableB15Sub();
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrint2("\nDone. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
					continue;
timestamp_cancel:
					DisableB15Sub();
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrint2("\nCancelled. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
					continue;
				}break;
				case op_touch:{
#ifndef _LIBNDS_MAJOR_
					_consolePrint2("Changing timestamp isn't supported on legacy version.\n");goto touch_cancel;

#endif
					_consoleClear2();
					_consolePrint2("*** Touch file ***\n\n");
					libprism_touch(file);

					DisableB15Sub();
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrint2("\nDone. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
					continue;
#ifndef _LIBNDS_MAJOR_
touch_cancel:
					DisableB15Sub();
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrint2("\nCancelled. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
					continue;
#endif
				}break;
				case op_attr:{
#ifndef _LIBNDS_MAJOR_
					_consolePrint2("Changing attribute isn't supported on legacy version.\n");goto attr_cancel;

#endif
					char attrib[6]="-----";
					int iattrib;
					{
						struct stat st;
						stat(file,&st);
						iattrib=st.st_spare1;
					}
					if(iattrib&ATTRIB_RO)attrib[0]='R';
					if(iattrib&ATTRIB_HID)attrib[1]='H';
					if(iattrib&ATTRIB_SYS)attrib[2]='S';
					if(iattrib&ATTRIB_DIR)attrib[3]='D';
					if(iattrib&ATTRIB_ARCH)attrib[4]='A';

attr_again:
					_consoleClear2();
					_consolePrint2("*** Change attribute ***\nA for Yes, B for No.\n\n");
					_consolePrintf2("Current attr: %s\n",attrib);
					u8 newattr=0;

					_consolePrint2("Set readonly? ");
					for(swiWaitForVBlank();;swiWaitForVBlank()){
						if(IPCZ->keysdown&KEY_A){_consolePrint2("Yes.\n");newattr|=ATTRIB_RO;break;}
						if(IPCZ->keysdown&KEY_B){_consolePrint2("No.\n");break;}
					}
					_consolePrint2("Set hidden? ");
					for(swiWaitForVBlank();;swiWaitForVBlank()){
						if(IPCZ->keysdown&KEY_A){_consolePrint2("Yes.\n");newattr|=ATTRIB_HID;break;}
						if(IPCZ->keysdown&KEY_B){_consolePrint2("No.\n");break;}
					}
					_consolePrint2("Set system? ");
					for(swiWaitForVBlank();;swiWaitForVBlank()){
						if(IPCZ->keysdown&KEY_A){_consolePrint2("Yes.\n");newattr|=ATTRIB_SYS;break;}
						if(IPCZ->keysdown&KEY_B){_consolePrint2("No.\n");break;}
					}

					_consolePrint2("\n"
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
					_consolePrint2("\nDone. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
					continue;
attr_cancel:
					DisableB15Sub();
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrint2("\nCancelled. Press any key.\n");
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
					ucs2tombs(lfn,LFN);
					int iattrib=st.st_spare1;//getAttributes(file);
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

					_consolePrint2("\n");
					_consolePrint2("Fragment:0 means the file can be accessed sequentially (WoodR4DLDI save OK on clones).\n");
					_consolePrint2("Last access time isn't recorded on FAT.\n");
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrint2("\nPress any key.\n");
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
						_consolePrint2("Cannot open file or filesize==0. Press any key.\n");
						while(!IPCZ->keysdown)swiWaitForVBlank();
					}else{
						u32 read,cur=0;
						u32 _crc32=crc32(0L, Z_NULL, 0),_adler32=adler32(0L, Z_NULL, 0);
						u16 _crc16=0xffff;
						MD5_CTX ctx;
						u8 digest[16];
						MD5Init(&ctx);
						_consoleStartProgress2();
						for(;(read=fread(libprism_buf,1,BUFLEN,f))>0;_consolePrintProgress2("Reading",cur,st.st_size)){
							_crc16 = swiCRC16(_crc16, libprism_buf, read);
							_crc32 = crc32(_crc32, libprism_buf, read);
							_adler32 = adler32(_adler32, libprism_buf, read);
							MD5Update(&ctx, libprism_buf, read);
							cur+=read;
						}
						_consoleEndProgress2();
						fclose(f);
						MD5Final(digest,&ctx);
						_consolePrintf2("CRC16:   %04x\n",_crc16);
						_consolePrintf2("CRC32:   %08x\n",_crc32);
						_consolePrintf2("Adler32: %08x\n",_adler32);
						_consolePrint2("MD5:     ");
						{int i=0;for(;i<16;i++)_consolePrintf2("%02x",digest[i]);}
						_consolePrint2("\n");
						while(IPCZ->keysheld)swiWaitForVBlank();
						_consolePrint2("\nPress any key.\n");
						while(!IPCZ->keysdown)swiWaitForVBlank();
					}
					usage();
				}break;
				case op_fixheader:{
					u8 head[0x160];
					FILE *f=fopen(file,"r+b");
					struct stat st;
					if(!f||fstat(fileno(f),&st)||st.st_size<0x160){
						if(f)fclose(f);
						while(IPCZ->keysheld)swiWaitForVBlank();
						_consolePrint2("Cannot open file or filesize too short. Press any key.\n");
						while(!IPCZ->keysdown)swiWaitForVBlank();
					}else{
						fread(head,1,0x15c,f);
						write16(head+0x15c,swiCRC16(0xffff,head+0xc0,0x9c));
						write16(head+0x15e,swiCRC16(0xffff,head,0x15e));
						fseek(f,0,SEEK_CUR); //This is garbage, but Windows Version requires this... I don't know why.
						fwrite(head+0x15c,1,4,f);
						fclose(f);
						while(IPCZ->keysheld)swiWaitForVBlank();
						_consolePrint2("Done. Press any key.\n");
						while(!IPCZ->keysdown)swiWaitForVBlank();
					}
					usage();
				}break;
				case op_trim:{
					u8 buf[0x240];
#ifdef _LIBNDS_MAJOR_
					FILE *f=fopen(file,"r+b");
					struct stat st;
					_consoleClear2();
					_consolePrintf2("Trimming %s\n\n",file);
					if(!f||fstat(fileno(f),&st)||st.st_size<0x1000){
						if(f)fclose(f);
						while(IPCZ->keysheld)swiWaitForVBlank();
						_consolePrint2("Cannot open file or filesize too short. Press any key.\n");
						while(!IPCZ->keysdown)swiWaitForVBlank();
					}else{
						fread(buf,1,0x240,f);
		//xenondstrim
		u32 array[16];
		_consolePrintf2("arm9 end:        %08x\n",array[0]=read32(buf+0x20)+read32(buf+0x2c));
		_consolePrintf2("arm7 end:        %08x\n",array[1]=read32(buf+0x30)+read32(buf+0x3c));
		_consolePrintf2("FNT end:         %08x\n",array[2]=read32(buf+0x40)+read32(buf+0x44));
		_consolePrintf2("FAT end:         %08x\n",array[3]=read32(buf+0x48)+read32(buf+0x4c));
		_consolePrintf2("arm9overlay end: %08x\n",array[4]=read32(buf+0x50)+read32(buf+0x54));
		_consolePrintf2("arm7overlay end: %08x\n",array[5]=read32(buf+0x58)+read32(buf+0x5c));
		_consolePrintf2("icon end:        %08x\n",array[6]=read32(buf+0x68)+2112);
		_consolePrintf2("NTR size:        %08x\n",array[7]=read32(buf+0x80));
		_consolePrintf2("arm9i end:       %08x\n",array[8]=read32(buf+0x1c0)+read32(buf+0x1cc));
		_consolePrintf2("arm7i end:       %08x\n",array[9]=read32(buf+0x1d0)+read32(buf+0x1dc));
		_consolePrintf2("Digest NTR end:  %08x\n",array[10]=read32(buf+0x1e0)+read32(buf+0x1e4));
		_consolePrintf2("Digest TWL end:  %08x\n",array[11]=read32(buf+0x1e8)+read32(buf+0x1ec));
		_consolePrintf2("Digest SecH end: %08x\n",array[12]=read32(buf+0x1f0)+read32(buf+0x1f4));
		_consolePrintf2("Digest BloH end: %08x\n",array[13]=read32(buf+0x1f8)+read32(buf+0x1fc));
		_consolePrintf2("Modcrypt1 end:   %08x\n",array[14]=read32(buf+0x220)+read32(buf+0x224));
		_consolePrintf2("Modcrypt2 end:   %08x\n",array[15]=read32(buf+0x228)+read32(buf+0x22c));
		u32 s=array[0];
		int j=1;
		for(;j<16;j++)if(s<array[j])s=array[j];
		if(st.st_size>s){
			_consolePrintf2("Trimmed %08x -> %08x\n",st.st_size,s);
			ftruncate(fileno(f),s);
		}else{
			_consolePrintf2("%08x -> %08x, cannot trim\n",st.st_size,s);
		}
						fclose(f);
#else
					{
						_consolePrint2("Trimming feature isn't available in legacy edition.\n");
#endif
						while(IPCZ->keysheld)swiWaitForVBlank();
						_consolePrint2("Press any key.\n");
						while(!IPCZ->keysdown)swiWaitForVBlank();
					}
					usage();
				}break;
				case op_openpatch:{
					_consoleClear2();
					_consolePrint2("*** OpenPatch ***\n\n");
					int ret=openpatch_single(file);
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrintf2("%s. Press any key.\n",ret?"Failed":"Success");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
				}break;
				case op_encryptsecure:{
					//if(GetRunningMode()){_consolePrint2("not supported in DSi mode.\n");continue;}
					EncryptSecureArea(file);
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrint2("Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
				}break;
				case op_decryptsecure:{
					//if(GetRunningMode()){_consolePrint2("not supported in DSi mode.\n");continue;}
					DecryptSecureArea(file);
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrint2("Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
				}break;
				case op_decryptr4:{
					//destroyfilelist();
					//_consoleClear();
					struct stat st;
					if(stat(file,&st)){_consolePrintf2("Can not stat R4Menu %s.\n",file);goto decryptr4_fail;}
					u32 addr=(u32)getFATEntryAddress(file);
					if(!addr){_consolePrintf2("Error occurred in getting sector of %s.\n",file);goto decryptr4_fail;}
					unsigned int size=align512(st.st_size),pos=0;
					strcpy((char*)libprism_buf,file);
					strcat((char*)libprism_buf,".nds");
					FILE *f=fopen((char*)libprism_buf,"wb");
					if(!f){_consolePrintf2("Cannot write output %s.\n",(char*)libprism_buf);goto decryptr4_fail;}
					R4_ReadCardInfo();
					R4_SendMap(addr&0xfffffffe);
					//R4_00();
					R4_ReadCardInfo();
					_consoleStartProgress2();
					for(pos=0;pos<size;pos+=512){
						R4_ReadMenu(pos,libprism_buf,128);
						fwrite(libprism_buf,1,512,f);
						_consolePrintProgress2("Decrypting",pos+512,size);
					}
					_consoleEndProgress2();
					fclose(f);
					_consolePrint2("Decrypted.\n");
decryptr4_fail:
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrint2("Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
					getfilelist(dir,filter);
				}break;
				case op_hb:{
					destroyfilelist();
					_consoleClear();
					_consolePrintf2("Selected %s\n",file);
					BootLibrary(file);
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrint("Failed. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
					getfilelist(dir,filter);
				}break;
				case op_hb_swapcart:{
					if(GetRunningMode()){_consolePrint2("not supported in DSi mode.\n");continue;}
					destroyfilelist();
					_consoleClear();
					_consolePrintf2("Selected %s\n",file);
					ret_menu9_callback=swapcartbeforeexec;
					ret_menu9_Gen(file);
					ret_menu9_callback=NULL;
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrint("Failed. Press any key.\n");
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
					_consolePrint("Failed. Press any key.\n");
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
					_consolePrint("Failed. Press any key.\n");
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
					_consolePrint("Failed. Press any key.\n");
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
						_consolePrint2("Cannot open file filesize==0. Press any key.\n");
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
					_consolePrint("Failed. Press any key.\n");
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
					_consolePrint2("\n");
					_consolePrint2("If FAT32, bytesPerCluster:32768 means that 512MB ROM \"might\" work on this microSD.\n");
					_consolePrint2("bytesPerCluster:65536 means that MoonShell2 fast mode doesn't work.\n");
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrint2("\nPress any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
				}break;
				case sys_systeminfo:{
					u16 name_utf16[11],message_utf16[27];
					char name[40],message[100],stime[30],fwver[20],temper[20];
					vramcpy(name_utf16,PersonalData->name,PersonalData->nameLen);name_utf16[PersonalData->nameLen]=0;
					vramcpy(message_utf16,PersonalData->message,PersonalData->messageLen);message_utf16[PersonalData->messageLen]=0;
					ucs2tombs(name,name_utf16);
					ucs2tombs(message,message_utf16);

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
					if(!IPCZ->fwchksum)strcpy(fwver,"DS(Lite) Preferences");
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
						"DLDI ID:   %s\n"
						"DLDI Name: %s\n\n",dldiid,DLDIDATA+friendlyName
					); 
					_consolePrintf2(
						"I'm:         Nintendo %s\n"
						"Firmware:    %s (%d bytes)\n"
						"ExtMem:      %s (%d KB)\n"
						"Battery:     %d%s\n"
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
						IPCZ->NDSType==NDSPhat?"DS (Phat)":IPCZ->NDSType==NDSLite?"DS Lite":
						IPCZ->NDSType==NDSi?(GetRunningMode()==1?"DSi (DSi mode)":"DSi (DS mode)"):
						IPCZ->NDSType==ThreeDS?(GetRunningMode()==2?"3DS (3DS mode)":GetRunningMode()==1?"3DS (DSi mode)":"3DS (DS mode)"):
						"New Device",
						fwver,IPCZ->fwsize,
						ram_type_string(),ram_size()>>10,
						IPCZ->battery&0xf,IPCZ->battery&BIT7?", charging":"",
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
					_consolePrint2("\nPress any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
				}break;
				case sys_dscardinfo:{ //Todo. Due to KEY2 limitation, I can read only first 0x200bytes.
char *region[]={
	"ASI:DSVision",
	"Unknown", //B
	"CHN:China",
	"NOE/GER:Germany", //D
	"USA", //E
	"FRA:France",
	"Unknown" //G
	"HOL:Netherlands",
	"ITA:Italy",
	"JPN:Japan",
	"KOR:Korea",
	"USA", //L
	"SWE:Sweden", //M
	"NOR:Norway",
	"INT:International", //O
	"EUR:Europe", //P
	"DEN/DAN:Denmark", //Q
	"RUS:Russia",
	"SPA:Spain",
	"AUS:Australia", //U
	"EUU:EuropeUnion", //V
	"EUU:EuropeUnion", //W
	"EUU:EuropeUnion", //X
	"EUU:EuropeUnion", //Y
	"EUU:EuropeUnion", //Z
};

					char *p=(char*)libprism_buf+512;
					_consoleClear2();
					_consolePrint2(
						"*** DS card info ***\n"
						"Trying to retrive information without re-initializing card. Might be buggy.\n"
						"If this info seems buggy, turn off NDS immediately or you might even lose your data.\n\n"
					);
					_consolePrintf2("Card ID:         %08x\n",cardReadID(0));
					cardReadHeader(libprism_buf);
					memset(p,0,13);
					memcpy(p,libprism_buf,12);
					_consolePrintf2("Game Name:       %s\n",p);
					p[4]=0;
					memcpy(p,libprism_buf+12,4);
					_consolePrintf2("Game ID:         %s\n",p);
					if('a'<=p[3]&&p[3]<='z')p[3]-=0x20;
					_consolePrintf2("Card Region:     %s\n",('A'<=p[3]&&p[3]<='Z')?region[p[3]-'A']:"Unknown");
					p[2]=0;
					memcpy(p,libprism_buf+16,2);
					_consolePrintf2("Maker code:      %s\n",p);
					_consolePrintf2("Unit code:       %d\n",libprism_buf[18]);
					_consolePrintf2("key2 seed index: %d(%02x)\n",libprism_buf[19],key_tbl[0x2a+libprism_buf[19]]);
					_consolePrintf2("Card size:       %d bytes\n",1<<(17+libprism_buf[20]));
					_consolePrintf2("Game version:    %02x\n",libprism_buf[30]);
					_consolePrintf2("Flags:           %02x\n",libprism_buf[31]);
					_consolePrintf2("Logo CRC:        %04x\n",read16(libprism_buf+0x15c));
					_consolePrintf2("header CRC:      %04x\n",read16(libprism_buf+0x15e));

					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrint2("\nPress any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
				}break;
				case sys_m3region:{
					_consoleClear();
					_consoleClear2();
					_consolePrint("*** Show M3 Region / R4 Jumper ***\n\n");

					u32 type=R4_ReadCardInfo()&0x3ff;
					_consolePrintf("Jumper: 0x%03X (%d%d%d-%d%d%d-%d%d%d)\n",type,
						(type>>8)&1,(type>>7)&1,(type>>6)&1,(type>>5)&1,(type>>4)&1,(type>>3)&1,(type>>2)&1,(type>>1)&1,(type>>0)&1);
					type=(u8)M3_ReadCardRegion()&0xff;
					_consolePrintf("Region: 0x%03X (%d-%d%d%d-%d%d-%d%d)\n",type,
						(type>>7)&1,(type>>6)&1,(type>>5)&1,(type>>4)&1,(type>>3)&1,(type>>2)&1,(type>>1)&1,(type>>0)&1);
					_consolePrint("\n");

					_consolePrint2(
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
					_consolePrint("Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
				}break;
				case sys_swapsd:{
					if(GetRunningMode()){_consolePrint2("not supported in DSi mode.\n");continue;}
					destroyfilelist();
					_consoleClear();
					disc_unmount();
					_consolePrint(
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
					_consolePrint("Reinitializing... ");
					if(!fatinited&&!disc_mount()){
						_consolePrint(
							"failed.\n"
							"Some flashcarts cannot initialize DLDI after ejected...\n"
							"In short swaping microSD doesn't work on this flashcart.\n"
						);
						die();
					}
					_consolePrint("done.\n");
					///

					strcpy(dir,"/");
					getfilelist(dir,filter);
					usage();
					cursor=0,oldcursor=0,top=0;
				}break;
#if 1
				case sys_changedldi:{
					if(DLDIToBoot==DLDIDATA){_consolePrint2("Select Target DLDI first.\n");break;}
					_consoleClear();
					destroyfilelist();
					disc_unmount();
					_consolePrint("microSD unmounted.\n");

					dldi(DLDIDATA,32*1024);
					_consolePrint("Reinitializing FAT...\n");
					if(!disc_mount()){_consolePrint("Failed.\n");die();}
					_consolePrint("Done.\n");
					DLDIToBoot=DLDIDATA;

					getfilelist(dir,filter);
					usage();
					cursor=0,oldcursor=0,top=0;
				}break;
#endif
				case sys_return:{
					char *file="/moonshl2/resetmse/zzzz.nds";
					memcpy(file+19,DLDIDATA+ioType,4);
					destroyfilelist();
					_consoleClear();
					_consolePrintf2("Selected %s\n",file);
					BootLibrary(file);
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrint("Failed. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
					getfilelist(dir,filter);
				}break;
				case sys_dsmenu:{
					destroyfilelist();
					_consoleClear();

					_consolePrint("*** Return to NDS Firmware ***\n\n");
					switch(returnDSMenu()){
						//case -1:_consolePrint2("non GPL version cannot return to DS Menu.\n");break;
						//case 1:_consolePrint2("NDSi cannot return to DS Menu.\n");break;
						//case 2:_consolePrint2("Firmware isn't 256KB.\n");break;
						case 3:_consolePrint2("Cannot alloc memory.\n");break;
						case 4:_consolePrint2("Firmware decode error.\n");break;
					}
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrint2("Failed. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
					getfilelist(dir,filter);
				}break;
				case sys_bootstub:{
					jumpBootStub();
					_consolePrint2("It seems that bootstub isn't installed.\n");
				}break;
				case sys_shutdown:{
					disc_unmount();
					IPCZ->cmd=Shutdown;
				}break;
				case sys_bios:{
					if(GetRunningMode()){_consolePrint2("not supported in DSi mode.\n");continue;}
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
					CallARM7(GetFirmware);
					DC_InvalidateAll();

					f=fopen(fwname,"wb");
					int i=0;
					_consoleStartProgress2();
					for(;i<IPCZ->fwsize;i+=min(BUFLEN,IPCZ->fwsize-i),_consolePrintProgress2("Writing",i,IPCZ->fwsize)){
						fwrite(p+i,1,min(BUFLEN,IPCZ->fwsize-i),f);
					}
					_consoleEndProgress2();
					free(p);
					fclose(f);
					_consolePrint2("\nDone.\n");
bios_end:
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrint2("Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
					getfilelist(dir,filter);
				}break;
				case sys_testsd:{
					FILE *f;
					u32 n=getSectors()/8;
					u32 i;
					double r;

					destroyfilelist();
					_consoleClear2();
					_consolePrint2("*** Test microSD speed ***\n\n");
					u8* buf=(u8*)malloc(1024*1024);
					if(!buf){
						_consolePrint2("Cannot alloc memory.\n");goto testsd_end;
					}

					for(i=0;i<100;i++){
						sprintf((char*)buf,"/TestSD%02d.txt",i);
						if(access((char*)buf,0/*2*/))break;
					}
					if(i==100){
						free(buf);
						_consolePrint2("Cannot create log file.\n");goto testsd_end;
					}
					f=fopen((char*)buf,"w");
					_consolePrintf2("Log file: %s\n\n",buf);
					fprintf(f,
						"*** XenoFile Test microSD Log ***\n"
						"DLDI ID:   %s\n"
						"DLDI Name: %s\n\n",dldiid,DLDIDATA+friendlyName
					); 

					//OK test from here
/*
					_consolePrint2("Random 4K: ");
					IPCZ->blanks=0;
					for(i=0;i<8;i++){
						disc_readSectors(i*n,1,buf);
					}
					r=IPCZ->blanks*1024/61263.0;
					_consolePrintf2("%.3fs (%.3fKB/s)\n",r,4/r);
					fprintf(f,"Random 4K: %.3fs (%.3fKB/s)\n",r,4/r);
*/
					_consolePrint2("Random 1M: ");
					IPCZ->blanks=0;
					for(i=0;i<1024*2;i++){
						disc_readSectors((i&7)*n+i/8,1,buf);
					}
					r=IPCZ->blanks*1024/61263.0;
					_consolePrintf2("%.3fs (%.3fKB/s)\n",r,1024/r);
					fprintf(f,"Random 1M: %.3fs (%.3fKB/s)\n",r,1024/r);

/*
					_consolePrint2("Sequential 4K: ");
					IPCZ->blanks=0;
					for(i=0;i<8;i++){
						disc_readSectors(i,1,buf);
					}
					r=IPCZ->blanks*1024/61263.0;
					_consolePrintf2("%.3fs (%.3fKB/s)\n",r,4/r);
					fprintf(f,"Sequential 4K: %.3fs (%.3fKB/s)\n",r,4/r);
*/

					_consolePrint2("Sequential 1M: ");
					IPCZ->blanks=0;
					for(i=0;i<1024*2;i++){
						disc_readSectors(i,1,buf);
					}
					r=IPCZ->blanks*1024/61263.0;
					_consolePrintf2("%.3fs (%.3fKB/s)\n",r,1024/r);
					fprintf(f,"Sequential 1M: %.3fs (%.3fKB/s)\n",r,1024/r);

					_consolePrint2("MultiBlock 1M: ");
					IPCZ->blanks=0;
					//for(i=0;i<1024*2;i++){
						disc_readSectors(n*4,2*1024,buf);
					//}
					r=IPCZ->blanks*1024/61263.0;
					_consolePrintf2("%.3fs (%.3fKB/s)\n",r,1024/r);
					fprintf(f,"MultiBlock 1M: %.3fs (%.3fKB/s)\n",r,1024/r);

					fclose(f);
					free(buf);
testsd_end:
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrint2("Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
					getfilelist(dir,filter);
				}break;
				case sys_fixfsinfo:{
#ifdef LIBELM
					_consolePrint2("not required in libelm.\n");continue;
#else
					/////
#ifndef USE_LIBFAT109
					_consolePrint2("not required in libfat 1.0.8.\n");continue;
#else
					_consoleClear2();
					_consolePrint2("*** Fix FSInfo Sector ***\n\n");
					struct statvfs st;
					memcpy(&st.f_flag,"SCAN",4);
					statvfs("fat:/",&st);
					while(IPCZ->keysheld)swiWaitForVBlank();
					_consolePrint2("Done. Press any key.\n");
					while(!IPCZ->keysdown)swiWaitForVBlank();
					usage();
#endif
#endif
				}break;
				case sys_slot2nds:{
					if(IPCZ->NDSType>=NDSi){_consolePrint2("not supported in DSi.\n");continue;}
					slot2nds();
				}break;
				case sys_slot2gba:{
					if(IPCZ->NDSType>=NDSi){_consolePrint2("not supported in DSi.\n");continue;}
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
	}

	die();
}
