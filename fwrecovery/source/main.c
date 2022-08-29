#include "../../libprism/libprism.h"
const u16 bgcolor=RGB15(4,0,12);

u8 buf[0x600];
void Main(){
	FILE *f;

	_consolePrintf(
		"Firmware Recovery\n"
		"%s\n%s\n\n",
		ROMDATE,ROMENV
	);

	_consolePrint("Initializing FAT... ");
	if(!disc_mount()){_consolePrint("Failed.\n");die();}
	_consolePrint("Done.\n");

	_consolePrint("Opening fw.bin... ");
	f=fopen("/fw.bin\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0","rb");
	if(!f){_consolePrint("Failed.\n");die();}
	_consolePrint("Done.\n");

	fread(buf,1,0x200,f);
	fseek(f,(read16(buf+0x20)<<3)-0x400,SEEK_SET);
	fread(buf,1,0x600,f);
	fclose(f);

	_consolePrint("Press A to proceed. If you want to abort, poweroff now.\nProceed with stable power supply.\n");
	while(!(IPCZ->keysdown&KEY_A))swiWaitForVBlank();
	_consolePrint("Start.\n\n");

	_consolePrint("Writing WFC 0...\n");
	IPCZ->firmware_write_addr=buf+0x000;
	IPCZ->firmware_write_index=0;
	_consolePrintf("Old=%04x New=%04x\n",*(vu16*)(IPCZ->firmware_write_addr+0xfe),swiCRC16(0,IPCZ->firmware_write_addr,0xfe));
	CallARM7(WriteFirmware);
	_consolePrint("Writing WFC 1...\n");
	IPCZ->firmware_write_addr=buf+0x100;
	IPCZ->firmware_write_index=1;
	_consolePrintf("Old=%04x New=%04x\n",*(vu16*)(IPCZ->firmware_write_addr+0xfe),swiCRC16(0,IPCZ->firmware_write_addr,0xfe));
	CallARM7(WriteFirmware);
	_consolePrint("Writing WFC 2...\n");
	IPCZ->firmware_write_addr=buf+0x200;
	IPCZ->firmware_write_index=2;
	_consolePrintf("Old=%04x New=%04x\n",*(vu16*)(IPCZ->firmware_write_addr+0xfe),swiCRC16(0,IPCZ->firmware_write_addr,0xfe));
	CallARM7(WriteFirmware);
	_consolePrint("Writing Config 0...\n");
	IPCZ->firmware_write_addr=buf+0x400;
	IPCZ->firmware_write_index=4;
	_consolePrintf("Old=%04x New=%04x\n",*(vu16*)(IPCZ->firmware_write_addr+0x72),swiCRC16(0xffff,IPCZ->firmware_write_addr,0x70));
	CallARM7(WriteFirmware);
	_consolePrint("Writing Config 1...\n");
	IPCZ->firmware_write_addr=buf+0x500;
	IPCZ->firmware_write_index=5;
	_consolePrintf("Old=%04x New=%04x\n",*(vu16*)(IPCZ->firmware_write_addr+0x72),swiCRC16(0xffff,IPCZ->firmware_write_addr,0x70));
	CallARM7(WriteFirmware);

	_consolePrint("Done!\n\n");
	die();
}
