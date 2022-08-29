#include "../../libprism/libprism.h"
#include "warning_eos_b15lzma.h"
#include "warning_dstwo_b15lzma.h"
#include "warning_m3sakura_b15lzma.h"
#include "warning_r4_b15lzma.h"
#include "warning_ismart_b15lzma.h"
#include "warning_ez_b15lzma.h"

#include "LzmaDec.h"

const u16 bgcolor=RGB15(10,10,0);

static u16 notice[3+256*192];

typedef bool (*type_constpchar_int_constpchar)(const char*,int,const char*);
type_constpchar_int_constpchar BootLibrary;

/*
I cannot remove it any longer, but... this is hidden as not working orz

R4/EX4DS support will be in alpha phase forever.
inilink moves nds to /default.nds then iniclear moves it back to original filename.
Multibyte filename isn't supported.
Make sure iniclear is set as /_ds_menu.dat (encrypted).

Press A to accept (from next time this notice will not be shown).
Press B to shutdown.
*/

///lzma
static void *SzAlloc(void *p, size_t size) { p = p; return malloc(size); }
static void SzFree(void *p, void *address) { p = p; free(address); }
static ISzAlloc g_Alloc = { SzAlloc, SzFree };

void ImageDecode(const u8 *img, u32 size, u16 *buf){
	//swiDecompressLZSSWram((u8*)img,buf);

	ELzmaStatus lzmastatus;
	u32 bufsize=2*256*192+6;
	u32 srcsize=size-13;
	LzmaDecode((u8*)buf,&bufsize,img+13,&srcsize,img,5,LZMA_FINISH_ANY,&lzmastatus,&g_Alloc);
}

void Main(){
	FILE *f;
	TExtLinkBody extlink;
	char ysmenu[768];
	char dldiid[5];
	unsigned char *dldiFileData=DLDIDATA;

	struct stat st;

	BootLibrary=BootNDSROM2;
	IPCZ->cmd=0;

	_consolePrintf(
		"MoonShell2 inilinker\n"
		"reset_mse_06b_for_ak2 by Moonlight, Rudolph, kzat3\n"
		"dldipatch aka dlditool public domain under CC0.\n"
		"%s\n%s\n\n",
		ROMDATE,ROMENV
	);

	{
		memcpy(dldiid,(char*)dldiFileData+ioType,4);
		dldiid[4]=0;
		_consolePrintf("DLDI ID: %s\n",dldiid);
		_consolePrintf("DLDI Name: %s\n\n",(char*)dldiFileData+friendlyName);
	}

	//ImageDecode(warning_m3sakura_b15lzma,warning_m3sakura_b15lzma_size,notice);
	//vramcpy(b15ptrSub,notice+3,256*192);
	//EnableB15Sub();

	_consolePrintf("Initializing libfat... ");
	if(!fatInitDefault()){_consolePrintf("Failed.\n");die();}
	_consolePrintf("Done.\n");

	_consolePrintf("Opening extlink... ");
	if(!(f=fopen("/MOONSHL2/EXTLINK.DAT","rb"))){_consolePrintf("Failed.\n");die();}
	fread(&extlink,1,sizeof(TExtLinkBody),f);
	fclose(f);
	if(extlink.ID!=ExtLinkBody_ID){_consolePrintf("Incorrect ID.\n");die();}
	_consolePrintf("Done.\n");

	_consolePrintf("Configuring... ");
{
	char ininame[768];
	int useak2=0,useeos=0/*,useeos2=0*/,usewoodr4=0,usem3=0;
	int eos_alpha_accepted=0,dstwo_alpha_accepted=0,r4_alpha_accepted=0,m3sakura_alpha_accepted=0,m3sakura_iniclear_method=0,ismart_alpha_accepted=0,ez_alpha_accepted=0;

	ini_gets("Config","YSMenu","/YSMENU/YSMENU.NDS",ysmenu,768,"/MOONSHL2/EXTLINK/inilink.ini");
	ini_gets("Config","YSini","/YSMENU/YSMENU.INI",ininame,768,"/MOONSHL2/EXTLINK/inilink.ini");
	if(!memcmp(dldiid,"SCDS",4)){
		useeos=0;//ini_getl("Config","UseEOS",0,"/MOONSHL2/EXTLINK/inilink.ini");
		SCDS_SetSDHCModeForDSTT();
		//if(useeos==0)ini_gets("Config","SCYSMenu",ysmenu,ysmenu,768,"/MOONSHL2/EXTLINK/inilink.ini");
		if(useeos==1)ini_gets("Config","DSOneEOS","/EOS.NDS",ysmenu,768,"/MOONSHL2/EXTLINK/inilink.ini");
	}
	if(!memcmp(dldiid,"RPGS",4)||!memcmp(dldiid,"R4DS",4)||!memcmp(dldiid,"_R4i",4)||!memcmp(dldiid,"RPGN",4)||!memcmp(dldiid,"XXXX",4)){
		useak2=ini_getl("Config","UseAK2",2,"/MOONSHL2/EXTLINK/inilink.ini");
		if(!memcmp(dldiid,"XXXX",4))memcpy(dldiid,"RPGS",4),useak2=2;
		if(strstr((char*)dldiFileData+friendlyName,"RPG"))
			useak2=ini_getl("Config","UseRPG",2,"/MOONSHL2/EXTLINK/inilink.ini");
		if(!memcmp(dldiid,"R4DS",4)||!memcmp(dldiid,"_R4i",4)){
			memcpy(dldiid,"RPGS",4);
			if(!useak2)memcpy((char*)dldiFileData+ioType,"RPGS",4);
		}
		if(useak2==3)ini_gets("Config","WoodRPG","/AKMENU.NDS",ysmenu,768,"/MOONSHL2/EXTLINK/inilink.ini");
		if(useak2==2)ini_gets("Config","AKAIO","/AKMENU.NDS",ysmenu,768,"/MOONSHL2/EXTLINK/inilink.ini");
		if(useak2==1)ini_gets("Config","AKMENU","/AKMENU.NDS",ysmenu,768,"/MOONSHL2/EXTLINK/inilink.ini");
	}
	if(!memcmp(dldiid,"M3DS",4)||!memcmp(dldiid,"iTDS",4)||!memcmp(dldiid,"R4_I",4)){
		usem3=ini_getl("Config","UseM3",0,"/MOONSHL2/EXTLINK/inilink.ini");
		if(!memcmp(dldiid,"iTDS",4)||!memcmp(dldiid,"R4_I",4)){
			memcpy(dldiid,"M3DS",4);
			if(!usem3)memcpy((char*)dldiFileData+ioType,"M3DS",4);
		}
		if(usem3==0)ini_gets("Config","YSM3",ysmenu,ysmenu,768,"/MOONSHL2/EXTLINK/inilink.ini");
		if(usem3==1)ini_gets("Config","TouchPod","/TOUCHPOD.NDS",ysmenu,768,"/MOONSHL2/EXTLINK/inilink.ini");
		if(usem3==2)ini_gets("Config","M3Sakura","/misakura.nds",ysmenu,768,"/MOONSHL2/EXTLINK/inilink.ini");
	}
	if(!memcmp(dldiid,"R4TF",4)){
		usewoodr4=ini_getl("Config","UseWoodR4",1,"/MOONSHL2/EXTLINK/inilink.ini");
		if(usewoodr4==1)ini_gets("Config","WoodR4","/WOODR4.NDS",ysmenu,768,"/MOONSHL2/EXTLINK/inilink.ini");
		if(usewoodr4==2)ini_gets("Config","R4Kernel","/r4kern.nds",ysmenu,768,"/MOONSHL2/EXTLINK/inilink.ini");
		if(usewoodr4==-1)ini_gets("Config","R4i3D","/R4i3D.NDS",ysmenu,768,"/MOONSHL2/EXTLINK/inilink.ini");
	}
	if(!memcmp(dldiid,"DEMO",4))memcpy((char*)dldiFileData+ioType,"TTIO",4);
	if(!memcmp(dldiid,"DSI2",4))ini_gets("Config","DSTwo","/_dstwo/dsgame.dat",ysmenu,768,"/MOONSHL2/EXTLINK/inilink.ini");
	if(!memcmp(dldiid,"Mati",4)||!memcmp(dldiid,"IMAT",4)){
		//useismart=ini_getl("Config","UseiSmart",0,"/MOONSHL2/EXTLINK/inilink.ini");
		if(!memcmp(dldiid,"Mati",4)){
			memcpy(dldiid,"IMAT",4);
		}
		ini_gets("Config","iSmartFishShell",ysmenu,ysmenu,768,"/MOONSHL2/EXTLINK/inilink.ini");
	}
	if(!memcmp(dldiid,"G003",4))ini_gets("Config","M3Sakura_G003","/g003_misakura.nds",ysmenu,768,"/MOONSHL2/EXTLINK/inilink.ini");
	if(!memcmp(dldiid,"EZ5H",4))ini_gets("Config","EZ5SYS","/ez5sys.nds",ysmenu,768,"/MOONSHL2/EXTLINK/inilink.ini");
	if(!memcmp(dldiid,"EZ5i",4))ini_gets("Config","EZ5iSYS","/ez5isys.nds",ysmenu,768,"/MOONSHL2/EXTLINK/inilink.ini");

	eos_alpha_accepted=ini_getl("Config","eos_alpha_accepted",0,"/MOONSHL2/EXTLINK/inilink.ini");
	dstwo_alpha_accepted=ini_getl("Config","dstwo_alpha_accepted",0,"/MOONSHL2/EXTLINK/inilink.ini");
	r4_alpha_accepted=ini_getl("Config","r4_alpha_accepted",0,"/MOONSHL2/EXTLINK/inilink.ini");
	m3sakura_alpha_accepted=ini_getl("Config","m3sakura_alpha_accepted",0,"/MOONSHL2/EXTLINK/inilink.ini");
	m3sakura_iniclear_method=ini_getl("Config","m3sakura_iniclear_method",0,"/MOONSHL2/EXTLINK/inilink.ini");
	ismart_alpha_accepted=ini_getl("Config","ismart_alpha_accepted",0,"/MOONSHL2/EXTLINK/inilink.ini");
	ez_alpha_accepted=ini_getl("Config","ez_alpha_accepted",0,"/MOONSHL2/EXTLINK/inilink.ini");

	//if(!autoboot){
	//	ini_puts("YSMenu","AUTO_BOOT","",ininame);
	//}else
	{
		char utf8[768];
		_FAT_directory_ucs2tombs(utf8,extlink.DataFullPathFilenameUnicode,768);
		if(usem3==1&&!memcmp(dldiid,"M3DS",4)){
			char buf[1024];
			FILE *in,*out;
			in=fopen("/system/boot.ini","rb");
			if(!in){_consolePrintf("Cannot open /system/boot.ini. Halt.");die();}
			out=fopen("/system/boot.in~","wb");
			while(myfgets(buf,1024,in)){
				if(!memcmp(buf,"[DirectRun]",11)){
					strcpy(buf,"[DirectRun]="),strcat(buf,utf8),strcat(buf,"\r\n");
					fputs(buf,out);
				}else{
					strcat(buf,"\r\n");
					fputs(buf,out);
				}
			}
			fclose(out);fclose(in);
			remove("/system/boot.ini");
			rename("/system/boot.in~","/system/boot.ini");
		}else if(usem3==2&&!memcmp(dldiid,"M3DS",4)){
			char target[768];
			if(!m3sakura_alpha_accepted){
				ImageDecode(warning_m3sakura_b15lzma,warning_m3sakura_b15lzma_size,notice);
				vramcpy(b15ptrSub,notice+3,256*192);
				EnableB15Sub();
				for(swiWaitForVBlank();;swiWaitForVBlank()){
					if(!(IPCZ->keysdown))continue;
					if(IPCZ->keysdown&KEY_A)break;
					if(IPCZ->keysdown&KEY_B)IPCZ->cmd=Shutdown;
				}
				DisableB15Sub();
				ini_putl("Config","m3sakura_alpha_accepted",1,"/MOONSHL2/EXTLINK/inilink.ini");
			}
			if(!stat("/defaultn.nds",&st)||!stat("/defaultx.nds",&st)||!stat("/defaulty.nds",&st)){_consolePrintf("/default{n,x,y}.nds exists. Make sure iniclear is executed.\n");die();}
			strcpy(target,"/defaulty.nds");
			rename(utf8,target);
			ini_puts("Config","m3sakura_recover",utf8,"/MOONSHL2/EXTLINK/inilink.ini");
			strcpy(getextname(utf8),".sav");
			strcpy(getextname(target),".sav");
			rename(utf8,target);
			if(m3sakura_iniclear_method){
				copy("/iniclear.nds","/defaultn.nds");
			}
			_consolePrintf("Press and HOLD Y button...\n");
			for(swiWaitForVBlank();;swiWaitForVBlank()){
				//if(!(IPCZ->keysdown))continue;
				if(IPCZ->keysdown&KEY_Y)break;
				//if(IPCZ->keysdown&KEY_B)IPCZ->cmd=Shutdown;
			}
		}else if(!memcmp(dldiid,"G003",4)){
			char target[768];
			if(!m3sakura_alpha_accepted){
				ImageDecode(warning_m3sakura_b15lzma,warning_m3sakura_b15lzma_size,notice);
				vramcpy(b15ptrSub,notice+3,256*192);
				EnableB15Sub();
				for(swiWaitForVBlank();;swiWaitForVBlank()){
					if(!(IPCZ->keysdown))continue;
					if(IPCZ->keysdown&KEY_A)break;
					if(IPCZ->keysdown&KEY_B)IPCZ->cmd=Shutdown;
				}
				DisableB15Sub();
				ini_putl("Config","m3sakura_alpha_accepted",1,"/MOONSHL2/EXTLINK/inilink.ini");
			}
			if(!stat("/defaultn.nds",&st)||!stat("/defaultx.nds",&st)||!stat("/defaulty.nds",&st)){_consolePrintf("/default{n,x,y}.nds exists. Make sure iniclear is executed.\n");die();}
			strcpy(target,"/defaulty.nds");
			rename(utf8,target);
			ini_puts("Config","m3sakura_recover",utf8,"/MOONSHL2/EXTLINK/inilink.ini");
			strcpy(getextname(utf8),".sav");
			strcpy(getextname(target),".sav");
			rename(utf8,target);
			if(m3sakura_iniclear_method){
				copy("/iniclear.nds","/defaultn.nds");
			}
			_consolePrintf("Press and HOLD Y button...\n");
			for(swiWaitForVBlank();;swiWaitForVBlank()){
				//if(!(IPCZ->keysdown))continue;
				if(IPCZ->keysdown&KEY_Y)break;
				//if(IPCZ->keysdown&KEY_B)IPCZ->cmd=Shutdown;
			}
		}else if(useak2==3&&!memcmp(dldiid,"RPG",3)){
			char tmp[776];
			strcpy(tmp,"fat1:");
			strcpy(tmp+5,utf8);
			ini_puts("Save Info","lastLoaded",tmp,"/__rpg/lastsave.ini");
			strcpy(tmp+5,extlink.DataFullPathFilenameAlias);
			ini_puts("Save Info","shortPathNDS",tmp,"/__rpg/lastsave.ini");
			ini_putl("system","autorunWithLastRom",1,"/__rpg/globalsettings.ini");
		}else if(useak2==2&&!memcmp(dldiid,"RPG",3)){
			char tmp[776];
			strcpy(tmp,"fat0:"); //0.66b
			strcpy(tmp+5,utf8);
			ini_puts("Save Info","lastLoaded",tmp,"/__aio/lastsave.ini");
			strcpy(tmp+5,extlink.DataFullPathFilenameAlias);
			ini_puts("Save Info","shortPathNDS",tmp,"/__aio/lastsave.ini");
			ini_putl("system","autorunWithLastRom",1,"/__aio/globalsettings.ini");
		}else if(useak2==1&&!memcmp(dldiid,"RPG",3)){
			char tmp[776];
			strcpy(tmp,"fat1:");
			strcpy(tmp+5,utf8);
			ini_puts("Save Info","lastLoaded",tmp,"/__ak2/lastsave.ini");
			strcpy(tmp+5,extlink.DataFullPathFilenameAlias);
			ini_puts("Save Info","shortPathNDS",tmp,"/__ak2/lastsave.ini");
			ini_putl("system","autorunWithLastRom",1,"/__ak2/globalsettings.ini");
		}else if(usewoodr4==1&&!memcmp(dldiid,"R4TF",4)){
			char tmp[776];
			strcpy(tmp,"fat0:"); //0.63b1
			strcpy(tmp+5,utf8);
			ini_puts("Save Info","lastLoaded",tmp,"/__rpg/lastsave.ini");
			//strcpy(tmp+5,extlink.DataFullPathFilenameAlias);
			//ini_puts("Save Info","shortPathNDS",tmp,"/__rpg/lastsave.ini");
			ini_putl("system","autorunWithLastRom",1,"/__rpg/globalsettings.ini");
		}else if(usewoodr4==2&&!memcmp(dldiid,"R4TF",4)){
			char target[768];
			if(!r4_alpha_accepted){
				ImageDecode(warning_r4_b15lzma,warning_r4_b15lzma_size,notice);
				vramcpy(b15ptrSub,notice+3,256*192);
				EnableB15Sub();
				for(swiWaitForVBlank();;swiWaitForVBlank()){
					if(!(IPCZ->keysdown))continue;
					if(IPCZ->keysdown&KEY_A)break;
					if(IPCZ->keysdown&KEY_B)IPCZ->cmd=Shutdown;
				}
				DisableB15Sub();
				ini_putl("Config","r4_alpha_accepted",1,"/MOONSHL2/EXTLINK/inilink.ini");
			}
			if(!stat("/default.nds",&st)){_consolePrintf("/default.nds exists. Make sure iniclear is executed.\n");die();}
			strcpy(target,"/default.nds");
			rename(utf8,target);
			ini_puts("Config","r4_recover",utf8,"/MOONSHL2/EXTLINK/inilink.ini");
			strcpy(getextname(utf8),".sav");
			strcpy(getextname(target),".sav");
			rename(utf8,target);
		}else if(usewoodr4==-1&&!memcmp(dldiid,"R4TF",4)){
			char tmp[776];
			strcpy(tmp,"fat1:");
			strcpy(tmp+5,utf8);
			ini_puts("Save Info","lastLoaded",tmp,"/system/lastsave.ini");
			//strcpy(tmp+5,extlink.DataFullPathFilenameAlias);
			//ini_puts("Save Info","shortPathNDS",tmp,"/system/lastsave.ini");
			ini_putl("system","autorunWithLastRom",1,"/system/globalsettings.ini");
		}else if(useeos&&!memcmp(dldiid,"SCDS",4)){
			char target[768],tmp[776];
			if(!eos_alpha_accepted){
				ImageDecode(warning_eos_b15lzma,warning_eos_b15lzma_size,notice);
				vramcpy(b15ptrSub,notice+3,256*192);
				EnableB15Sub();
				for(swiWaitForVBlank();;swiWaitForVBlank()){
					if(!(IPCZ->keysdown))continue;
					if(IPCZ->keysdown&KEY_A)break;
					if(IPCZ->keysdown&KEY_B)IPCZ->cmd=Shutdown;
				}
				DisableB15Sub();
				ini_putl("Config","eos_alpha_accepted",1,"/MOONSHL2/EXTLINK/inilink.ini");
			}
			if(!stat("/!!!extlink",&st)){_consolePrintf("/!!!extlink exists. Make sure iniclear is executed.\n");die();}
			mkdir("/!!!extlink",0755);
			strcpy(target,"/!!!extlink/");
			char *filename=target+strlen(target);
			_FAT_directory_ucs2tombs(tmp,extlink.DataFilenameUnicode,768);
			strcpy(filename,tmp);
			rename(utf8,target);
			ini_puts("Config","eos_recover",utf8,"/MOONSHL2/EXTLINK/inilink.ini");
			strcpy(getextname(utf8),".sav");
			strcpy(getextname(target),".sav");
			rename(utf8,target);
			strcpy(getextname(utf8),".dso");
			strcpy(getextname(target),".dso");
			rename(utf8,target);
			ini_putl("system","enterLastDirWhenBoot",1,"/_dsone/globalsettings.ini");

			strcpy(getextname(target),".nds");
			strcpy(tmp,"fat1:");
			getsfnlfn(target,tmp+5,NULL);
			ini_puts("Dir Info","last dir",tmp,"/_dsone/lastsave.ini");
			//BootLibrary=ret_menu9_Gen2;
#if 0
			char tmp[776];
			strcpy(tmp,"fat1:");
			//strcpy(tmp+5,utf8);
			//ini_puts("Dir Info","last dir",tmp,"/_dsone/lastsave.ini");
			strcpy(tmp+5,extlink.DataFullPathFilenameAlias);
			ini_puts("Dir Info","last dir",tmp,"/_dsone/lastsave.ini");
			ini_putl("system","enterLastDirWhenBoot",1,"/_dsone/globalsettings.ini");
#endif
		}else if(!memcmp(dldiid,"DSI2",4)){
			char dstwodir[768];
			ini_gets("Config","DSTwoDir","",dstwodir,768,"/MOONSHL2/EXTLINK/inilink.ini");
			if(!*dstwodir){
				_consolePrintf("DSTwoDir is null. Halt.\n");die();
			}
			char *dstwodirfilename=dstwodir+strlen(dstwodir);
			if(*(dstwodirfilename-1)!='/'){*dstwodirfilename='/';dstwodirfilename++;}

			char target[768],tmp[776];
			if(!dstwo_alpha_accepted){
				ImageDecode(warning_dstwo_b15lzma,warning_dstwo_b15lzma_size,notice);
				vramcpy(b15ptrSub,notice+3,256*192);
				EnableB15Sub();
				for(swiWaitForVBlank();;swiWaitForVBlank()){
					if(!(IPCZ->keysdown))continue;
					if(IPCZ->keysdown&KEY_A)break;
					if(IPCZ->keysdown&KEY_B)IPCZ->cmd=Shutdown;
				}
				DisableB15Sub();
				ini_putl("Config","dstwo_alpha_accepted",1,"/MOONSHL2/EXTLINK/inilink.ini");
			}
			if(!stat("/!!!extlink",&st)){_consolePrintf("/!!!extlink exists. Make sure iniclear is executed.\n");die();}
			mkdir("/!!!extlink",0755);
			strcpy(target,"/!!!extlink/");
			char *filename=target+strlen(target);
			_FAT_directory_ucs2tombs(tmp,extlink.DataFilenameUnicode,768);
			strcpy(filename,tmp);
			rename(utf8,target);
			ini_puts("Config","dstwo_recover",utf8,"/MOONSHL2/EXTLINK/inilink.ini");
			strcpy(getextname(utf8),".sav");
			strcpy(getextname(target),".sav");
			rename(utf8,target);
			strcpy(getextname(utf8),".dso");
			strcpy(getextname(target),".dso");
			rename(utf8,target);
			strcpy(dstwodirfilename,"globalsettings.ini");
			ini_putl("system","enterLastDirWhenBoot",1,dstwodirfilename);

			strcpy(getextname(target),".nds");
			strcpy(tmp,"fat1:");
			getsfnlfn(target,tmp+5,NULL);
			strcpy(dstwodirfilename,"lastsave.ini");
			ini_puts("Dir Info","last dir",tmp,dstwodirfilename);
			//BootLibrary=ret_menu9_Gen2;
#if 0
			char tmp[776];
			strcpy(tmp,"fat1:");
			//strcpy(tmp+5,utf8);
			//ini_puts("Dir Info","last dir",tmp,"/_dstwo/lastsave.ini");
			strcpy(tmp+5,extlink.DataFullPathFilenameAlias);
			ini_puts("Dir Info","last dir",tmp,"/_dstwo/lastsave.ini");
			ini_putl("system","enterLastDirWhenBoot",1,"/_dstwo/globalsettings.ini");
#endif
		}else if(!memcmp(dldiid,"IMAT",4)){
			char target[768],utf8_2[768];
			if(!ismart_alpha_accepted){
				ImageDecode(warning_ismart_b15lzma,warning_ismart_b15lzma_size,notice);
				vramcpy(b15ptrSub,notice+3,256*192);
				EnableB15Sub();
				for(swiWaitForVBlank();;swiWaitForVBlank()){
					if(!(IPCZ->keysdown))continue;
					if(IPCZ->keysdown&KEY_A)break;
					if(IPCZ->keysdown&KEY_B)IPCZ->cmd=Shutdown;
				}
				DisableB15Sub();
				ini_putl("Config","ismart_alpha_accepted",1,"/MOONSHL2/EXTLINK/inilink.ini");
			}
			if(!stat("/defaultn.nds",&st)||!stat("/defaultx.nds",&st)||!stat("/defaulty.nds",&st)||!stat("/save/defaulty.sav",&st)){_consolePrintf("/default{n,x,y}.nds exists. Make sure iniclear is executed.\n");die();}
			strcpy(target,"/defaultn.nds");
			rename(utf8,target);
			ini_puts("Config","ismart_recover_nds",utf8,"/MOONSHL2/EXTLINK/inilink.ini");
			strcpy(getextname(utf8),".sav");
			sprintf(utf8_2,"/save/%s",getfilename(utf8));
			strcpy(target,"/save/defaultn.sav");
			rename(utf8_2,target);
			ini_puts("Config","ismart_recover_sav",utf8_2,"/MOONSHL2/EXTLINK/inilink.ini");
			//copy("/iniclear.nds","/defaultn.nds");
/*
			_consolePrintf("Press and HOLD Y button...\n");
			for(swiWaitForVBlank();;swiWaitForVBlank()){
				//if(!(IPCZ->keysdown))continue;
				if(IPCZ->keysdown&KEY_Y)break;
				//if(IPCZ->keysdown&KEY_B)IPCZ->cmd=Shutdown;
			}
*/
		}else if(!memcmp(dldiid,"EZ5H",4)||!memcmp(dldiid,"EZ5i",4)){
			char target[768],tmp[776];
			if(!ez_alpha_accepted){
				ImageDecode(warning_ez_b15lzma,warning_ez_b15lzma_size,notice);
				vramcpy(b15ptrSub,notice+3,256*192);
				EnableB15Sub();
				for(swiWaitForVBlank();;swiWaitForVBlank()){
					if(!(IPCZ->keysdown))continue;
					if(IPCZ->keysdown&KEY_A)break;
					if(IPCZ->keysdown&KEY_B)IPCZ->cmd=Shutdown;
				}
				DisableB15Sub();
				ini_putl("Config","ez_alpha_accepted",1,"/MOONSHL2/EXTLINK/inilink.ini");
			}
			if(!stat("/!!!extlink",&st)){_consolePrintf("/!!!extlink exists. Make sure iniclear is executed.\n");die();}
			mkdir("/!!!extlink",0755);
			strcpy(target,"/!!!extlink/");
			char *filename=target+strlen(target);
			_FAT_directory_ucs2tombs(tmp,extlink.DataFilenameUnicode,768);
			strcpy(filename,tmp);
			rename(utf8,target);
			ini_puts("Config","ez_recover",utf8,"/MOONSHL2/EXTLINK/inilink.ini");
		}else{
			ini_puts("YSMenu","AUTO_BOOT",utf8,ininame);
		}
	}
}
	_consolePrintf("Done.\n");

	_consolePrintf("Rebooting... \n");
	BootLibrary(ysmenu,1,NULL);
	// ^^^^^^^^^^^^ add 2008.03.30 kzat3
}
