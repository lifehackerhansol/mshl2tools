#define MOONSHELL "/MOONSHL2/EXTLINK/_hn.HugeNDSLoader.nds"

#include <sys/stat.h>
#include "../../libprism/libprism.h"
const u16 bgcolor=RGB15(6,10,0);

void Main(){
	FILE *f;
	TExtLinkBody extlink;
	int size,hbmode=0;
	unsigned char head[0x200];
	char dldiid[5];

	IPCZ->cmd=0;
	memset(&extlink,0,sizeof(extlink));
	extlink.ID=ExtLinkBody_ID;

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

	_consolePrintf("Initializing libfat... ");
	if(!fatInitDefault()){_consolePrintf("Failed.\n");die();}
	_consolePrintf("Done.\n");

{
	char loader[768],filename[768],target[768];

	_consolePrintf("Loading /loadfile.dat... ");
	f=fopen("/loadfile.dat","rb");
	if(!f){_consolePrintf("Failed.\n");die();}
	myfgets(filename,768,f);
	fclose(f);
	remove("/loadfile.dat");
	_consolePrintf("Done.\n");
/*
	_consolePrintf("Loading /d_system/loader.cfg... ");
	f=fopen("/d_system/loader.cfg","rb");
	if(!f){_consolePrintf("Failed.\n");die();}
	myfgets(loader,768,f);
	fclose(f);
	_consolePrintf("Done.\n");
*/
	_consolePrintf("Setting loader... ");
	ini_gets("mshl2wrap",dldiid/*"loader"*/,MOONSHELL,loader,256*3,"/MOONSHL2/EXTLINK/mshl2wrap.ini");
	memset(&extlink,0,sizeof(TExtLinkBody));
	if(!(f=fopen(filename,"rb"))){_consolePrintf("Failed.\n");die();}
	{struct stat st;fstat(fileno(f),&st);size=st.st_size;}
	if(size<0x200){fclose(f);_consolePrintf("Failed.\n");die();}
	fread(head,1,0x200,f);
	if(!isHomebrew(head)){
		//_FAT_directory_ucs2tombs(target,extlink.DataFullPathFilenameUnicode,768);
			strcpy(target,filename);goto target_set;
	}else if(!strcmp((char*)head+0x1e0,"mshl2wrap link")){
		unsigned int s=(head[0x1f0]<<24)+(head[0x1f1]<<16)+(head[0x1f2]<<8)+head[0x1f3];
		_consolePrintf("Detected mshl2wrap link.\n");
		if(size<s+256*3){fclose(f);_consolePrintf("Failed.\n");die();}
		fseek(f,s,SEEK_SET);fread(target,1,256*3,f);goto target_set;
	}
	//_FAT_directory_ucs2tombs(target,extlink.DataFullPathFilenameUnicode,768);
	strcpy(target,filename);
/*
	hbmode=ini_getl("mshl2wrap","hbmode",0,"/MOONSHL2/EXTLINK/mshl2wrap.ini");
	if(!hbmode&&!strcmp(dldiid,"M3DS"))hbmode=1;
	if(hbmode==1&&read32(head+0x24)!=0x02000000)hbmode=2; //hn loader has some issue...
	if(hbmode){
		if(hbmode==1)strcpy(loader,MOONSHELL);
		else _FAT_directory_ucs2tombs(loader,extlink.NDSFullPathFilenameUnicode,768); //dummy
       }
*/
	target_set:
	fclose(f);

//
	_consolePrintf("Configuring extlink... ");
	extlink.ID=ExtLinkBody_ID;
	getsfnlfn(target,extlink.DataFullPathFilenameAlias,extlink.DataFullPathFilenameUnicode);
	SplitItemFromFullPathAlias(extlink.DataFullPathFilenameAlias,extlink.DataPathAlias,extlink.DataFilenameAlias);
	SplitItemFromFullPathUnicode(extlink.DataFullPathFilenameUnicode,extlink.DataPathUnicode,extlink.DataFilenameUnicode);

	getsfnlfn(loader,extlink.NDSFullPathFilenameAlias,extlink.NDSFullPathFilenameUnicode);
	SplitItemFromFullPathAlias(extlink.NDSFullPathFilenameAlias,extlink.NDSPathAlias,extlink.NDSFilenameAlias);
	SplitItemFromFullPathUnicode(extlink.NDSFullPathFilenameUnicode,extlink.NDSPathUnicode,extlink.NDSFilenameUnicode);

	_consolePrintf("Target NDS is:\n%s\n",extlink.DataFullPathFilenameAlias);
	_consolePrintf("Loader name is:\n%s\n",extlink.NDSFullPathFilenameAlias);

if(hbmode<2){
	if(!(f=fopen("/MOONSHL2/EXTLINK.DAT","wb"))){_consolePrintf("Failed.\n");die();}
	fwrite(&extlink,1,sizeof(TExtLinkBody),f);
	fclose(f);
	//if(!(f=fopen("/EXTLINK.LOG","wb"))){_consolePrintf("Failed.\n");die();}
	//fwrite(&extlink,1,sizeof(TExtLinkBody),f);
	//fclose(f);
	_consolePrintf("Done.\n\n");
}
//

	// vvvvvvvvvvv add 2008.03.30 kzat3
	_FAT_directory_ucs2tombs(filename,hbmode<2?extlink.NDSFullPathFilenameUnicode:extlink.DataFullPathFilenameUnicode,768);

	BootNDSROM(filename);
	// ^^^^^^^^^^^^ add 2008.03.30 kzat3
}
}
