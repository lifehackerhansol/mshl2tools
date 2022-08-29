#include "../../libprism/libprism.h"
const u16 bgcolor=RGB15(4,0,12);

void Main(){
	char loader[768],lang[10],config[768],target[768];
	u8 head[512];
	//TExtLinkBody extlink;
	FILE *f;
	int type;

	_consolePrintf(
		"m3loader\n"
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

	//_consolePrint("Waiting... ");
	//sleep(1);
	//_consolePrint("Done.\n");

	_consolePrint("initializing FAT... ");
	if(!disc_mount()){_consolePrint("failed.\n");die();}
	_consolePrint("done.\n");

	_consolePrint("Opening frontend... ");
	if(!readFrontend(target)){_consolePrint("Failed.\n");die();}
	_consolePrint("Done.\n");

	_consolePrint("Configuring loader... ");
	type=ini_getl("m3loader","Type",0,"/moonshl2/extlink/m3loader.ini");
	switch(type){
		case 0:
			ini_gets("m3loader","TouchPodLang","eng",lang,10,"/moonshl2/extlink/m3loader.ini");
			strcpy(loader,"/system/minigame.");
			strcat(loader,lang);
			strcpy(config,"/system/minibuff.swp");
		break;
		case 1:
			strcpy(loader,"/_system_/_sys_data/r4_firends.ext");
			strcpy(config,"/_system_/_sys_data/r4_homebrew.ini");
		break;
		case 2:
			ini_gets("m3loader","TouchPodLang","eng",lang,10,"/moonshl2/extlink/m3loader.ini");
			strcpy(loader,"/system/G003_minigame.");
			strcat(loader,lang);
			strcpy(config,"/system/minibuff.swp");
		break;
	}
	_consolePrint("Done.\n");

	_consolePrint("Setting target... ");
	if(!(f=fopen(target,"rb"))){_consolePrint("Failed.\n");die();}
	//{struct stat st;fstat(fileno(f),&st);size=st.st_size;}
	//if(size<0x200){fclose(f);goto fail;}
	fread(head,1,0x200,f);
	fclose(f);
	if(isHomebrew(head)){
		_consolePrint("Homebrew detected.\n"); //Using internal loader. Allocating %s...\n",target);
		if((type==0||type==2)&&!ini_getl("m3loader","UseR4iRTSForHomebrew",0,"/moonshl2/extlink/m3loader.ini")){
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

	if(type==1){
		char dir[768];SplitItemFromFullPathAlias(target,dir,NULL);
		if(dir[1])strcat(dir,"/");
		if(!(f=fopen(config,"wb"))){_consolePrint("Failed.\n");die();}
		fwrite(dir,1,512,f);
		fwrite(target,1,512,f);
		fclose(f);
	}else{
		memset(target+255,0,1+36);
		if(!(f=fopen(config,"wb"))){_consolePrint("Failed.\n");die();}
		fwrite(target,1,292/*256*/,f); //fixme
		fclose(f);
	}
	strcpy(target+strlen(target)-3,"sav");
	libprism_touch(target);

	_consolePrint("Done.\n");
	BootDSBooter(loader);
	//IPCZ->cmd=ResetBootlib;
	//runDSBooter(loader);
	//die(); //Rudolph's loader cannot load r4_firends.ext(white-out)
}
