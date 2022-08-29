#ifndef MSP_H
#define MSP_H

#include "mspdef.h"

typedef struct {
  u32 ID;
  u8 VersionHigh;
  u8 VersionLow;
  u8 PluginType;
  u8 Dummy0;
  u32 DataStart;
  u32 DataEnd;
  u32 gotStart;
  u32 gotEnd;
  u32 bssStart;
  u32 bssEnd;
  u32 LoadLibrary;
  u32 FreeLibrary;
  u32 QueryInterfaceLibrary;
  u32 Dummy1;
  u32 ext[4];
  char info[64];
} TPluginHeader;

typedef struct {
  TPluginHeader PluginHeader;
  void *pData;
  int DataSize;
  void *pbss;
  int bssSize;
  bool (*LoadLibrary)(const TPlugin_StdLib *_pStdLib,u32 _TextDataAddress);
  void (*FreeLibrary)(void);
  int (*QueryInterfaceLibrary)(void);
  const TPlugin_ImageLib *pIL;
  const TPlugin_SoundLib *pSL;
  const TPlugin_ClockLib *pCL;
  const TPlugin_SoundEffectLib *pSE;
  
  char *INIData;
  int INISize;
  int BINFileHandle;
  void *BINData;
  int BINSize;
} TPluginBody;

//extern bool DLL_LoadLibrary(TPluginBody *pPB,void *pbin,int binsize);
extern void DLL_FreeLibrary(TPluginBody *pPB,bool callfree);

TPluginBody* DLLList_LoadPlugin(char *fn);
void DLLList_FreePlugin(TPluginBody *pPB);

//TPluginBody* DLLList_LoadSoundEffectPlugin(u32 idx);
//void DLLList_FreeSoundEffectPlugin(u32 idx);

#define PluginFilenameMax (16)

#define SoundEffectPluginCount (4)

enum EPluginType {EPT_None=0,EPT_Image=1,EPT_Sound=2,EPT_Clock=3,EPT_SoundEffect=4};

#endif

