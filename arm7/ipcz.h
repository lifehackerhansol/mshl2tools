#ifndef NDS_IPCZ_INCLUDE
#define NDS_IPCZ_INCLUDE

#include <nds.h>

#ifdef __cplusplus
extern "C" {
#endif

enum IPCZCommand{
	ResetRudolph		= 0x01,
	ResetBootlib		= 0x02,
	ResetMoonlight	= 0x03,
	ResetRudolphM		= 0x04,

	Shutdown		= 0x11,
	ReturnBootstub	= 0x12,

	//Slot2NDS		= 0x18, //Use bootMoonlight(0x080000c0)
	Slot2GBA		= 0x19,
	//ReturnDSMenu	= 0x1a, //GetFirmware then decompress it to get image
	ReturnDSiMenu		= 0x1b,

	GetARM7Bios		= 0x101,
	GetFirmware		= 0x102,
	RequestBatteryLevel	= 0x111,

	EnableDSTTSDHCFlag	= 0x201,
	DisableDSTTSDHCFlag	= 0x202,

	PlaySound            = 0x301,
	StopSound            = 0x302,

	//CPTest		= 0xf1000000,
};

enum NDSType{
	NDSPhat	=0,
	NDSLite	=1,
	NDSi		=2,	//eXtra Large hardware is completely the same.
};

typedef struct{
	u32 cmd;
	u32 bootaddress; //Used with ResetMoonlight
	u32 blanks;

	u16 keysheld;
	u16 keysdown;
	u16 keysup;
	u16 keysrepeat;
	u16 touchX;
	u16 touchY;

	u16 battery;
	u8  NDSType;
	u8  flashmeversion;
	u8  MAC[6];
	u16 fwchksum;
	u32 temperature; // use with /0x1000
	u32 fwsize;

	u8  *arm7bios_addr;
	u32 arm7bios_bufsize;
	u8  *firmware_addr;
	u32 firmware_bufsize;

	void *PCM_L;
	void *PCM_R;
	u32  PCM_freq;
	u32  PCM_size;
	u32  PCM_bits; //8bit or 16bit

	//u8 resetARM9ready;
	//u8 resetARM7ready;
} TransferRegionZ;

//I hope IPC won't be overridden by FIFO...
//They say after 0x027FF800 is used for NDS settings, so I use 0x027FF700.
#define IPCZ ((TransferRegionZ volatile *)(0x027FF700))

//experiment... what should I do?
#define IPCZBuf ((volatile u8*)(0x027Fe800))

#define ROMVERSION "0.70g.101216 Final"
#define ROMDATE ""__DATE__" "__TIME__" GMT+09:00"
#ifdef _LIBNDS_MAJOR_
#define ROMENV "DevKitARMr32 + libnds 1.4.9 + libfat r4456(modified)"
#else
#define ROMENV "DevKitARMr23b + libnds-20071023 +\nlibfat-20080530less(modified) [legacy]"
#endif

#ifdef __cplusplus
}
#endif
#endif //included
