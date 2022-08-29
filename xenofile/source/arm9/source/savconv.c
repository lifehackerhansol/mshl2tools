#include "xenofile.h"
#include "zlib/zlib.h"
#include <fcntl.h>

//convertion library from Unique Geeks Media Offline NDS Save Converter... lol
u8 *dsvfooter=(u8*)"|<--Snip above here to create a raw sav by excluding this DeSmuME savedata footer:\0\0\x06\0\0\0\x08\0\0\0\0\0\x03\0\0\0\0\0\0\0\0\0\0\0|-DESMUME SAVE-|";

char *savmenu[]={
	"Backup to bak",
	"Convert to duc (ARDS MAX)",
	"Convert to duc (ARDS ME)",
	"Convert to dsv (desmume)",
	"Convert to gds (gameshark, beta)",
	"Convert to Code Freak Save [cfs]",
	"Convert to NO$GBA Save [ngs]",
	"Enable YSMenu softreset",
	"Disable YSMenu softreset",
};

//read p for size(bytes).
//if the chunk is larger than check(byte), RLE length(bytes) after copying retval(bytes).
static int myRLE(const u8 *p, const unsigned int size, const unsigned int check, unsigned int *length){ //if>3bytes, fill operation will be used.
	int i=0,j;
	if(size<3||check<3)return -1;
	for(;i<size-check;i++){
		for(j=i+1;j<size;j++)
			if(p[i]!=p[j])break;
		if(j>=i+check){*length=j-i;return i;}
	}
	return -1;
}

static u32 compsize;
static void ngscopy(const u8 *p, const unsigned int _size, FILE *out){
	unsigned int size=_size,offset=0;
	while(size){
		u16 writesize=min(size,0x7f);
		//_consolePrintf("copy %dbytes\n",writesize);
		fputc(writesize,out);
		fwrite(p+offset,1,writesize,out);
		size-=writesize;
		offset+=writesize;
		//compsize+=1+writesize;
	}
}

static void ngsrepeat(const u8 p, const unsigned int _size, FILE *out){
	unsigned int size=_size;
	while(size>0x7f){
		u16 writesize=min(size,65535);
		//_consolePrintf("repeat %dbytes\n",writesize);
		fputc(0x80,out);
		fputc(p,out);
		fwrite(&writesize,1,2,out);
		size-=writesize;
		//compsize+=4;
	}
	if(size){
		//_consolePrintf("repeat %dbytes\n",size);
		fputc(size+0x80,out);
		fputc(p,out);
		//compsize+=2;
	}
}

bool savConvert(char *file){
	char to[768];
	struct stat st,stto;
	FILE *in,*out;
	u32 size=0,read=0,cur=0;

	strcpy(to,file);
	if(!strcasecmp(to+strlen(to)-4,".duc")){
		strcpy(to+strlen(to)-4,".sav");
		_consolePrint2("*** convert duc to sav ***\n");
		if(!stat(to,&stto)){
			_consolePrintf2("%s already exists (%s)\n",to,(stat(file,&st),stto.st_mtime>st.st_mtime)?"NEWER":"older");
			_consolePrint2("A to overwrite, other to abort.\n");
			int key;
			for(;!(key=IPCZ->keysdown);)swiWaitForVBlank();
			if(!(key&KEY_A)){_consolePrint2("Aborted.\n");return false;}
		}
		in=fopen(file,"rb");
		if(!in){_consolePrintf2("cannot open %s\n",file);return false;}
		out=fopen(to,"wb");
		if(!out){fclose(in);_consolePrintf2("cannot open %s\n",file);return false;}
		size=filelength(fileno(in));
		u32 pad=size&0xfff;
		size-=pad;
		fseek(in,pad,SEEK_SET);
		_consoleStartProgress2();
		for(;(read=fread(libprism_buf,1,BUFLEN,in))>0;){
			cur+=read;
			fwrite(libprism_buf,1,read,out);
			_consolePrintProgress2("Converting",cur,size);
		}
		_consoleEndProgress2();
		fclose(out);
		fclose(in);
		return true;
	}
	if(!strcasecmp(to+strlen(to)-4,".dsv")){
		strcpy(to+strlen(to)-4,".sav");
		_consolePrint2("*** convert dsv to sav ***\n");
		if(!stat(to,&stto)){
			_consolePrintf2("%s already exists (%s)\n",to,(stat(file,&st),stto.st_mtime>st.st_mtime)?"NEWER":"older");
			_consolePrint2("A to overwrite, other to abort.\n");
			int key;
			for(;!(key=IPCZ->keysdown);)swiWaitForVBlank();
			if(!(key&KEY_A)){_consolePrint2("Aborted.\n");return false;}
		}
		in=fopen(file,"rb");
		if(!in){_consolePrintf2("cannot open %s\n",file);return false;}
		out=fopen(to,"wb");
		if(!out){fclose(in);_consolePrintf2("cannot open %s\n",file);return false;}
		size=filelength(fileno(in));
		u32 pad=size&0xfff;
		size-=pad;
		_consoleStartProgress2();
		for(;(read=fread(libprism_buf,1,BUFLEN,in))>0;){
			if(read&pad)break;
			cur+=read;
			fwrite(libprism_buf,1,read,out);
			_consolePrintProgress2("Converting",cur,size);
		}
		_consoleEndProgress2();
		fclose(out);
		fclose(in);
		return true;
	}
	if(!strcasecmp(to+strlen(to)-4,".gds")){
		strcpy(to+strlen(to)-4,".sav");
		_consolePrint2("*** convert gds to sav ***\n");
		if(!stat(to,&stto)){
			_consolePrintf2("%s already exists (%s)\n",to,(stat(file,&st),stto.st_mtime>st.st_mtime)?"NEWER":"older");
			_consolePrint2("A to overwrite, other to abort.\n");
			int key;
			for(;!(key=IPCZ->keysdown);)swiWaitForVBlank();
			if(!(key&KEY_A)){_consolePrint2("Aborted.\n");return false;}
		}
		int _in=open(file,O_RDONLY);
		if(_in<0){_consolePrintf2("cannot open %s\n",file);return false;}
		lseek(_in,0x100,SEEK_SET);
		gzFile gz=gzdopen(_in,"rb");
		if(!gz){close(_in),_consolePrint2("cannot get gzip handle to decompress\n");return false;}
		out=fopen(to,"wb");
		if(!out){gzclose(gz);_consolePrintf2("cannot open %s\n",file);return false;}

		u32 read,cur=0;
		_consoleStartProgress2();
		for(;(read=gzread(gz,libprism_buf,BUFLEN))>0;){
			cur+=read;
			fwrite(libprism_buf,1,read,out);
			_consolePrintProgress2("Converting",lseek(_in,0,SEEK_CUR)-0x100,filelength(_in)-0x100);
		}
		_consoleEndProgress2();
		fclose(out);
		gzclose(gz);
		return true;
	}
	if(!strcasecmp(to+strlen(to)-4,".bak")){
		strcpy(to+strlen(to)-4,".sav");
		_consolePrint2("*** restore bak to sav ***\n");
		stat(file,&st);
		if(!stat(to,&stto)){
			_consolePrintf2("%s already exists (%s)\n",to,stto.st_mtime>st.st_mtime?"NEWER":"older");
			_consolePrint2("A to overwrite, other to abort.\n");
			int key;
			for(;!(key=IPCZ->keysdown);)swiWaitForVBlank();
			if(!(key&KEY_A)){_consolePrint2("Aborted.\n");return false;}
		}
		bool ret = !copy(file,to);
		if(ret)libprism_utime(to,st.st_atime,st.st_mtime);
		return ret;
	}
	if(!strcasecmp(to+strlen(to)-4,".cfs")){
		strcpy(to+strlen(to)-4,".sav");
		_consolePrint2("*** convert cfs to sav ***\n");
		if(!stat(to,&stto)){
			_consolePrintf2("%s already exists (%s)\n",to,(stat(file,&st),stto.st_mtime>st.st_mtime)?"NEWER":"older");
			_consolePrint2("A to overwrite, other to abort.\n");
			int key;
			for(;!(key=IPCZ->keysdown);)swiWaitForVBlank();
			if(!(key&KEY_A)){_consolePrint2("Aborted.\n");return false;}
		}
		in=fopen(file,"rb");
		if(!in){_consolePrintf2("cannot open %s\n",file);return false;}
		out=fopen(to,"wb");
		if(!out){fclose(in);_consolePrintf2("cannot open %s\n",file);return false;}
		size=filelength(fileno(in));
		u32 pad=size&0xfff;
		size-=pad;
		fseek(in,pad,SEEK_SET);
		_consoleStartProgress2();
		for(;(read=fread(libprism_buf,1,BUFLEN,in))>0;){
			cur+=read;
			fwrite(libprism_buf,1,read,out);
			_consolePrintProgress2("Converting",cur,size);
		}
		_consoleEndProgress2();
		fclose(out);
		fclose(in);
		return true;
	}
	if(!strcasecmp(to+strlen(to)-4,".ngs")){
		strcpy(to+strlen(to)-4,".sav");
		_consolePrint2("*** convert ngs to sav ***\n");
		if(!stat(to,&stto)){
			_consolePrintf2("%s already exists (%s)\n",to,(stat(file,&st),stto.st_mtime>st.st_mtime)?"NEWER":"older");
			_consolePrint2("A to overwrite, other to abort.\n");
			int key;
			for(;!(key=IPCZ->keysdown);)swiWaitForVBlank();
			if(!(key&KEY_A)){_consolePrint2("Aborted.\n");return false;}
		}
		char head[0x48];
		in=fopen(file,"rb");
		if(!in){_consolePrintf2("cannot open %s\n",file);return false;}
		fread(head,1,0x48,in);
		if(memcmp(head,"NocashGbaBackupMediaSavDataFile",0x1f)||head[0x1f]!=0x1a||memcmp(head+0x40,"SRAM",4)){fclose(in);_consolePrintf2("not valid NO$GBA save %s\n",file);return false;}
		out=fopen(to,"wb");
		if(!out){fclose(in);_consolePrintf2("cannot open %s\n",file);return false;}
		u32 compmethod=read32(head+0x44);
		u32 compsize=0;
		u32 decompsize=0;
		u32 i=0;
		if(compmethod==0){
			_consoleStartProgress2();
			fread(&decompsize,1,4,in);
			for(;i<decompsize;){
				fputc(fgetc(in),out);
				i++;
				if(!(i&0xffff))_consolePrintProgress2("Converting",i,decompsize);
			}
			_consoleEndProgress2();
		}else if(compmethod==1){ //RLE
			//decompression based on desmume/src/mc.cpp (not copied)
			u32 _progress=0;
			_consoleStartProgress2();
			fread(&compsize,1,4,in);
			fread(&decompsize,1,4,in);
			for(;;){
				int dict=fgetc(in);
				if(!dict)break;
				else if(dict==0x80){//repeat long
					u8 copy=fgetc(in);
					u16 length;
					fread(&length,1,2,in);
					i+=length;
					for(;length;length--)fputc(copy,out);
				}else if(dict>0x80){//repeat short
					u8 copy=fgetc(in);
					u8 length=dict-0x80;
					i+=length;
					for(;length;length--)fputc(copy,out);
				}else{//copy
					u8 length=dict;
					i+=length;
					for(;length;length--)fputc(fgetc(in),out);
				}
				if(i>65535){
					_progress+=i;
					i=0;
					_consolePrintProgress2("Converting",_progress,decompsize);
				}
			}
			_consoleEndProgress2();
		}else{
			_consolePrintf2("unknown compression method %d\n",compmethod);
			fclose(in);fclose(out);return false;
		}
		fclose(in);fclose(out);return true;
	}
	if(!strcasecmp(to+strlen(to)-4,".sav")){
		int ret=selectpref("Sav convertion menu",arraysize(savmenu),savmenu);
		switch(ret){
			case -1:{_consolePrint2("\n\nAborted.\n");return false;}
			case 0:{
				_consoleClear2();
				strcpy(to+strlen(to)-4,".bak");
				_consolePrint2("*** backup sav to bak ***\n");
				stat(file,&st);
				if(!stat(to,&stto)){
					_consolePrintf2("%s already exists (%s)\n",to,stto.st_mtime>st.st_mtime?"NEWER":"older");
					_consolePrint2("A to overwrite, other to abort.\n");
					int key;
					for(;!(key=IPCZ->keysdown);)swiWaitForVBlank();
					if(!(key&KEY_A)){_consolePrint2("Aborted.\n");return false;}
				}
				bool ret = !copy(file,to);
				if(ret)libprism_utime(to,st.st_atime,st.st_mtime);
				return ret;
			}break;
			case 1:{
				_consoleClear2();
				strcpy(to+strlen(to)-4,".duc");
				_consolePrint2("*** convert sav to duc (ARDS MAX) ***\n");
				if(!stat(to,&stto)){
					_consolePrintf2("%s already exists (%s)\n",to,(stat(file,&st),stto.st_mtime>st.st_mtime)?"NEWER":"older");
					_consolePrint2("A to overwrite, other to abort.\n");
					int key;
					for(;!(key=IPCZ->keysdown);)swiWaitForVBlank();
					if(!(key&KEY_A)){_consolePrint2("Aborted.\n");return false;}
				}
				FILE *in=fopen(file,"rb");
				if(!in){_consolePrintf2("cannot open %s\n",file);return false;}
				FILE *out=fopen(to,"wb");
				if(!out){fclose(in);_consolePrintf2("cannot open %s\n",file);return false;}

				char header[500];
				char *gamename=header+344,*savename=header+373,*savedesc=header+405;
				u16 *lgamename=(u16*)(header+16),*lsavename=(u16*)(header+80),*lsavedesc=(u16*)(header+144);
				memset(header,0,sizeof(header));

				strncpy(gamename,"Converted Saves",28);
				strncpy(savename,"Converted Save",32);
				strncpy(savedesc,"Converted by XenoFile",95);
				mbstoucs2(lgamename,gamename);//,31);
				mbstoucs2(lsavename,savename);//,31);
				mbstoucs2(lsavedesc,savedesc);//,100);

				memcpy(header,"ARDS000000000001",16);
				header[78]=0x2e;
				header[372]=0x0d;
				fwrite(header,1,sizeof(header),out);

				u32 size=filelength(fileno(in)),cur=0;
				_consoleStartProgress2();
				for(;(read=fread(libprism_buf,1,BUFLEN,in))>0;){
					cur+=read;
					fwrite(libprism_buf,1,read,out);
					_consolePrintProgress2("Converting",cur,size);
				}
				_consoleEndProgress2();
				fclose(out);
				fclose(in);
				return true;
			}break;
			case 2:{
				_consoleClear2();
				strcpy(to+strlen(to)-4,".duc");
				_consolePrint2("*** convert sav to duc (ARDS ME) ***\n");
				if(!stat(to,&stto)){
					_consolePrintf2("%s already exists (%s)\n",to,(stat(file,&st),stto.st_mtime>st.st_mtime)?"NEWER":"older");
					_consolePrint2("A to overwrite, other to abort.\n");
					int key;
					for(;!(key=IPCZ->keysdown);)swiWaitForVBlank();
					if(!(key&KEY_A)){_consolePrint2("Aborted.\n");return false;}
				}
				FILE *in=fopen(file,"rb");
				if(!in){_consolePrintf2("cannot open %s\n",file);return false;}
				FILE *out=fopen(to,"wb");
				if(!out){fclose(in);_consolePrintf2("cannot open %s\n",file);return false;}

				char header[164];
				char *gamename=header+4,*savename=header+32,*savedesc=header+65;
				memset(header,0,sizeof(header));
				
				strncpy(gamename,"Converted Saves",23);
				header[27]=0xa1;
				strncpy(savename,"Converted Save",31);
				strncpy(savedesc,"Converted by XenoFile",70);
				memcpy(header+162,"\x08\xc0",2);
				fwrite(header,1,sizeof(header),out);

				u32 size=filelength(fileno(in)),cur=0;
				_consoleStartProgress2();
				for(;(read=fread(libprism_buf,1,BUFLEN,in))>0;){
					cur+=read;
					fwrite(libprism_buf,1,read,out);
					_consolePrintProgress2("Converting",cur,size);
				}
				_consoleEndProgress2();
				fclose(out);
				fclose(in);
				return true;
			}break;
			case 3:{
				_consoleClear2();
				strcpy(to+strlen(to)-4,".dsv");
				_consolePrint2("*** convert sav to dsv ***\n");
				if(!stat(to,&stto)){
					_consolePrintf2("%s already exists (%s)\n",to,(stat(file,&st),stto.st_mtime>st.st_mtime)?"NEWER":"older");
					_consolePrint2("A to overwrite, other to abort.\n");
					int key;
					for(;!(key=IPCZ->keysdown);)swiWaitForVBlank();
					if(!(key&KEY_A)){_consolePrint2("Aborted.\n");return false;}
				}
				FILE *in=fopen(file,"rb");
				if(!in){_consolePrintf2("cannot open %s\n",file);return false;}
				FILE *out=fopen(to,"wb");
				if(!out){fclose(in);_consolePrintf2("cannot open %s\n",file);return false;}
				u32 size=filelength(fileno(in)),cur=0;
				_consoleStartProgress2();
				for(;(read=fread(libprism_buf,1,BUFLEN,in))>0;){
					cur+=read;
					fwrite(libprism_buf,1,read,out);
					_consolePrintProgress2("Converting",cur,size);
				}
				_consoleEndProgress2();
				fwrite(dsvfooter,1,122,out);
				fclose(out);
				fclose(in);
				return true;
			}break;
			case 4:{
				_consoleClear2();
				strcpy(to+strlen(to)-4,".gds");
				_consolePrint2("*** convert sav to gds (gameshark) ***\n");
				if(!stat(to,&stto)){
					_consolePrintf2("%s already exists (%s)\n",to,(stat(file,&st),stto.st_mtime>st.st_mtime)?"NEWER":"older");
					_consolePrint2("A to overwrite, other to abort.\n");
					int key;
					for(;!(key=IPCZ->keysdown);)swiWaitForVBlank();
					if(!(key&KEY_A)){_consolePrint2("Aborted.\n");return false;}
				}
				FILE *in=fopen(file,"rb");
				if(!in){_consolePrintf2("cannot open %s\n",file);return false;}
				int _out=open(to,O_WRONLY|O_CREAT|O_TRUNC);
				if(!_out){fclose(in);_consolePrintf2("cannot open %s\n",file);return false;}

				char header[256];
				char *gamename=header+16,*savename=header+64,*savedesc=header+112;
				memset(header,0,sizeof(header));
				memcpy(header,"DSXS\x01\x08\0\0\xff\xff\xff\x22\0\0\0\0",16);
				strncpy(gamename,"Your game",48);
				strncpy(savename,"Converted savegame",48);
				strncpy(savedesc,"This is a converted savegame",144);
				write(_out,header,sizeof(header));

				gzFile gz=gzdopen(_out,"wb9");
				if(!gz){fclose(in);close(_out);_consolePrint2("cannot alloc gzip handle\n");return false;}
				//gzsetparams(gz,9,0);
				u32 size=filelength(fileno(in)),cur=0;
				_consoleStartProgress2();
				for(;(read=fread(libprism_buf,1,BUFLEN,in))>0;){
					cur+=read;
					gzwrite(gz,libprism_buf,read);
					_consolePrintProgress2("Converting",cur,size);
				}
				_consoleEndProgress2();
				fclose(in);
				gzclose(gz);
				_out=open(to,O_RDWR);
				int s=filelength(_out)-0x100;
				u8 z[3];
				z[0]=s&0xff,z[1]=(s>>8)&0xff,z[2]=(s>>16)&0xff;
				lseek(_out,8,SEEK_SET);
				write(_out,z,3);
				close(_out);
				return true;
			}break;
			case 5:{
				_consoleClear2();
				strcpy(to+strlen(to)-4,".cfs");
				_consolePrint2("*** convert sav to Code Freak Save ***\n");
				if(!stat(to,&stto)){
					_consolePrintf2("%s already exists (%s)\n",to,(stat(file,&st),stto.st_mtime>st.st_mtime)?"NEWER":"older");
					_consolePrint2("A to overwrite, other to abort.\n");
					int key;
					for(;!(key=IPCZ->keysdown);)swiWaitForVBlank();
					if(!(key&KEY_A)){_consolePrint2("Aborted.\n");return false;}
				}
				FILE *in=fopen(file,"rb");
				if(!in){_consolePrintf2("cannot open %s\n",file);return false;}
				FILE *out=fopen(to,"wb");
				if(!out){fclose(in);_consolePrintf2("cannot open %s\n",file);return false;}

				char header[154];
				u16 *lsavename=(u16*)(header+0x10);
				memset(header,0,sizeof(header));

				mbstoucs2(lsavename,"savconv");//,0x10);
				header[0]=0x9a,header[6]=0x10;
				fwrite(header,1,sizeof(header),out);

				u32 size=filelength(fileno(in)),cur=0;
				_consoleStartProgress2();
				for(;(read=fread(libprism_buf,1,BUFLEN,in))>0;){
					cur+=read;
					fwrite(libprism_buf,1,read,out);
					_consolePrintProgress2("Converting",cur,size);
				}
				_consoleEndProgress2();
				fclose(out);
				fclose(in);
				return true;
			}break;
			case 6:{
				_consoleClear2();
				strcpy(to+strlen(to)-4,".ngs");
				_consolePrint2("*** convert sav to NO$GBA save ***\n");
				if(!stat(to,&stto)){
					_consolePrintf2("%s already exists (%s)\n",to,(stat(file,&st),stto.st_mtime>st.st_mtime)?"NEWER":"older");
					_consolePrint2("A to overwrite, other to abort.\n");
					int key;
					for(;!(key=IPCZ->keysdown);)swiWaitForVBlank();
					if(!(key&KEY_A)){_consolePrint2("Aborted.\n");return false;}
				}
				FILE *in=fopen(file,"rb");
				if(!in){_consolePrintf2("cannot open %s\n",file);return false;}
				FILE *out=fopen(to,"wb");
				if(!out){fclose(in);_consolePrintf2("cannot open %s\n",file);return false;}
				u8 head[0x50];
				memset(head,0,0x50);
				memcpy(head,"NocashGbaBackupMediaSavDataFile",0x1f);
				head[0x1f]=0x1a;
				memcpy(head+0x40,"SRAM",4);

				u32 insize=filelength(fileno(in)),_insize=insize;
				if(insize>1024*1024){
					_consolePrint2("more than 1MB; cannot allocate buffer to compress.\n");
					write32(head+0x48,insize);
					fwrite(head,1,0x4c,out);
					_consoleStartProgress2();
					u32 read,cur=0;
					for(;(read=fread(libprism_buf,1,BUFLEN,in))>0;){
						cur+=read;
						fwrite(libprism_buf,1,read,out);
						_consolePrintProgress2("Converting",cur,insize);
					}
					_consoleEndProgress2();
				}else{
					head[0x44]=1;
					write32(head+0x4c,insize);
					fwrite(head,1,0x50,out);

					//compression
					compsize=0; //global
					u32 _progress=0;
					u8 *buffer=(u8*)malloc(insize);
					if(!buffer){
						fclose(out);fclose(in);
						_consolePrintf2("cannot alloc %dbytes.\n",insize);return false;
					}
					fread(buffer,1,insize,in);
					_consoleStartProgress2();

					//loop
	while(insize){
		u32 rlelength;
		u32 rleaddr=myRLE(buffer,insize,min(insize,3),&rlelength);
		if(~rleaddr){
			if(rleaddr)ngscopy(buffer,rleaddr,out);
			ngsrepeat(buffer[rleaddr],rlelength,out);
			insize-=rleaddr+rlelength;
			buffer+=rleaddr+rlelength;
			_progress+=rleaddr+rlelength;
			_consolePrintProgress2("Converting",_progress,_insize);
			continue;
		}
		ngscopy(buffer,insize,out);
		break; //
	}
					fwrite("\0LIFE\x01\0\0\0\x09\0\0\0\0 \0\0\x80\0\0 \0LIFF\x01\0\0\0\x09\0\0\0\0\x01\0\0\x80\0\0\x01\0STOP\0\0\0\0\0\0\0\0",1,0x37,out);//end mark
					_consoleEndProgress2();
					free(buffer);
					fflush(out);
					compsize=filelength(fileno(out))-130;
					fseek(out,0x48,SEEK_SET);
					fwrite(&compsize,1,4,out);
				}
				fclose(out);
				fclose(in);
				return true;				
			}break;
			case 7:{
				_consoleClear2();
				_consolePrint2("*** enable YSMenu softreset ***\n");
				struct stat st;
				stat(file,&st);
				FILE *f=fopen(file,"r+b");
				fseek(f,filelength(fileno(f))-8,SEEK_SET);
				fwrite("\0\0\x0f\0NMSY",1,8,f);
				fclose(f);
				libprism_utime(file,1,st.st_mtime); //recover timestamp
				return true;
			}break;
			case 8:{
				_consoleClear2();
				_consolePrint2("*** disable YSMenu softreset ***\n");
				FILE *f=fopen(file,"r+b");
				fseek(f,filelength(fileno(f))-8,SEEK_SET);
				fwrite("\0\0\x0d\0NMSY",1,8,f);
				fclose(f);
				libprism_utime(file,1,st.st_mtime); //recover timestamp
				return true;
			}break;
		}
		return false; //unreacahble
	}
	return false;
}
