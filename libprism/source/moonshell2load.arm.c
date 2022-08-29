#include "libprism.h"

///// functions in this file must not use any other functions. /////

attrnoinline attrnoreturn void resetMemory2load_ARM9_NoBIOS(){
	//header region is surely written in BootNDSROMex2() so memUncached() isn't required.
	vu32 *header=(vu32*)0x02fffe00;
	u32 *ARM9_pCopyFrom=(u32*)header[0x20/4];//IPCZ->ARMInfo9.pCopyFrom;
	u32 *ARM9_pCopyTo=(u32*)header[0x28/4];//(u32*)IPCZ->ARMInfo9.pCopyTo;
	u32 ARM9_CopySize=header[0x2c/4];//IPCZ->ARMInfo9.CopySize;

/*
	for(u32 idx=0;idx<ARM9_CopySize/4;idx++){
		*ARM9_pCopyTo++=*ARM9_pCopyFrom++;
	}
	if(IPCZ->RequestClearMemory==true){
		while(ARM9_pCopyTo!=(u32*)0x023ff000){
			*ARM9_pCopyTo++=0;
		}
	}
*/
	//reset_MemCopy32CPU(ARM9_pCopyFrom,ARM9_pCopyTo,ARM9_CopySize);
	u32 idx=0;
	for(;idx<ARM9_CopySize/4;idx++){
		*ARM9_pCopyTo++=*ARM9_pCopyFrom++;
	}

/*
	if(IPCZ->RequestClearMemory==true){
		u32 size=0x023ff000-(u32)ARM9_pCopyTo;
		reset_MemSet32CPU(0,ARM9_pCopyTo,size);
	}
*/

	//vu32 w;
	header[0x1fc/4]=5;
	while(header[0x1fc/4]!=6){
		// ARM7Wait: Copy EWRAM to ARM7InternalMemory. and, Reset memory.
		//for(w=0;w<0x100;w++);
	}
/*
	u32 ARM9ExecAddr=IPCZ->ARMInfo9.ExecAddr; //not working?
	asm(
		"ldr r0, =ARM9ExecAddr\n"
		"bx r0\n"
	);
*/
	//*(u32*)0x02fffe24=IPCZ->ARMInfo9.ExecAddr;
	asm("swi 0x00");
	while(1);
}

