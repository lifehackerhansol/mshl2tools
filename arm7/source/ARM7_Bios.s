@******************************************
@ ARM7_Bios.s (ARM7)
@ Part of libcarddump by Rudolph, but the same as CaitSith2's.
@
@ This code comes from a post by CaitSith2 at gbadev.org - THANKS!!
@
@ Code to dump the complete Nintendo DS ARM7 bios, including the
@ first 0x1204 bytes residing in the secure area.
@
@ The ARM7 bios has read protection where 0x(Word)[FFFF(Half word)[FF(Byte)[FF]]]
@ is returned, if any reads are attempted while PC is outside the arm7 bios range.
@
@ Additionally, if the PC is outside the 0x0000 - 0x1204 range, that range of the bios
@ is completely locked out from reading.
@******************************************

.align  4
.global	ARM7_Bios
.type   ARM7_Bios STT_FUNC
.global	SwitchUserMode
.type   SwitchUserMode STT_FUNC

@-----------------------------------------------------
@ key table from the NDS ARM7 BIOS (0x1078Byte)
@
@ void ARM7_Bios(u8 *tbl, u32 size)
@	*Encryption Seed Select table	0x002A..0x002F
@	*key table			0x0030..0x1077
@
@ Thus, set size as 0x1077 to get only the key table and 0x3fff to get complete bios.
@
@ Modified by X to enable setting size.
@-----------------------------------------------------

.arm
SwitchUserMode:
	push {r0}
	mov r0, #0x1F
	msr cpsr, r0
	pop {r0}
	bx lr

ARM7_Bios: @ here r0 and r1 are reserved.
	adr	r2, bios_dump+1
	bx	r2
    
.thumb

bios_dump:
	push	{r4-r7, lr}
	mov	r2, r0
	ldr	r0, =0x5ED
	@ldr 	r1, =0x1077 @ or =0x3fff

loop:
	mov	r6, #0x12
	sub	r3, r1, r6
	adr	r6, ret
	push	{r2-r6}
	bx	r0

.align 4

ret:
	strb	r3, [r2,r1]
	sub	r1, #1
	bpl	loop

	pop	{r4-r7}
	pop	{r3}
	bx	r3

.end
