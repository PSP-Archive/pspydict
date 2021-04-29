//
//	This program is free software; you can redistribute it and/or
//	modify it under the terms of the GNU General Public License
//
//	Copyright (c) 2005 ZYM <yanming.zhang@gmail.com>
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pspctrl.h>
#include "main.h"
#include "chinesedraw.h"

// we make '_' as ' ' for display
//char kbletters[]="abcdefghijklmnopqrstuvwxyz.-('@_";
char kbletters[]="fghijklmnopqrstuvwxyz.'-_/(abcde";
char kbnumbers[]="87654321!@#$%^&*()_+{}|\\:;][=-09";
//char kbletters2[]="FGHIJKLMNOPQRSTUVWXYZ.'-_@(ABCDE";
char *currkb;
int currkeyindex;

const int KEYCOS[8]={
// cos()*10000, from ...
//	5.625,	16.875,	28.125,	39.375
	9952,	9569,	8819,	7730,
//	50.625,	61.875,	73.125,	84.375
	6344,	4714,	2903,	980};

const int KEYTAN[8]={
// tan()*10000
//	5.625,	16.875,	28.125,	39.375
	985,	3033,	5345,	8207,
//	50.625,	61.875,	73.125,	84.375
        12185,  18709,	32966,	101532};

const int ANALOGTAN[4]={
// tan()*10000
//	11.25,	22.5,	33.75,	45
	1989,	4142,	6682,	10000};

struct key_info {
	unsigned int x;
	unsigned int y;
};

struct key_info	kb[KEYNUMBER];
#define INPUTBAR_SIZE	35
struct inputbar {
	char string[INPUTBAR_SIZE];
	int cursor;
};
struct inputbar	myinput;

int getcurrentkeyindex(unsigned char x, unsigned char y)
{
	unsigned int deltax, deltay;
	int i;
	
	deltax = abs(MAXANALOG_X/2 -x);
	deltay = abs(MAXANALOG_Y/2 -y);
	if ((x<MAXANALOG_X/2) &&(y<MAXANALOG_Y/2)) {
		//kbletter 0..7
		if (deltax>=deltay) {
			//kbletter 0..3
			for (i=0;i<4;i++) {
				if (deltay*10000/deltax <=ANALOGTAN[i])
					return i;
			}
		}
		else {
			// kbletter 4..7
			for (i=0;i<4;i++) {
				if (deltax*10000/deltay<=ANALOGTAN[i])
					return (7-i);
			}
		}
	}
	else if ((x>MAXANALOG_X/2)&&(y<MAXANALOG_Y/2)) {
		// kbletter 8..15
		if (deltax<=deltay) {
			// kbletter 8..11
			for (i=0;i<4;i++) {
				if (deltax*10000/deltay<=ANALOGTAN[i])
					return (8+i);
			}
		}
		else {
			// kbletter 12..15
			for (i=0;i<4;i++) {
				if (deltay*10000/deltax<=ANALOGTAN[i])
					return (15-i);
			}
		}
	}
	else if ((x>MAXANALOG_X/2)&&(y>MAXANALOG_Y/2)) {
		// kbletter 16..23
		if (deltax>=deltay) {
			// kbletter 16.. 19
			for (i=0;i<4;i++){
				if (deltay*10000/deltax<=ANALOGTAN[i])
					return (16+i);
			}
		}
		else {
			// kbletter 20..23
			for (i=0;i<4;i++) {
				if (deltax*10000/deltay<=ANALOGTAN[i])
					return (23-i);
			}
		}
	}
	else if ((x<MAXANALOG_X/2)&&(y>MAXANALOG_Y/2)) {
		// kbletter 24..31
		if (deltax<=deltay) {
			// kbletter 24..27
			for (i=0;i<4;i++) {
				if (deltax*10000/deltay<=ANALOGTAN[i])
					return (24+i);
			}
		}
		else {
			// kbletter 28..31
			for (i=0;i<4;i++) {
				if (deltay*10000/deltax<=ANALOGTAN[i])
					return (31-i);
			}
		}
	}
}

void buildkeyxy(void)
{
	int i;
	int deltax1, deltay1, deltax2,deltay2;
	//build letter array
	for (i=0;i<8; i++) {
		deltax1 = (KBRADIUS * KEYCOS[i]+5000) / 10000;
		deltay1 = (deltax1 * KEYTAN[i]+5000) / 10000;
		deltax2 = (KBRADIUS * KEYCOS[7-i]+5000) / 10000;
		deltay2 = (deltax2 * KEYTAN[7-i]+5000)/ 10000;
		kb[i].x = KBCENTRE_X - deltax1;
		kb[i].y = KBCENTRE_Y - deltay1;
		kb[i+8].x = KBCENTRE_X +deltax2;
		kb[i+8].y = KBCENTRE_Y - deltay2;
		kb[i+16].x = KBCENTRE_X + deltax1;
		kb[i+16].y = KBCENTRE_Y + deltay1;
		kb[i+24].x= KBCENTRE_X -deltax2;
		kb[i+24].y= KBCENTRE_Y+deltay2;
	}
}

int getindex(char *inputstr)
{
	int i, mid, first, last;

	first = 0;
	last = wordcount-1;
	while(1) {
		mid=(first+last)/2;
	
		i = strcasecmp(inputstr, *(wordarray+mid));
		if (i==0) {
			break;
		}
		else if (i>0) {
			first=mid;
		}
		else {
			last=mid;
		}
		if ((last-first)<=1){
			//DBG_PRINT("first %s last %s", *(wordarray+first), *(wordarray+last));
			break;
		}
	}
	//DBG_PRINT(" %s/ %s/ %s", *(wordarray+first),*(wordarray+mid),*(wordarray+last));
	if ((mid>0) && (strcasecmp(inputstr, *(wordarray+mid-1))==0))
		return (mid-1);
	if (strcasecmp(inputstr, *(wordarray+mid))>0)
		return last;
	return mid;
}

void disp_ringkey_page(void);

void ydict_ringkey(int laststate)
{
	unsigned char x, y;
	unsigned int button;
	struct ydictctrl yctrl;

	buildkeyxy();

	memset(&myinput, 0, sizeof(struct inputbar));
	updatelist(0);
	curword = -1;
	currkb = &kbletters;

	while (1) {
		disp_ringkey_page();
		
		getcontrol(&yctrl);
		x = yctrl.x;
		y = yctrl.y;
		button = yctrl.buttons;
		if ((abs(MAXANALOG_X/2 -x)+abs(MAXANALOG_Y/2 -y)) >ANALOG_IGNORE) {
			// maybe should use (dx*dx + dy*dy) >ignore*ignore
			currkeyindex= getcurrentkeyindex(x, y);
			if (gstate == STATE_VIEWWORD) {
				memset(&myinput, 0, sizeof(struct inputbar));
				gstate = STATE_RING_KEY;
			}
		}
		else {
			// no analog input
			currkeyindex= -1;
		}
		switch (button) {
		case PSP_CTRL_SELECT:
			// 
			gstate = STATE_SWITCHMODE;
			return;
		case PSP_CTRL_START:
			memset(&myinput, 0, sizeof(struct inputbar));
			updatelist(0);
			curword = -1;
			break;
		case PSP_CTRL_CIRCLE:
			// input the currkey
			if ((myinput.cursor<INPUTBAR_SIZE )&&(currkeyindex!=-1)) {
				if ((currkb==&kbletters) && (*(currkb+currkeyindex)=='_'))
					myinput.string[myinput.cursor] = ' ';
				else
					myinput.string[myinput.cursor] = *(currkb+currkeyindex);
				myinput.cursor++;
				updatelist(getindex(myinput.string));
				curword =-1;
				gstate = STATE_RING_KEY;
			}
			break;
		case PSP_CTRL_SQUARE:
			// erase last key
			if (myinput.cursor>0) {
				myinput.cursor--;
				myinput.string[myinput.cursor]=0;
				updatelist(getindex(myinput.string));
				curword =-1;
				gstate = STATE_RING_KEY;
			}
			break;
		case PSP_CTRL_UP:
			// previous word
			if (curword == -1)
				curword = WORDLIST_SIZE -1;
			else if (curword == 0){
				int i;

				if (wordlist[curword].index >0) {
					i = wordlist[curword].index -1;
					updatelist(i);
					//updateidxword(i, &currange.mid);
					//update currange first
					//if (currange.first.index >= currange.mid.index)
					//	updateidxword(currange.mid.index, &currange.first);
				}
			}
			else {
				curword +=(WORDLIST_SIZE -1);
				curword %= WORDLIST_SIZE;
			}
			gstate = STATE_RING_KEY;
			break;
		case PSP_CTRL_DOWN:
			// next word
			if (curword == -1)
				curword = 0;
			else if (curword == (WORDLIST_SIZE-1)) {
				int i;

				if (wordlist[curword].index <(wordcount-1)) {
					i = wordlist[curword].index +1;
					updatelist(i-WORDLIST_SIZE+1);
					//updateidxword(i, &currange.mid);
					//update currange last
					//if (currange.last.index <= currange.mid.index)
					//	updateidxword(currange.mid.index, &currange.last);
				}
			}
			else {
				curword++;
				curword %= WORDLIST_SIZE;
			}
			gstate = STATE_RING_KEY;
			break;
		case PSP_CTRL_LEFT:
			//previous list
			if (wordlist[0].index <WORDLIST_SIZE)
				updatelist(0);
			else
				updatelist(wordlist[0].index - WORDLIST_SIZE);
			// update currange mid
			//updateidxword(wordlist[0].index, &currange.mid);
			//if (currange.first.index >= currange.mid.index)
			//	updateidxword(currange.mid.index, &currange.first);
			curword = -1;					
			gstate = STATE_RING_KEY;
			break;
		case PSP_CTRL_RIGHT:
			//next list
			if ((wordlist[0].index + WORDLIST_SIZE) > (wordcount - WORDLIST_SIZE))
				updatelist(wordcount - WORDLIST_SIZE);
			else
				updatelist(wordlist[0].index + WORDLIST_SIZE);
			// update currange mid
			//updateidxword(wordlist[0].index, &currange.mid);
			//update currange last
			//if (currange.last.index <= currange.mid.index)
			//	updateidxword(currange.mid.index, &currange.last);
			curword = -1;					
			gstate = STATE_RING_KEY;
			break;
		case PSP_CTRL_TRIANGLE:
			// pop range stack, or previous page in dict mode
			{
				if (gstate == STATE_VIEWWORD) {
					if (curpage>0)
						curpage--;
					else {
						// now in first page 
						gstate = STATE_RING_KEY;
					}
				}
				else {
					// change ring keyboard
					if (currkb == &kbletters)
						currkb = &kbnumbers;
					else
						currkb = &kbletters;
				}
			}
			break;
		case PSP_CTRL_CROSS:
			// view word dict, or next page in dict mode
			if (gstate == STATE_RING_KEY) {
				if (curword == -1)
					curword = 0;
				build_dictraw(&wordlist[curword]);
				curwordrows= (strlen(*(wordarray+wordlist[curword].index))+C_LEN)/C_LEN;
				dictpages = (curwordrows+dictrows+R_LEN)/R_LEN;
				gstate = STATE_VIEWWORD;
				curpage = 0;
			}
			else {
				if ((curpage+1)<dictpages)
					curpage++;
				else
					gstate=STATE_RING_KEY;
			}
			break;
		case PSP_CTRL_LTRIGGER:
		case PSP_CTRL_RTRIGGER:
			break;
		default:
			break;
		}
	}
}

void disp_ringkey_page(void)
{
	int i, len;
	static int cursor;

	pgWaitV();
	pgFillvram(BG_COLOR);

	// draw the input string
	chDrawString(0, 0, FG_COLOR, myinput.string, 1, 1);
	// draw a nice flashing cursor
	{
		cursor++;
		if ((cursor/15)%2)
			chDrawString(myinput.cursor * 8, 0, FG_COLOR, "_", 1, 1);
	}

	// draw the dict chinese name
	chDrawString(SCREEN_WIDTH-strlen(ydictconf.dictcname)*8, 0,
		FG_COLOR, ydictconf.dictcname, 1, 1);
	// draw the color hint line
	chDrawRec(0, 17, SCREEN_WIDTH, 3, FG_COLOR, 1);
	chDrawRec(SCREEN_WIDTH * wordlist[0].index /wordcount, 17, 2, 3, RED_COLOR, 1);


	if (gstate != STATE_VIEWWORD) {
		if (currkeyindex==-1) {
			// draw ring keyboard in GRAY_COLOR
			for (i=0;i<KEYNUMBER;i++)
				chPutChar(kb[i].x, kb[i].y-8, GRAY_COLOR, BG_COLOR, *(currkb+i), 1, 0, 1);
			// draw wordlist all in FG_COLOR
			for (i=0;i<WORDLIST_SIZE;i++) {
				if (curword != i)
					chDrawString(0, 16+4 +i*16, FG_COLOR, *(wordarray+wordlist[i].index), 1, 1);
				else
					chDrawString(0, 16+4 +i*16, BLUE_COLOR, *(wordarray+wordlist[i].index), 1, 1);
			}
		}
		else {
			// draw wordlist tail in GRAY_COLOR
			for (i=0;i<WORDLIST_SIZE;i++) {
				if (curword != i)
					chDrawString(0, 16+4 +i*16, FG_COLOR, *(wordarray+wordlist[i].index), 1, 1);
				else
					chDrawString(0, 16+4 +i*16, BLUE_COLOR, *(wordarray+wordlist[i].index), 1, 1);
				if (strlen(*(wordarray+wordlist[i].index))>18)
					chDrawString(18*8, 16+4 +i*16, GRAY_COLOR, *(wordarray+wordlist[i].index)+18, 1, 1);
			}
			// draw ring keyboard in FG_COLOR
			for (i=0;i<KEYNUMBER;i++) {
				if (i==currkeyindex)
					chPutChar(kb[i].x, kb[i].y-8, CURRKEY_COLOR, BG_COLOR, *(currkb+currkeyindex), 1, 0, 1);
				else
					chPutChar(kb[i].x, kb[i].y-8, KEYS_COLOR, BG_COLOR, *(currkb+i), 1, 0, 1);
			}
			// draw the current key in center point
			//chPutChar(KBCENTRE_X, KBCENTRE_Y-8, CURRKEY_COLOR, BG_COLOR, currkey, 1, 0, 1);
		}
	}
	else {
		for (i=0;i<WORDLIST_SIZE;i++) {
			if (curword != i)
				chDrawString(0, 16+4 +i*16, FG_COLOR, wordlist[i].word, 1, 1);
			else
				chDrawString(0, 16+4 +i*16, BLUE_COLOR, wordlist[i].word, 1, 1);
			len = strlen(wordlist[i].word);
			if (len > WORDLIST_WIDTH) {
				// hide the word tail in dict area
				chDrawString(8*WORDLIST_WIDTH, 16+4+i*16, BG_COLOR, wordlist[i].word+WORDLIST_WIDTH, 1, 1);
			}
		}
		// draw the vertical line
		chDrawRec(8*WORDLIST_WIDTH -4, 16+4, 2, WORDLIST_SIZE *16, FG_COLOR, 1);
		chDrawRec(SCREEN_WIDTH-2, 16+4, 2, WORDLIST_SIZE *16, FG_COLOR, 1);
		// hide the char behind the vertical line
		chDrawRec(8*WORDLIST_WIDTH -2, 16+4, 2, WORDLIST_SIZE *16, BG_COLOR, 1);
	}

	// draw dict area in STATE_VIEWWORD
	if (gstate == STATE_VIEWWORD) {
		int i, len, currow;
		unsigned char temp[SCREEN_WIDTH/8];
		unsigned char *rowstr;

		if (curpage==0) {
			// first page, draw curword in RED_COLOR
			memset(temp, 0, SCREEN_WIDTH/8);
			len =strlen(*(wordarray+wordlist[curword].index));
			memcpy(temp, *(wordarray+wordlist[curword].index), (len<(SCREEN_WIDTH/8))?len:(SCREEN_WIDTH/8 -1));
			if (curwordrows == 2) {
				chDrawString(8*WORDLIST_WIDTH -2, 16+4+16, RED_COLOR, &temp[C_LEN], 1, 1);
				temp[C_LEN]=0x0;
				chDrawString(8*WORDLIST_WIDTH -2, 16+4, RED_COLOR, &temp, 1, 1);
			}
			else {
				chDrawString(8*WORDLIST_WIDTH -2, 16+4, RED_COLOR, &temp, 1, 1);
			}

			for (i=0; i<(WORDLIST_SIZE-curwordrows);i++) {
				rowstr = dictfilep+ wordlist[curword].dictoffset + dictrow[i];
				memcpy(temp, rowstr, SCREEN_WIDTH/8);
				if (i<(dictrows-1)) {
					// throw the next row data
					temp[dictrow[i+1]-dictrow[i]]= 0x0;
					chDrawString(8*WORDLIST_WIDTH-2, 16+4+(curwordrows*16)+ i*16, FG_COLOR, temp, 1, 1);
				}
				else if (i==(dictrows-1)) {
					// fix the last row, avoid garbage of next word
					temp[wordlist[curword].dictlen - dictrow[i]] = 0x0;
					chDrawString(8*WORDLIST_WIDTH-2, 16+4+(curwordrows*16)+ i*16, FG_COLOR, temp, 1, 1);
				}
			}
			
		}
		else {
			currow = (curpage*R_LEN-curwordrows);
			for (i=0; i<WORDLIST_SIZE;i++) {
				rowstr = dictfilep+ wordlist[curword].dictoffset + dictrow[currow+i];
				memcpy(temp, rowstr, SCREEN_WIDTH/8);
				if ((currow+i)<(dictrows-1)) {
					// throw the next row data
					temp[dictrow[currow+i+1]-dictrow[currow+i]]=0x0;
					chDrawString(8*WORDLIST_WIDTH-2, 16+4+i*16, FG_COLOR, temp, 1, 1);
				}
				else if ((currow+i)==(dictrows-1)) {
					// fix the last row, avoid garbage of next word
					memcpy(temp, rowstr, SCREEN_WIDTH/8);
					temp[wordlist[curword].dictlen - dictrow[currow+i]] = 0x0;
					chDrawString(8*WORDLIST_WIDTH-2, 16+4+i*16, FG_COLOR, temp, 1, 1);
				}
			}
		}
		// draw the vertical scroll bar
		if (dictpages>1) {
			int barlen;
			// if dictpages is big, barlen will have error in precision, maybe should use float type
			barlen = (R_LEN*16)/dictpages;
			chDrawRec(SCREEN_WIDTH-2, 16+4+barlen*curpage, 2, barlen, BLUE_COLOR, 1);
		}
	}

	// draw bottom horizon line
	chDrawRec(0, 16+4+WORDLIST_SIZE*16, SCREEN_WIDTH, 2, FG_COLOR, 1);
	// draw version, current word index/wordcount
	pgPrint(0, CMAX_Y-1, FG_COLOR, VERSION);
	// draw dict english name
	pgPrint(strlen(VERSION)+3, CMAX_Y-1, FG_COLOR, "<");
	pgPrint(strlen(VERSION)+4, CMAX_Y-1, FG_COLOR, ydictconf.dictename);
	pgPrint(strlen(VERSION)+4+strlen(ydictconf.dictename),
				CMAX_Y-1, FG_COLOR, ">");

	{
		// draw wordcount
		char temp[20];
		int index;

		if (curword != -1)
			index = wordlist[curword].index;
		else
			index = wordlist[0].index;
		sprintf(temp, "%d/%d",index+1,wordcount);
		pgPrint(CMAX_X-strlen(temp), CMAX_Y-1, FG_COLOR, temp);
	}

	pgScreenFlipV();
}

