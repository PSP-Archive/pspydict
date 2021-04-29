// primitive graphics for Hello World PSP
#ifndef PG_H
#define PG_H

void pgInit();
void pgWaitV();
void pgWaitVn(unsigned long count);
void pgScreenFrame(long mode,long frame);
void pgScreenFlip();
void pgScreenFlipV();

void pgPrint(unsigned long x,unsigned long y,unsigned long color,const char *str);

void pgPrint2(unsigned long x,unsigned long y,unsigned long color,const char *str);
void pgPrint4(unsigned long x,unsigned long y,unsigned long color,const char *str);

void pgFillvram(unsigned long color);
void pgBitBlt(unsigned long x,unsigned long y,unsigned long w,unsigned long h,unsigned long mag,const unsigned short *d);
void pgPutChar(unsigned long x,unsigned long y,unsigned long color,unsigned long bgcolor,unsigned char ch,char drawfg,char drawbg,char mag);
unsigned short rgb2col(unsigned char r,unsigned char g,unsigned char b);

#define		PIXELSIZE	1				//in short
#define		LINESIZE	512				//in short
#define		FRAMESIZE	0x44000			//in byte

//constants

//480*272 = 60*38
#define CMAX_X (480/8)
#define CMAX_Y (272/8)
#define CMAX2_X (480/16)
#define CMAX2_Y (272/16)
#define CMAX4_X (480/32)
#define CMAX4_Y (272/32)


#endif

