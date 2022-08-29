#include "../../libprism/libprism.h"
const u16 bgcolor=RGB15(4,0,12);

enum BPB {
	BPB_jmpBoot = 0x00,
	BPB_OEMName = 0x03,
	// BIOS Parameter Block
	BPB_bytesPerSector = 0x0B,
	BPB_sectorsPerCluster = 0x0D,
	BPB_reservedSectors = 0x0E,
	BPB_numFATs = 0x10,
	BPB_rootEntries = 0x11,
	BPB_numSectorsSmall = 0x13,
	BPB_mediaDesc = 0x15,
	BPB_sectorsPerFAT = 0x16,
	BPB_sectorsPerTrk = 0x18,
	BPB_numHeads = 0x1A,
	BPB_numHiddenSectors = 0x1C,
	BPB_numSectors = 0x20,
	// Ext BIOS Parameter Block for FAT16
	BPB_FAT16_driveNumber = 0x24,
	BPB_FAT16_reserved1 = 0x25,
	BPB_FAT16_extBootSig = 0x26,
	BPB_FAT16_volumeID = 0x27,
	BPB_FAT16_volumeLabel = 0x2B,
	BPB_FAT16_fileSysType = 0x36,
	// Bootcode
	BPB_FAT16_bootCode = 0x3E,
	// FAT32 extended block
	BPB_FAT32_sectorsPerFAT32 = 0x24,
	BPB_FAT32_extFlags = 0x28,
	BPB_FAT32_fsVer = 0x2A,
	BPB_FAT32_rootClus = 0x2C,
	BPB_FAT32_fsInfo = 0x30,
	BPB_FAT32_bkBootSec = 0x32,
	// Ext BIOS Parameter Block for FAT32
	BPB_FAT32_driveNumber = 0x40,
	BPB_FAT32_reserved1 = 0x41,
	BPB_FAT32_extBootSig = 0x42,
	BPB_FAT32_volumeID = 0x43,
	BPB_FAT32_volumeLabel = 0x47,
	BPB_FAT32_fileSysType = 0x52,
	// Bootcode
	BPB_FAT32_bootCode = 0x5A,
	BPB_bootSig_55 = 0x1FE,
	BPB_bootSig_AA = 0x1FF
};

static const char FAT_SIG[3] = {'F', 'A', 'T'};

static inline uint32_t u8array_to_u32 (const uint8_t* item, int offset) {
	return ( item[offset] | (item[offset + 1] << 8) | (item[offset + 2] << 16) | (item[offset + 3] << 24));
}

static inline uint16_t u8array_to_u16 (const uint8_t* item, int offset) {
	return ( item[offset] | (item[offset + 1] << 8));
}

u32 _FindFirstValidPartition()
{
	uint8_t part_table[16*4];
	uint8_t *ptr;
	int i;

	uint8_t sectorBuffer[512] = {0};

	// Read first sector of disc
	if (!disc_readSectors (0, 1, sectorBuffer)) {
		return 0;
	}

	memcpy(part_table,sectorBuffer+0x1BE,16*4);
	ptr = part_table;

	for(i=0;i<4;i++,ptr+=16) {
		u32 part_lba = u8array_to_u32(ptr, 0x8);

		if (!memcmp(sectorBuffer + BPB_FAT16_fileSysType, FAT_SIG, sizeof(FAT_SIG)) ||
			!memcmp(sectorBuffer + BPB_FAT32_fileSysType, FAT_SIG, sizeof(FAT_SIG))) {
			return part_lba;
		}

		if(ptr[4]==0) continue;

		if(ptr[4]==0x0F) {
			u32 part_lba2=part_lba;
			u32 next_lba2=0;
			int n;

			for(n=0;n<8;n++) // max 8 logic partitions
			{
				if(!disc_readSectors (part_lba+next_lba2, 1, sectorBuffer)) return 0;

				part_lba2 = part_lba + next_lba2 + u8array_to_u32(sectorBuffer, 0x1C6) ;
				next_lba2 = u8array_to_u32(sectorBuffer, 0x1D6);

				if(!disc_readSectors (part_lba2, 1, sectorBuffer)) return 0;

				if (!memcmp(sectorBuffer + BPB_FAT16_fileSysType, FAT_SIG, sizeof(FAT_SIG)) ||
					!memcmp(sectorBuffer + BPB_FAT32_fileSysType, FAT_SIG, sizeof(FAT_SIG)))
				{
					return part_lba2;
				}

				if(next_lba2==0) break;
			}
		} else {
			if(!disc_readSectors (part_lba, 1, sectorBuffer)) return 0;
			if (!memcmp(sectorBuffer + BPB_FAT16_fileSysType, FAT_SIG, sizeof(FAT_SIG)) ||
				!memcmp(sectorBuffer + BPB_FAT32_fileSysType, FAT_SIG, sizeof(FAT_SIG))) {
				return part_lba;
			}
		}
	}
	return 0;
}

void Main(){
	u32 startSector=0;
	uint8_t sectorBuffer[512] = {0};

	_consolePrintf(
		"stripmbr obly for emulators\n"
		"%s\n%s\n\n",
		ROMDATE,ROMENV
	);

	_consolePrint("[Phase 1]\n");
	_consolePrint("Starting up...\n");
	disc_startup();

	_consolePrint("Reading first sector...\n");
	// Read first sector of disc
	if (!disc_readSectors (startSector, 1, sectorBuffer)) {
		return;
	}
	
	// Make sure it is a valid MBR or boot sector
	if ( (sectorBuffer[BPB_bootSig_55] != 0x55) || (sectorBuffer[BPB_bootSig_AA] != 0xAA)) {
		return;
	}

	if (!memcmp(sectorBuffer + BPB_FAT16_fileSysType, FAT_SIG, sizeof(FAT_SIG))) {
		// Check if there is a FAT string, which indicates this is a boot sector
		startSector = 0;
	} else if (!memcmp(sectorBuffer + BPB_FAT32_fileSysType, FAT_SIG, sizeof(FAT_SIG))) {
		// Check for FAT32
		startSector = 0;
	} else {
		_consolePrint("Getting fat sector...\n");
		startSector = _FindFirstValidPartition();
		if (!disc_readSectors (startSector, 1, sectorBuffer)) {
			return;
		}
	}

	// Now verify that this is indeed a FAT partition
	if (memcmp(sectorBuffer + BPB_FAT16_fileSysType, FAT_SIG, sizeof(FAT_SIG)) &&
		memcmp(sectorBuffer + BPB_FAT32_fileSysType, FAT_SIG, sizeof(FAT_SIG)))
	{
		return;
	}

	_consolePrint("\n[Phase 2]\n");
	u32 numberOfSectors = u8array_to_u16( sectorBuffer, BPB_numSectorsSmall);
	if (numberOfSectors == 0) {
		numberOfSectors = u8array_to_u32( sectorBuffer, BPB_numSectors);
	}
	_consolePrintf("startSector=%u, numberOfSectors=%u\n",startSector,numberOfSectors);
	if(startSector==0){
		_consolePrint("You don't need to stripmbr.\n");return;
	}
	_consolePrint("Press A to proceed. If you want to abort, poweroff now.\n");
	while(!(IPCZ->keysdown&KEY_A))swiWaitForVBlank();
	_consolePrint("Start.\n\n");

	u32 i=0;
	_consoleStartProgress();
	for(;i<numberOfSectors;i++){
		disc_readSectors(i+startSector, 1, sectorBuffer);
		disc_writeSectors(i, 1, sectorBuffer);
		_consolePrintProgress("Copying",i,numberOfSectors);
	}
	_consoleEndProgress();
	_consolePrint("Done!\n\n");
	disc_shutdown();
}
