#include "../../libprism/libprism.h"
#include "hc128_ecrypt-sync.h"
const u16 bgcolor=RGB15(4,0,12);
const int useARM7Bios=0;

extern u8 ndshead[512];
static const u8 null256[256]={0};

// thanks a lot, zorgluf!
// http://www.teamcyclops.com/forum/showpost.php?p=93653&postcount=24
static const char *_key =       "c15c09d26939def94b2c110d6ffed971";
static const char *_iv_header = "3847D9EAC5B999457162C6E74F20420A";
//static const char *_iv_arm9 =   "6D78EBD08243DF63800BA2F00549A18F";
//static const char *_iv_arm7 =   "B2D75636E1F11C1315E06CA590E9F10F";

static int txt2bin(const char *src, u8 *dst){
	int i=0;
	for(;i<64;i++){
		unsigned char src0=src[2*i];
		if(!src0||src0==' '||src0=='\t'||src0=='\r'||src0=='\n'||src0=='#'||src0==';'||src0=='\''||src0=='"')break;
		if(0x60<src0&&src0<0x7b)src0-=0x20;
		//if(!( isdigit(src0)||(0x40<src0&&src0<0x47) )){fprintf(stderr,"Invalid character %c\n",src0);exit(-1);}
		src0=isdigit(src0)?(src0-'0'):(src0-55);

		unsigned char src1=src[2*i+1];
		if(0x60<src1&&src1<0x7b)src1-=0x20;
		//if(!( isdigit(src1)||(0x40<src1&&src1<0x47) )){fprintf(stderr,"Invalid character %c\n",src1);exit(-1);}
		src1=isdigit(src1)?(src1-'0'):(src1-55);
		dst[i]=(src0<<4)|src1;
		//fprintf(stderr,"%02X",dst[i]);
	}
	//if(i!=16){printf("key/iv length must be 16bytes in hex\n");exit(-1);}
	return i;
//puts("");
}

bool Bootievo(const char *pFilename){
	struct stat st;
	u32 FileSize;
	u8 *pFileBuf;
	u8 head[512];

	FILE *FileHandle=fopen(pFilename,"rb");
	fread(head,1,512,FileHandle);
  
	if(FileHandle==NULL)
		{_consolePrintf("Can not open NDS file %s.\n",pFilename);return false;}

	fstat(fileno(FileHandle),&st);
	FileSize=st.st_size;
  
	if(FileSize==0)
		{_consolePrintf("Can not open NDS file %s.\n",pFilename);return false;}

	_consolePrint("MainRam loader X with ievo hc128 cipher preparing.\n");

	u8 key[16],iv[16];
	txt2bin(_key,key);
	txt2bin(_iv_header,iv);

	ECRYPT_ctx ctx;
	ECRYPT_keysetup(&ctx,key,128,128);
	ECRYPT_ivsetup(&ctx,iv);

	fseek(FileHandle,0x80,SEEK_SET);
	fread(head,1,512,FileHandle);
	ECRYPT_process_bytes(0,&ctx,head,head,512); //first arg is 0:encrypt, 1:decrypt. but as synmetric, we don't care.

	u32 o9=read32(head+0x08);
	u32 r9=read32(head+0x0c);
	u32 a9=read32(head+0x10);
	u32 l9=read32(head+0x14);

	u32 o7=read32(head+0x1c);
	u32 r7=read32(head+0x20);
	u32 a7=read32(head+0x24);
	u32 l7=read32(head+0x28);

	//fwrite(head,1,512,stdout);
	//iv_arm9 = head+0xb0
	//iv_arm7 = head+0xc0

	u32 pad9=0x100-(l9&0xff);if(pad9==0x100)pad9=0;
	u32 pad7=0x100-(l7&0xff);if(pad7==0x100)pad7=0;

	//writing loader to memory...
	pFileBuf=(u8*)(0x02000000+1*1024*1024); //uses 0x02100000.
	memcpy(pFileBuf,ndshead,512);
	write32(pFileBuf+0x24,r9);
	write32(pFileBuf+0x28,a9);
	write32(pFileBuf+0x2c,l9/*+pad9*/);
	write32(pFileBuf+0x30,l9+pad9+0x200);
	write32(pFileBuf+0x34,r7);
	write32(pFileBuf+0x38,a7);
	write32(pFileBuf+0x3c,l7/*+pad7*/);
	write32(pFileBuf+0x80,0x200+l9+pad9+l7+pad7);
	write16(pFileBuf+0x15e,swiCRC16(0xffff,pFileBuf,0x15e));

	_consolePrint("decoding arm9...\n");
	//txt2bin(_iv_arm9,iv);
	//ECRYPT_ivsetup(&ctx,iv);
	ECRYPT_ivsetup(&ctx,head+0xb0);
	fseek(FileHandle,o9,SEEK_SET);
	fread(pFileBuf+0x200,1,l9,FileHandle);
	ECRYPT_process_bytes(0,&ctx,pFileBuf+0x200,pFileBuf+0x200,l9);

	_consolePrint("decoding arm7...\n");
	//txt2bin(_iv_arm7,iv);
	//ECRYPT_ivsetup(&ctx,iv);
	ECRYPT_ivsetup(&ctx,head+0xc0);
	fseek(FileHandle,o7,SEEK_SET);
	fread(pFileBuf+0x200+l9+pad9,1,l7,FileHandle);
	ECRYPT_process_bytes(0,&ctx,pFileBuf+0x200+l9+pad9,pFileBuf+0x200+l9+pad9,l7);
	fclose(FileHandle);

	//FileHandle=fopen("/loader.bin","wb");
	//fwrite(pFileBuf,1,0x200+l9+pad9+l7+pad7,FileHandle);
	//fclose(FileHandle);
  
	_consolePrint("Rebooting...\n");
	if(!argvToInstall)makeargv(pFilename);
	installargv(pFileBuf,(char*)0x02fff400);
	bootMoonlight((u32)pFileBuf+0xc0); //doesn't need to care about logo's address as it is already fixed.
	return true;
}

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
	_consolePrintf(
		"MoonShellExecute Soft Reset DLDI\n"
		"reset_mse_06b_for_ak2 by Moonlight, Rudolph, kzat3\n"
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

	// vvvvvvvvvvv add 2008.03.30 kzat3
	char *__name=processlinker(file);
	if(!__name){_consolePrintf("cannot get link for %s.\n",file);die();}
	_consolePrint("rebooting with resetmoonshell...\n");
	Bootievo(__name);
	_consolePrint("failed.\n");
	die();
	// ^^^^^^^^^^^^ add 2008.03.30 kzat3
}
