#include <nds.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/dir.h>

#include "_console.h"
#include "nds_mshl2wrap_nds_bin.h"

int getmshl2wrap(u8 **mshl2wrap){
	//char s[768];
	//FILE *f;
	//ini_gets("ndslink","mshl2wrap","/nds.mshl2wrap.nds",s,768,"/NDSLINK.INI");
	// /// mshl2wrap won't be freed
	//fopen ... *mshl2wrap=malloc() ...

	*mshl2wrap=(u8*)nds_mshl2wrap_nds_bin;
	return nds_mshl2wrap_nds_bin_size;
}

void rm_rf(char *target){ // last byte has to be '/' *** target itself won't be erased
	DIR_ITER *dp;
	if(!target)return;
	char *targetfile=target+strlen(target);
	struct stat st;
	dp=diropen(target);
	while(!dirnext(dp,targetfile,&st)){
		if(!strcmp(targetfile,".")||!strcmp(targetfile,".."))continue;
		if(st.st_mode&S_IFDIR){
			strcat(targetfile,"/");
			rm_rf(target);
			targetfile[strlen(targetfile)-1]=0;
			unlink(target);
		}else unlink(target);
	}
	dirclose(dp);
	*targetfile=0; //genjo fukki!
}

void mkpath(char *path){ // last byte has to be '/' (after final '/' will be ignored)
	int i=2;
	char dir[768];
	if(!path||!*path||!path[1])return;
	for(memset(dir,0,768);i<strlen(path);i++)
		if(path[i]=='/')strncpy(dir,path,i),mkdir(dir,0777),i++;
}

static u8 head[0x160];
static char tmp[768];
extern char source[768],target[768];
void recursive(u8 *p, int offset, int size/*, char *source, char *target*/){
	DIR_ITER *dp;
	struct stat st;
	int banneroffset;
	if(!p||!source||!target)return;
	char *sourcefile=source+strlen(source);
	char *targetfile=target+strlen(target);
	dp=diropen(source);
	while(!dirnext(dp,tmp,&st)){
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
				fread(p+((p[0x06b]<<24)+(p[0x06a]<<16)+(p[0x069]<<8)+p[0x068]),1,2112,f);
			}
			fclose(f);

			f=fopen(target,"wb");
			_consolePrintf("%s\n",target);
			memset(p+offset,0,768);
			strcpy(p+offset,source);
			fwrite(p,1,size,f);
			fclose(f);
		}
	}
	dirclose(dp);
	*sourcefile=*targetfile=0;
}
