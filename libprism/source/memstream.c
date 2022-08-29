#include "libprism.h"

memstream *mopen(void *p, u32 size, memstream *s){
	memstream *m=s?s:(memstream*)malloc(sizeof(memstream));
	if(!m)return NULL;
	m->p=(u8*)p;
	m->current=0;
	m->size=size;
	return m;
}

int mclose(memstream *s){ //do not call mclose if you call mopen using existing memstream.
	if(!s)return EOF;
	free(s);
	return 0;
}

int mgetc(memstream *s){
	if(!s||s->current==s->size)return EOF;
	return s->p[s->current++];
}

int mputc(int c, memstream *s){
	if(!s||s->current==s->size)return EOF;
	s->p[s->current++]=c&0xff;
	return c&0xff;
}

int mrewind(memstream *s){
	if(!s)return EOF;
	s->current=0;
	return 0;
}

int mavail(memstream *s){
	if(!s)return EOF;
	return s->size-s->current;
}

int mtell(memstream *s){
	if(!s)return EOF;
	return s->current;
}

int mlength(memstream *s){
	if(!s)return EOF;
	return s->size;
}

int mread(memstream *s, void *buf, u32 size){
	int i=0;
	u8 *p=(u8*)buf;
	if(!s||!p)return EOF;
	for(;i<size&&s->current<s->size;)p[i++]=s->p[s->current++];
	return i;
}

int mwrite(memstream *s, void *buf, u32 size){
	int i=0;
	u8 *p=(u8*)buf;
	if(!s||!p)return EOF;
	for(;i<size&&s->current<s->size;)s->p[s->current++]=p[i++];
	return i;
}

int mseek(memstream *s, int offset, int whence){
	if(!s)return EOF;
	switch(whence){
		case SEEK_SET:
			if(offset<0||s->size<offset)return EOF;
			s->current=offset;
			return s->current;
		case SEEK_CUR:
			if(s->current+offset<0||s->size<s->current+offset)return EOF;
			s->current+=offset;
			return s->current;
		case SEEK_END:
			if(s->size+offset<0||s->size<s->size+offset)return EOF;
			s->current=s->size+offset;
			return s->current;
	}
	return EOF;
}

int mread32(memstream *s){
	if(!s||s->size-s->current<4)return 0;
	const unsigned char *x=(const unsigned char*)s->p+s->current;
	s->current+=4;
	return x[0]|(x[1]<<8)|(x[2]<<16)|(x[3]<<24);
}

int mread16(memstream *s){
	if(!s||s->size-s->current<2)return 0;
	const unsigned char *x=(const unsigned char*)s->p+s->current;
	s->current+=2;
	return x[0]|(x[1]<<8);
}

int mwrite32(const unsigned int n, memstream *s){
	if(!s||s->size-s->current<4)return EOF;
	unsigned char *x=(unsigned char*)s->p+s->current;
	s->current+=4;
	x[0]=n&0xff,x[1]=(n>>8)&0xff,x[2]=(n>>16)&0xff,x[3]=(n>>24)&0xff;
	return 0;
}

int mwrite16(const unsigned int n, memstream *s){
	if(!s||s->size-s->current<2)return EOF;
	unsigned char *x=(unsigned char*)s->p+s->current;
	s->current+=2;
	x[0]=n&0xff,x[1]=(n>>8)&0xff;
	return 0;
}
