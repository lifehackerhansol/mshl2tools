#include "../../libprism/libprism.h"
const u16 bgcolor=RGB15(4,0,12);

void Main(){
	char loader[768],lang[10],config[768],dir[768],target[768];
	u8 head[512];
	//TExtLinkBody extlink;
	FILE *f;
	int type;

	IPCZ->cmd=0;
	_consolePrintf(
		"m3loader beta\n"
		"reset_mse_06b_for_ak2 by Moonlight, Rudolph, kzat3\n"
		"moonshellreset by Moonlight\n"
		//"dldipatch aka dlditool public domain under CC0.\n"
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

	//_consolePrintf("Waiting... ");
	//sleep(1);
	//_consolePrintf("Done.\n");

	_consolePrintf("initializing libfat... ");
	if(!fatInitDefault()){_consolePrintf("failed.\n");die();}
	_consolePrintf("done.\n");

	_consolePrintf("Loading /loadfile.dat... ");
	f=fopen("/loadfile.dat","rb");
	if(!f){_consolePrintf("Failed.\n");die();}
	myfgets(target,768,f);
	fclose(f);
	remove("/loadfile.dat");
	_consolePrintf("Done.\n");

	_consolePrintf("Configuring loader... ");
	type=ini_getl("m3loader","Type",0,"/MOONSHL2/EXTLINK/M3LOADER.INI");
	if(type){
		strcpy(loader,"/_system_/_sys_data/r4_firends.ext");
		strcpy(config,"/_system_/_sys_data/r4_homebrew.ini");
	}else{
		ini_gets("m3loader","TouchPodLang","eng",lang,10,"/MOONSHL2/EXTLINK/M3LOADER.INI");
		strcpy(loader,"/system/minigame.");
		strcat(loader,lang);
		strcpy(config,"/system/minibuff.swp");
	}
	_consolePrintf("Done.\n");

	_consolePrintf("Setting target... ");
	//_FAT_directory_ucs2tombs(target,extlink.DataFullPathFilenameUnicode,768);
	if(!(f=fopen(target,"rb"))){_consolePrintf("Failed.\n");die();}
	//{struct stat st;fstat(fileno(f),&st);size=st.st_size;}
	//if(size<0x200){fclose(f);goto fail;}
	fread(head,1,0x200,f);
	fclose(f);
	if(isHomebrew(head)){
		_consolePrintf("Homebrew detected.\n"); //Using internal loader. Allocating %s...\n",target);
		if(!type&&!ini_getl("m3loader","UseR4iRTSForHomebrew",0,"/MOONSHL2/EXTLINK/M3LOADER.INI")){
			_consolePrintf("Falling back to internal loader. Allocating %s...\n",target);
			if(!ret_menu9_Gen(target))die();
		}

		//if(type){
			strcpy(loader,"/_system_/_sys_data/r4_homebrew.ext");
			strcpy(config,"/_system_/_sys_data/r4_homebrew.ini");
			type=1;
		//}else{
		//	strcpy(loader,"/system/homebrew.");
		//	strcat(loader,lang);
		//}
	}

	if(type){
		SplitItemFromFullPathAlias(target,dir,NULL); //head);
		//_FAT_directory_ucs2tombs(dir,extlink.DataPathUnicode,768);
		if(dir[1])strcat(dir,"/");
		if(!(f=fopen(config,"wb"))){_consolePrintf("Failed.\n");die();}
		fwrite(dir,1,512,f);
		fwrite(target,1,512,f);
		fclose(f);
	}else{
		memset(target+255,0,1+36);
		if(!(f=fopen(config,"wb"))){_consolePrintf("Failed.\n");die();}
		fwrite(target,1,292/*256*/,f); //fixme
		fclose(f);
	}

	_consolePrintf("Done.\n");
	BootDSBooter(loader);
	//IPCZ->cmd=ResetBootlib;
	//runDSBooter(loader);
	//die(); //Rudolph's loader cannot load r4_firends.ext(white-out)
}
