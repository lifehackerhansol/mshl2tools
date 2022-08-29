#include <nds/card.h>
#include <nds/registers_alt.h> //to maintain r23
#include "libprism.h" // DLDIDATA

#if 0
static void cardWriteCommand(){
	//int index;

	CARD_CR1H = CARD_CR1_ENABLE | CARD_CR1_IRQ;

	//for(index = 0; index < 8; index++){
	//	CARD_COMMAND[7-index] = command[index];
	//}
}
#endif

#define cardWriteCommand() (CARD_CR1H = CARD_CR1_ENABLE | CARD_CR1_IRQ)

static void cardWaitReady(u32 flags){
	bool ready = false;
	do{
		cardWriteCommand();
		CARD_CR2 = flags;
		do{
			if(CARD_CR2 & CARD_DATA_READY)
				if(!CARD_DATA_RD)ready = true;
		}while(CARD_CR2 & CARD_BUSY);
	}while(!ready);
}

static void _cardPolledTransfer(u32 flags, u32 *destination, u32 length){
	u32 data;
	cardWriteCommand();
	CARD_CR2 = flags;
	u32 *target = destination + length;
	do{
		// Read data if available
		if(CARD_CR2 & CARD_DATA_READY){
			data=CARD_DATA_RD;
			if(destination < target)
				*destination++ = data;
		}
	}while(CARD_CR2 & CARD_BUSY);
}

static void bytecardPolledTransfer(u32 flags, u32 *destination, u32 length){
	u32 data;
	cardWriteCommand();
	CARD_CR2 = flags;
	u32 *target = destination + length;
	do{
		// Read data if available
		if(CARD_CR2 & CARD_DATA_READY){
			data=CARD_DATA_RD;
			if(destination < target){
				((u8*)destination)[0] = data & 0xff;
				((u8*)destination)[1] = (data >> 8) & 0xff;
				((u8*)destination)[2] = (data >> 16) & 0xff;
				((u8*)destination)[3] = (data >> 24) & 0xff;
				destination++;
			}
		}
	}while(CARD_CR2 & CARD_BUSY);
}

// B0
u32 R4_ReadCardInfo(){
	u32 ret;

	CARD_COMMAND[0] = 0xb0;
	CARD_COMMAND[1] = 0;
	CARD_COMMAND[2] = 0;
	CARD_COMMAND[3] = 0;
	CARD_COMMAND[4] = 0;
	CARD_COMMAND[5] = 0;
	CARD_COMMAND[6] = 0;
	CARD_COMMAND[7] = 0;
	_cardPolledTransfer(0xa7586000, &ret, 1);
	return ret;
}

// B4
void R4_SendMap(u32 address){
	CARD_COMMAND[0] = 0xb4;
	CARD_COMMAND[1] = (address >> 24) & 0xff;
	CARD_COMMAND[2] = (address >> 16) & 0xff;
	CARD_COMMAND[3] = (address >> 8)  & 0xff;
	CARD_COMMAND[4] =  address        & 0xff;
	CARD_COMMAND[5] = 0;
	CARD_COMMAND[6] = 0;
	CARD_COMMAND[7] = 0;
	cardWaitReady(0xa7586000);
}

void R4_Read(u32 address, u32 *destination, u32 length, u8 start, u8 end){
	CARD_COMMAND[0] = start;
	CARD_COMMAND[1] = (address >> 24) & 0xff;
	CARD_COMMAND[2] = (address >> 16) & 0xff;
	CARD_COMMAND[3] = (address >> 8)  & 0xff;
	CARD_COMMAND[4] =  address        & 0xff;
	CARD_COMMAND[5] = 0;
	CARD_COMMAND[6] = 0;
	CARD_COMMAND[7] = 0;
	cardWaitReady(0xa7586000);
	CARD_COMMAND[0] = end;
	if ((u32)destination & 0x03)
		bytecardPolledTransfer(0xa1586000, destination, length);
	else
		_cardPolledTransfer(0xa1586000, destination, length);
}

// B2/B3
void R4_ReadSave(u32 address, u32 *destination, u32 length){
	R4_Read(address,destination,length,0xb2,0xb3);
}

// B6/B7
void R4_ReadRom(u32 address, u32 *destination, u32 length){
	R4_Read(address,destination,length,0xb6,0xb7);
}

// B6/BF
void R4_ReadMenu(u32 address, u32 *destination, u32 length){
	R4_Read(address,destination,length,0xb6,0xbf);
}

// B9/BA
void R4_LogicCardRead(u32 address, u32 *destination, u32 length){
	R4_Read(address,destination,length,0xb9,0xba);
}

// BB/BC
void R4_LogicCardWrite(u32 address, u32 *source, u32 length){
	u32 data = 0;

	CARD_COMMAND[0] = 0xbb;
	CARD_COMMAND[1] = (address >> 24) & 0xff;
	CARD_COMMAND[2] = (address >> 16) & 0xff;
	CARD_COMMAND[3] = (address >> 8)  & 0xff;
	CARD_COMMAND[4] =  address        & 0xff;
	CARD_COMMAND[5] = 0;
	CARD_COMMAND[6] = 0;
	CARD_COMMAND[7] = 0;
	cardWriteCommand();
	CARD_CR2 = 0xe1586000;
	u32 *target = source + length;
	if((u32)source & 0x03){
		do{
			if(CARD_CR2 & CARD_DATA_READY){
				if(source < target){
					data = ((u8*)source)[0] | (((u8*)source)[1] << 8) | (((u8*)source)[2] << 16) | (((u8*)source)[3] << 24);
					source++;
				}
				CARD_DATA_RD = data;
			}
		}while(CARD_CR2 & CARD_BUSY);
	}else{
		do{
			if(CARD_CR2 & CARD_DATA_READY){
				if(source < target)
					data = *source++;
				CARD_DATA_RD = data;
			}
		}while(CARD_CR2 & CARD_BUSY);
	}
	CARD_COMMAND[0] = 0xbc;
	cardWaitReady(0xa7586000);
}

u32 M3_ReadCardRegion(){
	u32 ret;

	CARD_COMMAND[0] = 0xcd;
	CARD_COMMAND[1] = 0;
	CARD_COMMAND[2] = 0;
	CARD_COMMAND[3] = 0;
	CARD_COMMAND[4] = 0;
	CARD_COMMAND[5] = 0;
	CARD_COMMAND[6] = 0;
	CARD_COMMAND[7] = 0;
	_cardPolledTransfer(0xa7586000, &ret, 1);
	return ret;
}

void SCDS_SetSDHCModeForDSTT(){
	u32 addr=0x7f9e0,ret;
	if(read32(DLDIDATA+ioType)!=0x53444353)return;

	CARD_COMMAND[0] = 0xb9;
	CARD_COMMAND[1] = (addr >> 24) & 0xff;
	CARD_COMMAND[2] = (addr >> 16) & 0xff;
	CARD_COMMAND[3] = (addr >> 8)  & 0xff;
	CARD_COMMAND[4] =  addr        & 0xff;
	CARD_COMMAND[5] = 0;
	CARD_COMMAND[6] = 0;
	CARD_COMMAND[7] = 0;
	_cardPolledTransfer(0xa7180000, &ret, 1);
	if(ret!=0x32564353)return; //SCV2

	CARD_COMMAND[0] = 0x70;
	_cardPolledTransfer(0xa7180000, &ret, 1); //it seems DSTT always return true...
	//*(u32*)0x023ffc24=ret?1:0; //hack done. //this is invalid because dstt_sdhc is preserved in ARM7.
	IPCZ->cmd=ret?EnableDSTTSDHCFlag:DisableDSTTSDHCFlag;
}
