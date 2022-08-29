#include "../../libprism/libprism.h"
const u16 bgcolor=RGB15(0,15,5);
const int useARM7Bios=0;

char *wdays[7]={"sunday","monday","tuesday","wednesday","thursday","friday","saturday"};
char progress[26]="                         ";

void Main(){
	PrintfToDie=_consolePrintf2;
	_consolePrintf2(
		"MondayLoader NDS\n"
		"%s\n%s\n\n"
		"Please use L+R+Start+Select to quit.\n",
		ROMDATE,ROMENV
	);

	_consolePrint2("initializing FAT... ");
	if(!disc_mount()){_consolePrint2("failed.\n\n");die();}
	_consolePrint2("done.\n\n");
	char ininame[768];
	int diff=0;
	if(strcpy_safe(ininame,findpath(6,(char*[]){"/","/_dstwoplug/","/ismartplug/","/_iMenu/_ini/","/_plugin_/",mypath},"google2fa_time.ini")))
		diff=ini_getl("google2fa","timediff",0,ininame); // ^p^

	time_t timer;
	struct tm *pt;
	time(&timer);
	timer+=diff;
	pt=localtime(&timer);
	int wday=pt->tm_wday;

	for(;;){
		time(&timer);
		timer+=diff;
		pt=localtime(&timer);
		int t=pt->tm_hour*3600+pt->tm_min*60+pt->tm_sec;
		if(t<2)break;
		_consoleClear();
		int n=t*25/86400,i=0;
		for(;i<n;i++)progress[i]='*';
		nocashMessageMain=0;
		_consolePrintf(
			"Loading wd0:/%s.nds %02d%%...\n"
			"Remaining %02d:%02d:%02d\n"
			"[%s]\n"
			,wdays[(wday+1)%7],t*100/86400,(86400-t)/3600,(86400-t)/60%60,(86400-t)%60,progress
		);
		nocashMessageMain=1;
		for(i=0;i<20;i++)swiWaitForVBlank();
	}

	_consoleClear();
	int n=25,i=0;
	for(;i<n;i++)progress[i]='*';
	_consolePrintf(
		"Loaded wd0:/%s.nds 100%%!\n"
		"Remaining %02d:%02d:%02d\n"
		"[%s]\n"
		,wdays[(wday+1)%7],0,0,0,progress
	);
	for(;;)swiWaitForVBlank();
}
