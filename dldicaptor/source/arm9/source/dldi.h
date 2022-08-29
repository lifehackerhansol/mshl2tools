//dldipatch aka dlditool public domain

#define magicString	0x00
#define dldiVersion	0x0c
#define driverSize	0x0d
#define fixSections	0x0e
#define allocatedSpace	0x0f
#define friendlyName 0x10

#define dataStart	0x40
#define dataEnd	0x44
#define glueStart	0x48
#define glueEnd	0x4c
#define gotStart	0x50
#define gotEnd	0x54
#define bssStart	0x58
#define bssEnd	0x5c

#define ioType	0x60
#define dldiFeatures	0x64
#define dldiStartup	0x68
#define isInserted	0x6c
#define readSectors	0x70
#define writeSectors	0x74
#define clearStatus	0x78
#define shutdown	0x7c
#define dldiData	0x80

#define fixAll	0x01
#define fixGlue	0x02
#define fixGot	0x04
#define fixBss	0x08

extern const byte *dldimagic;

extern const byte *_io_dldi;
extern const byte *io_dldi_data;

int dldi(byte *nds,const int ndslen
#if !defined(ARM9) && !defined(ARM7)
	,const byte *pD,const int dldilen
#endif
);
