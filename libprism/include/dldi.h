//dldipatch aka dlditool public domain

#ifndef DLDI_H
#define DLDI_H

#ifdef __cplusplus
extern "C" {
#endif

enum{ //to make sure these are integers.
	magicString		= 0x00,
	dldiVersion		= 0x0c,
	driverSize		= 0x0d,
	fixSections		= 0x0e,
	allocatedSpace	= 0x0f,
	friendlyName		= 0x10,

	dataStart		= 0x40,
	dataEnd		= 0x44,
	glueStart		= 0x48,
	glueEnd		= 0x4c,
	gotStart		= 0x50,
	gotEnd			= 0x54,
	bssStart		= 0x58,
	bssEnd			= 0x5c,

	ioType			= 0x60,
	dldiFeatures		= 0x64,
	dldiStartup		= 0x68,
	isInserted		= 0x6c,
	readSectors		= 0x70,
	writeSectors		= 0x74,
	clearStatus		= 0x78,
	shutdown		= 0x7c,
	dldiData		= 0x80,

	fixAll			= 0x01,
	fixGlue		= 0x02,
	fixGot			= 0x04,
	fixBss			= 0x08,
};

extern const byte *dldimagic;

extern const byte *_io_dldi;
extern const byte *io_dldi_data;
#ifdef _LIBNDS_MAJOR_
#define DLDIDATA ( (byte*)io_dldi_data )
#else
#define DLDIDATA ( (byte*)(((u32*)(&_io_dldi))-24) )
#endif

extern byte *DLDIToBoot;
extern const byte DLDINull[];

int dldi2(byte *nds,const int ndslen,const int bypassYSMenu,const char *dumpname);
int dldi(byte *nds,const int ndslen);

#ifdef __cplusplus
}
#endif
#endif //included
