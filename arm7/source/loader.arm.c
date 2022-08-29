#include "../ipcz.h"

///// functions in this file must not use any other functions. /////

__attribute__((noreturn)) void _menu7_Gen_s(){
	u32	*adr;
	u32	*bufh, *buf7, *buf9;
	u32	siz;
	u32	i;
	u32	*arm9s, *arm9e;
	u32	*arm7s, *arm7e;

//relocation start
	bufh = *(u32**)0x02fFFDF4; //allocated in ret_menu9_Gen()

	adr = (u32*)0x02fFFE00;
	for(i = 0; i < 512/4; i++) {		// Header
		*adr = *bufh;
		adr++;
		bufh++;
	}

	buf9 = bufh;
	buf7 = buf9 + ((*(u32*)0x02fFFE2C) / 4);

	adr = *(u32**)0x02fFFE38;
	siz = *(u32*)0x02fFFE3C;
	for(i = 0; i < siz/4; i++) {		// ARM7
		*adr = *buf7;
		adr++;
		buf7++;
	}
	arm7e = adr;

	adr = *(u32**)0x02fFFE28;
	siz = *(u32*)0x02fFFE2C;
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
	arm9s = *(u32**)0x02fFFE28;
	while(adr < arm9s) {
		*adr = 0x00000000;
		adr++;
	}

	arm7s = *(u32**)0x02fFFE38;

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

	*(vu32*)0x02fFFDFC = *(vu32*)0x02fFFE24;

//jump to "copy address"
	asm("swi 0x00");			// JUMP 0x02fFFE34
/*
	asm(
		"ldr	r0,=0x2FFFE34\n"
		"ldr	r0,[r0]\n"
		"bx	r0\n"
	);
*/
	while(1);
}

__attribute__((noreturn)) void reboot(){
	//u32 i=0;
	//vu32 w;
	vu32 *MoonShellResetFlag=(vu32*)0x02fffffc;
	  
	u32 *ARM7_pCopyFrom=*(u32**)0x02fffe30;//IPCZ->ARMInfo7.pCopyFrom;
	u32 *ARM7_pCopyTo=*(u32**)0x02fffe38;//(u32*)IPCZ->ARMInfo7.pCopyTo;
	u32 ARM7_CopySize=*(u32*)0x02fffe3c;//IPCZ->ARMInfo7.CopySize;

	u32 idx=0;
	for(;idx<ARM7_CopySize/4;idx++){
		*ARM7_pCopyTo++=*ARM7_pCopyFrom++;
	}

/*
	//We shouldn't clear ARM7
	if(IPCZ->RequestClearMemory==true){
		while(ARM7_pCopyTo!=(u32*)0x380f000){
			*ARM7_pCopyTo++=0;
		}
	}
*/

	//switch to user mode
	//u32 r0;
	asm(
		"mov r0, #0x1F\n"
		"msr cpsr, r0\n"
	);

	REG_IE = 0;
	REG_IF = ~0;
	(*(vu32*)(0x04000000-4)) = 0;  //IRQ_HANDLER ARM7 version
	(*(vu32*)(0x04000000-8)) = ~0; //VBLANK_INTR_WAIT_FLAGS, ARM7 version
	REG_POWERCNT = 1;  //turn off power to stuffs

	//u32 ARM7ExecAddr=*(u32*)0x02fffe34;//IPCZ->ARMInfo7.ExecAddr;

	*MoonShellResetFlag=4;
	while(*MoonShellResetFlag!=5){
		// ARM9Wait: Copy EWRAM to ARM9InternalMemory(EWRAM). and, Reset memory.
		//for(w=0;w<0x100;w++);
	}
	*MoonShellResetFlag=6;

	//*(vu32*)0x02fFFe34 = ARM7ExecAddr;
	asm("swi 0x00");			// JUMP 0x02fFFE34
/*
	asm(
		"ldr	r0,=0x2FFFE34\n"
		"ldr	r0,[r0]\n"
		"bx	r0\n"
	);
*/
	while(1);
}

