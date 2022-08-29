#include "../../libprism/libprism.h"
#include "nds_mshl2wrap_nds.h"
const u16 bgcolor=RGB15(10,10,0);

int getmshl2wrap(u8 **mshl2wrap){
	//char s[768];
	//FILE *f;
	//ini_gets("ndslink","mshl2wrap","/nds.mshl2wrap.nds",s,768,"/NDSLINK.INI");
	// /// mshl2wrap won't be freed
	//fopen ... *mshl2wrap=malloc() ...

	*mshl2wrap=(u8*)nds_mshl2wrap_nds;
	return nds_mshl2wrap_nds_size;
}

static u8 head[0x160];
static char tmp[768];
static char source[768],target[768];
void recursive(u8 *p, int offset, int size/*, char *source, char *target*/){
	DIR_ITER *dp;
	struct stat st;
	int banneroffset;
	if(!p/*||!source||!target*/)return;
	char *sourcefile=source+strlen(source);
	char *targetfile=target+strlen(target);
	dp=mydiropen(source);
	if(!dp){_consolePrintf("Fatal: Failed to open %s\n",source);die();}
	while(!mydirnext(dp,tmp,&st)){
		strcpy(targetfile,tmp);
		if(!strcmp(targetfile,".")||!strcmp(targetfile,".."))continue;
		if(st.st_mode&S_IFDIR){
			mkdir(target,0777);
			strcat(targetfile,"/");
			strcpy(sourcefile,targetfile);
			recursive(p,offset,size);//,source,target);
		}else{
			if(strlen(targetfile)<4||strcasecmp(targetfile+strlen(targetfile)-4,".nds"))continue;
			strcpy(sourcefile,targetfile);
			FILE *f=fopen(source,"rb");
			fread(head,1,0x160,f);
			banneroffset=(head[0x06b]<<24)+(head[0x06a]<<16)+(head[0x069]<<8)+head[0x068];
			if(banneroffset){
				fseek(f,banneroffset,SEEK_SET);
				u8 *banner=p+((p[0x06b]<<24)+(p[0x06a]<<16)+(p[0x069]<<8)+p[0x068]);
				fread(banner,1,2112,f);
				if(banner[0]!=1)banner[0]=1; //for moonshell2
				if(banner[1]!=0)banner[1]=0; //for moonshell2
			}
			fclose(f);

			f=fopen(target,"wb");
			_consolePrintf("%s\n",target);
			memset(p+offset,0,768);
			strcpy((char*)p+offset,source);
			fwrite(p,1,size,f);
			fclose(f);
		}
	}
	mydirclose(dp);
	*sourcefile=*targetfile=0;
}

void Main(){
	char dldiid[5];
	unsigned char *dldiFileData=DLDIDATA;
	char ininame[768];

	_consolePrintf(
		"NDSLink on DS\n"
		//"reset_mse_06b_for_ak2 by Moonlight, Rudolph, kzat3\n"
		//"dldipatch aka dlditool public domain under CC0.\n"
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

	_consolePrint("Configuring... ");
{
	u8 *p;
	int size,offset;

	if(!strcpy_safe(ininame,findpath(6,(char*[]){"/","/_dstwoplug/","/ismartplug/","/_iMenu/_ini/","/_plugin_/",mypath},"ndslink.ini"))){_consolePrint("As this removes old files, ndslink.ini configuration is required.\n");die();}
	ini_gets("ndslink","source","",source,768,ininame);
	ini_gets("ndslink","target","",target,768,ininame);
	if(!*source||!*target){_consolePrint("As this removes old files, ndslink.ini configuration is required.\n");die();}
	if(source[strlen(source)-1]!='/')strcat(source,"/");
	if(target[strlen(target)-1]!='/')strcat(target,"/");
	mkpath(target);
	rm_rf(target);
	size=getmshl2wrap(&p);
	if(strcmp((char*)p+0x1e0,"mshl2wrap link"))
		{_consolePrint("template not mshl2wrap link.\n");die();}
	offset=(p[0x1f0]<<24)+(p[0x1f1]<<16)+(p[0x1f2]<<8)+p[0x1f3];
	if(size<offset+256*3){_consolePrint("template too small or offset invalid.\n");die();}
	_consolePrint("Done.\n");

	_consolePrint("Linking...\n");
	recursive(p,offset,size/*,source,target*/);
}
	_consolePrint("Done.\n");die();
/*
	// vvvvvvvvvvv add 2008.03.30 kzat3
#ifdef GPL
	//if (!strcmp(dldiid,"EZ5H")||!strcmp(dldiid,"EDGE")||!strcmp(dldiid,"SCDS")){
		_consolePrint("falling back to Chishm VRAM bootlib.\n");
		IPCZ->cmd=ResetBootlib;
		//fifoSendValue32(FIFO_USER_07,2);
		runNdsFile(ysmenu);
	//}
#endif
	//RESET=RESET_MENU_GEN;
	//IPCEX->RESET=RESET;
       //fifoSendValue32(FIFO_USER_07,1);
	//IPCZ->cmd=ResetRudolph;
	_consolePrint("Rebooting... \n");
	BootNDSROM(ysmenu);
	//ret_menu9_GENs();
	_consolePrint("Failed.\n");
	die();
	// ^^^^^^^^^^^^ add 2008.03.30 kzat3
*/
}
