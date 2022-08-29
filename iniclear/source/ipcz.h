#ifndef NDS_IPCZ_INCLUDE
#define NDS_IPCZ_INCLUDE

#include <nds.h>

#define ResetRudolph	1
#define ResetBootlib	2
#define ResetMoonlight	3

//#define QueryKeyXY	0x10

typedef struct{
	u32 cmd;
	u32 bootaddress;

	u8 keyxy;
} TransferRegionZ;

//I hope IPC won't be overridden by FIFO...
//They say after 0x027FF800 is used for NDS settings, so I use 0x027FF700.
#define IPCZ ((TransferRegionZ volatile *)(0x027FF700))

#endif
