#include "../../libprism/libprism.h"
const u16 bgcolor=RGB15(10,0,10);
const int useARM7Bios=0;

void Main(){
	char ysmenu[768],tmp[768];
	char dldiid[5];
	unsigned char *dldiFileData=DLDIDATA;

	struct stat st;

	_consolePrintf(
		"iniclear launcher\n"
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

	_consolePrint("Initializing FAT... ");
	if(!disc_mount()){_consolePrint("Failed.\n");die();}
	_consolePrint("Done.\n");

	_consolePrint("Clearing ini... ");
/*{
	char bootini[30],ininame[768],defaultdir[768];
	ini_gets("YSBoot","YSMenu","",ysmenu,768,"/YSMENU/YSBOOT.INI");
	if(*ysmenu){strcpy(bootini,"/YSMENU/YSBOOT.INI");goto done;}
	ini_gets("YSBoot","YSMenu","",ysmenu,768,"/_SYSTEM_/YSBOOT.INI");
	if(*ysmenu){strcpy(bootini,"/_SYSTEM_/YSBOOT.INI");goto done;}
	ini_gets("YSBoot","YSMenu","",ysmenu,768,"/TTMENU/YSBOOT.INI");
	if(*ysmenu){strcpy(bootini,"/TTMENU/YSBOOT.INI");goto done;}
	ini_gets("YSBoot","YSMenu","",ysmenu,768,"/__AK2/YSBOOT.INI");
	if(*ysmenu){strcpy(bootini,"/__AK2/YSBOOT.INI");goto done;}
	ini_gets("YSBoot","YSMenu","",ysmenu,768,"/__RPG/YSBOOT.INI");
	if(*ysmenu){strcpy(bootini,"/__RPG/YSBOOT.INI");goto done;}
	ini_gets("YSBoot","YSMenu","",ysmenu,768,"/YSBOOT.INI");
	if(*ysmenu){strcpy(bootini,"/YSBOOT.INI");goto done;}

	done:
	if(*bootini){
		ini_gets("YSBoot","YSini","",ininame,768,bootini);
		ini_gets("YSBoot","DefaultDir","",defaultdir,768,bootini);
		ini_puts("YSMenu","AUTO_BOOT",defaultdir,ininame);
	}else{ //fallback
		strcpy(ysmenu,"/YSMENU/YSMENU.NDS");
		//ininame is dummy here
		ini_gets("YSMenu","AUTO_BOOT","",ininame,768,"/YSMENU/YSMENU.INI");
		if(*ininame)ini_puts("YSMenu","AUTO_BOOT","","/YSMENU/YSMENU.INI");
		ini_gets("YSMenu","AUTO_BOOT","",ininame,768,"/_SYSTEM_/YSMENU.INI");
		if(*ininame)ini_puts("YSMenu","AUTO_BOOT","","/_SYSTEM_/YSMENU.INI");
		ini_gets("YSMenu","AUTO_BOOT","",ininame,768,"/TTMENU/YSMENU.INI");
		if(*ininame)ini_puts("YSMenu","AUTO_BOOT","","/TTMENU/YSMENU.INI");
		ini_gets("YSMenu","AUTO_BOOT","",ininame,768,"/__AK2/YSMENU.INI");
		if(*ininame)ini_puts("YSMenu","AUTO_BOOT","","/__AK2/YSMENU.INI");
		ini_gets("YSMenu","AUTO_BOOT","",ininame,768,"/__RPG/YSMENU.INI");
		if(*ininame)ini_puts("YSMenu","AUTO_BOOT","","/__RPG/YSMENU.INI");
		ini_gets("YSMenu","AUTO_BOOT","",ininame,768,"/YSMENU.INI");
		if(*ininame)ini_puts("YSMenu","AUTO_BOOT","","/YSMENU.INI");
	}
}*/
{
	char ininame[768],defaultdir[768];
	int useak2=0,useeos=0/*,useeos2=0*/,usewoodr4=0,usem3=0,useiniclear=0;
	//int r4_alpha_accepted=0,m3sakura_alpha_accepted=0,
	int m3sakura_iniclear_method=0;
	tmp[0]=0;

	ini_gets("Config","YSMenu","/YSMENU/YSMENU.NDS",ysmenu,768,"/moonshl2/extlink/inilink.ini");
	ini_gets("Config","YSini","/YSMENU/YSMENU.INI",ininame,768,"/moonshl2/extlink/inilink.ini");
	ini_gets("Config","DefaultDir","",defaultdir,768,"/moonshl2/extlink/inilink.ini");
	if(!memcmp(dldiid,"SCDS",4)){
		useeos=0;//ini_getl("Config","UseEOS",0,"/moonshl2/extlink/inilink.ini");
		//SCDS_SetSDHCModeForDSTT();
		ini_gets("Config","UseEOS_iniclear","",tmp,768,"/moonshl2/extlink/inilink.ini");
		if(tmp[0]=='/')goto copy;
		useiniclear=tmp[0]?strtol(tmp,NULL,10):useeos;
		//if(useiniclear==0)ini_gets("Config","SCYSMenu",ysmenu,ysmenu,768,"/moonshl2/extlink/inilink.ini");
		if(useiniclear==1)ini_gets("Config","DSOneEOS","/EOS.NDS",ysmenu,768,"/moonshl2/extlink/inilink.ini");
	}
	if(!memcmp(dldiid,"RPGS",4)||!memcmp(dldiid,"R4DS",4)||!memcmp(dldiid,"_R4i",4)||!memcmp(dldiid,"RPGN",4)||!memcmp(dldiid,"XXXX",4)){
		useak2=ini_getl("Config","UseAK2",2,"/moonshl2/extlink/inilink.ini");
		ini_gets("Config","UseAK2_iniclear","",tmp,768,"/moonshl2/extlink/inilink.ini");
		if(!memcmp(dldiid,"XXXX",4))memcpy(dldiid,"RPGS",4),useak2=2,tmp[0]=0;
		if(strstr((char*)dldiFileData+friendlyName,"RPG")){
			useak2=ini_getl("Config","UseRPG",2,"/moonshl2/extlink/inilink.ini");
			ini_gets("Config","UseRPG_iniclear","",tmp,768,"/moonshl2/extlink/inilink.ini");
		}
		if(tmp[0]=='/')goto copy;
		useiniclear=tmp[0]?strtol(tmp,NULL,10):useak2;
		if(!memcmp(dldiid,"R4DS",4)||!memcmp(dldiid,"_R4i",4)){
			memcpy(dldiid,"RPGS",4);
			if(!useiniclear)memcpy((char*)dldiFileData+ioType,"RPGS",4);
		}
		if(useiniclear==3)ini_gets("Config","WoodRPG","/AKMENU.NDS",ysmenu,768,"/moonshl2/extlink/inilink.ini");
		if(useiniclear==2)ini_gets("Config","AKAIO","/AKMENU.NDS",ysmenu,768,"/moonshl2/extlink/inilink.ini");
		if(useiniclear==1)ini_gets("Config","AKMENU","/AKMENU.NDS",ysmenu,768,"/moonshl2/extlink/inilink.ini");
	}
	if(!memcmp(dldiid,"M3DS",4)||!memcmp(dldiid,"iTDS",4)||!memcmp(dldiid,"R4_I",4)){
		usem3=ini_getl("Config","UseM3",0,"/moonshl2/extlink/inilink.ini");
		ini_gets("Config","UseM3_iniclear","",tmp,768,"/moonshl2/extlink/inilink.ini");
		if(tmp[0]=='/')goto copy;
		useiniclear=tmp[0]?strtol(tmp,NULL,10):usem3;
		if(!memcmp(dldiid,"iTDS",4)||!memcmp(dldiid,"R4_I",4)){
			memcpy(dldiid,"M3DS",4);
			if(!useiniclear)memcpy((char*)dldiFileData+ioType,"M3DS",4);
		}
		if(useiniclear==0)ini_gets("Config","YSM3",ysmenu,ysmenu,768,"/moonshl2/extlink/inilink.ini");
		if(useiniclear==1)ini_gets("Config","TouchPod","/TOUCHPOD.NDS",ysmenu,768,"/moonshl2/extlink/inilink.ini");
		if(useiniclear==2)ini_gets("Config","M3Sakura","/misakura.nds",ysmenu,768,"/moonshl2/extlink/inilink.ini");
		if(useiniclear==3)ini_gets("Config","WoodM3","/WOODM3.NDS",ysmenu,768,"/moonshl2/extlink/inilink.ini");
	}
	if(!memcmp(dldiid,"R4TF",4)){
		usewoodr4=ini_getl("Config","UseWoodR4",1,"/moonshl2/extlink/inilink.ini");
		ini_gets("Config","UseWoodR4_iniclear","",tmp,768,"/moonshl2/extlink/inilink.ini");
		if(tmp[0]=='/')goto copy;
		useiniclear=tmp[0]?strtol(tmp,NULL,10):usewoodr4;
		if(useiniclear==1)ini_gets("Config","WoodR4","/WOODR4.NDS",ysmenu,768,"/moonshl2/extlink/inilink.ini");
		if(useiniclear==2)ini_gets("Config","R4Kernel","/r4kern.nds",ysmenu,768,"/moonshl2/extlink/inilink.ini");
		if(useiniclear==-1)ini_gets("Config","R4i3D","/R4i3D.NDS",ysmenu,768,"/moonshl2/extlink/inilink.ini");
	}
	if(!memcmp(dldiid,"DEMO",4))memcpy((char*)dldiFileData+ioType,"TTIO",4);
	if(!memcmp(dldiid,"DSI2",4))ini_gets("Config","DSTwo","/DSTWO.NDS",ysmenu,768,"/moonshl2/extlink/inilink.ini");
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

	goto notcopy;
copy:
	strcpy(ysmenu,tmp);
notcopy:

	//dstwo_alpha_accepted=ini_getl("Config","dstwo_alpha_accepted",0,"/moonshl2/extlink/inilink.ini");
	//r4_alpha_accepted=ini_getl("Config","r4_alpha_accepted",0,"/moonshl2/extlink/inilink.ini");
	//m3sakura_alpha_accepted=ini_getl("Config","m3sakura_alpha_accepted",0,"/moonshl2/extlink/inilink.ini");
	m3sakura_iniclear_method=ini_getl("Config","m3sakura_iniclear_method",0,"/moonshl2/extlink/inilink.ini");

	//if(!autoboot){
	//	ini_puts("YSMenu","AUTO_BOOT","",ininame);
	//}else
	{
		//char utf8[768];
		//ucs2tombs(utf8,extlink.DataFullPathFilenameUnicode);
		if(usem3==1&&!memcmp(dldiid,"M3DS",4)){
			char buf[1024];
			FILE *in,*out;
			in=fopen("/system/boot.ini","rb");
			if(!in){_consolePrint("Cannot open /system/boot.ini. Halt.");die();}
			out=fopen("/system/boot.in~","wb");
			while(myfgets(buf,1024,in)){
				if(!memcmp(buf,"[DirectRun]",11)){
					strcpy(buf,"[DirectRun]=\r\n");
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
			if(!stat("/defaulty.nds",&st)){
				char fullname[768],tmpname[768];
				strcpy(tmpname,"/defaulty.nds");
				ini_gets("Config","m3sakura_recover","",fullname,768,"/moonshl2/extlink/inilink.ini");
				rename(tmpname,fullname);
				strcpy(getextname(tmpname),".sav");
				strcpy(getextname(fullname),".sav");
				rename(tmpname,fullname);
				ini_puts("Config","m3sakura_recover","","/moonshl2/extlink/inilink.ini");
				//rm_rf("/extlink_eos/");
				//unlink("/extlink_eos");
				if(m3sakura_iniclear_method){
					unlink("/defaultn.nds");
					_consolePrint("iniclear OK. Please restart NDS.\n");
					int i=0;for(;i<10;i++)swiWaitForVBlank();
					IPCZ->cmd=Shutdown;
				}
			}
		}else if(!memcmp(dldiid,"G003",4)){
			if(!stat("/defaulty.nds",&st)){
				char fullname[768],tmpname[768];
				strcpy(tmpname,"/defaulty.nds");
				ini_gets("Config","m3sakura_recover","",fullname,768,"/moonshl2/extlink/inilink.ini");
				rename(tmpname,fullname);
				strcpy(getextname(tmpname),".sav");
				strcpy(getextname(fullname),".sav");
				rename(tmpname,fullname);
				ini_puts("Config","m3sakura_recover","","/moonshl2/extlink/inilink.ini");
				//rm_rf("/extlink_eos/");
				//unlink("/extlink_eos");
				if(m3sakura_iniclear_method){
					unlink("/defaultn.nds");
					_consolePrint("iniclear OK. Please restart NDS.\n");
					int i=0;for(;i<10;i++)swiWaitForVBlank();
					IPCZ->cmd=Shutdown;
				}
			}
		}else if(useak2==3&&!memcmp(dldiid,"RPG",3)){
			//char tmp[776];
			//strcpy(tmp,"fat1:");
			//strcpy(tmp+5,utf8);
			//ini_puts("Save Info","lastLoaded",tmp,"/__rpg/lastsave.ini");
			//strcpy(tmp+5,extlink.DataFullPathFilenameAlias);
			//ini_puts("Save Info","shortPathNDS",tmp,"/_rpg/lastsave.ini");
			ini_putl("system","autorunWithLastRom",0,"/__rpg/globalsettings.ini");
		}else if(useak2==2&&!memcmp(dldiid,"RPG",3)){
			//char tmp[776];
			//strcpy(tmp,"fat0:");
			//strcpy(tmp+5,utf8);
			//ini_puts("Save Info","lastLoaded",tmp,"/__aio/lastsave.ini");
			//strcpy(tmp+5,extlink.DataFullPathFilenameAlias);
			//ini_puts("Save Info","shortPathNDS",tmp,"/__aio/lastsave.ini");
			ini_putl("system","autorunWithLastRom",0,"/__aio/globalsettings.ini");
		}else if(useak2==1&&!memcmp(dldiid,"RPG",3)){
			//char tmp[776];
			//strcpy(tmp,"fat1:");
			//strcpy(tmp+5,utf8);
			//ini_puts("Save Info","lastLoaded",tmp,"/__ak2/lastsave.ini");
			//strcpy(tmp+5,extlink.DataFullPathFilenameAlias);
			//ini_puts("Save Info","shortPathNDS",tmp,"/__ak2/lastsave.ini");
			ini_putl("system","autorunWithLastRom",0,"/__ak2/globalsettings.ini");
		}else if((usewoodr4==1&&!memcmp(dldiid,"R4TF",4)) || (usem3==3&&!memcmp(dldiid,"M3DS",4))){
			//char tmp[776];
			//strcpy(tmp,"fat0:");
			//strcpy(tmp+5,utf8);
			//ini_puts("Save Info","lastLoaded",tmp,"/__rpg/lastsave.ini");
			//strcpy(tmp+5,extlink.DataFullPathFilenameAlias);
			//ini_puts("Save Info","s/MOONSHL2/EXTLINKhortPathNDS",tmp,"/__rpg/lastsave.ini");
			ini_putl("system","autorunWithLastRom",0,"/__rpg/globalsettings.ini");
		}else if(usewoodr4==2&&!memcmp(dldiid,"R4TF",4)){
			if(!stat("/default.nds",&st)){
				char fullname[768],tmpname[768];
				strcpy(tmpname,"/default.nds");
				ini_gets("Config","r4_recover","",fullname,768,"/moonshl2/extlink/inilink.ini");
				rename(tmpname,fullname);
				strcpy(getextname(tmpname),".sav");
				strcpy(getextname(fullname),".sav");
				rename(tmpname,fullname);
				ini_puts("Config","r4_recover","","/moonshl2/extlink/inilink.ini");
				//rm_rf("/extlink_eos/");
				//unlink("/extlink_eos");
			}
		}else if(usewoodr4==-1&&!memcmp(dldiid,"R4TF",4)){
			//char tmp[776];
			//strcpy(tmp,"fat1:");
			//strcpy(tmp+5,utf8);
			//ini_puts("Save Info","lastLoaded",tmp,"/system/lastsave.ini");
			//strcpy(tmp+5,extlink.DataFullPathFilenameAlias);
			//ini_puts("Save Info","shortPathNDS",tmp,"/system/lastsave.ini");
			ini_putl("system","autorunWithLastRom",0,"/system/globalsettings.ini");
		}else if(useeos&&!memcmp(dldiid,"SCDS",4)){
			if(!stat("/!!!extlink",&st)){
				char fullname[768],tmpname[768];
				strcpy(tmpname,"/!!!extlink/");
				char *filename=tmpname+strlen(tmpname);
				ini_gets("Config","eos_recover","",fullname,768,"/moonshl2/extlink/inilink.ini");
				if(!*fullname){
					_consolePrint("Cannot find recover filename in eos_recover. Halt.\n");die();
				}
				SplitItemFromFullPathAlias(fullname,NULL,filename);
				rename(tmpname,fullname);
				strcpy(getextname(tmpname),".sav");
				strcpy(getextname(fullname),".sav");
				rename(tmpname,fullname);
				strcpy(getextname(tmpname),".dso");
				strcpy(getextname(fullname),".dso");
				rename(tmpname,fullname);
				ini_puts("Config","eos_recover","","/moonshl2/extlink/inilink.ini");
				rm_rf("/!!!extlink/");
				unlink("/!!!extlink");

				strcpy(getextname(fullname),".nds");
				strcpy(tmpname,"fat1:");
				getsfnlfn(fullname,tmpname+5,NULL);
				ini_puts("Dir Info","last dir",fullname,"/_dsone/lastsave.ini");
			}
			//strcpy(tmp,"fat1:");
			//strcpy(tmp+5,utf8);
			//ini_puts("Dir Info","last dir",tmp,"/_dsone/lastsave.ini");
			//strcpy(tmp+5,extlink.DataFullPathFilenameAlias);
			//ini_puts("Dir Info","last dir",tmp,"/_dsone/lastsave.ini");
			//ini_putl("system","enterLastDirWhenBoot",0,"/_dsone/globalsettings.ini");
		}else if(!memcmp(dldiid,"DSI2",4)){
			char dstwodir[768];
			ini_gets("Config","DSTwoDir","",dstwodir,768,"/moonshl2/extlink/inilink.ini");
			if(!*dstwodir){
				_consolePrint("DSTwoDir is null. Halt.\n");die();
			}
			char *dstwodirfilename=dstwodir+strlen(dstwodir);
			if(*(dstwodirfilename-1)!='/'){*dstwodirfilename='/';dstwodirfilename++;}

			//char tmp[776];
			//strcpy(tmp,"fat1:");
			//strcpy(tmp+5,utf8);
			//ini_puts("Dir Info","last dir",tmp,"/_dstwo/lastsave.ini");
			//strcpy(tmp+5,extlink.DataFullPathFilenameAlias);
			//ini_puts("Dir Info","last dir",tmp,"/_dstwo/lastsave.ini");
			strcpy(dstwodirfilename,"globalsettings.ini");
			ini_putl("system","lastRomRun",0,dstwodir);
		}else if(!memcmp(dldiid,"IMAT",4)){
			if(!stat("/defaultn.nds",&st)){
				char fullname[768],tmpname[768];
				strcpy(tmpname,"/defaultn.nds");
				ini_gets("Config","ismart_recover_nds","",fullname,768,"/moonshl2/extlink/inilink.ini");
				rename(tmpname,fullname);
				strcpy(tmpname,"/save/defaultn.sav");
				ini_gets("Config","ismart_recover_sav","",fullname,768,"/moonshl2/extlink/inilink.ini");
				rename(tmpname,fullname);
				ini_puts("Config","ismart_recover_nds","","/moonshl2/extlink/inilink.ini");
				ini_puts("Config","ismart_recover_sav","","/moonshl2/extlink/inilink.ini");
			}
		}else if(!memcmp(dldiid,"EZ5H",4)||!memcmp(dldiid,"EZ5i",4)){
			if(!stat("/!!!extlink",&st)){
				char fullname[768],tmpname[768];
				strcpy(tmpname,"/!!!extlink/");
				char *filename=tmpname+strlen(tmpname);
				ini_gets("Config","ez_recover","",fullname,768,"/moonshl2/extlink/inilink.ini");
				if(!*fullname){
					_consolePrint("Cannot find recover filename in ez_recover. Halt.\n");die();
				}
				SplitItemFromFullPathAlias(fullname,NULL,filename);
				rename(tmpname,fullname);
				ini_puts("Config","ez_recover","","/moonshl2/extlink/inilink.ini");
				rm_rf("/!!!extlink/");
				unlink("/!!!extlink");
			}
		//}else if(!memcmp(dldiid,"CEVO")){ nothing is required here
		}else{
			//ini_puts("YSMenu","AUTO_BOOT","",ininame);
			ini_puts("YSMenu","AUTO_BOOT",defaultdir,ininame);
		}
	}
}
	_consolePrint("Done.\n");

_consolePrint("Release all keys.\n");
while(IPCZ->keysheld)swiWaitForVBlank();

	// vvvvvvvvvvv add 2008.03.30 kzat3
	_consolePrintf("Rebooting to %s...\n",ysmenu);
	BootNDSROM(ysmenu);
	// ^^^^^^^^^^^^ add 2008.03.30 kzat3
}
