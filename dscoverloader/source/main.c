#include <sys/stat.h>
#include "../../libprism/libprism.h"
const u16 bgcolor=RGB15(6,10,0);
const int useARM7Bios=0;

void Main(){
	FILE *f;
	//TExtLinkBody extlink;
	int size;//,hbmode=0;
	unsigned char head[0x200];
	char dldiid[5];

	_consolePrintf(
		"Alternative DSision loader modified Extlink Wrapper\n"
		"VRAM bootlib by Chishm\n"
		"dldipatch aka dlditool public domain under CC0.\n"
		"%s\n%s\n\n",
		ROMDATE,ROMENV
	);

	{
		unsigned char *dldiFileData=DLDIDATA;
		memcpy(dldiid,(char*)dldiFileData+ioType,4);
		dldiid[4]=0;
		_consolePrintf("DLDI ID: %s\n",dldiid);
		_consolePrintf("DLDI Name: %s\n\n",(char*)dldiFileData+friendlyName);
	}

	_consolePrint("Initializing FAT... ");
	if(!disc_mount()){_consolePrint("Failed.\n");die();}
	_consolePrint("Done.\n");

{
	char loader[768],filename[768],target[768];

	_consolePrint("Opening frontend... ");
	if(!readFrontend(filename)){_consolePrint("Failed.\n");die();}
	_consolePrint("Done.\n");

/*
	_consolePrint("Loading /d_system/loader.cfg... ");
	f=fopen("/d_system/loader.cfg","rb");
	if(!f){_consolePrint("Failed.\n");die();}
	myfgets(loader,768,f);
	fclose(f);
	_consolePrint("Done.\n");
*/
	_consolePrint("Setting loader... ");
	getExtlinkWrapperLoaderName(loader);
	if(!(f=fopen(filename,"rb"))){_consolePrint("Failed.\n");die();}
	{struct stat st;fstat(fileno(f),&st);size=st.st_size;}
	if(size<0x200){fclose(f);_consolePrint("Failed.\n");die();}
	fread(head,1,0x200,f);
	if(!isHomebrew(head)){
		//ucs2tombs(target,extlink.DataFullPathFilenameUnicode);
			strcpy(target,filename);goto target_set;
	}else if(!strcmp((char*)head+0x1e0,"mshl2wrap link")){
		unsigned int s=(head[0x1f0]<<24)+(head[0x1f1]<<16)+(head[0x1f2]<<8)+head[0x1f3];
		_consolePrint("Detected mshl2wrap link.\n");
		if(size<s+256*3){fclose(f);_consolePrint("Failed.\n");die();}
		fseek(f,s,SEEK_SET);fread(target,1,256*3,f);goto target_set;
	}
	//ucs2tombs(target,extlink.DataFullPathFilenameUnicode);
	strcpy(target,filename);
/*
	hbmode=getExtlinkWrapperHBMode();
	if(!hbmode&&!strcmp(dldiid,"M3DS"))hbmode=1;
	if(hbmode==1&&read32(head+0x24)!=0x02000000)hbmode=2; //hn loader has some issue...
	if(hbmode){
		if(hbmode==1)strcpy(loader,MOONSHELL);
		else ucs2tombs(loader,extlink.NDSFullPathFilenameUnicode); //dummy
       }
*/
	target_set:
	fclose(f);

//
	_consolePrint("Configuring extlink... ");
	writeFrontend(FRONTEND_EXTLINK,loader,target);

	// vvvvvvvvvvv add 2008.03.30 kzat3
	BootNDSROM(loader);
	// ^^^^^^^^^^^^ add 2008.03.30 kzat3
}
}
