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
	"Enable YSMenu softreset",
	"Disable YSMenu softreset",
};

bool savConvert(char *file){
	char to[768];
	struct stat st,stto;
	FILE *in,*out;
	u32 size=0,read=0,cur=0;

	strcpy(to,file);
	if(!strcasecmp(to+strlen(to)-4,".duc")){
		strcpy(to+strlen(to)-4,".sav");
		_consolePrintf2("*** convert duc to sav ***\n");
		if(!stat(to,&stto)){
			_consolePrintf2("%s already exists (%s)\n",to,(stat(file,&st),stto.st_mtime>st.st_mtime)?"NEWER":"older");
			_consolePrintf2("A to overwrite, other to abort.\n");
			int key;
			for(;!(key=IPCZ->keysdown);)swiWaitForVBlank();
			if(!(key&KEY_A)){_consolePrintf2("Aborted.\n");return false;}
		}
		in=fopen(file,"rb");
		if(!in){_consolePrintf2("cannot open %s\n",file);return false;}
		out=fopen(to,"wb");
		if(!out){fclose(in);_consolePrintf2("cannot open %s\n",file);return false;}
		size=filelength(fileno(in));
		u32 pad=size&0xfff;
		size-=pad;
		fseek(in,pad,SEEK_SET);
		for(;(read=fread(libprism_buf,1,65536,in))>0;){
			cur+=read;
			fwrite(libprism_buf,1,read,out);
			_consolePrintf2("Converting %8d / %8d\r",cur,size); //99MB is enough I think...
		}
		_consolePrint2("                              \r");
		fclose(out);
		fclose(in);
		return true;
	}
	if(!strcasecmp(to+strlen(to)-4,".dsv")){
		strcpy(to+strlen(to)-4,".sav");
		_consolePrintf2("*** convert dsv to sav ***\n");
		if(!stat(to,&stto)){
			_consolePrintf2("%s already exists (%s)\n",to,(stat(file,&st),stto.st_mtime>st.st_mtime)?"NEWER":"older");
			_consolePrintf2("A to overwrite, other to abort.\n");
			int key;
			for(;!(key=IPCZ->keysdown);)swiWaitForVBlank();
			if(!(key&KEY_A)){_consolePrintf2("Aborted.\n");return false;}
		}
		in=fopen(file,"rb");
		if(!in){_consolePrintf2("cannot open %s\n",file);return false;}
		out=fopen(to,"wb");
		if(!out){fclose(in);_consolePrintf2("cannot open %s\n",file);return false;}
		size=filelength(fileno(in));
		u32 pad=size&0xfff;
		size-=pad;
		for(;(read=fread(libprism_buf,1,65536,in))>0;){
			if(read&pad)break;
			cur+=read;
			fwrite(libprism_buf,1,read,out);
			_consolePrintf2("Converting %8d / %8d\r",cur,size); //99MB is enough I think...
		}
		_consolePrint2("                              \r");
		fclose(out);
		fclose(in);
		return true;
	}
	if(!strcasecmp(to+strlen(to)-4,".gds")){
		strcpy(to+strlen(to)-4,".sav");
		_consolePrintf2("*** convert gds to sav ***\n");
		if(!stat(to,&stto)){
			_consolePrintf2("%s already exists (%s)\n",to,(stat(file,&st),stto.st_mtime>st.st_mtime)?"NEWER":"older");
			_consolePrintf2("A to overwrite, other to abort.\n");
			int key;
			for(;!(key=IPCZ->keysdown);)swiWaitForVBlank();
			if(!(key&KEY_A)){_consolePrintf2("Aborted.\n");return false;}
		}
		int _in=open(file,O_RDONLY);
		if(_in<0){_consolePrintf2("cannot open %s\n",file);return false;}
		lseek(_in,0x100,SEEK_SET);
		gzFile gz=gzdopen(_in,"rb");
		if(!gz){close(_in),_consolePrintf2("cannot get gzip handle to decompress\n",file);return false;}
		out=fopen(to,"wb");
		if(!out){gzclose(gz);_consolePrintf2("cannot open %s\n",file);return false;}

		u32 read,cur=0;
		for(;(read=gzread(gz,libprism_buf,65536))>0;){
			cur+=read;
			fwrite(libprism_buf,1,read,out);
			_consolePrintf2("Converting %8d / %8d\r",lseek(_in,0,SEEK_CUR)-0x100,filelength(_in)-0x100); //99MB is enough I think...
		}
		_consolePrint2("                              \r");
		fclose(out);
		gzclose(gz);
		return true;
	}
	if(!strcasecmp(to+strlen(to)-4,".bak")){
		strcpy(to+strlen(to)-4,".sav");
		_consolePrintf2("*** restore bak to sav ***\n");
		stat(file,&st);
		if(!stat(to,&stto)){
			_consolePrintf2("%s already exists (%s)\n",to,stto.st_mtime>st.st_mtime?"NEWER":"older");
			_consolePrintf2("A to overwrite, other to abort.\n");
			int key;
			for(;!(key=IPCZ->keysdown);)swiWaitForVBlank();
			if(!(key&KEY_A)){_consolePrintf2("Aborted.\n");return false;}
		}
		bool ret = !copy(file,to);
		if(ret)libprism_utime(to,st.st_atime,st.st_mtime);
		return ret;
	}
	if(!strcasecmp(to+strlen(to)-4,".sav")){
		int ret=selectpref("Sav convertion menu",arraysize(savmenu),savmenu);
		switch(ret){
			case -1:{_consolePrintf2("\n\nAborted.\n");return false;}
			case 0:{
				_consoleClear2();
				strcpy(to+strlen(to)-4,".bak");
				_consolePrintf2("*** backup sav to bak ***\n");
				stat(file,&st);
				if(!stat(to,&stto)){
					_consolePrintf2("%s already exists (%s)\n",to,stto.st_mtime>st.st_mtime?"NEWER":"older");
					_consolePrintf2("A to overwrite, other to abort.\n");
					int key;
					for(;!(key=IPCZ->keysdown);)swiWaitForVBlank();
					if(!(key&KEY_A)){_consolePrintf2("Aborted.\n");return false;}
				}
				bool ret = !copy(file,to);
				if(ret)libprism_utime(to,st.st_atime,st.st_mtime);
				return ret;
			}break;
			case 1:{
				_consoleClear2();
				strcpy(to+strlen(to)-4,".duc");
				_consolePrintf2("*** convert sav to duc (ARDS MAX) ***\n");
				if(!stat(to,&stto)){
					_consolePrintf2("%s already exists (%s)\n",to,(stat(file,&st),stto.st_mtime>st.st_mtime)?"NEWER":"older");
					_consolePrintf2("A to overwrite, other to abort.\n");
					int key;
					for(;!(key=IPCZ->keysdown);)swiWaitForVBlank();
					if(!(key&KEY_A)){_consolePrintf2("Aborted.\n");return false;}
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
				_FAT_directory_mbstoucs2(lgamename,gamename,31);
				_FAT_directory_mbstoucs2(lsavename,savename,31);
				_FAT_directory_mbstoucs2(lsavedesc,savedesc,100);

				memcpy(header,"ARDS000000000001",16);
				header[78]=0x2e;
				header[372]=0x0d;
				fwrite(header,1,sizeof(header),out);

				u32 size=filelength(fileno(in)),cur=0;
				for(;(read=fread(libprism_buf,1,65536,in))>0;){
					cur+=read;
					fwrite(libprism_buf,1,read,out);
					_consolePrintf2("Converting %8d / %8d\r",cur,size); //99MB is enough I think...
				}
				_consolePrint2("                              \r");
				fclose(out);
				fclose(in);
				return true;
			}break;
			case 2:{
				_consoleClear2();
				strcpy(to+strlen(to)-4,".duc");
				_consolePrintf2("*** convert sav to duc (ARDS ME) ***\n");
				if(!stat(to,&stto)){
					_consolePrintf2("%s already exists (%s)\n",to,(stat(file,&st),stto.st_mtime>st.st_mtime)?"NEWER":"older");
					_consolePrintf2("A to overwrite, other to abort.\n");
					int key;
					for(;!(key=IPCZ->keysdown);)swiWaitForVBlank();
					if(!(key&KEY_A)){_consolePrintf2("Aborted.\n");return false;}
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
				for(;(read=fread(libprism_buf,1,65536,in))>0;){
					cur+=read;
					fwrite(libprism_buf,1,read,out);
					_consolePrintf2("Converting %8d / %8d\r",cur,size); //99MB is enough I think...
				}
				_consolePrint2("                              \r");
				fclose(out);
				fclose(in);
				return true;
			}break;
			case 3:{
				_consoleClear2();
				strcpy(to+strlen(to)-4,".dsv");
				_consolePrintf2("*** convert sav to dsv ***\n");
				if(!stat(to,&stto)){
					_consolePrintf2("%s already exists (%s)\n",to,(stat(file,&st),stto.st_mtime>st.st_mtime)?"NEWER":"older");
					_consolePrintf2("A to overwrite, other to abort.\n");
					int key;
					for(;!(key=IPCZ->keysdown);)swiWaitForVBlank();
					if(!(key&KEY_A)){_consolePrintf2("Aborted.\n");return false;}
				}
				FILE *in=fopen(file,"rb");
				if(!in){_consolePrintf2("cannot open %s\n",file);return false;}
				FILE *out=fopen(to,"wb");
				if(!out){fclose(in);_consolePrintf2("cannot open %s\n",file);return false;}
				u32 size=filelength(fileno(in)),cur=0;
				for(;(read=fread(libprism_buf,1,65536,in))>0;){
					cur+=read;
					fwrite(libprism_buf,1,read,out);
					_consolePrintf2("Converting %8d / %8d\r",cur,size); //99MB is enough I think...
				}
				_consolePrint2("                              \r");
				fwrite(dsvfooter,1,122,out);
				fclose(out);
				fclose(in);
				return true;
			}break;
			case 4:{
				_consoleClear2();
				strcpy(to+strlen(to)-4,".gds");
				_consolePrintf2("*** convert sav to gds (gameshark) ***\n");
				if(!stat(to,&stto)){
					_consolePrintf2("%s already exists (%s)\n",to,(stat(file,&st),stto.st_mtime>st.st_mtime)?"NEWER":"older");
					_consolePrintf2("A to overwrite, other to abort.\n");
					int key;
					for(;!(key=IPCZ->keysdown);)swiWaitForVBlank();
					if(!(key&KEY_A)){_consolePrintf2("Aborted.\n");return false;}
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
				if(!gz){fclose(in);close(_out);_consolePrintf2("cannot alloc gzip handle\n");return false;}
				//gzsetparams(gz,9,0);
				u32 size=filelength(fileno(in)),cur=0;
				for(;(read=fread(libprism_buf,1,65536,in))>0;){
					cur+=read;
					gzwrite(gz,libprism_buf,read);
					_consolePrintf2("Converting %8d / %8d\r",cur,size); //99MB is enough I think...
				}
				_consolePrint2("                              \r");
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
				_consolePrintf2("*** enable YSMenu softreset ***\n");
				FILE *f=fopen(file,"r+b");
				libprism_futime(fileno(f),1,1); //no timestamp change
				fseek(f,filelength(fileno(f))-8,SEEK_SET);
				fwrite("\0\0\x0f\0NMSY",1,8,f);
				fclose(f);
				return true;
			}break;
			case 6:{
				_consoleClear2();
				_consolePrintf2("*** disable YSMenu softreset ***\n");
				FILE *f=fopen(file,"r+b");
				libprism_futime(fileno(f),1,1); //no timestamp change
				fseek(f,filelength(fileno(f))-8,SEEK_SET);
				fwrite("\0\0\x0d\0NMSY",1,8,f);
				fclose(f);
				return true;
			}break;
		}
		return false; //unreacahble
	}
	return false;
}
