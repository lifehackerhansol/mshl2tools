#include "../../libprism/libprism.h"
#include <fcntl.h>
const u16 bgcolor=RGB15(4,0,12);
const int useARM7Bios=0;

#define CLUSTER_EOF_16	0xFFFF
#define CLUSTER_EOF		0x0FFFFFFF
#define CLUSTER_FREE	0x00000000
#define CLUSTER_ROOT	0x00000000
#define CLUSTER_FIRST	0x00000002
#define CLUSTER_ERROR	0xFFFFFFFF

static int fsetClusterMap(int fd, u32 *clustmap){
	u32 clust,tmp,ret=1;
	u32 p=getPartitionHandle();
	if(!p)return -1;
	struct stat st;
	if(fstat(fd,&st))return 0;
	for(clust=*clustmap++=st.st_ino;(tmp=nextCluster(p,clust))!=CLUSTER_EOF;){
		if(tmp==CLUSTER_ERROR)return -1;
		clust=*clustmap++=tmp;
		ret++;
	}
	return ret;
}

static u32 getTrueSector(u32 sec, u32 isSave){
	u32 area;
	sec>>=9; //physical addr -> sector
	//512MB ROMs use 16384 clusters. 64MB SAVs use 2048 clusters. Map Total 73728bytes.
	area=sec%64;
	sec>>=6; //compress to 1/64
	if(isSave)sec+=16384;
	return (((vu32*)0x02fd0000)[sec] << 6) + *(vu32*)0x02fe2000 + area;
}

static u8 WooD_DLDI[]={
	0x00,0xA5,0x8D,0xBF,0x20,0x43,0x68,0x69,0x73,0x68,0x6D,0x00,0x01,0x0F,0x00,0x0F,
	0x44,0x75,0x6D,0x6D,0x79,0x20,0x44,0x4C,0x44,0x49,0x20,0x66,0x6F,0x72,0x20,0x57,
	0x6F,0x6F,0x64,0x20,0x44,0x69,0x73,0x74,0x6F,0x72,0x74,0x69,0x6F,0x6E,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x80,0x3E,0x02,0x88,0x80,0x3E,0x02,0x88,0x80,0x3E,0x02,0x88,0x80,0x3E,0x02,
	0x88,0x80,0x3E,0x02,0x88,0x80,0x3E,0x02,0x88,0x80,0x3E,0x02,0x88,0x80,0x3E,0x02,
	0x57,0x6F,0x6F,0x44,0x23,0x00,0x00,0x00,0x80,0x80,0x3E,0x02,0x80,0x80,0x3E,0x02,
	0x80,0x80,0x3E,0x02,0x80,0x80,0x3E,0x02,0x80,0x80,0x3E,0x02,0x80,0x80,0x3E,0x02,
	0x00,0x00,0xA0,0xE3,0x1E,0xFF,0x2F,0xE1,
};

static char file[256*3]="MoonShellExecute\0\0\0\0\0\0\0\0\0\0\0" //will be modified externally
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

void Main(){
	*WooD_DLDI=0xED;

	_consolePrintf(
		"reset_mse_arco\n"
		"MoonShell Simply Loader by Moonlight\n"
		"dldipatch aka dlditool public domain under CC0.\n"
		"%s\n%s\n\n",
		ROMDATE,ROMENV
	);

	{
		unsigned char dldiid[5];
		unsigned char *dldiFileData=DLDIDATA;
		_consolePrintf("DLDI Interface: 0x%08x\n",dldiFileData);
		memcpy(dldiid,(char*)dldiFileData+ioType,4);
		dldiid[4]=0;
		_consolePrintf("DLDI ID: %s\n",dldiid);
		_consolePrintf("DLDI Name: %s\n\n",(char*)dldiFileData+friendlyName);
	}

	//_consolePrint("Waiting... ");
	//sleep(1);
	//_consolePrint("Done.\n");

	_consolePrint("initializing FAT... ");
	if(!disc_mount()){_consolePrint("failed.\n");die();}
	_consolePrint("done.\n");

	_consolePrint("Building cluster map... ");
	int fd=open(file,O_RDONLY);
	if(fd<0){_consolePrintf("cannot open %s\n",file);die();}
	memset((u8*)0x02fd0000,0,0x20000);
	u32 clusters=fsetClusterMap(fd,(u32*)0x02fd0000);
	*(u32*)0x02fe2000=getFatDataPointer();
	close(fd);
	_consolePrint("Done.\n");

	//We don't need libfat
	disc_unmount();

	//Now that we have lost FAT, let's cast the spell.
	//Basically, we can copy DLDI template to 0x02fe8000 then patch it again.
	memcpy((u8*)0x02fe8000,WooD_DLDI,sizeof(WooD_DLDI));
	dldi2((u8*)0x02fe8000,32768,0,NULL);
	typedef bool (*type_dldiStartup)();
	type_dldiStartup dldistartup=*(type_dldiStartup*)0x02fe8068;
	dldistartup();

	_consolePrint("Making NDS image... ");
	//Direct DLDI call
	typedef bool (*type_readSectors)(u32, u32, void*);
	type_readSectors readsector=*(type_readSectors*)0x02fe8070;
	u8 *buf=(u8*)0x02200000;
	int i=0;
	for(;i<clusters*64;i++){
		readsector(getTrueSector(i<<9,0),1,buf+512*i);
	}
	_consolePrint("Done.\n");

	DLDIToBoot=(u8*)0x02fe8000;
	dldi2(buf,clusters*32768,0,NULL);
	if(!argvToInstall)makeargv(file);
	installargv(buf,(char*)0x02fff400);
	bootMoonlight((u32)buf+0xc0);
}
