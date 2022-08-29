// ndslink
// only for gcc.
#include <stdio.h>
#include <ctype.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#endif

int link(unsigned char *p, const int offset, const int size, const char *target, const char *link){
	unsigned banneroffset;
	FILE *f=fopen(target+1,"rb");
	unsigned char head[0x200],z[256*3];
	if(!f)return -1;

	fread(head,1,0x160,f);
	banneroffset=(head[0x06b]<<24)+(head[0x06a]<<16)+(head[0x069]<<8)+head[0x068];
	if(banneroffset){
		unsigned char *banner=p+((p[0x06b]<<24)+(p[0x06a]<<16)+(p[0x069]<<8)+p[0x068]);
		fseek(f,banneroffset,SEEK_SET);
		fread(banner,1,2112,f);
		if(banner[0]!=1)banner[0]=1; //for moonshell2
		if(banner[1]!=0)banner[1]=0; //for moonshell2
	}
	fclose(f);

	f=fopen(link,"wb");
	if(!f)return -1;
	memset(p+offset,0,256*3);
#ifdef _WIN32
	{
		wchar_t x[256];
		MultiByteToWideChar(CP_ACP, 0, target, -1, x, 256);
		WideCharToMultiByte(CP_UTF8, 0, x, -1, p+offset, 768, NULL, NULL);
	}
#else
	strcpy(p+offset,target);
#endif
	fwrite(p,1,size,f);
	fclose(f);
	return 0;
}

#define Between(n1, x, n2) (((n1)<=(x))&&((x)<=(n2)))
#define jms1(c) (Between(0x81,(unsigned char)(c),0x9f)||Between(0xe0,(unsigned char)(c),0xfc))
#define jms2(c) (((unsigned char)(c)!=0x7F)&&Between(0x40,(unsigned char)(c),0xFC))
#define isJMS(p,i) ((*(p)==0)?0:(jms1(*(p))&&jms2(*(p+1))&&(i==0||i==2))?1:(i==1)?2:0)

int makedir(const char* dir){
	char name[512];
	int l;
	int ret=0,i=0,ascii=0;//ret should be -1 if using CreateDirectory

	if(!dir)return -1;
	l=strlen(dir);

	memset(name,0,512);
	for(;i<l;i++){
		ascii=isJMS(dir+i,ascii);
		memcpy(name,dir,i+1);
		if((dir[i]=='\\'||dir[i]=='/')&&!ascii)
#ifdef _WIN32
			ret=mkdir(name); //!CreateDirectory(name,NULL);
#else
			ret=mkdir(name,0755);
#endif
	}
	return ret;
}

int recursive(
	unsigned char *p, const int offset, const int size,
	char *targetd, char *targetf,
	char *linkd, char *linkf){
	DIR *d=opendir(targetd+1);
	struct dirent *ent;
	if(!d)return 1;

	while(ent=readdir(d)){
		strcpy(targetf,ent->d_name);
		strcpy(linkf,ent->d_name);
		if(!strcmp(targetf,".")||!strcmp(targetf,".."))continue;
#ifdef _WIN32
		if(d->dd_dta.attrib&_A_SUBDIR){
#else
		if(ent->d_type&DT_DIR){
#endif
			strcat(targetf,"/");
			strcat(linkf,"/");
			recursive(p,offset,size,targetd,targetd+strlen(targetd),linkd,linkd+strlen(linkd));
		}else{
			if(strlen(targetf)<4||strcmp(targetf+strlen(targetf)-4,".nds"))continue;
			makedir(linkd);
			printf("Linking %s to %s... %s\n",targetd+1,linkd,link(p,offset,size,targetd,linkd)?"Failed":"Done");
		}
	}
	closedir(d);
	return 0;
}

int main(int argc, char **argv){
	FILE *f;
	unsigned char *p;
	int s;
	unsigned int offset;

	char targetd[768],linkd[768];

	if(argc<4){puts("ndslink template /target-dir/ link-dir/");return -1;}
	strcpy(targetd,argv[2]);
	strcpy(linkd,argv[3]);

	f=fopen(argv[1],"rb");
	if(!f){puts("cannot open template");return -1;}
	{struct stat st;fstat(fileno(f),&st);s=st.st_size;}
	if(s<0x200){fclose(f);puts("template too small");return -1;}
	p=malloc(s);
	fread(p,1,s,f);
	fclose(f);

	if(strcmp(p+0x1e0,"mshl2wrap link")){free(p);puts("template not mshl2wrap link");return 1;}
	offset=(p[0x1f0]<<24)+(p[0x1f1]<<16)+(p[0x1f2]<<8)+p[0x1f3];
	if(s<offset+256*3){fclose(f);puts("template too small or offset invalid");return -1;}

	recursive(p,offset,s,targetd,targetd+strlen(targetd),linkd,linkd+strlen(linkd));
	free(p);
	return 0;
}
