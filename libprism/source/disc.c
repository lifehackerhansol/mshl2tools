#include "libprism.h"

typedef bool (*__FN_MEDIUM_STARTUP)() ;
typedef bool (*__FN_MEDIUM_ISINSERTED)() ;
typedef bool (*__FN_MEDIUM_READSECTORS)(u32 sector, u32 numSectors, void* buffer) ;
typedef bool (*__FN_MEDIUM_WRITESECTORS)(u32 sector, u32 numSectors, const void* buffer) ;
typedef bool (*__FN_MEDIUM_CLEARSTATUS)() ;
typedef bool (*__FN_MEDIUM_SHUTDOWN)() ;

bool disc_startup()
{return ( *(__FN_MEDIUM_STARTUP*)(DLDIDATA+dldiStartup) )();}
bool disc_isInserted()
{return ( *(__FN_MEDIUM_ISINSERTED*)(DLDIDATA+isInserted) )();}
bool disc_readSectors(u32 sector, u32 numSectors, void* buffer)
{return ( *(__FN_MEDIUM_READSECTORS*)(DLDIDATA+readSectors) )(sector,numSectors,buffer);}
bool disc_writeSectors(u32 sector, u32 numSectors, void* buffer)
{return ( *(__FN_MEDIUM_WRITESECTORS*)(DLDIDATA+writeSectors) )(sector,numSectors,buffer);}
bool disc_clearStatus()
{return ( *(__FN_MEDIUM_CLEARSTATUS*)(DLDIDATA+clearStatus) )();}
bool disc_shutdown()
{return ( *(__FN_MEDIUM_SHUTDOWN*)(DLDIDATA+shutdown) )();}
