#include "../../libprism/libprism.h"
#include "sha1.h"
const u16 bgcolor=RGB15(0,15,5);
const int useARM7Bios=0;

static char *t="ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

static int base32_decode(memstream *in,memstream *out){
	int b=0,c;
	u32 x=0;
	char *p;
	for(;~(c=mgetc(in));){
		if(c=='='){
			break;
		}
		if('a'<=c&&c<='z')c-=0x20;
		if(p=strchr(t,c)){
			x=(x<<5)+(p-t);
			b+=5;
			if(b>=8)b-=8,mputc((x>>b)&0xff,out);
		}
	}
	while(b>=8)b-=8,mputc((x>>b)&0xff,out);
	return 0;
}

#define HMAC_SHA1_DIGESTSIZE 20
#define HMAC_SHA1_BLOCKSIZE  64

//out must have HMAC_SHA1_DIGESTSIZE bytes.
static void hmac_sha1(const u8 *key, int lkey, const u8 *data, int ldata, u8 *out){
	u8 key2[HMAC_SHA1_DIGESTSIZE];
	u8 tmp_digest[HMAC_SHA1_DIGESTSIZE];
	u8 buf[HMAC_SHA1_BLOCKSIZE];
	int i;
	struct sha1_ctxt ctx;

	//truncate
	if(lkey>HMAC_SHA1_BLOCKSIZE){
		sha1_init(&ctx);
		sha1_loop(&ctx,key,lkey);
		sha1_result(&ctx,key2);
		key = key2;
		lkey = HMAC_SHA1_DIGESTSIZE;
	}

	//stage1
	for(i=0;i<lkey;i++)buf[i]=key[i]^0x36;
	for(;i<HMAC_SHA1_BLOCKSIZE;i++)buf[i]=0x36;
	sha1_init(&ctx);
	sha1_loop(&ctx,buf,HMAC_SHA1_BLOCKSIZE);
	sha1_loop(&ctx,data,ldata);
	sha1_result(&ctx,tmp_digest);

	//stage2
	for(i=0;i<lkey;i++)buf[i]=key[i]^0x5c;
	for(;i<HMAC_SHA1_BLOCKSIZE;i++)buf[i]=0x5c;
	sha1_init(&ctx);
	sha1_loop(&ctx,buf,HMAC_SHA1_BLOCKSIZE);
	sha1_loop(&ctx,tmp_digest,HMAC_SHA1_DIGESTSIZE);
	sha1_result(&ctx,out);
}

void Main(){
	_consoleClear();

	_consolePrintf(
		"Google TwoFactor Authenticator on NDS\n"
		"%s\n%s\n"
		"Please use L+R+Start+Select to quit.\n\n"
		"Tokens will be updated every 30sec.\n"
		"Please make sure your clock is correct.\n"
		"Also please configure timezone.\n\n"
		"google2fa_time.ini/google2fa::timediff\n"
		"google2fa_time.ini/google2fa::timezone\n\n",
		ROMDATE,ROMENV
	);

	_consolePrint("initializing FAT... ");
	if(!disc_mount()){_consolePrint("failed.\n\n");die();}
	_consolePrint("done.\n\n");

	char ininame[768];
	int diff=0;
	int timezone=0;
	if(strcpy_safe(ininame,findpath(6,(char*[]){"/","/_dstwoplug/","/ismartplug/","/_iMenu/_ini/","/_plugin_/",mypath},"google2fa_time.ini"))){
		diff=ini_getl("google2fa","timediff",0,ininame);
		timezone=ini_getl("google2fa","timezone",0,ininame);
	}

	u8 key[15][40];
	u32 lkey[15];memset(lkey,0,sizeof(lkey));
	int i=0;
#if 1
	if(!strcpy_safe(ininame,findpath(6,(char*[]){"/","/_dstwoplug/","/ismartplug/","/_iMenu/_ini/","/_plugin_/",mypath},"google2fa_keys.txt"))){_consolePrint("not found google2fa_keys.txt\n");die();}
	FILE *f=fopen(ininame,"rb");
	if(!f){_consolePrint("cannot open google2fa_keys.txt\n");die();}
	for(;i<15;i++){
		if(!myfgets(libprism_cbuf,BUFLEN,f))break;
		libprism_cbuf[64]=0;
		memstream in,out;
		mopen(libprism_cbuf,strlen(libprism_cbuf),&in),mopen(key[i],40,&out),
		base32_decode(&in,&out);
		lkey[i]=mtell(&out);
	}
	fclose(f);
#endif

	time_t timer_old=0,timer;
	struct tm *pt;

	u8 T[8],hash[20];
	int otp=0;
	for(;;){ //This program is meant to be terminated with Ctrl+C.
		for(;;){
			time(&timer);
			timer+=diff;
			pt=localtime(&timer);
			timer-=timezone*3600;
			if((pt->tm_sec%30==0||timer_old==0)&&timer!=timer_old){timer_old=timer;break;}
			nocashMessageMain=0;
			_consolePrintf("Time: %02d:%02d:%02d\r",pt->tm_hour,pt->tm_min,pt->tm_sec);
			nocashMessageMain=1;
			for(i=0;i<20;i++)swiWaitForVBlank();
		}
		u64 TIMER=(u64)timer/30;
		memcpy(T,&TIMER,8);
		{ //fix endian to little
			u8 z;
			z=T[0],T[0]=T[7],T[7]=z;
			z=T[1],T[1]=T[6],T[6]=z;
			z=T[2],T[2]=T[5],T[5]=z;
			z=T[3],T[3]=T[4],T[4]=z;
		}
		_consoleClear2();
		for(i=0;i<15;i++){
			if(!lkey[i])break;
			hmac_sha1(key[i],lkey[i],T,8,hash);
			int offset=hash[19]&0xf;
			otp=( ((hash[offset]&0x7f)<<24) | (hash[offset+1]<<16) | (hash[offset+2]<<8) | hash[offset+3] )%1000000;
			if(i>0)_consolePrint2("---\n");
			_consolePrintf2("%06d\n",otp);
		}
	}
}
