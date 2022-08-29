#define MOONSHELL "/MOONSHL2/EXTLINK/_hn.HugeNDSLoader.nds"

#include "../../libprism/libprism.h"
const u16 bgcolor=RGB15(0,8,8);

char *template="*mshl2wrap link template";
char ext[32][768];

void Main(){
	FILE *f;
	TExtLinkBody extlink;
	int size,hbmode=0;
	//ERESET RESET=RESET_NULL;
	int flag=0;

	char target[256*3]=
				//"GUIDDIUG\0\0\0\0\0\0\0\0\0\0\0\0"
				"mshl2wrap link template\0\0\0\0" //will be modified from external link maker
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

	char loader[256*3];
	char utf8[768];
	char dldiid[5];
	unsigned char head[0x204]; //0.65: 0xea000112 (r4isdhc hack) check to avoid issue

	IPCZ->cmd=0;
	if(strcmp(target,template+1))flag=1;

	_consolePrintf(
		"MoonShell2 %s\nVersion %s\n"
		"reset_mse_06b_for_ak2 by Moonlight, Rudolph, kzat3\n"
		"dldipatch aka dlditool public domain under CC0.\n"
		"%s\n%s\n\n",
		flag?"Link Launcher":"Extlink Wrapper",ROMVERSION,ROMDATE,ROMENV
	);

	{
		unsigned char *dldiFileData=DLDIDATA;
		memcpy(dldiid,(unsigned char*)dldiFileData+ioType,4);
		dldiid[4]=0;
		_consolePrintf("DLDI ID: %s\n",dldiid);
		_consolePrintf("DLDI Name: %s\n\n",(char*)dldiFileData+friendlyName);
	}

	_consolePrintf("Initializing libfat... ");
	if(!fatInitDefault())goto fail;
	_consolePrintf("Done.\n");

	_consolePrintf("Checking condition of /moonshl2/extlink/... ");
	{
		char tmp1[768],tmp2[768];
		u8 head[512];
		int n=0,l=-1,i=0;
		int l1,l2;
		DIR_ITER *dir=diropen("/moonshl2/extlink/");
		strcpy(tmp1,"/moonshl2/extlink/");l1=strlen(tmp1);
		strcpy(tmp2,"/moonshl2/extlink/__link_rearrange/");l2=strlen(tmp2);

		if(dir){
			while(!dirnext(dir,ext[n],NULL)){
				if(strlen(ext[n])>6&&!memcmp(ext[n],"nds.",4)&&!memcmp(ext[n]+strlen(ext[n])-4,".nds",4)){
					if(n==32){dirclose(dir);_consolePrintf("too many extlink. Halt.\n");die();}
					strcpy(tmp1+l1,ext[n]);
					if(!(f=fopen(tmp1,"rb"))){dirclose(dir);_consolePrintf("Cannot open %s. Possibly bug... Halt.\n",tmp1);die();}
					fread(head,1,512,f);
					fclose(f);
					if(!strcmp((char*)head+0x1e0,"mshl2wrap link")){
						if(l>=0){dirclose(dir);_consolePrintf("multiple mshl2wrap. Halt.\n");die();}
						l=n;
					}
					n++;
				}
			}
			dirclose(dir);
			if(l<0&&n>0){_consolePrintf("mshl2wrap has to be in /moonshl2/extlink/ if you put nds.*.nds there. Halt.\n");die();}
			if(l>0){
				_consolePrintf("Need rearranging.\n");
				mkdir("/moonshl2/extlink/__link_rearrange",0777);
				for(i=0;i<n;i++){
					strcpy(tmp1+l1,ext[i]);
					strcpy(tmp2+l2,ext[i]);
					rename(tmp1,tmp2);
				}
					strcpy(tmp1+l1,ext[l]);
					strcpy(tmp2+l2,ext[l]);
					rename(tmp2,tmp1);
				for(i=0;i<n;i++){
					strcpy(tmp1+l1,ext[i]);
					strcpy(tmp2+l2,ext[i]);
					rename(tmp2,tmp1);
				}
				unlink("/moonshl2/extlink/__link_rearrange"); //in libnds unlink is used for dir
			}
		}
	}
	_consolePrintf("Done.\n");
	//die();

	ini_gets("mshl2wrap",dldiid/*"loader"*/,MOONSHELL,loader,256*3,"/MOONSHL2/EXTLINK/mshl2wrap.ini");
/*	//This might be used on 2.06...
	if(!strcmp(loader,MOONSHELL)){
		_consolePrintf(
			"Loader not set for cardtype %s.\n"
			"To force to use MoonShell HugeNDSLoader, press A\n"
			To return to the firmware, press B\n"
		,dldiid);
	}
*/

if(!flag){
	_consolePrintf("Opening moonshl2/extlink.dat... ");
	if(!(f=fopen("/MOONSHL2/EXTLINK.DAT","rb")))goto fail;
	_consolePrintf("Done.\n");
	_consolePrintf("Reading moonshl2/extlink.dat... ");
	memset(&extlink,0,sizeof(TExtLinkBody));
	fread(&extlink,1,sizeof(TExtLinkBody),f);
	fclose(f);

	//if(!(f=fopen("/EXTLINK.BAK","wb")))goto fail;
	//fwrite(&extlink,1,sizeof(TExtLinkBody),f);
	//fclose(f);

	_consolePrintf("0x%08x\n",extlink.ID);
	//extlink.ID=ExtLinkBody_ID;
	if(extlink.ID!=ExtLinkBody_ID)goto fail;
	_consolePrintf("Target NDS is:\n%s\n",extlink.DataFullPathFilenameAlias);
	_consolePrintf("My name is:\n%s\n\n",extlink.NDSFullPathFilenameAlias);
}

	_consolePrintf("Setting desired filenames to moonshl2/extlink.dat...\n");


if(!flag){
	//_consolePrintf("Linked NDS is:\n%s\n",extlink.DataFullPathFilenameAlias);
	_FAT_directory_ucs2tombs(utf8,extlink.DataFullPathFilenameUnicode,768);
	if(!(f=fopen(utf8,"rb")))goto fail;
	{struct stat st;fstat(fileno(f),&st);size=st.st_size;}
	if(size<0x204){fclose(f);goto fail;}
	fread(head,1,0x204,f);
	if(!isHomebrew(head)){
		_FAT_directory_ucs2tombs(target,extlink.DataFullPathFilenameUnicode,768);goto target_set;
	}else if(!strcmp((char*)head+0x1e0,"mshl2wrap link")){
		unsigned int s=(head[0x1f0]<<24)+(head[0x1f1]<<16)+(head[0x1f2]<<8)+head[0x1f3];
		_consolePrintf("Detected mshl2wrap link.\n");
		if(size<s+256*3){fclose(f);goto fail;}
		fseek(f,s,SEEK_SET);fread(target,1,256*3,f);goto target_set;
	}
	_FAT_directory_ucs2tombs(target,extlink.DataFullPathFilenameUnicode,768);
	hbmode=ini_getl("mshl2wrap","hbmode",0,"/MOONSHL2/EXTLINK/mshl2wrap.ini");
	if(!hbmode&&!strcmp(dldiid,"M3DS"))hbmode=1;
	if(hbmode==1&&(read32(head+0x24)!=0x02000000||read32(head+0x200)==0xea000112))hbmode=2; //hn loader has some issue...
	if(hbmode){
		if(hbmode==1)strcpy(loader,MOONSHELL);
		else _FAT_directory_ucs2tombs(loader,extlink.NDSFullPathFilenameUnicode,768); //dummy
       }
	target_set:
	fclose(f);
}else{ //applying target/loader manually.
	hbmode=ini_getl("mshl2wrap","hbmode",0,"/MOONSHL2/EXTLINK/mshl2wrap.ini");
	if(!hbmode&&!strcmp(dldiid,"M3DS"))hbmode=1;
	//Now confirm target.
	_consolePrintf("Linked NDS is:\n%s\n",target);
	if(!(f=fopen(target,"rb")))goto fail;
	{struct stat st;fstat(fileno(f),&st);size=st.st_size;}
	if(size<0x204){fclose(f);goto fail;}
	fread(head,1,0x204,f);
	fclose(f);
	if(!isHomebrew(head))hbmode=0;
	if(hbmode==1&&(read32(head+0x24)!=0x02000000||read32(head+0x200)==0xea000112))hbmode=2; //hn loader has some issue...
	if(hbmode==1)strcpy(loader,MOONSHELL);
}

	memset(&extlink,0,sizeof(TExtLinkBody));
	extlink.ID=ExtLinkBody_ID;
	getsfnlfn(target,extlink.DataFullPathFilenameAlias,extlink.DataFullPathFilenameUnicode);
	SplitItemFromFullPathAlias(extlink.DataFullPathFilenameAlias,extlink.DataPathAlias,extlink.DataFilenameAlias);
	SplitItemFromFullPathUnicode(extlink.DataFullPathFilenameUnicode,extlink.DataPathUnicode,extlink.DataFilenameUnicode);

	getsfnlfn(loader,extlink.NDSFullPathFilenameAlias,extlink.NDSFullPathFilenameUnicode);
	SplitItemFromFullPathAlias(extlink.NDSFullPathFilenameAlias,extlink.NDSPathAlias,extlink.NDSFilenameAlias);
	SplitItemFromFullPathUnicode(extlink.NDSFullPathFilenameUnicode,extlink.NDSPathUnicode,extlink.NDSFilenameUnicode);

	_consolePrintf("Target NDS is:\n%s\n",extlink.DataFullPathFilenameAlias);
	_consolePrintf("Loader name is:\n%s\n",extlink.NDSFullPathFilenameAlias);

	//if(strstr(extlink.DataFullPathFilenameAlias,"/MOONSHL2/EXTLINK/NDS"))
	//	{_consolePrintf("You must not set loader to /moonshl2/extlink/nds*. Read warning_about_mshl2wrap_configuration.txt again. Halted.\n");die();}

if(hbmode<2){
	if(!(f=fopen("/MOONSHL2/EXTLINK.DAT","wb")))goto fail;
	fwrite(&extlink,1,sizeof(TExtLinkBody),f);
	fclose(f);
	//if(!(f=fopen("/EXTLINK.LOG","wb")))goto fail;
	//fwrite(&extlink,1,sizeof(TExtLinkBody),f);
	//fclose(f);
	_consolePrintf("Done.\n\n");
}

	//if(hbmode<0 || 2<hbmode){_consolePrintf("hbmode has to 0, 1 or 2.\nLaunch ");goto fail;}

	// vvvvvvvvvvv add 2008.03.30 kzat3
	_FAT_directory_ucs2tombs(utf8,hbmode<2?extlink.NDSFullPathFilenameUnicode:extlink.DataFullPathFilenameUnicode,768);

	BootNDSROM(utf8);
/*
	_consolePrintf("Allocating actual loader...\n");
	if(!ret_menu9_Gen(utf8))goto fail;
	_consolePrintf("Done.\n");

	_consolePrintf("Rebooting... ");
#if 1
	//RESET=RESET_MENU_GEN;
	//IPCEX->RESET=RESET;
	IPCZ->cmd=ResetRudolph;
       //fifoSendValue32(FIFO_USER_07,1);
	ret_menu9_GENs();
#endif
*/
fail:
	_consolePrintf("Failed.\nProcess cannot continue. Accept your fate.\n");
	die();
	// ^^^^^^^^^^^^ add 2008.03.30 kzat3
}

