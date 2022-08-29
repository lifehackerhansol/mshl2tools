#include "../../libprism/libprism.h"
const u16 bgcolor=RGB15(10,10,0);

int BackupAsSav,BidirectionalCopy;

static char tmp[768];
char source[768],target[768];
void recursive(){
	DIR_ITER *dp;
	struct stat sts,stt;
	char *sourcefile=source+strlen(source);
	char *targetfile=target+strlen(target);
	dp=mydiropen(source);
	if(!dp){_consolePrintf("Fatal: Failed to open %s\n",source);die();}
	while(!mydirnext(dp,tmp,&sts)){
		strcpy(targetfile,tmp);
		if(!strcmp(targetfile,".")||!strcmp(targetfile,".."))continue;
		if(sts.st_mode&S_IFDIR){
			mkdir(target,0777);
			strcat(targetfile,"/");
			strcpy(sourcefile,targetfile);
			recursive();
		}else{
			if(strlen(targetfile)<4||strcasecmp(targetfile+strlen(targetfile)-4,".sav"))continue;
			strcpy(sourcefile,targetfile);
			if(!BackupAsSav)strcpy(targetfile+strlen(targetfile)-4,".bak");
			if(!stat(target,&stt)&&sts.st_mtime<=stt.st_mtime){
				if(sts.st_mtime==stt.st_mtime||!BidirectionalCopy)_consolePrintf("Skip:%s\n",source);
				else{
					_consolePrintf("Restore:%s\n",target);
					copy(target,source);
					libprism_utime(source,stt.st_atime,stt.st_mtime);
				}
			}
			if(stat(target,&stt)||sts.st_mtime>stt.st_mtime){
				_consolePrintf("Backup:%s\n",source);
				copy(source,target);
				libprism_utime(target,sts.st_atime,sts.st_mtime);
			}
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
		"SavBackup\n"
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

	_consolePrint("Configuring...\n");
{
	if(!strcpy_safe(ininame,findpath(6,(char*[]){"/","/_dstwoplug/","/ismartplug/","/_iMenu/_ini/","/_plugin_/",mypath},"savbackup.ini"))){_consolePrint("As this scans whole folder, savbackup.ini configuration is required.\n");die();}
	ini_gets("savbackup","scandir","",source,768,ininame);
	ini_gets("savbackup","bakdir",source,target,768,ininame);
	BackupAsSav=ini_getl("savbackup","BackupAsSav",0,ininame);
	BidirectionalCopy=ini_getl("savbackup","BidirectionalCopy",0,ininame);
#ifndef _LIBNDS_MAJOR_
	if(BidirectionalCopy){
		_consolePrint("BidirectionalCopy option cannot be enabled on legacy version. Halt.\n");die();
	}
#endif
	if(!*source||!*target){_consolePrint("As this scans whole folder, savbackup.ini configuration is required.\n");die();}
	if(source[strlen(source)-1]!='/')strcat(source,"/");
	if(target[strlen(target)-1]!='/')strcat(target,"/");
	if(!strcasecmp(source,target)){
		if(BackupAsSav){_consolePrint("source==target, BackupAsSav disabled.\n");BackupAsSav=0;}
	}else{
		mkpath(target);
	}
	_consolePrint("Backing up...\n");
	recursive();
}
	_consolePrint("Done.\n");die();
}
