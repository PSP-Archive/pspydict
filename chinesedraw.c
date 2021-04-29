/*
  月光下的床
  2005.05.11
  
  中文显示
*/

#include "main.h"

#include "hz16.c" 

#define HZ_SIZE 16
#define ASC_SIZE 8

/*
	显示一个汉字
*/
void chPutHz(unsigned long x,unsigned long y,unsigned long color,unsigned long bgcolor,unsigned char *str,char drawfg,char drawbg,char mag)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned char *vptr;		//pointer to vram
	const unsigned char *cfont;
	unsigned char ch1,ch2;		//pointer to font
	unsigned long cx,cy;
	unsigned char b;
	char mx,my;
	
	
	unsigned short	quIndex;
	unsigned short	weiIndex;
		
	
    quIndex = str[0];
    weiIndex = str[1];
	
	cfont = hazfont+(unsigned long)(((quIndex-0x81)*(0xFF-0x40)+(weiIndex-0x40))*32);
	vptr0=(unsigned char*)pgGetVramAddr(x,y);	
	for (cy=0; cy<16; cy++) {
		ch1 =*cfont;
		cfont++;
		ch2 =*cfont;
		for (my=0; my<mag; my++) {
			vptr=vptr0;
			b=0x80;
			for (cx=0; cx<8; cx++) {
				for (mx=0; mx<mag; mx++) {
					if ((ch1&b)!=0) {
						if (drawfg) *(unsigned short *)vptr=color;
					} else {
						if (drawbg) *(unsigned short *)vptr=bgcolor;
					}
					vptr+=PIXELSIZE*2;
				}
				b=b>>1;
			}			
			b=0x80;
			for (cx=0; cx<8; cx++) {
				for (mx=0; mx<mag; mx++) {
					if ((ch2&b)!=0) {
						if (drawfg) *(unsigned short *)vptr=color;
					} else {
						if (drawbg) *(unsigned short *)vptr=bgcolor;
					}
					vptr+=PIXELSIZE*2;
				}
				b=b>>1;
			}
			vptr0+=LINESIZE*2;
		}
		cfont++;
	}
}

/*

  显示一个音文字母

*/


void chPutChar(unsigned long x,unsigned long y,unsigned long color,unsigned long bgcolor,unsigned char ch,char drawfg,char drawbg,char mag)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned char *vptr;		//pointer to vram
	const unsigned char *cfont;
	unsigned char ch1;		//pointer to font
	unsigned long cx,cy;
	unsigned char b;
	char mx,my;
	
	cfont = ascfont+ch*16;
	vptr0=(unsigned char*)pgGetVramAddr(x,y);	
	for (cy=0; cy<16; cy++) {
		ch1 =*cfont;		
		for (my=0; my<mag; my++) {
			vptr=vptr0;
			b=0x80;
			for (cx=0; cx<8; cx++) {
				for (mx=0; mx<mag; mx++) {
					if ((ch1&b)==0) {
						if (drawfg) *(unsigned short *)vptr=color;
					} else {
						if (drawbg) *(unsigned short *)vptr=bgcolor;
					}
					vptr+=PIXELSIZE*2;
				}
				b=b>>1;
			}
			vptr0+=LINESIZE*2;
		}
		cfont++;
	}
}
void chDrawRecLine(unsigned long x,unsigned long y,unsigned long width,unsigned long height, unsigned long color)
{
	chDrawRec(x,y,width,1, color,1);
	chDrawRec(x,y,1,height, color,1);	
	chDrawRec(x,y+height-1,width,1, color,1);
	chDrawRec(x+width-1,y,1,height, color,1);
}
void chDrawRec(unsigned long x,unsigned long y,unsigned long width,unsigned long height, unsigned long color,char mag)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned char *vptr;		//pointer to vram
	unsigned long cy,cx;
	char mx,my;
	
	vptr0=(unsigned char*)pgGetVramAddr(x,y);
	for (cy=0; cy<height; cy++)
	{
		for (my=0; my<mag; my++)
		{
			//if((y+cy)>=SCREEN_HEIGHT)
				//break;
			vptr=vptr0;
			for (cx=0; cx<width; cx++)
			{
				for (mx=0; mx<mag; mx++)
				{
					//if((x+cx)>=SCREEN_WIDTH)
						//break;
					*(unsigned short *)vptr=color;
					vptr+=PIXELSIZE*2;
				}
			}
			vptr0+=LINESIZE*2;
		}
	}
}
void chDrawString(unsigned long x,unsigned long y,unsigned long color,unsigned char* str,char mag,char re)
{
	unsigned short	quIndex;
	unsigned short	weiIndex;
	unsigned char* pr;
	unsigned long cx,cy;
	
	cx=x;cy=y;
	pr = str;
	while(pr[0]!=0 && x<SCREEN_WIDTH && y<SCREEN_HEIGHT)
	{    		
    	if(pr[0]>=0x81 && pr[0]>=0x81)
    	{
		///// avoid half HZ in line end, fixed by zym
		if ((cx>=(SCREEN_WIDTH-HZ_SIZE/2-2)) &&(re))
			return;
		/////
    		chPutHz(cx,cy,color,0,pr,1,0,mag);
    		cx+=HZ_SIZE*mag;
    		if(cx>=SCREEN_WIDTH)
    		{
    			if(re)
    				return;
    			cx=0;
    			cy+=HZ_SIZE*mag;
    		}
    		pr+=2;
    		
    	}
    	else if(pr[0]<0x81)
    	{
    		chPutChar(cx,cy,color,0,pr[0],1,0,mag);
    		cx+=ASC_SIZE*mag;
    		if(cx>=SCREEN_WIDTH)
    		{
    			if(re)
    				return;
    			cx=0;
    			cy+=HZ_SIZE*mag;
    		}
    		pr++;
    	}
    	else
    	{
    		pr++;
    	}
	}
}

void chDrawStringEx(unsigned long x,unsigned long y,unsigned long color,unsigned char* str,char mag,char re,char inter)
{
	unsigned short	quIndex;
	unsigned short	weiIndex;
	unsigned char* pr;
	unsigned long cx,cy;
	
	cx=x;cy=y;
	pr = str;
	while(pr[0]!=0 && x<SCREEN_WIDTH && y<SCREEN_HEIGHT)
	{    		
    	if(pr[0]>=0x81 && pr[0]>=0x81)
    	{
    		chPutHz(cx,cy,color,0,pr,1,0,mag);
    		cx+=HZ_SIZE*mag+2*inter;
    		if(cx>=SCREEN_WIDTH)
    		{
    			if(re)
    				return;
    			cx=0;
    			cy+=HZ_SIZE*mag;
    		}
    		pr+=2;
    		
    	}
    	else if(pr[0]<0x81)
    	{
    		chPutChar(cx,cy,color,0,pr[0],1,0,mag);
    		cx+=ASC_SIZE*mag+inter;
    		if(cx>=SCREEN_WIDTH)
    		{
    			if(re)
    				return;
    			cx=0;
    			cy+=HZ_SIZE*mag;
    		}
    		pr++;
    	}
    	else
    	{
    		pr++;
    	}
	}
}

