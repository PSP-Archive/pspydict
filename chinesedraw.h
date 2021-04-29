#ifndef CHINESEDRAW_H
#define CHINESEDRAW_H
/*
  月光下的床
  2005.05.11
  
  中文显示
*/



void chPutHz(unsigned long x,unsigned long y,unsigned long color,unsigned long bgcolor,unsigned char *str,char drawfg,char drawbg,char mag);
void chPutChar(unsigned long x,unsigned long y,unsigned long color,unsigned long bgcolor,unsigned char ch,char drawfg,char drawbg,char mag);
void chDrawString(unsigned long x,unsigned long y,unsigned long color,unsigned char* str,char mag,char re);
void chDrawStringEx(unsigned long x,unsigned long y,unsigned long color,unsigned char* str,char mag,char re,char inter);
void chDrawRec(unsigned long x,unsigned long y,unsigned long width,unsigned long height, unsigned long color,char mag);
void chDrawRecLine(unsigned long x,unsigned long y,unsigned long width,unsigned long height, unsigned long color);
#endif
