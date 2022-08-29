#include "../../libprism/libprism.h"

const u16 bgcolor=RGB15(4,0,12);

char ininame[768];
char menu_a[768],menu_b[768],menu_x[768],menu_y[768],menu_st[768],menu_se[768],menu_l[768],menu_r[768];
char menu_touch[768],menu_crossu[768],menu_crossd[768],menu_crossl[768],menu_crossr[768];
char menu_none[768];
void Main(){
	char *file;
	int c=0;
	u32 keys=0;
	char dldiid[5];

	int usenone=0;

	IPCZ->cmd=0;
	_consolePrintf(
		"FavLauncher ms\n"
		"reset_mse_06b_for_ak2 by Moonlight, Rudolph, kzat3\n"
		"dldipatch aka dlditool public domain under CC0.\n"
		"%s\n%s\n\n",
		ROMDATE,ROMENV
	);

	{
		unsigned char *dldiFileData=DLDIDATA;
		memcpy(dldiid,(char*)dldiFileData+ioType,4);
		dldiid[4]=0;
		_consolePrintf("DLDI ID: %s\n",dldiid);
		_consolePrintf("DLDI Name: %s\n\n",(char*)dldiFileData+friendlyName);
	}

	_consolePrintf("Initializing libfat... ");
	if(!fatInitDefault()){_consolePrintf("Failed.\n");die();}
	_consolePrintf("Done.\n");

	//_consolePrintf("Reading /favlauncher.ini Phase1 ");
	_consolePrintf("Checking for favlauncher.ini...\n");

if(strcpy_safe(ininame,findpath(2,(char*[]){"/",mypath},"favlauncher.ini"))){
	_consolePrintf("Reading favlauncher.ini...\n");
	ini_gets("General","A","/_menu_a.nds",menu_a,768,ininame);
	ini_gets("General","B","/_menu_b.nds",menu_b,768,ininame);
	ini_gets("General","X","/_menu_x.nds",menu_x,768,ininame);
	ini_gets("General","Y","/_menu_y.nds",menu_y,768,ininame);
	ini_gets("General","Start","/_menu_st.nds",menu_st,768,ininame);
	ini_gets("General","Select","/_menu_se.nds",menu_se,768,ininame);
	ini_gets("General","L","/_menu_l.nds",menu_l,768,ininame);
	ini_gets("General","R","/_menu_r.nds",menu_r,768,ininame);
	ini_gets("General","Touch","/_menu_touch.nds",menu_touch,768,ininame);
	ini_gets("General","Up","/_menu_crossu.nds",menu_crossu,768,ininame);
	ini_gets("General","Down","/_menu_crossd.nds",menu_crossd,768,ininame);
	ini_gets("General","Left","/_menu_crossl.nds",menu_crossl,768,ininame);
	ini_gets("General","Right","/_menu_crossr.nds",menu_crossr,768,ininame);
	ini_gets("General","None","/_menu_.nds",menu_none,768,ininame);
	usenone=ini_getl("General","UseNone",0,ininame);

	//_consolePrintf("Phase2");
	ini_gets(dldiid,"A",menu_a,menu_a,768,ininame);
	ini_gets(dldiid,"B",menu_b,menu_b,768,ininame);
	ini_gets(dldiid,"X",menu_x,menu_x,768,ininame);
	ini_gets(dldiid,"Y",menu_y,menu_y,768,ininame);
	ini_gets(dldiid,"Start",menu_st,menu_st,768,ininame);
	ini_gets(dldiid,"Select",menu_se,menu_se,768,ininame);
	ini_gets(dldiid,"L",menu_l,menu_l,768,ininame);
	ini_gets(dldiid,"R",menu_r,menu_r,768,ininame);
	ini_gets(dldiid,"Touch",menu_touch,menu_touch,768,ininame);
	ini_gets(dldiid,"Up",menu_crossu,menu_crossu,768,ininame);
	ini_gets(dldiid,"Down",menu_crossd,menu_crossd,768,ininame);
	ini_gets(dldiid,"Left",menu_crossl,menu_crossl,768,ininame);
	ini_gets(dldiid,"Right",menu_crossr,menu_crossr,768,ininame);
	ini_gets(dldiid,"None",menu_none,menu_none,768,ininame);
	usenone=ini_getl(dldiid,"UseNone",usenone,ininame);
}else{
	strcpy(menu_a,"/_menu_a.nds");
	strcpy(menu_b,"/_menu_b.nds");
	strcpy(menu_x,"/_menu_x.nds");
	strcpy(menu_y,"/_menu_y.nds");
	strcpy(menu_st,"/_menu_st.nds");
	strcpy(menu_se,"/_menu_se.nds");
	strcpy(menu_l,"/_menu_l.nds");
	strcpy(menu_r,"/_menu_r.nds");
	strcpy(menu_touch,"/_menu_touch.nds");
	strcpy(menu_crossu,"/_menu_crossu.nds");
	strcpy(menu_crossd,"/_menu_crossd.nds");
	strcpy(menu_crossl,"/_menu_crossl.nds");
	strcpy(menu_crossr,"/_menu_crossr.nds");
	//strcpy(menu_none,"/_menu_.nds");
}

if(!usenone){
	_consolePrintf("     A ");
	_consolePrintf(menu_a);
	_consolePrintf("\n     B ");
	_consolePrintf(menu_b);
	_consolePrintf("\n     X ");
	_consolePrintf(menu_x);
	_consolePrintf("\n     Y ");
	_consolePrintf(menu_y);
	_consolePrintf("\n Start ");
	_consolePrintf(menu_st);
	_consolePrintf("\nSelect ");
	_consolePrintf(menu_se);
	_consolePrintf("\n     L ");
	_consolePrintf(menu_l);
	_consolePrintf("\n     R ");
	_consolePrintf(menu_r);
	_consolePrintf("\n Touch ");
	_consolePrintf(menu_touch);
	_consolePrintf("\n    Up ");
	_consolePrintf(menu_crossu);
	_consolePrintf("\n  Down ");
	_consolePrintf(menu_crossd);
	_consolePrintf("\n  Left ");
	_consolePrintf(menu_crossl);
	_consolePrintf("\n Right ");
	_consolePrintf(menu_crossr);
	//_consolePrintf("\n  None ");
	//_consolePrintf(menu_none);
}
	_consolePrintf("\n");

	//_consolePrintf("Now start select procedure.\n");

	for(;;c++){
		swiWaitForVBlank();
		keys=0;
		if(!c)_consolePrintf("\nPress desired key: ");
		//_consolePrintf("\r");
		keys = IPCZ->keysheld;
		if(keys/*&0x1fff*/||usenone)break;
	}

			if(keys&KEY_A){_consolePrintf("A");file=menu_a;}
		else	if(keys&KEY_B){_consolePrintf("B");file=menu_b;}
		else	if(keys&KEY_X){_consolePrintf("X");file=menu_x;}
		else	if(keys&KEY_Y){_consolePrintf("Y");file=menu_y;}
		else	if(keys&KEY_L){_consolePrintf("L");file=menu_l;}
		else	if(keys&KEY_R){_consolePrintf("R");file=menu_r;}
		else	if(keys&KEY_START){_consolePrintf("Start");file=menu_st;}
		else	if(keys&KEY_SELECT){_consolePrintf("Select");file=menu_se;}
		else	if(keys&KEY_TOUCH){_consolePrintf("Touch");file=menu_touch;}
		else	if(keys&KEY_UP){_consolePrintf("Up");file=menu_crossu;}
		else	if(keys&KEY_DOWN){_consolePrintf("Down");file=menu_crossd;}
		else	if(keys&KEY_LEFT){_consolePrintf("Left");file=menu_crossl;}
		else	if(keys&KEY_RIGHT){_consolePrintf("Right");file=menu_crossr;}
		else	{_consolePrintf("None");file=menu_none;}

	_consolePrintf(".\n");

	for(c=0;;c++){
		swiWaitForVBlank();
		if(!c)_consolePrintf("Now release all keys to ensure to boot the NDS cleanly... ");
		if(!IPCZ->keysheld)break;
	}
	_consolePrintf("OK.\n\n");

	//_consolePrintf("Waiting... ");
	//sleep(1);
	//_consolePrintf("Done.\n");
	BootNDSROM(file);
	// ^^^^^^^^^^^^ add 2008.03.30 kzat3
}
