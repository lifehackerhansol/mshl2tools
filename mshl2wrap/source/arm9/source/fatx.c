#include <nds.h>
#include <stdio.h>
#include <string.h>
#include "fatx.h"
#include "_console.h"

#define DIR_ENTRY_DATA_SIZE 0x20
#define MAX_FILENAME_LENGTH 256
#define MAX_ALIAS_LENGTH 13
#define LFN_ENTRY_LENGTH 13
#define FAT16_ROOT_DIR_CLUSTER 0

#define DIR_SEPARATOR '/'

// Directory entry offsets
enum DIR_ENTRY_offset {
	DIR_ENTRY_name = 0x00,
	DIR_ENTRY_extension = 0x08,
	DIR_ENTRY_attributes = 0x0B,
	DIR_ENTRY_reserved = 0x0C,
	DIR_ENTRY_cTime_ms = 0x0D,
	DIR_ENTRY_cTime = 0x0E,
	DIR_ENTRY_cDate = 0x10,
	DIR_ENTRY_aDate = 0x12,
	DIR_ENTRY_clusterHigh = 0x14,
	DIR_ENTRY_mTime = 0x16,
	DIR_ENTRY_mDate = 0x18,
	DIR_ENTRY_cluster = 0x1A,
	DIR_ENTRY_fileSize = 0x1C
};

typedef struct {
	u32 cluster;
	u32 sector;
	s32 offset;
} DIR_ENTRY_POSITION;

typedef struct {
	u8 entryData[DIR_ENTRY_DATA_SIZE];
	DIR_ENTRY_POSITION dataStart;		// Points to the start of the LFN entries of a file, or the alias for no LFN
	DIR_ENTRY_POSITION dataEnd;			// Always points to the file/directory's alias entry
	char filename[MAX_FILENAME_LENGTH];
	char alias[MAX_ALIAS_LENGTH];
} DIR_ENTRY;

typedef struct {
	u32 cluster;
	u32 sector;
	s32 byte;
} FILE_POSITION;

typedef struct {
	u32 filesize;
	u32 startCluster;
	u32 currentPosition;
	FILE_POSITION rwPosition;
	FILE_POSITION appendPosition;
	bool read;
	bool write;
	bool append;
	bool inUse;
	void/*PARTITION*/* partition;
	DIR_ENTRY_POSITION dirEntryStart;		// Points to the start of the LFN entries of a file, or the alias for no LFN
	DIR_ENTRY_POSITION dirEntryEnd;			// Always points to the file's alias entry

	DIR_ENTRY dirEntry;
} FILE_STRUCT;

extern const int LFN_offset_table[13];

void getsfnlfn(const char *path,char *sfn,u16 *lfn){ //path should be in UTF8
	static char buf[256*3],ret[256*3];
	int i,len,fd;
	FILE *f;

	if(!sfn&&!lfn)return;
	memset(&buf,0,256*3);
	memset( sfn,0,256);
	memset(&ret,0,256*3);
	len=strlen(path);
	for(i=1;i<=len;i++){
		if(path[i]=='/'||i==len){
			strncpy(buf,path,i);
			f=fopen(buf,"rb"); //Modified libfat doesn't fail to open dirs, but don't do other thing than fileno()/fclose() 
			//_consolePrintf("***%s\n",buf);
			if(!f){_consolePrintf("\nCannot open %s.\nAccept your fate.\n",buf);while(1);}
			fd=fileno(f);
			FILE_STRUCT *p=(FILE_STRUCT *)(fd+8); //8 is magic number...
			if(lfn){
				bool NTF_lowfn=false,NTF_lowext=false;
				char *x;
				strcat(ret,"/");
				strcpy(x=ret+strlen(ret),p->dirEntry.filename);

		//must consider LFN (from MoonShell2.00beta5 source)
		if(p->dirEntry.entryData[DIR_ENTRY_reserved]&BIT(3)) NTF_lowfn=true;
		if(p->dirEntry.entryData[DIR_ENTRY_reserved]&BIT(4)) NTF_lowext=true;

		if((NTF_lowfn==false)&&(NTF_lowext==false)){
			; //use alias as filename
		}else{
			u32 posperiod=(u32)-1;
			{
				u32 idx;
				for(idx=0;idx<MAX_FILENAME_LENGTH;idx++){
					char fc=x[idx];
					if(fc=='.') posperiod=idx;
					if(fc==0) break;
				}
			}
			if(posperiod==(u32)-1){ //with ext
				u32 idx;
				for(idx=0;idx<MAX_FILENAME_LENGTH;idx++){
					char fc=x[idx];
					if(NTF_lowfn==true){
						if(('A'<=fc)&&(fc<='Z')) fc+=0x20;
					}
					x[idx]=fc;
					if(fc==0) break;
				}
			}else{
				u32 idx;
				for(idx=0;idx<MAX_FILENAME_LENGTH;idx++){
					char fc=x[idx];
					if(NTF_lowfn==true){
						if(('A'<=fc)&&(fc<='Z')) fc+=0x20;
					}
					x[idx]=fc;
					if(fc=='.') break;
				}
				for(;idx<MAX_FILENAME_LENGTH;idx++){
					char fc=x[idx];
					if(NTF_lowext==true){
						if(('A'<=fc)&&(fc<='Z')) fc+=0x20;
            				}
					x[idx]=fc;
					if(fc==0) break;
				}
			}
		}
			}
			if(sfn){
				strcat(sfn,"/");
				_FAT_directory_entryGetAlias(p->dirEntry.entryData,sfn+strlen(sfn));
			}
			fclose(f);
		}
	}
	if(lfn)_FAT_directory_mbstoucs2(lfn,ret,256);
}
