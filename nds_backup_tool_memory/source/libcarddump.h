#ifndef _LIBCARDDUMP_H_
#define _LIBCARDDUMP_H_

/*
	libcarddump by Rudolph
	All is not supported. Please do not inquire.

	build devkitARM_r26(libnds 1.3.8).
*/


#ifdef __cplusplus
extern "C" {
#endif

u32 Card_Open(u8 *key_tbl);		// Card recognition and initialization of card dump.
u32 Card_Retry(void);			// Recognizing of card retry.

bool Card_Read(u32 addr, char *data);	// Card dump of specified address(512*n)

bool Card_Close(void);			// End of card dump

#ifdef __cplusplus
}
#endif

#endif	// _LIBCARDDUMP_H_
