#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

int main(int argc, char **argv){
  FILE *f;
  int i,siz;
  struct stat st;
  unsigned char *nds;
  if(argc<2){puts("linkmaker NDS-to-be-linkfile");return 1;}
  if(!(f=fopen(argv[1],"rb+"))){puts("cannot open nds");return 1;}
  fstat(fileno(f),&st);siz=st.st_size;
  if(siz<0x200-20){fclose(f);puts("nds too small");return 1;}
  if(siz>2*1024*1024){fclose(f);puts("link nds cannot be >=2MB");return 1;}
  nds=malloc(siz);
  fread(nds,1,siz,f);
  for(i=0x200;i<siz-20;i+=4)
    if(!memcmp(nds+i,"MoonShellExecute\0\0\0\0",20))goto done;
  fclose(f);puts("not link nds");return 1;
  done:
  strcpy(nds+0x1e0,"reset_mse DLDI");
  nds[0x1f0]=(i>>24)&0xff,nds[0x1f1]=(i>>16)&0xff,nds[0x1f2]=(i>>8)&0xff,nds[0x1f3]=i&0xff;
  fseek(f,0x1e0,SEEK_SET);fwrite(nds+0x1e0,1,32,f);
  fclose(f);puts("link made successfully");return 0;
}
