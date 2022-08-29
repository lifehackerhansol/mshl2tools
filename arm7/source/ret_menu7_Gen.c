/***********************************************************
	Arm7 Soft rest for General purpose

		by Rudolph (çcíÈ)
***************************************************************/

#include <nds.h>
//#include <nds/registers_alt.h>	// devkitPror20
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define	ARM7_PROG	(0x03810000 - 0xA00)
typedef void (* FN_MEDIUM_ARM7)(void);
FN_MEDIUM_ARM7	_menu7_Gen;

extern	void	_menu7_Gen_s();

void ret_menu7_Gen()
{
	u32	*adr;
	u32	*buf;
	u32	i;

	REG_IME = 0;
	REG_IE = 0;
	REG_IF = REG_IF;

	REG_IPC_SYNC = 0;
	DMA0_CR = 0;
	DMA1_CR = 0;
	DMA2_CR = 0;
	DMA3_CR = 0;

//copy final loader to private RAM
	adr = (u32*)ARM7_PROG;
	buf = (u32*)_menu7_Gen_s;
	for(i = 0; i < 0x200/4; i++) {
		*adr = *buf;
		adr++;
		buf++;
	}

	while((*(vu32*)0x027FFDFC) != 0x027FFDF8);	// Timing adjustment with ARM9

	_menu7_Gen = (FN_MEDIUM_ARM7)ARM7_PROG;
	_menu7_Gen();

	while(1);

}


void _menu7_Gen_s()
{
	u32	*adr;
	u32	*bufh, *buf7, *buf9;
	u32	siz;
	u32	i;
	u32	*arm9s, *arm9e;
	u32	*arm7s, *arm7e;

//relocation start
	bufh = (u32*)(*(vu32*)0x027FFDF4); //allocated in ret_menu9_Gen()

	adr = (u32*)0x027FFE00;
	for(i = 0; i < 512/4; i++) {		// Header
		*adr = *bufh;
		adr++;
		bufh++;
	}

	buf9 = bufh;
	buf7 = buf9 + ((*(vu32*)0x027FFE2C) / 4);

	adr = (u32*)(*(vu32*)0x027FFE38);
	siz = (*(vu32*)0x027FFE3C);
	for(i = 0; i < siz/4; i++) {		// ARM7
		*adr = *buf7;
		adr++;
		buf7++;
	}
	arm7e = adr;

	adr = (u32*)(*(vu32*)0x027FFE28);
	siz = (*(vu32*)0x027FFE2C);
	if(adr < buf9) {			// ARM9
		for(i = 0; i < siz/4; i++) {
			*adr = *buf9;
			adr++;
			buf9++;
		}
		arm9e = adr;
	} else { //if ldrBuf is before adr, we have to copy from back side.
		adr += (siz/4 - 1);
		buf9 += (siz/4 - 1);
		arm9e = adr + 1;
		for(i = 0; i < siz/4; i++) {
			*adr = *buf9;
			adr--;
			buf9--;
		}
	}
//relocation end

//clear main memory
	adr = (u32*)0x02000000;
	arm9s = (u32*)(*(vu32*)0x027FFE28);
	while(adr < arm9s) {
		*adr = 0x00000000;
		adr++;
	}

	arm7s = (u32*)(*(vu32*)0x027FFE38);

	//We will stop at 0x023F4000 rather than 0x023FF800
	if(arm7s > (u32*)0x023F4000)
		arm7s = (u32*)0x023F4000;
	while(arm9e < arm7s) {
		*arm9e = 0x00000000;
		arm9e++;
	}

	while(arm7e < (u32*)0x023F4000) {
		*arm7e = 0x00000000;
		arm7e++;
	}

	*(vu32*)0x027FFDFC = *(vu32*)0x027FFE24;

//jump to "copy address"
	asm("swi 0x00");			// JUMP 0x027FFE34

	while(1);

}
