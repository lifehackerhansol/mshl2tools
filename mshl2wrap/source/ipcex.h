/*---------------------------------------------------------------------------------
	$Id: ipc.h,v 1.13 2006/01/17 09:47:00 wntrmute Exp $

	Inter Processor Communication

	Copyright (C) 2005
		Michael Noland (joat)
		Jason Rogers (dovoto)
		Dave Murphy (WinterMute)

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any
	damages arising from the use of this software.

	Permission is granted to anyone to use this software for any
	purpose, including commercial applications, and to alter it and
	redistribute it freely, subject to the following restrictions:

	1.	The origin of this software must not be misrepresented; you
		must not claim that you wrote the original software. If you use
		this software in a product, an acknowledgment in the product
		documentation would be appreciated but is not required.
	2.	Altered source versions must be plainly marked as such, and
		must not be misrepresented as being the original software.
	3.	This notice may not be removed or altered from any source
		distribution.

	$Log: ipc.h,v $
	Revision 1.13  2006/01/17 09:47:00  wntrmute
	*** empty log message ***
	
	Revision 1.12  2005/09/20 04:59:04  wntrmute
	replaced defines with enums
	replaced macros with static inlines
	
	Revision 1.11  2005/08/23 17:06:10  wntrmute
	converted all endings to unix

	Revision 1.10  2005/08/03 05:26:42  wntrmute
	use BIT macro
	corrected header include

	Revision 1.9  2005/07/27 02:20:05  wntrmute
	resynchronise with ndslib
	Updated GL with float wrappers for NeHe


---------------------------------------------------------------------------------*/

#ifndef NDS_IPCEX_INCLUDE
#define NDS_IPCEX_INCLUDE

#include <nds/jtypes.h>

typedef u32 ERESET;
#define RESET_NULL (0)
#define RESET_VRAM (1)
#define RESET_GBAMP (2)
#define RESET_GBAROM (3)
#define RESET_MENU_DSLink (4)
#define RESET_MENU_MPCF (5)
#define RESET_MENU_M3CF (6)
#define RESET_MENU_M3SD (7)
#define RESET_MENU_SCCF (8)
#define RESET_MENU_SCSD (9)
#define RESET_MENU_EZSD (10)
#define RESET_MENU_R4TF (11)		//====== R4TF was added. by Rudolph (2007/05/23)
#define RESET_MENU_EZ5S (12)		//====== EZ5S was added. by Rudolph (2007/05/25)
#define RESET_MENU_GEN (13)		//====== General was added. by Rudolph (2007/10/22)
//---------------------------------------------------------------------------------
typedef struct sTransferRegionEX {
//---------------------------------------------------------------------------------
  ERESET RESET;
} TransferRegionEX, * pTransferRegionEX;

#define IPCEX ((TransferRegionEX volatile *)(0x027FF000+sizeof(TransferRegion)))

#endif


