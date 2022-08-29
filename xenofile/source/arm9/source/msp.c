#include "xenofile.h"
#include "msp.h"

#include <math.h>

#define ShowPluginInfo

/*
	MoonShell msp.c
	ToDo: refactoring
*/

static s32 max;
static char *title;
static TPluginBody *pCurrentPluginBody;

static void MWin_ProgressShow(char *TitleStr,s32 _Max){title=TitleStr,max=_Max;_consoleStartProgress2();}
static void MWin_ProgressSetPos(s32 _Position){_consolePrintProgress2(title,_Position,max);} //("%s %d%%\r",title,_Position*100/max);}
static void MWin_ProgressHide(void){_consoleEndProgress2();_consolePrint2("Open Success.            \n");}

static int Plugin_msp_fopen(const char *fn){return fopen(fn,"rb");}
static bool Plugin_msp_fclose(int fh){fclose(fh);return true;}
static char *Plugin_GetINIData(){
	TPluginBody *pPB=pCurrentPluginBody;
	return pPB->INIData;
}
static int Plugin_GetINISize(){
	TPluginBody *pPB=pCurrentPluginBody;
	return pPB->INISize;
}
static void *Plugin_GetBINData(){
	TPluginBody *pPB=pCurrentPluginBody;
	if(!pPB->BINFileHandle||!pPB->BINSize)return NULL;

	if(pPB->BINData==NULL){
		pPB->BINData=safemalloc(pPB->BINSize);
		if(pPB->BINData!=NULL){
			fread(pPB->BINData,1,pPB->BINSize,pPB->BINFileHandle);
		}
	}
	return pPB->BINData;
}
static int Plugin_GetBINSize(){
	TPluginBody *pPB=pCurrentPluginBody;
	return(pPB->BINSize);
}
static int Plugin_GetBINFileHandle(){
	TPluginBody *pPB=pCurrentPluginBody;
	if(!pPB->BINFileHandle||!pPB->BINSize)return 0;
	return pPB->BINFileHandle;
}

static u32 formdt_FormatDate(char *str, u32 size, const u32 year, const u32 month, const u32 day){*str=0;return 0;}

static void _consolePrintSet2(int x, int y){}//_consoleClear2();}

static inline const TPlugin_StdLib *Plugin_GetStdLib(){
  static TPlugin_StdLib sPlugin_StdLib={
    _consolePrint2,_consolePrintf2,
    _consolePrintSet2,
    die,
    MWin_ProgressShow,MWin_ProgressSetPos,MWin_ProgressHide,
    Plugin_GetINIData,Plugin_GetINISize,
    Plugin_GetBINData,Plugin_GetBINSize,
    
    DC_FlushRangeOverrun,
    MemCopy8CPU,MemCopy16CPU,MemCopy32CPU,
    MemSet16CPU,MemSet32CPU,MemCopy16DMA3,
    MemCopy32DMA3,MemSet16DMA3,MemSet32DMA3,
    MemSet8DMA3,MemCopy16DMA2,MemSet16DMA2,
    MemCopy32swi256bit,
    malloc,free,
    
    calloc,malloc,free,realloc,
    rand,
    fread,fseek,ftell,
    sprintf,snprintf,
    memchr,memcmp,memcpy,memmove,memset,
    abs,labs,llabs,fabs,fabsf,
    atof,atoi,atol,atoll,
    strcat,strchr,strcmp,strcoll,strcpy,strcspn,strdup,strlen,strncat,strncmp,strncpy,strpbrk,strrchr,strsep,strspn,strstr,strtok,strxfrm,
    
    Plugin_GetBINFileHandle,Plugin_msp_fopen,Plugin_msp_fclose,
    
    extmem_SetCount,extmem_Exists,extmem_Alloc,extmem_Write,extmem_Read,
    
    formdt_FormatDate
  };
  
  return(&sPlugin_StdLib);
}

bool DLL_LoadLibrary(TPluginBody *pPB,const TPlugin_StdLib *pStdLib,void *pbin,int binsize){
  pCurrentPluginBody=NULL;
  memset(pPB,0,sizeof(TPluginBody));
  
  TPluginHeader *pPH=&pPB->PluginHeader;
  
  memmove(pPH,pbin,sizeof(TPluginHeader));
  
#ifdef ShowPluginInfo
  _consolePrint2("MoonShellPlugin Header\n");
  _consolePrintf2("ID=%x ver%d.%d\n",pPH->ID,pPH->VersionHigh,pPH->VersionLow);
  {
    char *pts;
    switch(pPH->PluginType){
      case EPT_None: pts="NULL"; break;
      case EPT_Image: pts="Image"; break;
      case EPT_Sound: pts="Sound"; break;
      case EPT_Clock: pts="Clock"; break;
      default: pts="undefined"; break;
    }
    _consolePrintf2("PluginType=%x %s\n",pPH->PluginType,pts);
  }
  _consolePrintf2("Data 0x%x-0x%x\n",pPH->DataStart,pPH->DataEnd);
  _consolePrintf2("got  0x%x-0x%x\n",pPH->gotStart,pPH->gotEnd);
  _consolePrintf2("bss  0x%x-0x%x\n",pPH->bssStart,pPH->bssEnd);
  {
    char *str=pPH->info;
    _consolePrintf2("Name=%s\n",str);
    while(*str!=0){
      *str++;
    }
    *str++;
    _consolePrintf2("Author=%s\n",str);
  }
  _consolePrint2("\n");
#endif
  
  pPB->DataSize=binsize;
  pPB->pData=pbin;
  
#ifdef ShowPluginInfo
  _consolePrintf2("Plugin LoadAddress=0x%08x\n",(u32)pPB->pData);
#endif
  
  pPB->bssSize=pPH->bssEnd-pPH->bssStart;
  pPB->pbss=malloc(pPB->bssSize);
  
  if(pPB->pbss==NULL){
    _consolePrint2("LoadLibrary:bss malloc error.\n");
    DLL_FreeLibrary(pPB,false);
    return(false);
  }
  
  memset(pPB->pbss,0,pPB->bssSize);
  
#ifdef ShowPluginInfo
  u32 plug_got_bssbaseofs=pPH->bssStart;
#endif
  u32 plug_got_ofs=pPH->gotStart-pPH->DataStart;
  u32 plug_got_size=pPH->gotEnd-pPH->gotStart;
  
#ifdef ShowPluginInfo
  _consolePrintf2("allocbss 0x%08x (0x%xbyte)\n",(u32)pPB->pbss,pPB->bssSize);
  _consolePrintf2("got_bssbaseofs=0x%x\n",plug_got_bssbaseofs);
  _consolePrintf2("got_ofs=0x%x got_size=0x%x\n",plug_got_ofs,plug_got_size);
#endif
  
  {
    u32 *padr=(u32*)pPB->pData;
    u32 idx=64/4;
    for(;idx<plug_got_ofs/4;idx++){
      u32 adr=padr[idx];
      if((pPH->bssStart<=adr)&&(adr<pPH->bssEnd)){
#ifdef ShowPluginInfo
//        _consolePrintf2("b%x:%x ",idx*4,adr);
#endif
        padr[idx]=(u32)pPB->pbss+(adr-pPH->bssStart);
        }else{
        if((pPH->DataStart<=adr)&&(adr<pPH->DataEnd)){
#ifdef ShowPluginInfo
//          _consolePrintf2("d%x:%x ",idx*4,adr);
#endif
          padr[idx]=(u32)pPB->pData+(adr-pPH->DataStart);
        }
      }
    }
  }
  
  {
    u32 *padr=(u32*)pPB->pData;
    u32 idx=(plug_got_ofs+plug_got_size)/4;
    for(;idx<((u32)pPH->DataEnd-(u32)pPH->DataStart)/4;idx++){
      u32 adr=padr[idx];
      if((pPH->bssStart<=adr)&&(adr<pPH->bssEnd)){
#ifdef ShowPluginInfo
//        _consolePrintf2("b%x:%x ",idx*4,adr);
#endif
        padr[idx]=(u32)pPB->pbss+(adr-pPH->bssStart);
        }else{
        if((pPH->DataStart<=adr)&&(adr<pPH->DataEnd)){
#ifdef ShowPluginInfo
//          _consolePrintf2("d%x:%x ",idx*4,adr);
#endif
          padr[idx]=(u32)pPB->pData+(adr-pPH->DataStart);
        }
      }
    }
  }
  
  {
    u32 *padr=(u32*)((u32)pPB->pData+plug_got_ofs);
    u32 idx=0;
    for(;idx<plug_got_size/4;idx++){
      u32 adr=padr[idx];
      if((pPH->bssStart<=adr)&&(adr<pPH->bssEnd)){
#ifdef ShowPluginInfo
//        _consolePrintf2("b%x:%x ",idx*4,adr);
#endif
        padr[idx]=(u32)pPB->pbss+(adr-pPH->bssStart);
        }else{
        if((pPH->DataStart<=adr)&&(adr<pPH->DataEnd)){
#ifdef ShowPluginInfo
//          _consolePrintf2("d%x:%x ",idx*4,adr);
#endif
          padr[idx]=(u32)pPB->pData+(adr-pPH->DataStart);
        }
      }
    }
    
  }
  
  {
    u32 src;
    u32 *pdst;
    
    src=pPH->LoadLibrary;
    if(src==0){
      _consolePrint2("LoadLibrary:BootStrap function is NULL.\n");
      DLL_FreeLibrary(pPB,false);
      return(false);
    }
    pdst=(u32*)&pPB->LoadLibrary;
    *pdst=(u32)pPB->pData+(src-pPH->DataStart);
    
    src=pPH->FreeLibrary;
    if(src==0){
      _consolePrint2("LoadLibrary:BootStrap function is NULL.\n");
      DLL_FreeLibrary(pPB,false);
      return(false);
    }
    pdst=(u32*)&pPB->FreeLibrary;
    *pdst=(u32)pPB->pData+(src-pPH->DataStart);
    
    src=pPH->QueryInterfaceLibrary;
    if(src==0){
      _consolePrint2("LoadLibrary:BootStrap function is NULL.\n");
      DLL_FreeLibrary(pPB,false);
      return(false);
    }
    pdst=(u32*)&pPB->QueryInterfaceLibrary;
    *pdst=(u32)pPB->pData+(src-pPH->DataStart);
  }
  
  _consolePrintf2("0x%08x LoadLibrary\n",(u32)pPB->LoadLibrary);
  _consolePrintf2("0x%08x FreeLibrary\n",(u32)pPB->FreeLibrary);
  _consolePrintf2("0x%08x QueryInterfaceLib.\n",(u32)pPB->QueryInterfaceLibrary);
  
  if(pPB->LoadLibrary==NULL){
    _consolePrint2("LoadLibrary:LoadLibrary() is NULL.\n");
    DLL_FreeLibrary(pPB,false);
    return(false);
  }
  
  bool res=pPB->LoadLibrary(pStdLib,(u32)pPB->pData);
  
  if(res==false){
    _consolePrint2("LoadLibrary:LoadLibrary() false.\n");
    DLL_FreeLibrary(pPB,false);
    return(false);
  }
  
  pPB->pIL=NULL;
  pPB->pSL=NULL;
  pPB->pCL=NULL;
  pPB->pSE=NULL;
  
  switch(pPH->PluginType){
    case EPT_None: {
#ifdef ShowPluginInfo
      _consolePrint2("LoadLibrary:PluginType == None.\n");
#endif
      DLL_FreeLibrary(pPB,false);
      return(false);
    } break;
    case EPT_Image: {
      pPB->pIL=(TPlugin_ImageLib*)pPB->QueryInterfaceLibrary();
#ifdef ShowPluginInfo
      _consolePrintf2("ImageInterfacePtr 0x%08x\n",(u32)pPB->pIL);
#endif
    } break;
    case EPT_Sound: {
      pPB->pSL=(TPlugin_SoundLib*)pPB->QueryInterfaceLibrary();
#ifdef ShowPluginInfo
      _consolePrintf2("SoundInterfacePtr 0x%08x\n",(u32)pPB->pSL);
#endif
    } break;
/*
    case EPT_Clock: {
      pPB->pCL=(TPlugin_ClockLib*)pPB->QueryInterfaceLibrary();
#ifdef ShowPluginInfo
      _consolePrintf2("ClockInterfacePtr 0x%08x\n",(u32)pPB->pCL);
#endif
    } break;
    case EPT_SoundEffect: {
      pPB->pSE=(TPlugin_SoundEffectLib*)pPB->QueryInterfaceLibrary();
#ifdef ShowPluginInfo
      _consolePrintf2("SoundEffectInterfacePtr 0x%08x\n",(u32)pPB->pSE);
#endif
    } break;
*/
  }
  
  if((pPB->pIL==NULL)&&(pPB->pSL==NULL)&&(pPB->pCL==NULL)&&(pPB->pSE==NULL)){
    _consolePrint2("LoadLibrary:not found function list error.\n");
    DLL_FreeLibrary(pPB,false);
    return(false);
  }
  
#ifdef ShowPluginInfo
  _consolePrint2("LoadLibrary:Initialized.\n");
#endif

  return(true);
}

void DLL_FreeLibrary(TPluginBody *pPB,bool callfree)
{
  if(callfree==true){
    if(pPB!=NULL){
      if(pPB->FreeLibrary!=NULL) pPB->FreeLibrary();
    }
  }
  
  if(pPB->pData!=NULL){
    free(pPB->pData); pPB->pData=NULL;
  }
  if(pPB->pbss!=NULL){
    free(pPB->pbss); pPB->pbss=NULL;
  }
  
//  memset(pPB,0,sizeof(TPluginBody));
  
#ifdef ShowPluginInfo
  _consolePrint2("FreeLibrary:Destroied.\n");
#endif
}

TPluginBody* DLLList_LoadPlugin(char *fn)
{
  void *buf;
  int size;
  pCurrentPluginBody=NULL;

	FILE *f=fopen(fn,"rb");
	if(!f)return NULL;
	size=filelength(fileno(f));
	buf=malloc(size);
	fread(buf,1,size,f);
	fclose(f);

  if((buf==NULL)||(size==0)){
    _consolePrintf2("%s file read error.\n",fn);
    return(NULL);
  }
  
  TPluginBody *pPB=(TPluginBody*)malloc(sizeof(TPluginBody));
  
  if(pPB==NULL){
    _consolePrint2("Memory overflow.\n");
    return(NULL);
  }
  extmem_Init();
  if(DLL_LoadLibrary(pPB,Plugin_GetStdLib(),buf,size)==false){
    free(pPB); pPB=NULL;
    extmem_Free();
    return(NULL);
  }

  pPB->INIData=NULL;
  pPB->INISize=0;
  pPB->BINFileHandle=0;
  pPB->BINData=NULL;
  pPB->BINSize=0;

	changefileext(fn,".ini");
	f=fopen(fn,"rb");
	if(f){
		pPB->INISize=filelength(fileno(f));
		pPB->INIData=malloc(pPB->INISize);
		fread(pPB->INIData,1,pPB->INISize,f);
		fclose(f);
	}

	changefileext(fn,".bin");
	f=fopen(fn,"rb");
	if(f){
		pPB->BINFileHandle=(int)f;
		pPB->BINSize=filelength(fileno(f));
	}

	changefileext(fn,".msp");

  pCurrentPluginBody=pPB;
  
  return(pPB);
}

void DLLList_FreePlugin(TPluginBody *pPB)
{
  pCurrentPluginBody=NULL;
  
  if(pPB==NULL) return;
  
  if(pPB->pIL!=NULL) pPB->pIL->Free();
  if(pPB->pSL!=NULL) pPB->pSL->Free();
  if(pPB->pCL!=NULL) pPB->pCL->Free();
  if(pPB->pSE!=NULL) pPB->pSE->Free();
  
  DLL_FreeLibrary(pPB,true);

  if(pPB->INIData!=NULL){
    free(pPB->INIData); pPB->INIData=NULL;
    pPB->INISize=0;
  }
  
  if(pPB->BINFileHandle!=0){
    fclose(pPB->BINFileHandle);
    pPB->BINFileHandle=0;
  }
  
  if(pPB->BINData!=NULL){
    free(pPB->BINData); pPB->BINData=NULL;
    pPB->BINSize=0;
  }

  free(pPB);
  extmem_Free();
}
