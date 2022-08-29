#include "../../libprism/libprism.h"
const u16 bgcolor=RGB15(4,0,12);

void decodehead(u8* buf){
	_consolePrintf("Decoding header(1)...\n");
	if(memcmp(buf+3,"####",4)){
		int i=0,j=0;
		u8 *p=(u8*)buf;
		u8 *t=(u8*)"CuNt"; /////
		for(;i<64;i++)
		if(p[i])
			p[i]=p[i]!=t[j]?p[i]^t[j]:p[i],j++,j%=strlen((char*)t); //fixed in V2
	}
}

void decodebody(u8* buf){
	_consolePrintf("Decoding header(2)...\n");
	if(memcmp(buf+3,"####",4)){
		int i=0,j=0;
		u8 *p=(u8*)buf;
		u8 *t=(u8*)"CuNt"; /////
		for(;i<512;i++)
		if(p[i])
			p[i]=p[i]!=t[j]?p[i]^t[j]:p[i],j++,j%=strlen((char*)t); //fixed in V2
	}

	u32 *hed=(u32*)buf;
	_consolePrintf("Decoding body...\n");
	u8 *ldrBuf=buf+512;
	if((ldrBuf[3]&0xf0)!=0xe0){
		int i=0,j;
		u8 *k1=(u8*)"DoNt hAx"; /////
		for(j=0;i<hed[11];i++){
			ldrBuf[i]=ldrBuf[i]^k1[j++],j%=strlen((char*)k1);
		}
	}

	ldrBuf=buf+512+hed[11];
	if((ldrBuf[3]&0xf0)!=0xe0){
		int i=0,j;
		u8 *k2=(u8*)"My SHitz"; /////
		for(j=0;i<hed[15];i++){
			ldrBuf[i]=ldrBuf[i]^k2[j++],j%=strlen((char*)k2);
		}
	}
}

void Main(){
	char file[256*3]="MoonShellExecute\0\0\0\0\0\0\0\0\0\0\0" //will be modified externally
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"; //744 paddings
	TExtLinkBody extlink;
	FILE *f;

	IPCZ->cmd=0;
	_consolePrintf(
		"ak2loader technology preview\n"
		"reset_mse_06b_for_ak2 by Moonlight, Rudolph, kzat3\n"
		"dldipatch aka dlditool public domain under CC0.\n"
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

	_consolePrintf("Opening extlink... ");
	if(!(f=fopen("/MOONSHL2/EXTLINK.DAT","rb"))){_consolePrintf("Failed.\n");die();}
	fread(&extlink,1,sizeof(TExtLinkBody),f);
	fclose(f);
	if(extlink.ID!=ExtLinkBody_ID){_consolePrintf("Incorrect ID.\n");die();}
	_consolePrintf("Done.\n");

	//magic
	*(u8*)0x023fdbff=4; //reset

	strcpy((char*)0x023fda00,"fat1:");
	strcpy((char*)0x023fda05,extlink.DataFullPathFilenameAlias);
	strcpy((char*)0x023fdc00,"fat1:");
	char utf8[768];
	_FAT_directory_ucs2tombs(utf8,extlink.DataFullPathFilenameUnicode,768);
	strcpy(utf8+strlen(utf8)-3,"sav");
	if(!stat(utf8,NULL))getsfnlfn(utf8,(char*)0x023fdc05,NULL);

	//strcpy(0x023fde00,"fat1:/");

	ret_menu9_callbackpre=decodehead;
	ret_menu9_callback=decodebody;

	_consolePrintf("Rebooting to %s...\n",file);
	ret_menu9_Gen(file);
}
