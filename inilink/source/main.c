#include "../../libprism/libprism.h"
#include "warning_eos_b15lzma.h"
//#include "warning_dstwo_b15lzma.h"
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

static char ysmenu[768],utf8[768];
void Main(){
	FILE *f;
	//TExtLinkBody extlink;
	char dldiid[5];
	unsigned char *dldiFileData=DLDIDATA;

	struct stat st;

	BootLibrary=BootNDSROM2;

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

	_consolePrint("Initializing FAT... ");
	if(!disc_mount()){_consolePrint("Failed.\n");die();}
	_consolePrint("Done.\n");

	_consolePrint("Opening frontend... ");
	if(!readFrontend(utf8)){_consolePrint("Failed.\n");die();}
	_consolePrint("Done.\n");

	_consolePrint("Configuring... ");
{
	char ininame[768];
	int useak2=0,useeos=0/*,useeos2=0*/,usewoodr4=0,usem3=0;
	int eos_alpha_accepted=0,dstwo_alpha_accepted=0,r4_alpha_accepted=0,m3sakura_alpha_accepted=0,m3sakura_iniclear_method=0,ismart_alpha_accepted=0,ez_alpha_accepted=0;

	ini_gets("Config","YSMenu","/YSMENU/YSMENU.NDS",ysmenu,768,"/moonshl2/extlink/inilink.ini");
	ini_gets("Config","YSini","/YSMENU/YSMENU.INI",ininame,768,"/moonshl2/extlink/inilink.ini");
	if(!memcmp(dldiid,"SCDS",4)){
		useeos=0;//ini_getl("Config","UseEOS",0,"/moonshl2/extlink/inilink.ini");
		//SCDS_SetSDHCModeForDSTT();
		//if(useeos==0)ini_gets("Config","SCYSMenu",ysmenu,ysmenu,768,"/moonshl2/extlink/inilink.ini");
		if(useeos==1)ini_gets("Config","DSOneEOS","/EOS.NDS",ysmenu,768,"/moonshl2/extlink/inilink.ini");
	}
	if(!memcmp(dldiid,"RPGS",4)||!memcmp(dldiid,"R4DS",4)||!memcmp(dldiid,"_R4i",4)||!memcmp(dldiid,"RPGN",4)||!memcmp(dldiid,"XXXX",4)){
		useak2=ini_getl("Config","UseAK2",2,"/moonshl2/extlink/inilink.ini");
		if(!memcmp(dldiid,"XXXX",4))memcpy(dldiid,"RPGS",4),useak2=2;
		if(strstr((char*)dldiFileData+friendlyName,"RPG"))
			useak2=ini_getl("Config","UseRPG",2,"/moonshl2/extlink/inilink.ini");
		if(!memcmp(dldiid,"R4DS",4)||!memcmp(dldiid,"_R4i",4)){
			memcpy(dldiid,"RPGS",4);
			if(!useak2)memcpy((char*)dldiFileData+ioType,"RPGS",4);
		}
		if(useak2==3)ini_gets("Config","WoodRPG","/AKMENU.NDS",ysmenu,768,"/moonshl2/extlink/inilink.ini");
		if(useak2==2)ini_gets("Config","AKAIO","/AKMENU.NDS",ysmenu,768,"/moonshl2/extlink/inilink.ini");
		if(useak2==1)ini_gets("Config","AKMENU","/AKMENU.NDS",ysmenu,768,"/moonshl2/extlink/inilink.ini");
	}
	if(!memcmp(dldiid,"M3DS",4)||!memcmp(dldiid,"iTDS",4)||!memcmp(dldiid,"R4_I",4)){
		usem3=ini_getl("Config","UseM3",0,"/moonshl2/extlink/inilink.ini");
		if(!memcmp(dldiid,"iTDS",4)||!memcmp(dldiid,"R4_I",4)){
			memcpy(dldiid,"M3DS",4);
			if(!usem3)memcpy((char*)dldiFileData+ioType,"M3DS",4);
		}
		if(usem3==0)ini_gets("Config","YSM3",ysmenu,ysmenu,768,"/moonshl2/extlink/inilink.ini");
		if(usem3==1)ini_gets("Config","TouchPod","/TOUCHPOD.NDS",ysmenu,768,"/moonshl2/extlink/inilink.ini");
		if(usem3==2)ini_gets("Config","M3Sakura","/misakura.nds",ysmenu,768,"/moonshl2/extlink/inilink.ini");
		if(usem3==3)ini_gets("Config","WoodM3","/WOODM3.NDS",ysmenu,768,"/moonshl2/extlink/inilink.ini");
	}
	if(!memcmp(dldiid,"R4TF",4)){
		usewoodr4=ini_getl("Config","UseWoodR4",1,"/moonshl2/extlink/inilink.ini");
		if(usewoodr4==1)ini_gets("Config","WoodR4","/WOODR4.NDS",ysmenu,768,"/moonshl2/extlink/inilink.ini");
		if(usewoodr4==2)ini_gets("Config","R4Kernel","/r4kern.nds",ysmenu,768,"/moonshl2/extlink/inilink.ini");
		if(usewoodr4==-1)ini_gets("Config","R4i3D","/R4i3D.NDS",ysmenu,768,"/moonshl2/extlink/inilink.ini");
	}
	if(!memcmp(dldiid,"DEMO",4))memcpy((char*)dldiFileData+ioType,"TTIO",4);
	if(!memcmp(dldiid,"DSI2",4))ini_gets("Config","DSTwo","/_dstwo/dsgame.dat",ysmenu,768,"/moonshl2/extlink/inilink.ini");
	if(!memcmp(dldiid,"Mati",4)||!memcmp(dldiid,"IMAT",4)){
		//useismart=ini_getl("Config","UseiSmart",0,"/moonshl2/extlink/inilink.ini");
		if(!memcmp(dldiid,"Mati",4)){
			memcpy(dldiid,"IMAT",4);
		}
		ini_gets("Config","iSmartFishShell",ysmenu,ysmenu,768,"/moonshl2/extlink/inilink.ini");
	}
	if(!memcmp(dldiid,"G003",4))ini_gets("Config","M3Sakura_G003","/g003_misakura.nds",ysmenu,768,"/moonshl2/extlink/inilink.ini");
	if(!memcmp(dldiid,"EZ5H",4))ini_gets("Config","EZ5SYS","/ez5sys.nds",ysmenu,768,"/moonshl2/extlink/inilink.ini");
	if(!memcmp(dldiid,"EZ5i",4))ini_gets("Config","EZ5iSYS","/ez5isys.nds",ysmenu,768,"/moonshl2/extlink/inilink.ini");

	eos_alpha_accepted=ini_getl("Config","eos_alpha_accepted",0,"/moonshl2/extlink/inilink.ini");
	dstwo_alpha_accepted=ini_getl("Config","dstwo_alpha_accepted",0,"/moonshl2/extlink/inilink.ini");
	r4_alpha_accepted=ini_getl("Config","r4_alpha_accepted",0,"/moonshl2/extlink/inilink.ini");
	m3sakura_alpha_accepted=ini_getl("Config","m3sakura_alpha_accepted",0,"/moonshl2/extlink/inilink.ini");
	m3sakura_iniclear_method=ini_getl("Config","m3sakura_iniclear_method",0,"/moonshl2/extlink/inilink.ini");
	ismart_alpha_accepted=ini_getl("Config","ismart_alpha_accepted",0,"/moonshl2/extlink/inilink.ini");
	ez_alpha_accepted=ini_getl("Config","ez_alpha_accepted",0,"/moonshl2/extlink/inilink.ini");

	//if(!autoboot){
	//	ini_puts("YSMenu","AUTO_BOOT","",ininame);
	//}else
	{
		if(usem3==1&&!memcmp(dldiid,"M3DS",4)){
			char buf[1024];
			FILE *in,*out;
			in=fopen("/system/boot.ini","rb");
			if(!in){_consolePrint("Cannot open /system/boot.ini. Halt.");die();}
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
			unlink("/system/boot.ini");
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
				ini_putl("Config","m3sakura_alpha_accepted",1,"/moonshl2/extlink/inilink.ini");
			}
			if(!stat("/defaultn.nds",&st)||!stat("/defaultx.nds",&st)||!stat("/defaulty.nds",&st)){_consolePrint("/default{n,x,y}.nds exists. Make sure iniclear is executed.\n");die();}
			strcpy(target,"/defaulty.nds");
			rename(utf8,target);
			ini_puts("Config","m3sakura_recover",utf8,"/moonshl2/extlink/inilink.ini");
			strcpy(getextname(utf8),".sav");
			strcpy(getextname(target),".sav");
			rename(utf8,target);
			if(m3sakura_iniclear_method){
				copy("/iniclear.nds","/defaultn.nds");
			}
			_consolePrint("Press and HOLD Y button...\n");
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
				ini_putl("Config","m3sakura_alpha_accepted",1,"/moonshl2/extlink/inilink.ini");
			}
			if(!stat("/defaultn.nds",&st)||!stat("/defaultx.nds",&st)||!stat("/defaulty.nds",&st)){_consolePrint("/default{n,x,y}.nds exists. Make sure iniclear is executed.\n");die();}
			strcpy(target,"/defaulty.nds");
			rename(utf8,target);
			ini_puts("Config","m3sakura_recover",utf8,"/moonshl2/extlink/inilink.ini");
			strcpy(getextname(utf8),".sav");
			strcpy(getextname(target),".sav");
			rename(utf8,target);
			if(m3sakura_iniclear_method){
				copy("/iniclear.nds","/defaultn.nds");
			}
			_consolePrint("Press and HOLD Y button...\n");
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
			getsfnlfn(utf8,tmp+5,NULL);
			ini_puts("Save Info","shortPathNDS",tmp,"/__rpg/lastsave.ini");
			ini_putl("system","autorunWithLastRom",1,"/__rpg/globalsettings.ini");
		}else if(useak2==2&&!memcmp(dldiid,"RPG",3)){
			char tmp[776];
			strcpy(tmp,"fat0:"); //0.66b
			strcpy(tmp+5,utf8);
			ini_puts("Save Info","lastLoaded",tmp,"/__aio/lastsave.ini");
			getsfnlfn(utf8,tmp+5,NULL);
			ini_puts("Save Info","shortPathNDS",tmp,"/__aio/lastsave.ini");
			ini_putl("system","autorunWithLastRom",1,"/__aio/globalsettings.ini");
		}else if(useak2==1&&!memcmp(dldiid,"RPG",3)){
			char tmp[776];
			strcpy(tmp,"fat1:");
			strcpy(tmp+5,utf8);
			ini_puts("Save Info","lastLoaded",tmp,"/__ak2/lastsave.ini");
			getsfnlfn(utf8,tmp+5,NULL);
			ini_puts("Save Info","shortPathNDS",tmp,"/__ak2/lastsave.ini");
			ini_putl("system","autorunWithLastRom",1,"/__ak2/globalsettings.ini");
		}else if((usewoodr4==1&&!memcmp(dldiid,"R4TF",4)) || (usem3==3&&!memcmp(dldiid,"M3DS",4))){
			char tmp[776];
			strcpy(tmp,"fat0:"); //0.63b1
			strcpy(tmp+5,utf8);
			ini_puts("Save Info","lastLoaded",tmp,"/__rpg/lastsave.ini");
			//getsfnlfn(utf8,tmp+5,NULL);
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
				ini_putl("Config","r4_alpha_accepted",1,"/moonshl2/extlink/inilink.ini");
			}
			if(!stat("/default.nds",&st)){_consolePrint("/default.nds exists. Make sure iniclear is executed.\n");die();}
			strcpy(target,"/default.nds");
			rename(utf8,target);
			ini_puts("Config","r4_recover",utf8,"/moonshl2/extlink/inilink.ini");
			strcpy(getextname(utf8),".sav");
			strcpy(getextname(target),".sav");
			rename(utf8,target);
		}else if(usewoodr4==-1&&!memcmp(dldiid,"R4TF",4)){
			char tmp[776];
			strcpy(tmp,"fat1:");
			strcpy(tmp+5,utf8);
			ini_puts("Save Info","lastLoaded",tmp,"/system/lastsave.ini");
			//getsfnlfn(utf8,tmp+5,NULL);
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
				ini_putl("Config","eos_alpha_accepted",1,"/moonshl2/extlink/inilink.ini");
			}
			if(!stat("/!!!extlink",&st)){_consolePrint("/!!!extlink exists. Make sure iniclear is executed.\n");die();}
			mkdir("/!!!extlink",0755);
			strcpy(target,"/!!!extlink/");
			char *filename=target+strlen(target);
			SplitItemFromFullPathAlias(utf8,NULL,filename);
			rename(utf8,target);
			ini_puts("Config","eos_recover",utf8,"/moonshl2/extlink/inilink.ini");
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
			getsfnlfn(utf8,tmp+5,NULL);
			ini_puts("Dir Info","last dir",tmp,"/_dsone/lastsave.ini");
			ini_putl("system","enterLastDirWhenBoot",1,"/_dsone/globalsettings.ini");
#endif
		}else if(!memcmp(dldiid,"DSI2",4)){
			char dstwodir[768];
			ini_gets("Config","DSTwoDir","",dstwodir,768,"/moonshl2/extlink/inilink.ini");
			if(!*dstwodir){
				_consolePrint("DSTwoDir is null. Halt.\n");die();
			}
			char *dstwodirfilename=dstwodir+strlen(dstwodir);
			if(*(dstwodirfilename-1)!='/'){*dstwodirfilename='/';dstwodirfilename++;}
			char tmp[776];
			strcpy(tmp,"fat1:");
			strcpy(tmp+5,utf8);
			strcpy(dstwodirfilename,"lastsave.ini");
			ini_puts("Dir Info","fullName",tmp,dstwodir);
			getsfnlfn(utf8,tmp+5,NULL);
			ini_puts("Dir Info","shortName",tmp,dstwodir);
			strcpy(dstwodirfilename,"globalsettings.ini");
			ini_putl("system","lastRomRun",1,dstwodir);
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
				ini_putl("Config","ismart_alpha_accepted",1,"/moonshl2/extlink/inilink.ini");
			}
			if(!stat("/defaultn.nds",&st)||!stat("/defaultx.nds",&st)||!stat("/defaulty.nds",&st)||!stat("/save/defaulty.sav",&st)){_consolePrint("/default{n,x,y}.nds exists. Make sure iniclear is executed.\n");die();}
			strcpy(target,"/defaultn.nds");
			rename(utf8,target);
			ini_puts("Config","ismart_recover_nds",utf8,"/moonshl2/extlink/inilink.ini");
			strcpy(getextname(utf8),".sav");
			sprintf(utf8_2,"/save/%s",getfilename(utf8));
			strcpy(target,"/save/defaultn.sav");
			rename(utf8_2,target);
			ini_puts("Config","ismart_recover_sav",utf8_2,"/moonshl2/extlink/inilink.ini");
			//copy("/iniclear.nds","/defaultn.nds");
/*
			_consolePrint("Press and HOLD Y button...\n");
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
				ini_putl("Config","ez_alpha_accepted",1,"/moonshl2/extlink/inilink.ini");
			}
			if(!stat("/!!!extlink",&st)){_consolePrint("/!!!extlink exists. Make sure iniclear is executed.\n");die();}
			mkdir("/!!!extlink",0755);
			strcpy(target,"/!!!extlink/");
			char *filename=target+strlen(target);
			SplitItemFromFullPathAlias(utf8,NULL,filename);
			rename(utf8,target);
			ini_puts("Config","ez_recover",utf8,"/moonshl2/extlink/inilink.ini");
		}else if(!memcmp(dldiid,"CEVO",4)){
			f=fopen("/CycloDS/config.xml","r+b");
			if(!f){_consolePrint("cannot open /CycloDS/config.xml.\n");die();}
			u32 size=filelength(fileno(f));
			char *p=(char*)malloc(size+1);
			if(!p){fclose(f);_consolePrint("cannot alloc memory.\n");die();}
			fread(p,1,size,f);p[size]=0;
			rewind(f);
			char *str1="<lastFileExecuted>",*str2="</lastFileExecuted>";
			int idx1=strstrindex(p,str1,0),idx2=strstrindex(p,str2,0);
			if(idx1==-1||idx2==-1){fclose(f);free(p);_consolePrint("/CycloDS/config.xml isn't good.\n");die();}
			fwrite(p,1,idx1+strlen(str1),f);
			fwrite(utf8,1,strlen(utf8),f);
			fwrite(p+idx2,1,strlen(p+idx2),f);
			fclose(f);
			//_consolePrint("/CycloDS/config.xml modified.\nHold L+R in next boot.\nPress A to shutdown.\n");
			//for(swiWaitForVBlank();;swiWaitForVBlank()){
			//	if(IPCZ->keysdown&KEY_A)IPCZ->cmd=Shutdown;
			//}

			str1="<lastPlayedButtons>",str2="</lastPlayedButtons>";
			idx1=strstrindex(p,str1,0),idx2=strstrindex(p,str2,0);
			if(idx1==-1||idx2==-1){free(p);_consolePrint("/CycloDS/config.xml isn't good.\n");die();}
			p[idx2]=p[idx2+1]=0;
			char *buttons=p+(idx1+strlen(str1));
			_consolePrintf("/CycloDS/config.xml lastPlayedButtons=%s\n",buttons);

			int i=0;
			for(;buttons[i];i++){
				if(buttons[i]==',')buttons[i]=0;
			}
			u32 keys=0;
			for(;*buttons;){
				if(!strcasecmp(buttons,"A"))keys|=KEY_A;
				if(!strcasecmp(buttons,"B"))keys|=KEY_B;
				if(!strcasecmp(buttons,"X"))keys|=KEY_X;
				if(!strcasecmp(buttons,"Y"))keys|=KEY_Y;
				if(!strcasecmp(buttons,"L"))keys|=KEY_L;
				if(!strcasecmp(buttons,"R"))keys|=KEY_R;
				if(!strcasecmp(buttons,"START"))keys|=KEY_START;
				if(!strcasecmp(buttons,"SELECT"))keys|=KEY_SELECT;
				if(!strcasecmp(buttons,"UP"))keys|=KEY_UP;
				if(!strcasecmp(buttons,"DOWN"))keys|=KEY_DOWN;
				if(!strcasecmp(buttons,"LEFT"))keys|=KEY_LEFT;
				if(!strcasecmp(buttons,"RIGHT"))keys|=KEY_RIGHT;
				buttons+=strlen(buttons)+1;
			}
			free(p);
			_consolePrint("/CycloDS/config.xml modified.\nHold autoboot button (in default L+R)... ");
			for(swiWaitForVBlank();;swiWaitForVBlank()){
				if((IPCZ->keysheld&keys)==keys)break;
			}

			_consolePrint("OK.\nRebooting to /CycloDS/reset.mse...\n");
			ret_menu9_Gen("/CycloDS/reset.mse");
		}else{
			ini_puts("YSMenu","AUTO_BOOT",utf8,ininame);
		}
	}
}
	_consolePrint("Done.\n");

	_consolePrint("Rebooting... \n");
	BootLibrary(ysmenu,1,NULL);
	// ^^^^^^^^^^^^ add 2008.03.30 kzat3
}
