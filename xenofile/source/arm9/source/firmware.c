#ifdef ARM9
#include "xenofile.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#if !defined(ARM9) || defined(GPL)
/*	Copyright (C) 2009 DeSmuME Team

    This file is part of DeSmuME

    DeSmuME is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    DeSmuME is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with DeSmuME; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#if defined(ARM9)
#define printf _consolePrintf2
#else
unsigned int read32(const void *p){
	const unsigned char *x=(const unsigned char*)p;
	return x[0]|(x[1]<<8)|(x[2]<<16)|(x[3]<<24);
}

unsigned char read8(const void *p){
	const unsigned char *x=(const unsigned char*)p;
	return x[0];
}

void write32(void *p, const unsigned int n){
	unsigned char *x=(unsigned char*)p;
	x[0]=n&0xff,x[1]=(n>>8)&0xff,x[2]=(n>>16)&0xff,x[3]=(n>>24)&0xff;
}

void write16(void *p, const unsigned int n){
	unsigned char *x=(unsigned char*)p;
	x[0]=n&0xff,x[1]=(n>>8)&0xff;
}

void write8(void *p, const unsigned int n){
	unsigned char *x=(unsigned char*)p;
	x[0]=n&0xff;
}


typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef int   s32;
typedef enum { false, true } bool;

/* http://www.digi.com/wiki/developer/index.php/Python_CRC16_Modbus_DF1 */
const unsigned short crc16table[256]={
0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

unsigned short crc16(unsigned short crc, unsigned char *p, int length){
	for(;length;length--)
		crc=(crc>>8)^crc16table[(crc^(*p++))&0xff];
	return crc;
}
#define swiCRC16 crc16
#endif

#define T1ReadLong(p,s) (read32((u8*)(p)+(s)))
#define T1ReadByte(p,s) (read8((u8*)(p)+(s)))
#define T1WriteByte(p,s,n) (write8((u8*)(p)+(s),(n)))

#define DWNUM(i) ((i) >> 2)

static u8		*tmp_data9;
static u8		*tmp_data7;
static u32		size9, size7;

static	u32		keyBuf[0x412];
static u32		keyCode[3];
static u32		ARM9bootAddr;
static u32		ARM7bootAddr;
static bool		patched;

typedef struct{
	u16	part3_rom_gui9_addr;		// 000h
	u16	part4_rom_wifi7_addr;		// 002h
	u16	part34_gui_wifi_crc16;		// 004h
	u16	part12_boot_crc16;			// 006h
	u8	fw_identifier[4];			// 008h
	u16	part1_rom_boot9_addr;		// 00Ch
	u16	part1_ram_boot9_addr;		// 00Eh
	u16	part2_rom_boot7_addr;		// 010h
	u16	part2_ram_boot7_addr;		// 012h
	u16	shift_amounts;				// 014h
	u16	part5_data_gfx_addr;		// 016h

	u8	fw_timestamp[5];			// 018h
	u8	console_type;				// 01Dh
	u16	unused1;					// 01Eh
	u16	user_settings_offset;		// 020h
	u16	unknown1;					// 022h
	u16	unknown2;					// 024h
	u16	part5_crc16;				// 026h
	u16	unused2;					// 028h	- FFh filled 
} HEADER;
static HEADER header;

bool getKeyBuf()
{
	extern const u8 *encr_data;
	memcpy(keyBuf,encr_data,0x1048);
	return true;
#if 0
	FILE *file = fopen("biosnds7.rom", "rb");
	if (!file) return false;

	fseek(file, 0x30, SEEK_SET);
	size_t res = fread(keyBuf, 4, 0x412, file);
	fclose(file);
	return (res == 0x412);
#endif
}

static void crypt64BitUp(u32 *ptr)
{
	u32 Y = ptr[0];
	u32 X = ptr[1];

	u32 i = 0x00;
	for(; i <= 0x0F; i++)
	{
		u32 Z = (keyBuf[i] ^ X);
		X = keyBuf[DWNUM(0x048 + (((Z >> 24) & 0xFF) << 2))];
		X = (keyBuf[DWNUM(0x448 + (((Z >> 16) & 0xFF) << 2))] + X);
		X = (keyBuf[DWNUM(0x848 + (((Z >> 8) & 0xFF) << 2))] ^ X);
		X = (keyBuf[DWNUM(0xC48 + ((Z & 0xFF) << 2))] + X);
		X = (Y ^ X);
		Y = Z;
	}

	ptr[0] = (X ^ keyBuf[DWNUM(0x40)]);
	ptr[1] = (Y ^ keyBuf[DWNUM(0x44)]);
}

static void crypt64BitDown(u32 *ptr)
{
	u32 Y = ptr[0];
	u32 X = ptr[1];

	u32 i = 0x11;
	for(; i >= 0x02; i--)
	{
		u32 Z = (keyBuf[i] ^ X);
		X = keyBuf[DWNUM(0x048 + (((Z >> 24) & 0xFF) << 2))];
		X = (keyBuf[DWNUM(0x448 + (((Z >> 16) & 0xFF) << 2))] + X);
		X = (keyBuf[DWNUM(0x848 + (((Z >> 8)  & 0xFF) << 2))] ^ X);
		X = (keyBuf[DWNUM(0xC48 + ((Z & 0xFF) << 2))] + X);
		X = (Y ^ X);
		Y = Z;
	}

	ptr[0] = (X ^ keyBuf[DWNUM(0x04)]);
	ptr[1] = (Y ^ keyBuf[DWNUM(0x00)]);
}

#define bswap32(val) (((val & 0x000000FF) << 24) | ((val & 0x0000FF00) << 8) | ((val & 0x00FF0000) >> 8) | ((val & 0xFF000000) >> 24))
static void applyKeycode(u32 modulo)
{
	crypt64BitUp(&keyCode[1]);
	crypt64BitUp(&keyCode[0]);

	u32 scratch[2] = {0x00000000, 0x00000000};

	u32 i = 0;
	for(; i <= 0x44; i += 4)
	{
		keyBuf[DWNUM(i)] = (keyBuf[DWNUM(i)] ^ bswap32(keyCode[DWNUM(i % modulo)]));
	}

	for(i = 0; i <= 0x1040; i += 8)
	{
		crypt64BitUp(scratch);
		keyBuf[DWNUM(i)] = scratch[1];
		keyBuf[DWNUM(i+4)] = scratch[0];
	}
}
#undef bswap32

static bool initKeycode(u32 idCode, int level, u32 modulo)
{
	if(getKeyBuf() == false)return false;

	keyCode[0] = idCode;
	keyCode[1] = (idCode >> 1);
	keyCode[2] = (idCode << 1);

	if(level >= 1) applyKeycode(modulo);
	if(level >= 2) applyKeycode(modulo);

	keyCode[1] <<= 1;
	keyCode[2] >>= 1;

	if(level >= 3) applyKeycode(modulo);

	return true;
}

static u16 getBootCodeCRC16()
{
	unsigned int i, j;
	u32 crc = 0xFFFF;
	const u16 val[8] = {0xC0C1, 0xC181, 0xC301, 0xC601, 0xCC01, 0xD801, 0xF001, 0xA001};

	for(i = 0; i < size9; i++)
	{
		crc = (crc ^ tmp_data9[i]);

		for(j = 0; j < 8; j++) 
		{
			if(crc & 0x0001)
				crc = ((crc >> 1) ^ (val[j] << (7-j)));
			else
				crc =  (crc >> 1);
		}
	}

	for(i = 0; i < size7; i++)
	{
		crc = (crc ^ tmp_data7[i]);

		for(j = 0; j < 8; j++) 
		{
			if(crc & 0x0001)
				crc = ((crc >> 1) ^ (val[j] << (7-j)));
			else
				crc =  (crc >> 1);
		}
	}

	return (crc & 0xFFFF);
}

static u32 decrypt(const u8 *in, u8** _out)
{
	u32 curBlock[2] = { 0 };
	u32 blockSize = 0;
	u32 xLen = 0;

	u32 i = 0, j = 0;
	u32 xIn = 4, xOut = 0;
	u32 len = 0;
	u32 offset = 0;
	u32 windowOffset = 0;
	u8 d = 0;
	u16 data = 0;

	memcpy(curBlock, in, 8);
	crypt64BitDown(curBlock);
	blockSize = (curBlock[0] >> 8);

	if (blockSize == 0) return (0);

	*_out = (u8*)malloc(blockSize);
	if (!*_out ) return (0);
	u8 *out=*_out;
	memset(out, 0xFF, blockSize);

	xLen = blockSize;
	while(xLen > 0)
	{
		d = T1ReadByte((u8*)curBlock, (xIn % 8));
		xIn++;
		if((xIn % 8) == 0)
		{
			memcpy(curBlock, in + xIn, 8);
			crypt64BitDown(curBlock);
		}

		for(i = 0; i < 8; i++)
		{
			if(d & 0x80)
			{
				data = (T1ReadByte((u8*)curBlock, (xIn % 8)) << 8);
				xIn++;
				if((xIn % 8) == 0)
				{
					memcpy(curBlock, in + xIn, 8);
					crypt64BitDown(curBlock);
				}
				data |= T1ReadByte((u8*)curBlock, (xIn % 8));
				xIn++;
				if((xIn % 8) == 0)
				{
					memcpy(curBlock, in + xIn, 8);
					crypt64BitDown(curBlock);
				}

				len = (data >> 12) + 3;
				offset = (data & 0xFFF);
				windowOffset = (xOut - offset - 1);

				for(j = 0; j < len; j++)
				{
					T1WriteByte(out, xOut, T1ReadByte(out, windowOffset));
					xOut++;
					windowOffset++;

					xLen--;
					if(xLen == 0) return (blockSize);
				}
			}
			else
			{
				T1WriteByte(out, xOut, T1ReadByte((u8*)curBlock, (xIn % 8)));
				xOut++;
				xIn++;
				if((xIn % 8) == 0)
				{
					memcpy(curBlock, in + xIn, 8);
					crypt64BitDown(curBlock);
				}

				xLen--;
				if(xLen == 0) return (blockSize);
			}

			d = ((d << 1) & 0xFF);
		}
	}
	
	return (blockSize);
}

static u32 _decompress(const u8 *in, u8** _out)
{
	u32 curBlock[2] = { 0 };
	u32 blockSize = 0;
	u32 xLen = 0;

	u32 i = 0, j = 0;
	u32 xIn = 4, xOut = 0;
	u32 len = 0;
	u32 offset = 0;
	u32 windowOffset = 0;
	u8 d = 0;
	u16 data = 0;

	memcpy(curBlock, in, 8);
	blockSize = (curBlock[0] >> 8);

	if (blockSize == 0) return (0);

	*_out = (u8*)malloc(blockSize);
	if (!*_out ) return (0);
	u8 *out=*_out;
	memset(out, 0xFF, blockSize);

	xLen = blockSize;
	while(xLen > 0)
	{
		d = T1ReadByte((u8*)curBlock, (xIn % 8));
		xIn++;
		if((xIn % 8) == 0)
		{
			memcpy(curBlock, in + xIn, 8);
		}

		for(i = 0; i < 8; i++)
		{
			if(d & 0x80)
			{
				data = (T1ReadByte((u8*)curBlock, (xIn % 8)) << 8);
				xIn++;
				if((xIn % 8) == 0)
				{
					memcpy(curBlock, in + xIn, 8);
				}
				data |= T1ReadByte((u8*)curBlock, (xIn % 8));
				xIn++;
				if((xIn % 8) == 0)
				{
					memcpy(curBlock, in + xIn, 8);
				}

				len = (data >> 12) + 3;
				offset = (data & 0xFFF);
				windowOffset = (xOut - offset - 1);

				for(j = 0; j < len; j++)
				{
					T1WriteByte(out, xOut, T1ReadByte(out, windowOffset));
					xOut++;
					windowOffset++;

					xLen--;
					if(xLen == 0) return (blockSize);
				}
			}
			else
			{
				T1WriteByte(out, xOut, T1ReadByte((u8*)curBlock, (xIn % 8)));
				xOut++;
				xIn++;
				if((xIn % 8) == 0)
				{
					memcpy(curBlock, in + xIn, 8);
				}

				xLen--;
				if(xLen == 0) return (blockSize);
			}

			d = ((d << 1) & 0xFF);
		}
	}
	
	return (blockSize);
}
//================================================================================
static int load(u8 *data){
	//u32 size = 0;
	u16 shift1 = 0, shift2 = 0, shift3 = 0, shift4 = 0;
	u32 part1addr = 0, part2addr = 0, part3addr = 0, part4addr = 0, part5addr = 0;
	u32 part1ram = 0, part2ram = 0;
	
	//u32	src = 0;

	memcpy(&header, data, sizeof(header));
	if ((header.fw_identifier[0] != 'M') ||
			(header.fw_identifier[1] != 'A') ||
				(header.fw_identifier[2] != 'C')){
					free(data);
					return 4;
				}

	// only 12bits are used
	shift1 = ((header.shift_amounts >> 0) & 0x07);
	shift2 = ((header.shift_amounts >> 3) & 0x07);
	shift3 = ((header.shift_amounts >> 6) & 0x07);
	shift4 = ((header.shift_amounts >> 9) & 0x07);

	// todo - add support for 512Kb 
	part1addr = (header.part1_rom_boot9_addr << (2 + shift1));
	part1ram = (0x02800000 - (header.part1_ram_boot9_addr << (2+shift2)));
	part2addr = (header.part2_rom_boot7_addr << (2+shift3));
	part2ram = (((header.shift_amounts&0x1000)?0x02800000:0x03810000) - (header.part2_ram_boot7_addr << (2+shift4)));
	part3addr = (header.part3_rom_gui9_addr << 3);
	part4addr = (header.part4_rom_wifi7_addr << 3);
	part5addr = (header.part5_data_gfx_addr << 3);

	ARM9bootAddr = part1ram;
	ARM7bootAddr = part2ram;

	if(initKeycode(T1ReadLong(data, 0x08), 1, 0xC) == false){
		free(data);
		return 4;
	}

#if 0
	crypt64BitDown((u32*)&data[0x18]);
#else
	// fix touch coords
	data[0x18] = 0x00;
	data[0x19] = 0x00;
	data[0x1A] = 0x00;
	data[0x1B] = 0x00;

	data[0x1C] = 0x00;
	data[0x1D] = 0xFF;
	data[0x1E] = 0x00;
	data[0x1F] = 0x00;
#endif

	if(initKeycode(T1ReadLong(data, 0x08), 2, 0xC) == false){
		free(data);
		return 4;
	}

	size9 = decrypt(data + part1addr, &tmp_data9);
	if (!tmp_data9){
		free(data);
		return 3;
	}

	size7 = decrypt(data + part2addr, &tmp_data7);
	if (!tmp_data7){
		free(tmp_data9);
		free(data);
		return 3;
	}

	u16 crc16_mine = getBootCodeCRC16();

	if (crc16_mine != header.part12_boot_crc16){
		printf("Firmware: ERROR: the boot code CRC16 (0x%04X) doesn't match the value in the firmware header (0x%04X)", crc16_mine, header.part12_boot_crc16);
		free(tmp_data7);
		free(tmp_data9);
		free(data);
		return 4;
	}

	printf("Firmware:\n");
	//printf("- path: %s\n", CommonSettings.Firmware);
	//printf("- size: %i bytes (%i Mbit)\n", size, size/1024/8);
	printf("- CRC : 0x%04X\n", header.part12_boot_crc16);
	printf("- header: \n");
	printf("   * size firmware %i\n", ((header.shift_amounts >> 12) & 0xF) * 128 * 1024);
	printf("   * ARM9 boot code address:     0x%08X\n", part1addr);
	printf("   * ARM9 boot code RAM address: 0x%08X\n", ARM9bootAddr);
	printf("   * ARM9 unpacked size:         0x%08X (%i) bytes\n", size9, size9);
	printf("   * ARM9 GUI code address:      0x%08X\n", part3addr);
	printf("\n");
	printf("   * ARM7 boot code address:     0x%08X\n", part2addr);
	printf("   * ARM7 boot code RAM address: 0x%08X\n", ARM7bootAddr);
	printf("   * ARM7 WiFi code address:     0x%08X\n", part4addr);
	printf("   * ARM7 unpacked size:         0x%08X (%i) bytes\n", size7, size7);
	printf("\n");
	printf("   * Data/GFX address:           0x%08X\n", part5addr);

	patched = false;
	if(data[0x17C] != 0xFF)patched = true;

	if(patched){
		free(tmp_data7);
		free(tmp_data9);

		u32 patch_offset = 0x3FC80;
		if (data[0x17C] > 1)
			patch_offset = 0x3F680;

		memcpy(&header, data + patch_offset, sizeof(header));

		shift1 = ((header.shift_amounts >> 0) & 0x07);
		shift2 = ((header.shift_amounts >> 3) & 0x07);
		shift3 = ((header.shift_amounts >> 6) & 0x07);
		shift4 = ((header.shift_amounts >> 9) & 0x07);

		// todo - add support for 512Kb
		part1addr = (header.part1_rom_boot9_addr << (2 + shift1));
		part1ram = (0x02800000 - (header.part1_ram_boot9_addr << (2+shift2)));
		part2addr = (header.part2_rom_boot7_addr << (2+shift3));
		part2ram = (((header.shift_amounts&0x1000)?0x02800000:0x03810000) - (header.part2_ram_boot7_addr << (2+shift4)));

		ARM9bootAddr = part1ram;
		ARM7bootAddr = part2ram;

		size9 = _decompress(data + part1addr, &tmp_data9);
		if (!tmp_data9){
			free(data);
			return 3;
		}

		size7 = _decompress(data + part2addr, &tmp_data7);
		if (!tmp_data7){
			free(tmp_data9);
			free(data);
			return 3;
		};

		printf("\nFlashme:\n");
		printf("- header: \n");
		printf("   * ARM9 boot code address:     0x%08X\n", part1addr);
		printf("   * ARM9 boot code RAM address: 0x%08X\n", ARM9bootAddr);
		printf("   * ARM9 unpacked size:         0x%08X (%i) bytes\n", size9, size9);
		printf("\n");
		printf("   * ARM7 boot code address:     0x%08X\n", part2addr);
		printf("   * ARM7 boot code RAM address: 0x%08X\n", ARM7bootAddr);
		printf("   * ARM7 unpacked size:         0x%08X (%i) bytes\n", size7, size7);
	}

	free(data);
	u32 pad9=0x100-(size9&0xff);
	u32 pad7=0x100-(size7&0xff);

#ifdef ARM9
	//Build NDS image from tmp_dataX and sizeX
	extern u8 ndshead[];

	u8 *pFileBuf=(u8*)malloc(0x200+size9+size7);
#else
unsigned char ndshead[512]={
  0x2e,0x00,0x00,0xea,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x23,0x23,0x23,0x23,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,
  0x00,0x02,0x00,0x00,0x00,0x00,0x38,0x02,0x00,0x00,0x38,0x02,0x00,0xa0,0x00,0x00,
  0x00,0xa2,0x00,0x00,0x00,0xd8,0x3a,0x02,0x00,0xd8,0x3a,0x02,0x00,0x19,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0xff,0x7f,0x7f,0x00,0xff,0x1f,0x3f,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x1e,0x05,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0xbe,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x53,0x52,0x41,0x4d,0x5f,0x56,0x31,0x31,0x30,0x00,0x00,0x00,0x50,0x41,0x53,0x53,
  0x30,0x31,0x96,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0xc8,0x60,0x4f,0xe2,0x01,0x70,0x8f,0xe2,0x17,0xff,0x2f,0xe1,0x12,0x4f,0x11,0x48,
  0x12,0x4c,0x20,0x60,0x64,0x60,0x7c,0x62,0x30,0x1c,0x39,0x1c,0x10,0x4a,0x00,0xf0,
  0x14,0xf8,0x30,0x6a,0x80,0x19,0xb1,0x6a,0xf2,0x6a,0x00,0xf0,0x0b,0xf8,0x30,0x6b,
  0x80,0x19,0xb1,0x6b,0xf2,0x6b,0x00,0xf0,0x08,0xf8,0x70,0x6a,0x77,0x6b,0x07,0x4c,
  0x60,0x60,0x38,0x47,0x07,0x4b,0xd2,0x18,0x9a,0x43,0x07,0x4b,0x92,0x08,0xd2,0x18,
  0x0c,0xdf,0xf7,0x46,0x04,0xf0,0x1f,0xe5,0x00,0xfe,0x7f,0x02,0xf0,0xff,0x7f,0x02,
  0xf0,0x01,0x00,0x00,0xff,0x01,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1a,0x9e,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

	u8 *pFileBuf=(u8*)malloc(0x200+size9+pad9+size7+pad7);
#endif
	memcpy(pFileBuf,ndshead,512);
	write32(pFileBuf+0x24,ARM9bootAddr);
	write32(pFileBuf+0x28,ARM9bootAddr);
	write32(pFileBuf+0x2c,size9/*+pad9*/);
	write32(pFileBuf+0x30,size9+pad9+0x200);
	write32(pFileBuf+0x34,ARM7bootAddr);
	write32(pFileBuf+0x38,ARM7bootAddr);
	write32(pFileBuf+0x3c,size7/*+pad7*/);
	write32(pFileBuf+0x80,0x200+size9+pad9+size7+pad7);
	write16(pFileBuf+0x15e,swiCRC16(0xffff,pFileBuf,0x15e));
	memcpy(pFileBuf+0x200,tmp_data9,size9);
#ifdef ARM9
	memcpy(pFileBuf+0x200+size9,tmp_data7,size7);
#else
	memcpy(pFileBuf+0x200+size9+pad9,tmp_data7,size7);
#endif

#ifdef ARM9
	_consolePrint("Rebooting...\n");
	//free(tmp_data7);
	//free(tmp_data9);
	//installargv(pFileBuf,(char*)0x023ff400,pFilename);
	*(vu32*)0x027FFDF4=(u32)pFileBuf;
	DC_FlushAll();
	IPCZ->cmd=ResetRudolph;
	ret_menu9_GENs();
	//bootMoonlight((u32)pFileBuf+0xc0);
#else
	FILE *f=fopen("fw.nds","wb");
	fwrite(pFileBuf,1,0x200+size9+pad9+size7+pad7,f);
	fclose(f);
#endif

	return 0;
}

#ifdef ARM9
int returnDSMenu(){
	//if(IPCZ->NDSType==NDSi)return 1;
	u8 *p=(u8*)malloc(IPCZ->fwsize);
	if(!p)return 3;
	_consolePrint("Getting Firmware...\n");
	IPCZ->firmware_addr=p;
	IPCZ->firmware_bufsize=IPCZ->fwsize;
	DC_FlushAll();
	IPCZ->cmd=GetFirmware;
	while(IPCZ->cmd)swiWaitForVBlank();
	//if(IPCZ->firmware_bufsize!=256*1024)return 2;
	DC_InvalidateAll();
	_consolePrint("Decoding Firmware...\n");
	return load(p);
}
#else
#include <sys/stat.h>
int main(int argc, char **argv){
	struct stat st;
	if(argc==1){
		fprintf(stderr,"XenoBootFW fw.bin\n");
		return 1;
	}
	int i=1;
	for(;i<2;i++){
		FILE *f=fopen(argv[i],"rb");
		if(!f)continue;
		fstat(fileno(f),&st);
		u8 *p=(u8*)malloc(st.st_size);
		if(!p){fclose(f);continue;}
		fread(p,1,st.st_size,f);
		fclose(f);
		//fprintf(stderr,"Processing %s... ",argv[i]);
		load(p);
	}
	return 0;
}
#endif

#else
int returnDSMenu(){
	//if(IPCZ->NDSType==NDSi)return 1;
	return -1;
}
#endif
