#include "xenofile.h"

static vu32 DMAFIXSRC;

#define CACHE_LINE_SIZE (32)
#define prcdiv (0x40)

#define memchk if(false)

//#include "tcmstart.h"

void DC_FlushRangeOverrun(const void *v,u32 size)
{//memchk{ if(v==NULL){ _consolePrintf("Hooked memory address error.%s%d\n",__FILE__,__LINE__); while(1); } }
  
  u32 va=(u32)v;
  va&=~(CACHE_LINE_SIZE-1);
  size+=CACHE_LINE_SIZE;
  
  size+=CACHE_LINE_SIZE-1;
  size&=~(CACHE_LINE_SIZE-1);
  
  if(va==0) return;
  if(size==0) return;
  
  DC_FlushRange((void*)va,size);
  DC_InvalidateRange((void*)va,size);
}

void MemCopy8CPU(const void *src,void *dst,u32 len)
{//memchk{ if((src==NULL)||(dst==NULL)){ _consolePrintf("Hooked memory address error.%s%d\n",__FILE__,__LINE__); while(1); } }
  if(len==0) return;
  
  u8 *_src=(u8*)src;
  u8 *_dst=(u8*)dst;
  u32 idx=0;
  for(;idx<len;idx++){
    _dst[idx]=_src[idx];
  }
}

void MemCopy16CPU(const void *src,void *dst,u32 len)
{//memchk{ if((src==NULL)||(dst==NULL)){ _consolePrintf("Hooked memory address error.%s%d\n",__FILE__,__LINE__); while(1); } }
  len/=2;
  
  u16 *_src=(u16*)src;
  u16 *_dst=(u16*)dst;
  u32 idx=0;
  for(;idx<len;idx++){
    _dst[idx]=_src[idx];
  }
}

void MemCopy32CPU(const void *src,void *dst,u32 len)
{//memchk{ if((src==NULL)||(dst==NULL)){ _consolePrintf("Hooked memory address error.%s%d\n",__FILE__,__LINE__); while(1); } }
  len/=4;
  
  u32 *_src=(u32*)src;
  u32 *_dst=(u32*)dst;
  u32 idx=0;
  for(;idx<len;idx++){
    _dst[idx]=_src[idx];
  }
}

void MemSet16CPU(const vu16 v,void *dst,u32 len)
{//memchk{ if(dst==NULL){ _consolePrintf("Hooked memory address error.%s%d\n",__FILE__,__LINE__); while(1); } }
  len/=2;
  
  u16 _src=v;
  u16 *_dst=(u16*)dst;
  u32 idx=0;
  for(;idx<len;idx++){
    _dst[idx]=_src;
  }
}

void MemSet32CPU(const u32 v,void *dst,u32 len)
{//memchk{ if(dst==NULL){ _consolePrintf("Hooked memory address error.%s%d\n",__FILE__,__LINE__); while(1); } }
  len/=4;
  
  u32 _src=v;
  u32 *_dst=(u32*)dst;
  u32 idx=0;
  for(;idx<len;idx++){
    _dst[idx]=_src;
  }
}

void MemSet8CPU(const u8 v,void *dst,u32 len)
{//memchk{ if(dst==NULL){ _consolePrintf("Hooked memory address error.%s%d\n",__FILE__,__LINE__); while(1); } }
  len/=1;

  u8 _src=v;
  u8 *_dst=(u8*)dst;
  
  while(len!=0){
    *_dst++=_src;
    len--;
  }
}

void MemCopy16DMA3(const void *src,void *dst,u32 len)
{//memchk{ if((src==NULL)||(dst==NULL)){ _consolePrintf("Hooked memory address error.%s%d\n",__FILE__,__LINE__); while(1); } }
#ifdef notuseMemDMA3
  {
    u16 *_src=(u16*)src;
    u16 *_dst=(u16*)dst;
    
    len/=2;
    
    for(u32 idx=0;idx<len;idx++){
      _dst[idx]=_src[idx];
    }
  }
  return;
#endif
  
  if(len<2) return;
  
  DC_FlushRangeOverrun((void*)src,len);
  DC_FlushRangeOverrun((void*)dst,len);
  
  u8 *_src=(u8*)src;
  u8 *_dst=(u8*)dst;
  
  DMA3_SRC = (uint32)_src;
  DMA3_DEST = (uint32)_dst;
  DMA3_CR = DMA_ENABLE | DMA_SRC_INC | DMA_DST_INC | DMA_16_BIT | (len>>1);
  while(DMA3_CR & DMA_BUSY);
}

void MemCopy32DMA3(const void *src,void *dst,u32 len)
{//memchk{ if((src==NULL)||(dst==NULL)){ _consolePrintf("Hooked memory address error.%s%d\n",__FILE__,__LINE__); while(1); } }
#ifdef notuseMemDMA3
  {
    u32 *_src=(u32*)src;
    u32 *_dst=(u32*)dst;
    
    len/=4;
    
    for(u32 idx=0;idx<len;idx++){
      _dst[idx]=_src[idx];
    }
  }
  return;
#endif
  
  if(len<4) return;
  
  DC_FlushRangeOverrun(src,len);
  DC_FlushRangeOverrun(dst,len);
  
  u8 *_src=(u8*)src;
  u8 *_dst=(u8*)dst;
  
  DMA3_SRC = (uint32)_src;
  DMA3_DEST = (uint32)_dst;
  DMA3_CR = DMA_ENABLE | DMA_SRC_INC | DMA_DST_INC | DMA_32_BIT | (len>>2);
  while(DMA3_CR & DMA_BUSY);
}

void MemSet16DMA3(const u16 v,void *dst,u32 len)
{//memchk{ if(dst==NULL){ _consolePrintf("Hooked memory address error.%s%d\n",__FILE__,__LINE__); while(1); } }
#ifdef notuseMemDMA3
  MemSet16CPU(v,dst,len);
  return;
#endif
  
  if(len<2) return;
  
  DMAFIXSRC=(vu32)v+((vu32)v<<16);
  
  DC_FlushRangeOverrun((void*)&DMAFIXSRC,4);
  DC_FlushRangeOverrun(dst,len);
  
  u8 *_dst=(u8*)dst;
  DMA3_SRC = (uint32)&DMAFIXSRC;
  
  DMA3_DEST = (uint32)_dst;
  DMA3_CR = DMA_ENABLE | DMA_SRC_FIX | DMA_DST_INC | DMA_16_BIT | (len>>1);
  while(DMA3_CR & DMA_BUSY);
}

void MemSet32DMA3(const u32 v,void *dst,u32 len)
{//memchk{ if(dst==NULL){ _consolePrintf("Hooked memory address error.%s%d\n",__FILE__,__LINE__); while(1); } }
#ifdef notuseMemDMA3
  MemSet32CPU(v,dst,len);
  return;
#endif
  
  if(len<4) return;
  
  DMAFIXSRC=(vu32)v;
  
  DC_FlushRangeOverrun((void*)&DMAFIXSRC,4);
  DC_FlushRangeOverrun(dst,len);
  
  u8 *_dst=(u8*)dst;
  DMA3_SRC = (uint32)&DMAFIXSRC;
  
  DMA3_DEST = (uint32)_dst;
  DMA3_CR = DMA_ENABLE | DMA_SRC_FIX | DMA_DST_INC | DMA_32_BIT | (len>>2);
  while(DMA3_CR & DMA_BUSY);
}

void MemSet8DMA3(const u8 v,void *dst,u32 len)
{//memchk{ if(dst==NULL){ _consolePrintf("Hooked memory address error.%s%d\n",__FILE__,__LINE__); while(1); } }
#ifdef notuseMemDMA3
  MemSet8CPU(v,dst,len);
  return;
#endif
  
  if(len<1) return;
  
  u32 v32=v;
  v32=v32 | (v32<<8) | (v32<<16) | (v32<<24);
  
  u8 *pb=(u8*)dst;
  
  if((((u32)pb)&BIT(0))!=0){
    pb[0]=v32;
    pb+=1;
    len-=1;
  }
  
  if((((u32)pb)&BIT(1))!=0){
    u16 *pb16=(u16*)pb;
    pb16[0]=v32;
    pb+=2;
    len-=2;
  }
  
  MemSet32DMA3(v32,pb,len);
  
  if((len&3)==0) return;
  
  pb+=len&(~3);
  len-=len&(~3);
  
  if((len&BIT(1))!=0){
    u16 *pb16=(u16*)pb;
    pb16[0]=v32;
    pb+=2;
    len-=2;
  }
  
  if((len&BIT(0))!=0){
    pb[0]=v32;
  }
  
}

void MemCopy16DMA2(const void *src,void *dst,u32 len)
{//memchk{ if((src==NULL)||(dst==NULL)){ _consolePrintf("Hooked memory address error.%s%d\n",__FILE__,__LINE__); while(1); } }
#ifdef notuseMemDMA2
  MemCopy16CPU(src,dst,len);
  return;
#endif

  DC_FlushRangeOverrun(src,len);
  DC_FlushRangeOverrun(dst,len);
  
  u8 *_src=(u8*)src;
  u8 *_dst=(u8*)dst;
  
  DMA2_SRC = (uint32)_src;
  DMA2_DEST = (uint32)_dst;
  DMA2_CR = DMA_ENABLE | DMA_SRC_INC | DMA_DST_INC | DMA_16_BIT | (len>>1);
  while(DMA2_CR & DMA_BUSY);
}

void MemSet16DMA2(const u16 v,void *dst,u32 len)
{//memchk{ if(dst==NULL){ _consolePrintf("Hooked memory address error.%s%d\n",__FILE__,__LINE__); while(1); } }
#ifdef notuseMemDMA2
  MemSet16CPU(v,dst,len);
  return;
#endif

  DMAFIXSRC=(vu32)v+((vu32)v<<16);
  
  DC_FlushRangeOverrun((void*)&DMAFIXSRC,4);
  DC_FlushRangeOverrun(dst,len);
  
  u8 *_dst=(u8*)dst;
  DMA2_SRC = (uint32)&DMAFIXSRC;
  
  DMA2_DEST = (uint32)_dst;
  DMA2_CR = DMA_ENABLE | DMA_SRC_FIX | DMA_DST_INC | DMA_16_BIT | (len>>1);
  while(DMA2_CR & DMA_BUSY);
}

void MemCopy32swi256bit(const void *src,void *dst,u32 len)
{//memchk{ if((src==NULL)||(dst==NULL)){ _consolePrintf("Hooked memory address error.%s%d\n",__FILE__,__LINE__); while(1); } }
  swiFastCopy((void*)src,dst,COPY_MODE_COPY | (len/4));
}

void safemalloc_halt(void)
{
  //_consolePrint("Please start with the A button if you use M3.\n");
  //ShowLogHalt();
}

void *safemalloc(const int size)
{
  if(size<=0) return(NULL);
  
  return(malloc(size));
}

void safefree(const void *ptr)
{
//  _consolePrintf("_%x_",(u32)ptr);
  /*if(ptr==NULL){
    _consolePrint("safefree Request NullPointer.\n");
    die();
  }*/
  
  free((void*)ptr);
}

//#include "tcmend.h"

bool testmalloc(int size)
{
  if(size<=0) return(false);
  
  void *ptr;
  
  ptr=malloc(size);
  
  if(ptr==NULL) return(false);
  
  free(ptr);
  
  return(true);
}

#define PrintFreeMem_Seg (1*1024)

u32 PrintFreeMem(void)
{
  u32 FreeMemSize=0;
  s32 i=4*1024*1024;
  for(;i!=0;i-=PrintFreeMem_Seg){
    if(testmalloc(i)==true){
      FreeMemSize=i;
      break;
    }
  }
  
  _consolePrintf("FreeMem=%dbyte    \n",FreeMemSize);
  return(FreeMemSize);
}

