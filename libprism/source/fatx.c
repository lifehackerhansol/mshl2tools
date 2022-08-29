#define MSHL2TOOLS_FATX

/*
	fatx.c - Very special libfat helper API
	Make sure to use my modified libfat.
	Calling getsfnlfn()/libprism_[f]utime() with unmodified libfat will cause unexpected result.
	If you use other functions with unmodified libfat, it will mess something when used for folder.
*/

#include "libprism.h"

#include <fcntl.h> //O_
#include <sys/iosupport.h> //for safer getting FILE_STRUCT
__handle* __get_handle(int);

#define DIR_ENTRY_DATA_SIZE 0x20
#define MAX_FILENAME_LENGTH 768
#define MAX_ALIAS_LENGTH 13
#define LFN_ENTRY_LENGTH 13
#define FAT16_ROOT_DIR_CLUSTER 0

#define DIR_SEPARATOR '/'

#define CLUSTER_EOF_16	0xFFFF
#define CLUSTER_EOF		0x0FFFFFFF
#define CLUSTER_FREE	0x00000000
#define CLUSTER_ROOT	0x00000000
#define CLUSTER_FIRST	0x00000002
#define CLUSTER_ERROR	0xFFFFFFFF

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
} DIR_ENTRY;

typedef struct {
	u32 cluster;
	u32 sector;
	s32 byte;
} FILE_POSITION;

typedef struct {
	u32 fatStart;
	u32 sectorsPerFat;
	u32 lastCluster;
	u32 firstFree;
} FAT;

typedef enum {FS_UNKNOWN, FS_FAT12, FS_FAT16, FS_FAT32} FS_TYPE;

#ifdef _LIBNDS_MAJOR_
typedef int mutex_t;
//typedef u32 sec_t;

typedef struct {
	void* /*const DISC_INTERFACE**/ disc;
	void* /*CACHE**/                cache;
	// Info about the partition
	FS_TYPE               filesysType;
	uint64_t              totalSize;
	sec_t                 rootDirStart;
	uint32_t              rootDirCluster;
	uint32_t              numberOfSectors;
	sec_t                 dataStart;
	uint32_t              bytesPerSector;
	uint32_t              sectorsPerCluster;
	uint32_t              bytesPerCluster;
	FAT                   fat;
	// Values that may change after construction
	uint32_t              cwdCluster;			// Current working directory cluster
	int                   openFileCount;
	void* /*struct _FILE_STRUCT**/  firstOpenFile;		// The start of a linked list of files
	mutex_t               lock;					// A lock for partition operations
	bool                  readOnly;				// If this is set, then do not try writing to the disc
	char                  label[12];			// Volume label
} PARTITION;

typedef struct {
	uint32_t             filesize;
	uint32_t             startCluster;
	uint32_t             currentPosition;
	FILE_POSITION        rwPosition;
	FILE_POSITION        appendPosition;
	DIR_ENTRY_POSITION   dirEntryStart;		// Points to the start of the LFN entries of a file, or the alias for no LFN
	DIR_ENTRY_POSITION   dirEntryEnd;		// Always points to the file's alias entry
	PARTITION*           partition;
	struct _FILE_STRUCT* prevOpenFile;		// The previous entry in a double-linked list of open files
	struct _FILE_STRUCT* nextOpenFile;		// The next entry in a double-linked list of open files
	bool                 read;
	bool                 write;
	bool                 append;
	bool                 inUse;
	bool                 modified;
	DIR_ENTRY dirEntry;
	u16 fsyncmoddate;
	u16 fsyncmodtime;
	u16 fsyncacdate;
	u8  fsyncattr;
} FILE_STRUCT;

int _FAT_syncToDisc (FILE_STRUCT* file);
#else
typedef struct {
	void* /*const DISC_INTERFACE**/ disc;
	void* /*CACHE**/                cache;
	// Info about the partition
	bool readOnly;		// If this is set, then do not try writing to the disc
	FS_TYPE filesysType;
	u32 totalSize;
	u32 rootDirStart;
	u32 rootDirCluster;
	u32 numberOfSectors;
	u32 dataStart;
	u32 bytesPerSector;
	u32 sectorsPerCluster;
	u32 bytesPerCluster;
	FAT fat;
	// Values that may change after construction
	u32 cwdCluster;			// Current working directory cluser
	u32 openFileCount;
} PARTITION;

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
	PARTITION* partition;
	DIR_ENTRY_POSITION dirEntryStart;		// Points to the start of the LFN entries of a file, or the alias for no LFN
	DIR_ENTRY_POSITION dirEntryEnd;			// Always points to the file's alias entry
	DIR_ENTRY dirEntry;
} FILE_STRUCT;
#endif

typedef struct {
	PARTITION* partition;
	DIR_ENTRY  currentEntry;
	uint32_t   startCluster;
	bool       inUse;
	bool       validEntry;
} DIR_STATE_STRUCT;

extern const int LFN_offset_table[13];

/*
extern int _FAT_open_r (struct _reent *r, void *fileStruct, const char *path, int flags, int mode);
extern int _FAT_close_r (struct _reent *r, int fd);
extern ssize_t _FAT_write_r (struct _reent *r,int fd, const char *ptr, size_t len);
extern ssize_t _FAT_read_r (struct _reent *r, int fd, char *ptr, size_t len);
extern off_t _FAT_seek_r (struct _reent *r, int fd, off_t pos, int dir);
extern int _FAT_fstat_r (struct _reent *r, int fd, struct stat *st);
extern int _FAT_stat_r (struct _reent *r, const char *path, struct stat *st);
extern int _FAT_link_r (struct _reent *r, const char *existing, const char *newLink);
extern int _FAT_unlink_r (struct _reent *r, const char *name);
extern int _FAT_chdir_r (struct _reent *r, const char *name);
extern int _FAT_rename_r (struct _reent *r, const char *oldName, const char *newName);
extern int _FAT_ftruncate_r (struct _reent *r, int fd, off_t len);
extern int _FAT_fsync_r (struct _reent *r, int fd);
*/

extern bool _FAT_directory_entryGetAlias (const u8* entryData, char* destName);
extern PARTITION* _FAT_partition_getPartitionFromPath (const char* path);
extern uint32_t _FAT_fat_nextCluster(PARTITION* partition, uint32_t cluster);

u32 _FAT_fat_clusterToSector(PARTITION* partition, u32 cluster){
	return (cluster >= CLUSTER_FIRST) ? 
		(cluster - CLUSTER_FIRST) * partition->sectorsPerCluster + partition->dataStart :
		partition->rootDirStart;
}

u32 getFatDataPointer(){ //for getTrueSector
	PARTITION *p=_FAT_partition_getPartitionFromPath("fat:/");
	if(p->bytesPerSector!=512||p->sectorsPerCluster!=64){_consolePrintf("getFatDataPointer() will be useless if bytesPerSector!=512 || sectorsPerCluster!=64.\n");die();}
	return p->dataStart-128;
}

void getsfnlfn(const char *path,char *sfn,u16 *lfn){ //path should be in UTF8
	static char buf[256*3],ret[256*3];
	int i,len;
	FILE_STRUCT *fs;
	int fd;

	if(!sfn&&!lfn)return;
	memset(&buf,0,256*3);
	memset( sfn,0,256);
	memset(&ret,0,256*3);
	len=strlen(path);
	for(i=1;i<=len;i++){
		if(path[i]=='/'||i==len){
			//memset(&fs,0,sizeof(FILE_STRUCT));
			strncpy(buf,path,i);
			//_consolePrintf("***%s\n",buf);
			fd=open(buf,O_RDONLY);
			//if(_FAT_open_r(_REENT,&fs,buf,O_RDONLY,0)<0)
			if(fd<0){_consolePrintf("\nCannot open %s.\nAccept your fate.\n",buf);die();}
			fs=(FILE_STRUCT*)__get_handle(fd)->fileStruct;
			//_consolePrintf("***%s\n",fs->dirEntry.filename);
			if(lfn){
				bool NTF_lowfn=false,NTF_lowext=false;
				char *x;
				strcat(ret,"/");
				strcpy(x=ret+strlen(ret),fs->dirEntry.filename);

		//must consider LFN (from MoonShell2.00beta5 source)
		if(fs->dirEntry.entryData[DIR_ENTRY_reserved]&BIT(3)) NTF_lowfn=true;
		if(fs->dirEntry.entryData[DIR_ENTRY_reserved]&BIT(4)) NTF_lowext=true;

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
				_FAT_directory_entryGetAlias(fs->dirEntry.entryData,sfn+strlen(sfn));
			}
			//_FAT_close_r(_REENT,(int)fs);
			close(fd);
		}
	}
	if(lfn)_FAT_directory_mbstoucs2(lfn,ret,256);
}

u64 fgetFATEntryAddress(int fd){ //only for R4 B4command //Working but still beta, I think
	PARTITION *p=_FAT_partition_getPartitionFromPath("fat:/");
	if(!p)return 0;

	FILE_STRUCT *fs;
	if(fd<0){return 0;}//_consolePrintf("\nCannot open %s.\nAccept your fate.\n",path);die();}
	fs=(FILE_STRUCT*)__get_handle(fd)->fileStruct;
	_consolePrintf2("fse_offset  = 0x%08X\n", fs->dirEntryEnd.offset);
	_consolePrintf2("fse_sector  = 0x%08X\n", fs->dirEntryEnd.sector);
	_consolePrintf2("fse_cluster = 0x%08X\n", fs->dirEntryEnd.cluster);
	_consolePrintf2("\nSector = %d (0x%08X), Offset = %d\n",
		_FAT_fat_clusterToSector(p,fs->dirEntryEnd.cluster)+fs->dirEntryEnd.sector,
		_FAT_fat_clusterToSector(p,fs->dirEntryEnd.cluster)+fs->dirEntryEnd.sector,
		fs->dirEntryEnd.offset);
	return (u64)(_FAT_fat_clusterToSector(p,fs->dirEntryEnd.cluster)+fs->dirEntryEnd.sector)*p->bytesPerSector
			+fs->dirEntryEnd.offset*DIR_ENTRY_DATA_SIZE;
	//return ret;
}

u64 getFATEntryAddress(const char *path){
	int fd=open(path,O_RDONLY);
	if(fd<0)return -1;
	u64 ret=fgetFATEntryAddress(fd);
	close(fd);
	return ret;
}

u32 fgetSector(int fd){
	PARTITION *p=_FAT_partition_getPartitionFromPath("fat:/");
	if(!p)return 0;
	struct stat st;
	if(fstat(fd,&st))return 0;
	return _FAT_fat_clusterToSector(p,st.st_ino);
}

u32 getSector(const char *path){
	int fd=open(path,O_RDONLY);
	if(fd<0)return -1;
	int ret=fgetSector(fd);
	close(fd);
	return ret;
}

int fgetFragments(int fd){ //based on MoonShell source code
	int ret=0;
	u32 clust,tmp;
	PARTITION *p=_FAT_partition_getPartitionFromPath("fat:/");
	if(!p)return -1;
	struct stat st;
	if(fstat(fd,&st))return -1;
	for(clust=st.st_ino;(tmp=_FAT_fat_nextCluster(p,clust))!=CLUSTER_EOF;){
		if(tmp==CLUSTER_ERROR)return -1;
		if(clust+1!=tmp)ret++;
		clust=tmp;
	}
	return ret;
}

int getFragments(const char *path){
	int fd=open(path,O_RDONLY);
	if(fd<0)return -1;
	int ret=fgetFragments(fd);
	close(fd);
	return ret;
}

int writePartitionInfo(type_printf writer){
	PARTITION *p=_FAT_partition_getPartitionFromPath("fat:/");
	if(!p||!writer)return -1;
	writer(
		"filesysType:       %s\n"
		"totalSize:         %dK\n"
		"rootDirStart:      %u\n"
		"rootDirCluster:    %u\n"
		"numberOfSectors:   %u\n"
		"dataStart:         %u\n"
		"bytesPerSector:    %u\n"
		"sectorsPerCluster: %u\n"
		"bytesPerCluster:   %u\n"
		"fat.fatStart:      %u\n"
		"fat.sectorsPerFat: %u\n"
		"fat.lastCluster:   %u\n"
		"fat.firstFree:     %u\n",
		p->filesysType==FS_FAT12?"FAT12":p->filesysType==FS_FAT16?"FAT16":p->filesysType==FS_FAT32?"FAT32":"Unknown",
		(u32)(p->totalSize>>10),
		p->rootDirStart,
		p->rootDirCluster,
		p->numberOfSectors,
		p->dataStart,
		p->bytesPerSector,
		p->sectorsPerCluster,
		p->bytesPerCluster,
		p->fat.fatStart,
		p->fat.sectorsPerFat,
		p->fat.lastCluster,
		p->fat.firstFree
	);
	return 0;
}

int libprism_futime(int fd, time_t actime, time_t modtime){
	if(fd<0){return -1;}
#ifdef _LIBNDS_MAJOR_
	struct stat st;
	fstat(fd,&st);
	FILE_STRUCT *fs=(FILE_STRUCT*)__get_handle(fd)->fileStruct;
	if(actime){
		if(actime==1)actime=st.st_atime;
		UTCToDateTime(actime,&fs->fsyncacdate,NULL);
	}
	if(modtime){
		if(modtime==1)modtime=st.st_mtime;
		UTCToDateTime(modtime,&fs->fsyncmoddate,&fs->fsyncmodtime);
	}
	fs->modified=true;
	_FAT_syncToDisc(fs);
	return 0;
#endif
	return -1; //not implemented
}

int libprism_utime(const char *path, time_t actime, time_t modtime){
	int fd=open(path,O_RDWR);
	if(fd<0)return -1;
	int ret=libprism_futime(fd,actime,modtime);
	close(fd);
	return ret;
}

int fgetAttributes(int fd){
	if(fd<0){return -1;}
	FILE_STRUCT *fs=(FILE_STRUCT*)__get_handle(fd)->fileStruct;
	u8 ret=fs->dirEntry.entryData[DIR_ENTRY_attributes];
	return ret;
}

int getAttributes(const char *path){
	int fd=open(path,O_RDONLY);
	if(fd<0)return -1;
	int ret=fgetAttributes(fd);
	close(fd);
	return ret;
}

u8* getDirEntFromDirIter(DIR_ITER *dp){
	if(!dp)return NULL;
	DIR_STATE_STRUCT* state=(DIR_STATE_STRUCT*)dp->dirStruct;
	if(!state->validEntry)return NULL;
	return state->currentEntry.entryData;
}

int libprism_touch(const char *path){
#ifdef _LIBNDS_MAJOR_
	int fd=open(path,O_RDWR);
	if(fd<0){return -1;}
	struct stat st;
	fstat(fd,&st);
	FILE_STRUCT *fs=(FILE_STRUCT*)__get_handle(fd)->fileStruct;
	fs->modified=true;
	_FAT_syncToDisc(fs);
	close(fd);
	return 0;
#endif
	return -1; //not implemented
}

int libprism_fchattr(int fd, u8 attr){
	if(fd<0){return -1;}
#ifdef _LIBNDS_MAJOR_
	attr&=7;
	FILE_STRUCT *fs=(FILE_STRUCT*)__get_handle(fd)->fileStruct;
	fs->fsyncattr=attr;
	fs->modified=true;
	_FAT_syncToDisc(fs);
	return 0;
#endif
	return -1; //not implemented
}

int libprism_chattr(const char *path, u8 attr){
	int fd=open(path,O_RDWR);
	if(fd<0)return -1;
	int ret=libprism_fchattr(fd,attr);
	close(fd);
	return ret;
}

u32 getSectors(){
	PARTITION *p=_FAT_partition_getPartitionFromPath("fat:/");
	if(!p)return 0;
	return p->numberOfSectors;
}
