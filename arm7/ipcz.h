#ifndef NDS_IPCZ_INCLUDE
#define NDS_IPCZ_INCLUDE

#include <nds.h>

#define ResetRudolph	1
#define ResetBootlib	2
#define ResetMoonlight	3

//#define QueryKeyXY	0x10
#define Shutdown	0x11
#define ReturnBootstub	0x12

//#define CPTest	0xf1000000

typedef struct{
	u32 cmd;
	u32 bootaddress; //Used with ResetMoonlight

	u8 keyxy;
} TransferRegionZ;

//I hope IPC won't be overridden by FIFO...
//They say after 0x027FF800 is used for NDS settings, so I use 0x027FF700.
#define IPCZ ((TransferRegionZ volatile *)(0x027FF700))
#define IPCZBuf ((volatile u8*)(0x027FF500))

#endif
