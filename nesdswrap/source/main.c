#include "../../libprism/libprism.h"
#include "nesds_arm7_lzma.h"
#include "nesds_lzma.h"
#include "nesds_ex031_lzma.h"
#include "nesds_ex040_lzma.h"
#include "nesds_ex042_lzma.h"
#include "nesds_ex043_lzma.h"
#include "nesds_ex044_lzma.h"

#include "nesterds_nds_lzma.h"
#include "nesDS_r62_nds_lzma.h"
#include "nesDS_r69_nds_lzma.h"
#include "nesDS_r87_nds_lzma.h"
#include "nesDS_nds_lzma.h"

#include "LzmaDec.h"

const u16 bgcolor=RGB15(31,31,31);
//const u16 bgcolor=RGB15(0,0,0);
const int useARM7Bios=0;
extern unsigned char ndshead[512];
u32 keys;

/*
	nesDS all in one

	hold Start:  nesDS EX 031
	hold Select: nesterDS moonlight (only extlink)
	hold A:      nesDS ORG
	hold B:      nesDS (EX) 040
	hold X:      nesDS (EX) 042
	hold Y:      nesDS (EX) 043
	hold L:      nesDS (EX) 044
	hold R:      nesDS r62 (0.48b)
	hold Up:     nesDS r69 (0.51a)
	hold Down:   nesDS r87+ (0.56a1)
	hold Left:   
	hold Right:  
	none:        nesDS r114 (1.1b)

	nesDS EX series 0.22/0.30 aren't implemented. Use 0.31 (final version).
	Please note that nesDS ORG/EX/0.44's arm7 are shared.
*/

static void *SzAlloc(void *p, size_t size) { p = p; return malloc(size); }
static void SzFree(void *p, void *address) { p = p; free(address); }
static ISzAlloc g_Alloc = { SzAlloc, SzFree };

void ImageDecode(const u8 *img, u32 size, u8 *buf){
	//swiDecompressLZSSWram((u8*)img,buf);

	ELzmaStatus lzmastatus;
	u32 bufsize=(u32)read64(img+5);
	u32 srcsize=size-13;
	LzmaDecode((u8*)buf,&bufsize,img+13,&srcsize,img,5,LZMA_FINISH_ANY,&lzmastatus,&g_Alloc);
}

void loadNesDSLegacy(){
	int i;

	u32 l9,cl9;
	const u8* p9;
	u32 l7,cl7;
	const u8* p7;

	if(keys&KEY_A){
		l9=(u32)read64(nesds_lzma+5);
		cl9=nesds_lzma_size;
		p9=nesds_lzma;
	}else if(keys&KEY_START){
		l9=(u32)read64(nesds_ex031_lzma+5);
		cl9=nesds_ex031_lzma_size;
		p9=nesds_ex031_lzma;
	}else if(keys&KEY_B){
		l9=(u32)read64(nesds_ex040_lzma+5);
		cl9=nesds_ex040_lzma_size;
		p9=nesds_ex040_lzma;
	}else if(keys&KEY_X){
		l9=(u32)read64(nesds_ex042_lzma+5);
		cl9=nesds_ex042_lzma_size;
		p9=nesds_ex042_lzma;
	}else if(keys&KEY_Y){
		l9=(u32)read64(nesds_ex043_lzma+5);
		cl9=nesds_ex043_lzma_size;
		p9=nesds_ex043_lzma;
	}else if(keys&KEY_L){
		l9=(u32)read64(nesds_ex044_lzma+5);
		cl9=nesds_ex044_lzma_size;
		p9=nesds_ex044_lzma;
	}else{die();} //lol

		l7=(u32)read64(nesds_arm7_lzma+5);
		cl7=nesds_arm7_lzma_size;
		p7=nesds_arm7_lzma;

	//writing nesds to memory...
	u8 *pFileBuf=(u8*)(0x02000000+2*1024*1024);
	memcpy(pFileBuf,ndshead,512);
	write32(pFileBuf+0x24,0x02000000);
	write32(pFileBuf+0x28,0x02000000);
	write32(pFileBuf+0x2c,l9+32768);
	write32(pFileBuf+0x30,l9+32768+0x200);
	write32(pFileBuf+0x34,0x037f8000);
	write32(pFileBuf+0x38,0x037f8000);
	write32(pFileBuf+0x3c,l7);
	write32(pFileBuf+0x80,0x200+l9+32768+l7);
	write16(pFileBuf+0x15e,swiCRC16(0xffff,pFileBuf,0x15e));
	ImageDecode(p9,cl9,pFileBuf+0x200);
	memcpy(pFileBuf+0x200+l9,DLDINull,0x88);
	for(i=0x40;i<0x88;i+=4)if(read24(pFileBuf+0x200+l9+i+1)==0xbf8000)write24(pFileBuf+0x200+l9+i+1,0x023d80); //align to 0x023d8000
	dldi2(pFileBuf+0x200+l9,32768,0,NULL);
	ImageDecode(p7,cl7,pFileBuf+0x200+l9+32768);
	bootMoonlight((u32)pFileBuf+0xc0);
}

void Main(){
	strcpy(libprism_cbuf,myname);
	if(*argname){
		strcat(libprism_cbuf,"\n");
		strcat(libprism_cbuf,argname);
	}
	makeargv(libprism_cbuf);

	keys=IPCZ->keysheld;
	if(keys&(KEY_A|KEY_B|KEY_X|KEY_Y|KEY_L|KEY_START))loadNesDSLegacy();

	u32 l9,cl9;
	const u8* p9;

	if(keys&KEY_SELECT){
		l9=(u32)read64(nesterds_nds_lzma+5);
		cl9=nesterds_nds_lzma_size;
		p9=nesterds_nds_lzma;
	}else if(keys&KEY_R){
		l9=(u32)read64(nesDS_r62_nds_lzma+5);
		cl9=nesDS_r62_nds_lzma_size;
		p9=nesDS_r62_nds_lzma;
	}else if(keys&KEY_UP){
		l9=(u32)read64(nesDS_r69_nds_lzma+5);
		cl9=nesDS_r69_nds_lzma_size;
		p9=nesDS_r69_nds_lzma;
	}else if(keys&KEY_DOWN){
		l9=(u32)read64(nesDS_r87_nds_lzma+5);
		cl9=nesDS_r87_nds_lzma_size;
		p9=nesDS_r87_nds_lzma;
	}else{
		l9=(u32)read64(nesDS_nds_lzma+5);
		cl9=nesDS_nds_lzma_size;
		p9=nesDS_nds_lzma;
	}

	//loadNormal
	u8 *pFileBuf=(u8*)(0x02000000+2*1024*1024);
	ImageDecode(p9,cl9,pFileBuf);
	dldi2(pFileBuf+0x200,l9-0x200,0,NULL);
	bootMoonlight((u32)pFileBuf+0xc0);
}

