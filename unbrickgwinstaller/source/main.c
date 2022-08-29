#include "../../libprism/libprism.h"
const u16 bgcolor=RGB15(4,0,12);
const int useARM7Bios=0;

u8 buf[0x600];
void Main(){
	FILE *f;

	_consolePrintf(
		"Unbrick Gateway Installer\n"
		"Warning: make sure fw.bin's version is the same as your DS(lite)'s, or it will mess your console permanently.\n"
		"If you have your own firmware dump, it should be used.\n"
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

	fseek(f,0x1fe00,SEEK_SET);
	fread(buf,1,0x200,f);
	fclose(f);

	_consolePrint("Press A to proceed. If you want to abort, poweroff now.\nProceed with stable power supply.\n");
	while(!(IPCZ->keysdown&KEY_A))swiWaitForVBlank();
	_consolePrint("Start.\n\n");

	IPCZ->firmware_write_addr=buf;
	CallARM7(UnbrickGWInstaller);

	_consolePrint("Done!\n\n");
	die();
}
