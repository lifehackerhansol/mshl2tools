#undef ARM7
#define ARM9
#include <nds.h>

#define ARM9_START_FLAG (*(vu8*)0x02FFFDFB)
/*-------------------------------------------------------------------------
resetMemory2_ARM9
Clears the ARM9's DMA channels and resets video memory
Written by Darkain.
Modified by Chishm:
 * Changed MultiNDS specific stuff
--------------------------------------------------------------------------*/
#define resetMemory2_ARM9_size 0x400
void __attribute__ ((long_call)) __attribute__((naked)) __attribute__((noreturn)) resetMemory2_ARM9()
{
 	register int i;
	REG_IPC_FIFO_CR=IPC_FIFO_ENABLE|IPC_FIFO_SEND_CLEAR;
  
	//clear out ARM9 DMA channels
	for (i=0; i<4; i++) {
		DMA_CR(i) = 0;
		DMA_SRC(i) = 0;
		DMA_DEST(i) = 0;
		TIMER_CR(i) = 0;
		TIMER_DATA(i) = 0;
	}

	VRAM_CR = (VRAM_CR & 0xffff0000) | 0x00008080 ;
	
	u16 *mainregs = (u16*)0x04000000;
	u16 *subregs = (u16*)0x04001000;
	
	for (i=0; i<43; i++) {
		mainregs[i] = 0;
		subregs[i] = 0;
	}
	
	REG_DISPSTAT = 0;

	VRAM_A_CR = 0;
	VRAM_B_CR = 0;
// Don't mess with the ARM7's VRAM
//	VRAM_C_CR = 0;
	VRAM_D_CR = 0;
	VRAM_E_CR = 0;
	VRAM_F_CR = 0;
	VRAM_G_CR = 0;
	VRAM_H_CR = 0;
	VRAM_I_CR = 0;
	REG_POWERCNT  = 0x820F;

	//set shared ram to ARM7
	WRAM_CR = 0x03;

	// Return to passme loop
	*((vu32*)0x02FFFE04) = (u32)0xE59FF018;		// ldr pc, 0x02FFFE24
	*((vu32*)0x02FFFE24) = (u32)0x02FFFE04;		// Set ARM9 Loop address

	asm volatile(
		"\tbx %0\n"
		: : "r" (0x02FFFE04)
	);
	while(1);
}

/*-------------------------------------------------------------------------
startBinary_ARM9
Jumps to the ARM9 NDS binary in sync with the display and ARM7
Written by Darkain.
Modified by Chishm:
 * Removed MultiNDS specific stuff
--------------------------------------------------------------------------*/
#define startBinary_ARM9_size 0x100
void __attribute__ ((long_call)) __attribute__((noreturn)) __attribute__((naked)) startBinary_ARM9()
{
	REG_IME=0;
	REG_EXMEMCNT = 0xE880;
	// set ARM9 load address to 0 and wait for it to change again
	ARM9_START_FLAG = 0;
	while(REG_VCOUNT!=191);
	while(REG_VCOUNT==191);
	while ( ARM9_START_FLAG != 1 );
	VoidFn arm9code = *(VoidFn*)(0x2FFFE24);
	arm9code();
	while(1);
}

