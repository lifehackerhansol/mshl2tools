/*
	fatx.c - Very special FAT helper API
	Make sure to use my modified libfat.
	Calling getsfnlfn()/libprism_[f]utime() with unmodified libfat will cause unexpected result.
	If you use other functions with unmodified libfat, it will mess something when used for folder.
*/

#include "libprism.h"

#include <fcntl.h> //O_
#include <sys/iosupport.h> //for safer getting FILE_STRUCT
__handle* __get_handle(int);
//static u8 direntry[0x20];

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

///// start of actual code /////

#if defined(LIBFAT) && defined(LIBELM)
#error both LIBFAT and LIBELM are defined
#elif defined(LIBFAT)
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
#ifdef USE_LIBFAT109
	u32 numberFreeCluster;
	u32 numberLastAllocCluster;
#endif
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
#ifdef USE_LIBFAT109
	uint32_t              fsInfoSector;
#endif
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

static u32 _FAT_fat_clusterToSector(PARTITION* partition, u32 cluster){
	return (cluster >= CLUSTER_FIRST) ? 
		(cluster - CLUSTER_FIRST) * partition->sectorsPerCluster + partition->dataStart :
		partition->rootDirStart;
}

void mbstoucs2(u16* dst, const char* src){_FAT_directory_mbstoucs2(dst,src,256);}
void ucs2tombs(char* dst, const u16* src){_FAT_directory_ucs2tombs(dst,src,768);}
u32  getPartitionHandle(){return (u32)_FAT_partition_getPartitionFromPath("fat:/");}
u32  nextCluster(u32 handle, u32 cluster){return _FAT_fat_nextCluster((PARTITION*)handle,cluster);}

u32 getFatDataPointer(){ //for getTrueSector
	PARTITION *p=_FAT_partition_getPartitionFromPath("fat:/");
	if(p->bytesPerSector!=512||p->sectorsPerCluster!=64){_consolePrint("getFatDataPointer() will be useless if bytesPerSector!=512 || sectorsPerCluster!=64.\n");die();}
	return p->dataStart-128; //back 2 clusters
}

void getsfnlfn(const char *path,char *sfn,u16 *lfn){ //path should be in UTF8
	static char buf[256*3],ret[256*3];
	int i=1,len;
	FILE_STRUCT *fs;
	int fd;

	if(!path)return;
	if(!sfn&&!lfn)return;
	memset(&buf,0,256*3);
	memset( sfn,0,256);
	memset(&ret,0,256*3);
	//if(!memcmp(path,"/./",3))path+=2;
	len=strlen(path);
	for(;i<=len;i++){
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
				//bool NTF_lowfn=false,NTF_lowext=false;
				//char *x;
				strcat(ret,"/");
				strcat(ret+strlen(ret),fs->dirEntry.filename);
#if 0
		///// Now my modified libfat returns LFN also for 8.3 files in fs->dirEntry.filename
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
			if(posperiod==(u32)-1){ //without ext
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
#endif
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
		"filesysType:        %s\n"
		"totalSize:          %dK\n"
		"rootDirStart:       %u\n"
		"rootDirCluster:     %u\n"
		"numberOfSectors:    %u\n"
		"dataStart:          %u\n"
		"bytesPerSector:     %u\n"
		"sectorsPerCluster:  %u\n"
		"bytesPerCluster:    %u\n"
		"fat.fatStart:       %u\n"
		"fat.sectorsPerFat:  %u\n"
		"fat.lastCluster:    %u\n"
		"fat.firstFree:      %u\n",
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
#ifdef USE_LIBFAT109
	writer(
		"fsInfoSector:       %u\n"
		"fat.numFreeCluster: %u\n"
		"fat.numLastAlloc:   %u\n",
		p->fsInfoSector,
		p->fat.numberFreeCluster,
		p->fat.numberLastAllocCluster
	);
#endif
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

bool disc_mount(){
	return fatInitDefault();
}

void disc_unmount(){ //You must call this before softreset; otherwise fsinfo will got messed up.
#ifdef _LIBNDS_MAJOR_
	fatUnmount("fat:/");
#else
	fatUnmount(0);
#endif
	//make sure...
	disc_shutdown();
}

#elif defined(LIBELM)
u32 UTCToDosTime(const u32 timer){
	struct tm *t=localtime(&timer);
	return (u32)(
		((t->tm_year-80)<<25) |
		((t->tm_mon+1)<<21) |
		(t->tm_mday<<16) |
		(t->tm_hour<<11) |
		(t->tm_min<<5) |
		(t->tm_sec>>1)
	);
}

u32 DosTimeToUTC(const u32 dostime){
	struct tm t;
	memset(&t,0xff,sizeof(t));
	t.tm_year=(dostime>>25)+80;
	t.tm_mon=((dostime>>21)&0x0f)-1;
	t.tm_mday=(dostime>>16)&0x1f;
	t.tm_hour=(dostime>>11)&0x1f;
	t.tm_min=(dostime>>5)&0x3f;
	t.tm_sec=(dostime&0x1f)<<1;
	return mktime(&t);
}

void mbstoucs2(u16* dst, const char* src){u16 *p=_ELM_mbstoucs2(src,NULL);Unicode_Copy(dst,p);}
void ucs2tombs(char* dst, const u16* src){_ELM_ucs2tombs(dst,src);}
u32  getPartitionHandle(){return (u32)_ELM_getFATFS();}
u32  nextCluster(u32 handle, u32 cluster){return get_fat((FATFS*)handle,cluster);}

u32 getFatDataPointer(){ //for getTrueSector
	FATFS *p=_ELM_getFATFS();
	if(/*p->ssize!=512||*/p->csize!=64){_consolePrint("getFatDataPointer() will be useless if bytesPerSector!=512 || sectorsPerCluster!=64.\n");die();}
	return p->database-128; //back 2 clusters
}

void getsfnlfn(const char *path,char *sfn,u16 *lfn){ //path should be in UTF8
	static char buf[256*3],ret[256*3];
	int i=1,len;
	FILINFO fs;

	if(!sfn&&!lfn)return;
	memset(&buf,0,256*3);
	memset( sfn,0,256);
	memset(&ret,0,256*3);
	//if(!memcmp(path,"/./",3))path+=2;
	len=strlen(path);
	for(;i<=len;i++){
		if(path[i]=='/'||i==len){
			memset(&fs,0,sizeof(FILINFO));
			strncpy(buf,path,i);
			//_consolePrintf("***%s\n",buf);
			if(f_stat(_ELM_mbstoucs2(buf,NULL),&fs)){_consolePrintf("\nCannot open %s.\nAccept your fate.\n",buf);die();}
			//_consolePrintf("***%s\n",fs.dirEntry.filename);
			//if(lfn){
				//bool NTF_lowfn=false,NTF_lowext=false;
				//char *x;
			//	strcat(ret,"/");
			//	strcat(ret+strlen(ret),fs->dirEntry.filename);
			//}
			if(sfn){
				int j=0,l;
				strcat(sfn,"/");
				for(l=strlen(sfn);fs.fname[j];j++)
					sfn[l+j]=fs.fname[j];
				sfn[l+j]=0;
			}
		}
	}
	if(lfn)mbstoucs2(lfn,path); //no ways to get LFN for 8.3 files? I just hope path is already LFN.
}

u64 fgetFATEntryAddress(int fd){
	if(fd<0)return 0;
	FIL *fil=(FIL*)__get_handle(fd)->fileStruct;
	if(!fil)return 0;

	return fil->dir_sect*512 + fil->dir_ptr - fil->fs->win;
}

u64 getFATEntryAddress(const char *path){
	int fd=open(path,O_RDONLY);
	if(fd<0)return -1;
	u64 ret=fgetFATEntryAddress(fd);
	close(fd);
	return ret;
}

u32 fgetSector(int fd){
	FATFS *p=_ELM_getFATFS();
	if(!p)return 0;
	struct stat st;
	if(fstat(fd,&st))return 0;
	return clust2sect(p,st.st_ino);
}

u32 getSector(const char *path){
	FATFS *p=_ELM_getFATFS();
	if(!p)return 0;
	struct stat st;
	if(stat(path,&st))return 0;
	return clust2sect(p,st.st_ino);
}

int fgetFragments(int fd){
	if(fd<0)return -1;
	FIL *fil=(FIL*)__get_handle(fd)->fileStruct;
	u32 ret=0;
	u32 clust,tmp,i=1;//,size=((fil->fsize+0x7fff)&~0x7fff)>>15;
	//dbg_printf("size: %d\n",size);
#if 0
	for(clust=fil->sclust;i<size;){
		tmp=get_fat(fil->fs,clust);
		if(tmp==-1)return -1;
		if(clust+1!=tmp)ret++;
		clust=tmp;
		i++;
	}
#endif
	for(clust=fil->sclust;;){
		tmp=get_fat(fil->fs,clust);
		if(tmp<2||tmp>=fil->fs->n_fatent)break;
		if(clust+1!=tmp)ret++;
		clust=tmp;
		i++;
	}
	return ret;
}

int getFragments(const char *path){
	struct stat st;
	stat(path,&st);
	if(st.st_spare1&ATTRIB_DIR)return 0;

	int fd=open(path,O_RDONLY);
	if(fd<0)return -1;
	int ret=fgetFragments(fd);
	close(fd);
	return ret;
}

int writePartitionInfo(type_printf writer){
	FATFS *p=_ELM_getFATFS();
	if(!p||!writer)return -1;

	u32 free_clusters=0;
	f_getfree("\0",&free_clusters,&p);
	writer(
		"filesysType:        %s\n"
		"totalSize:          %dK\n"
		"rootDirStart:       %u\n"
		"numberOfSectors:    %u\n"
		"dataStart:          %u\n"
		"bytesPerSector:     %u\n"
		"sectorsPerCluster:  %u\n"
		"bytesPerCluster:    %u\n"
		"fsInfoSector:       %u\n"
		"fat.fatStart:       %u\n"
		"fat.sectorsPerFat:  %u\n"
		"fat.lastCluster:    %u\n"
		"fat.numFreeCluster: %u\n"
		"fat.numLastAlloc:   %u\n",
		p->fs_type==FS_FAT12?"FAT12":p->fs_type==FS_FAT16?"FAT16":p->fs_type==FS_FAT32?"FAT32":"Unknown",
		((p->n_fatent-2)*p->csize)>>1,
		p->dirbase,
		(p->n_fatent-2)*p->csize,
		p->database,
		512,
		p->csize,
		p->csize*512,
		p->fsi_sector,
		p->fatbase,
		p->fsize,
		p->n_fatent,
		free_clusters,//p->free_clust,
		p->last_clust
	);
	return 0;
}

//not implemented in libelm edition
//int libprism_futime(int fd, time_t actime, time_t modtime);

int libprism_utime(const char *path, time_t actime, time_t modtime){
	if(modtime==1)return 0;
	if(modtime==0)return libprism_touch(path);
	u32 dostime=UTCToDosTime(modtime);
	FILINFO fi;
	fi.fdate=dostime>>16;
	fi.ftime=(u16)dostime;
	return f_utime(_ELM_mbstoucs2(_ELM_realpath(path),NULL),&fi);
}

int libprism_touch(const char *path){
	u32 dostime=UTCToDosTime(time(NULL));
	FILINFO fi;
	fi.fdate=dostime>>16;
	fi.ftime=(u16)dostime;
	return f_utime(_ELM_mbstoucs2(_ELM_realpath(path),NULL),&fi);
}

//not implemented in libelm edition
//int libprism_fchattr(int fd, u8 attr);

int libprism_chattr(const char *path, u8 attr){
	int ret=f_chmod(_ELM_mbstoucs2(_ELM_realpath(path),NULL),attr,0x27); //mask all
	return ret;
}

u32 getSectors(){
	FATFS *p=_ELM_getFATFS();
	if(!p)return 0;
	return p->csize*(p->n_fatent-2);
}

bool disc_mount(){
	return ELM_Mount();
}

void disc_unmount(){ //You must call this before softreset; otherwise fsinfo will got messed up.
	ELM_Unmount();
	//make sure...
	disc_shutdown();
}

#else
#error define one of LIBFAT / LIBELM
#endif

