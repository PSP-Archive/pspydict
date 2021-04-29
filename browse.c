//
//	This program is free software; you can redistribute it and/or
//	modify it under the terms of the GNU General Public License
//
//	Copyright (c) 2005 ZYM <yanming.zhang@gmail.com>
//

#include <pspctrl.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "chinesedraw.h"

unsigned long strbuftolong(unsigned char *s)
{
	long l;

	l = (*s) << 24;
	l += (*(s+1)) << 16;
	l += (*(s+2)) << 8;
	l += (*(s+3));
	return l;
}

int updateidxword(unsigned int idx, struct idxword *idxword)
{
	unsigned char *p;
	int len;

	// idx must in range [0, wordcount-1]
	if (idx>=wordcount)
		idx = wordcount -1;
	
	p= *(wordarray+idx);
	len = strlen((char *)p)+1;
	memcpy(idxword->word, p, ((len>MAXWORD)?MAXWORD:len));
	idxword->index = idx;
	idxword->dictoffset = strbuftolong(p+len);
	idxword->dictlen = strbuftolong(p+len+4);

	return idx;
}

int updatelist(unsigned int idx)
{
	int i;
	
	if (idx > (wordcount - WORDLIST_SIZE))
		idx = wordcount - WORDLIST_SIZE;

	memset((void *)&wordlist, 0, sizeof(struct idxword) * WORDLIST_SIZE);
	for (i=0; i<WORDLIST_SIZE; i++) {
		updateidxword(idx+i, &wordlist[i]);
	}
	return idx;
}

int updatecurrange(unsigned int first, unsigned int last)
{
	unsigned int mid;

	if ((first > last) || (last >wordcount)) {
		// error
	}
	if ((last -first) <10)
		return 0;
	updateidxword(first, &currange.first);
	updateidxword(last, &currange.last);
	// maybe we should set it
	// mid = first + (last -first) * 0.618
	mid = (first + last)/2;
	updateidxword(mid, &currange.mid);
	
	return 1;	
}

struct analogdirect {
	// last delta move 
	unsigned int lastspeed;
	// analog data [0..255]
	unsigned char lastdata;
	unsigned int count;
};

// analog pad get data about 30 counts in one second,
#define XPAGEDELTASIZE	90
unsigned int xpagedelta[XPAGEDELTASIZE]=
{
// first second
	0, 0, 0, 1, 0, 0, 0, 0, 1, 0,
	0, 0, 0, 1, 0, 0, 1, 0, 1, 0,
	1, 0, 1, 0, 1, 1, 1, 1, 1, 2,
// second second
	2, 2, 2, 3, 3, 4, 4, 5, 6, 6,
	7, 7, 8, 8, 9, 9, 10, 10, 11, 12,
	13, 14, 14, 15, 15, 16, 16, 17, 17, 18,
// third second
	18, 19, 19, 20, 21, 21, 22, 22, 23, 23,
	24, 24, 25, 25, 26, 26, 27, 27, 28, 28,
	29, 29, 30, 30, 31, 31, 32, 32, 33, 33,
};

#define YWORDDELTASIZE 30
unsigned int yworddelta[YWORDDELTASIZE]=
{
// first second
	0, 0, 0, 1, 0, 0, 0, 0, 1, 0,
	0, 0, 0, 1, 0, 0, 1, 0, 1, 0,
	1, 0, 1, 0, 1, 1, 1, 1, 1, 2,
};

struct analogdirect xleft, xright, yup, ydown;

void disp_browse_page(void);
void build_dictraw(struct idxword *idxp);

void ydict_browse(int laststate)
{
	unsigned char x, y;
	unsigned int button, lastinput;
	struct ydictctrl yctrl;

	if ((laststate == STATE_SELECTDICT)||(laststate==STATE_SWITCHMODE)) {
		// init currange and wordlist
		//updatecurrange(wordcount/3, wordcount / 3 * 2);
		updatecurrange(0, wordcount -1);
		updatelist(currange.mid.index);
		// push word stack
		memset((void *)&myidxstack, 0, sizeof(struct wordstack));
		memcpy((void *)&myidxstack.stack[myidxstack.stacktop], (void *)&currange, sizeof(struct idxrange));
		myidxstack.stacktop=1;
		curword = -1;
	}

	lastinput =0;

	while(1) {
		disp_browse_page();
		
		getcontrol(&yctrl);
		x = yctrl.x;
		y = yctrl.y;
		button = yctrl.buttons;
		if (x<(MAXANALOG_X/2 - ANALOG_IGNORE)){
			int delta;
			// move previous pages
			if (lastinput !=LASTINPUT_X_LEFT) {
				xleft.count=1;
				xleft.lastspeed=xpagedelta[xleft.count-1];
				lastinput = LASTINPUT_X_LEFT;
			}
			else {
				xleft.count++;
				if (x<=xleft.lastdata) {
					if (xleft.count<XPAGEDELTASIZE) 
						xleft.lastspeed=xpagedelta[xleft.count-1];
					else {
						xleft.lastspeed=xpagedelta[XPAGEDELTASIZE-1];
						xleft.count--;
					}
				}
				else {
					if (xleft.count>30)
						xleft.count -=30;
					else
						xleft.count=1;
					xleft.lastspeed=xpagedelta[xleft.count-1];
				}
			}
			xleft.lastdata=x;
			delta =xleft.lastspeed * WORDLIST_SIZE;
			
			if ((wordlist[0].index-delta) <WORDLIST_SIZE)
				updatelist(0);
			else
				updatelist(wordlist[0].index -delta);
			// update currange mid
			updateidxword(wordlist[0].index, &currange.mid);
			if (currange.first.index >= currange.mid.index)
				updateidxword(currange.mid.index, &currange.first);
			curword = -1;					
			gstate = STATE_BROWSE;
		}
		else if (x>(MAXANALOG_X/2+ANALOG_IGNORE)){
			int delta;
			// move next pages
			if (lastinput !=LASTINPUT_X_RIGHT) {
				xright.count=1;
				xright.lastspeed=xpagedelta[xright.count-1];
				lastinput = LASTINPUT_X_RIGHT;
			}
			else {
				xright.count++;
				if (x>=xright.lastdata) {
					if (xright.count<XPAGEDELTASIZE) 
						xright.lastspeed=xpagedelta[xright.count-1];
					else {
						xright.lastspeed=xpagedelta[XPAGEDELTASIZE-1];
						xright.count--;
					}
				}
				else {
					if (xright.count>30)
						xright.count -=30;
					else
						xright.count=1;
					xright.lastspeed=xpagedelta[xright.count-1];
				}
			}
			xright.lastdata=x;
			delta =xright.lastspeed * WORDLIST_SIZE;
			if ((wordlist[0].index + delta) > (wordcount - WORDLIST_SIZE))
				updatelist(wordcount - WORDLIST_SIZE);
			else
				updatelist(wordlist[0].index + delta);
			// update currange mid
			updateidxword(wordlist[0].index, &currange.mid);
			//update currange last
			if (currange.last.index <= currange.mid.index)
				updateidxword(currange.mid.index, &currange.last);
			curword = -1;					
			gstate = STATE_BROWSE;
		}
		else if (y<(MAXANALOG_Y/2 - ANALOG_IGNORE)){
			int delta;
			//move previous words
			if (lastinput != LASTINPUT_Y_UP) {
				yup.count=1;
				yup.lastspeed =yworddelta[yup.count-1];
				lastinput = LASTINPUT_Y_UP;
			}
			else {
				yup.count++;
				if (y>=yup.lastdata) {
					if (yup.count<YWORDDELTASIZE) 
						yup.lastspeed=yworddelta[yup.count-1];
					else {
						yup.lastspeed=yworddelta[YWORDDELTASIZE-1];
						yup.count--;
					}
				}
				else {
					yup.count=1;
					yup.lastspeed=yworddelta[yup.count-1];
				}
			}
			yup.lastdata=y;
			delta = yup.lastspeed;
			if (curword == -1) {
				if (delta<WORDLIST_SIZE)
					curword = WORDLIST_SIZE -delta;
			}
			else {
				if (curword <delta) {
					// jump to previous page
					int i;

					if (wordlist[curword].index >delta) {
						i = wordlist[curword].index -delta;
						updatelist(i);
						updateidxword(i, &currange.mid);
					}
					else {
						// jump to the first word in idx
						updatelist(0);
						// update currange mid
						updateidxword(wordlist[0].index, &currange.mid);
					}
					//update currange first
					if (currange.first.index >= currange.mid.index)
						updateidxword(currange.mid.index, &currange.first);
					curword = 0;
				}
				else {
					curword -= delta;
				}
			}
			gstate = STATE_BROWSE;
		}
		else if (y>(MAXANALOG_Y/2+ANALOG_IGNORE)) {
			//move next words
			int delta;
			if (lastinput !=LASTINPUT_Y_DOWN) {
				ydown.count=1;
				ydown.lastspeed =yworddelta[ydown.count-1];
				lastinput = LASTINPUT_Y_DOWN;
			}
			else {
				ydown.count++;
				if (y>=ydown.lastdata) {
					if (ydown.count<YWORDDELTASIZE) 
						ydown.lastspeed=yworddelta[ydown.count-1];
					else {
						ydown.lastspeed=yworddelta[YWORDDELTASIZE-1];
						ydown.count--;
					}
				}
				else {
					ydown.count=1;
					ydown.lastspeed=yworddelta[ydown.count-1];
				}
			}
			ydown.lastdata=y;
			delta = ydown.lastspeed;
			if (curword == -1) {
				if (delta<WORDLIST_SIZE)
					curword = delta;
			}
			else {
				if ((curword+delta)>(WORDLIST_SIZE-1)) {
					// jump to next page
					int i;
					if ((wordlist[curword].index +delta) <(wordcount-1)) {
						i=wordlist[curword].index +delta;
						updatelist(i-WORDLIST_SIZE+1);
						updateidxword(i, &currange.mid);
					}
					else {
						updatelist(wordcount-WORDLIST_SIZE);
						updateidxword(wordcount-1, &currange.mid);
					}
					if (currange.last.index<=currange.mid.index)
						updateidxword(currange.mid.index, &currange.last);
					curword = WORDLIST_SIZE -1;
				}
				else {
					curword +=delta;
				}
			}
			gstate = STATE_BROWSE;
		}
		else if (!button){
			lastinput= LASTINPUT_ANALOGNULL;
		} 
		else {
			lastinput=LASTINPUT_KEY;
		switch(button)
		{
			case PSP_CTRL_SELECT:
				// 
				gstate = STATE_SWITCHMODE;
				return;
			case PSP_CTRL_START:
				// reset browse page
				// clear myidxstack
				memset((void *)&myidxstack, 0, sizeof(struct wordstack));
				//updatecurrange(wordcount/3, wordcount / 3 * 2);
				updatecurrange(0, wordcount -1);
				updatelist(currange.mid.index);
				// push word stack
				memcpy((void *)&myidxstack.stack[myidxstack.stacktop], (void *)&currange, sizeof(struct idxrange));
				myidxstack.stacktop++;
				myidxstack.stacktop %= WORDSTACKSIZE;
				curword = -1;
				gstate = STATE_BROWSE;
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
						updateidxword(i, &currange.mid);
						//update currange first
						if (currange.first.index >= currange.mid.index)
							updateidxword(currange.mid.index, &currange.first);
					}
				}
				else {
					curword +=(WORDLIST_SIZE -1);
					curword %= WORDLIST_SIZE;
				}
				gstate = STATE_BROWSE;
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
						updateidxword(i, &currange.mid);
						//update currange last
						if (currange.last.index <= currange.mid.index)
							updateidxword(currange.mid.index, &currange.last);
					}
				}
				else {
					curword++;
					curword %= WORDLIST_SIZE;
				}
				gstate = STATE_BROWSE;
				break;
			case PSP_CTRL_LEFT:
				//previous list
				{
					if (wordlist[0].index <WORDLIST_SIZE)
						updatelist(0);
					else
						updatelist(wordlist[0].index - WORDLIST_SIZE);
					// update currange mid
					updateidxword(wordlist[0].index, &currange.mid);
					if (currange.first.index >= currange.mid.index)
						updateidxword(currange.mid.index, &currange.first);
					curword = -1;					
					gstate = STATE_BROWSE;
				}
				break;
			case PSP_CTRL_RIGHT:
				//next list
				{
					if ((wordlist[0].index + WORDLIST_SIZE) > (wordcount - WORDLIST_SIZE))
						updatelist(wordcount - WORDLIST_SIZE);
					else
						updatelist(wordlist[0].index + WORDLIST_SIZE);
					// update currange mid
					updateidxword(wordlist[0].index, &currange.mid);
					//update currange last
					if (currange.last.index <= currange.mid.index)
						updateidxword(currange.mid.index, &currange.last);
					curword = -1;					
					gstate = STATE_BROWSE;
				}
				break;
			case PSP_CTRL_SQUARE:
				// first half of current range
				{
					int ret;
					gstate = STATE_BROWSE;
					ret = updatecurrange(currange.first.index, currange.mid.index);
					if (ret == 0)
						break;
					else {
						updatelist(currange.mid.index);
						// push word stack
						memcpy((void *)&myidxstack.stack[myidxstack.stacktop], (void *)&currange, sizeof(struct idxrange));
						myidxstack.stacktop++;
						myidxstack.stacktop %= WORDSTACKSIZE;
						curword = -1;
					}
				}	
				break;
			case PSP_CTRL_CIRCLE:
				// last half of current range
				{
					int ret;
					gstate = STATE_BROWSE;
					ret = updatecurrange(currange.mid.index, currange.last.index);
					if (ret ==0)
						break;
					else {
						updatelist(currange.mid.index);
						// push word stack
						memcpy((void *)&myidxstack.stack[myidxstack.stacktop], (void *)&currange, sizeof(struct idxrange));
						myidxstack.stacktop++;
						myidxstack.stacktop %= WORDSTACKSIZE;
						curword = -1;
					}
				}
				break;
			case PSP_CTRL_TRIANGLE:
				// pop range stack, or previous page in dict mode
				{
					unsigned int i;
					
					if (gstate == STATE_VIEWWORD) {
						if (curpage>0) {
							curpage--;
						}
						else {
							// now in first page 
							gstate = STATE_BROWSE;
						}
					}
					else {
						//pop range stack
						i = (myidxstack.stacktop + (WORDSTACKSIZE-2)) % WORDSTACKSIZE;
						if (strlen(myidxstack.stack[i].first.word) ==0)
							break;
						else {
							myidxstack.stacktop += (WORDSTACKSIZE -1);
							myidxstack.stacktop %= WORDSTACKSIZE;
							i = (myidxstack.stacktop + (WORDSTACKSIZE-1)) % WORDSTACKSIZE;
							memset((void*)&myidxstack.stack[myidxstack.stacktop], 0, sizeof(struct idxrange));
							memcpy((void *)&currange, (void *)&myidxstack.stack[i], sizeof(struct idxrange));
							updatelist(currange.mid.index);
						}
					}
				}
				break;
			case PSP_CTRL_LTRIGGER:
				// previous range
				{
					int i;

					gstate = STATE_BROWSE;
					if ((currange.first.index == 0) && ( currange.mid.index == 0))
						break;
					if (currange.first.index == 0) {
						// update currange mid to 0
						updateidxword(0, &currange.mid);
					}
					else {
						i = currange.last.index - currange.first.index;
						if ((currange.first.index -i) <0)
							updatecurrange(0, currange.first.index);
						else
							updatecurrange(currange.first.index -i, currange.first.index);
					}
					updatelist(currange.mid.index);
					// push word stack
					memcpy((void *)&myidxstack.stack[myidxstack.stacktop], (void *)&currange, sizeof(struct idxrange));
					myidxstack.stacktop++;
					myidxstack.stacktop %= WORDSTACKSIZE;
					curword = -1;
				}
				break;
			case PSP_CTRL_RTRIGGER:
				// next range
				{
					int i;

					gstate = STATE_BROWSE;
					if ((currange.mid.index == (wordcount -1)) && (currange.last.index == (wordcount -1)))
						break;
					if (currange.last.index == (wordcount -1)) {
						// update currange mid to last word
						updateidxword(wordcount-1, &currange.mid);
					}
					else {
						i = currange.last.index - currange.first.index;
						if ((currange.last.index +i) > (wordcount -1))
							updatecurrange(currange.last.index, wordcount -1);
						else
							updatecurrange(currange.last.index, currange.last.index +i);
					}
					updatelist(currange.mid.index);
					// push word stack
					memcpy((void *)&myidxstack.stack[myidxstack.stacktop], (void *)&currange, sizeof(struct idxrange));
					myidxstack.stacktop++;
					myidxstack.stacktop %= WORDSTACKSIZE;
					curword = -1;
				}
				break;
			case PSP_CTRL_CROSS:
				// view word dict, or next page in dict mode
				//if (curword == -1)
				//	break;
				if (gstate == STATE_BROWSE) {
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
					else {
						gstate=STATE_BROWSE;
					}
				}
				break;
			default:
				break;
			}
		}
	}
}


void disp_browse_page(void)
{
	int i, len, x;
	int hint_first, hint_mid, hint_last;
	
	pgWaitV();
	pgFillvram(rgb2col(0,0,0));
	// draw currange
	len = strlen(currange.first.word);
	if (len > RANGEWORDLEN) {
		currange.first.word[RANGEWORDLEN-1]='.';
		currange.first.word[RANGEWORDLEN]='.';
		currange.first.word[RANGEWORDLEN+1]=0x0;
	}
	chDrawString(0, 0, FG_COLOR, currange.first.word, 1, 1);
	len = strlen(currange.mid.word);
	if (len > RANGEWORDLEN) {
		currange.mid.word[RANGEWORDLEN-1]='.';
		currange.mid.word[RANGEWORDLEN]='.';
		currange.mid.word[RANGEWORDLEN+1]=0x0;
	}
	x = SCREEN_WIDTH/2 - strlen(currange.mid.word)*8/2;
	chDrawString(x, 0, FG_COLOR, currange.mid.word, 1, 1);
	len = strlen(currange.last.word);
	if (len > RANGEWORDLEN) {
		currange.last.word[RANGEWORDLEN-1]='.';
		currange.last.word[RANGEWORDLEN]='.';
		currange.last.word[RANGEWORDLEN+1]=0x0;
	}
	x = SCREEN_WIDTH- strlen(currange.last.word)*8;
	chDrawString(x, 0, FG_COLOR, currange.last.word, 1, 1);

	// draw the color hint line
	chDrawRec(0, 16, SCREEN_WIDTH, 4, FG_COLOR, 1);
	hint_first = SCREEN_WIDTH * currange.first.index / wordcount;
	hint_last= SCREEN_WIDTH * currange.last.index / wordcount;
	hint_mid = SCREEN_WIDTH * currange.mid.index / wordcount;
	// draw width must >0
	if ((hint_last-hint_first)>0)
		chDrawRec(hint_first, 16, hint_last-hint_first, 4, BLUE_COLOR, 1);
	chDrawRec(hint_mid, 16, 2, 4, RED_COLOR, 1);

	// draw the word list
	if (gstate == STATE_VIEWWORD) {
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
	else {
		for (i=0;i<WORDLIST_SIZE;i++) {
			if (curword != i)
				chDrawString(0, 16+4 +i*16, FG_COLOR, *(wordarray+wordlist[i].index), 1, 1);
			else
				chDrawString(0, 16+4 +i*16, BLUE_COLOR, *(wordarray+wordlist[curword].index), 1, 1);
		}
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
			index = currange.mid.index;
		sprintf(temp, "%d/%d",index+1,wordcount);
		pgPrint(CMAX_X-strlen(temp), CMAX_Y-1, FG_COLOR, temp);
	}

	pgScreenFlipV();

}

void build_dictraw(struct idxword *idxp)
{
	char chflag;
	int dictlen;
	unsigned char *dictp, *gPr;
	int offset, seekoffset, len, i;
	
	dictp	= dictfilep + idxp->dictoffset;
	dictlen = idxp->dictlen;

	memset((void *)&dictrow,0, MAX_ROW*4);
	gPr= dictp;
	dictrows=0;
	offset = 0;
	seekoffset = 0;
	len =0;
	while(1)
	{
		dictrow[dictrows]= offset;
		if (offset>=dictlen)
			break;
		else {
			if ((dictlen - offset) > C_LEN)
				len = C_LEN;
			else
				len = dictlen-offset +1;
			chflag = 0;
			for (i=0;i<len;i++)
			{
				if (gPr[i] >= 0x81)
				{
					if (chflag)
						chflag = 0;
					else
						chflag = 1;
				}
				else {
					chflag = 0;
				}
				if(gPr[i]==0x0D && gPr[i+1]==0x0A&&gPr[i+2]==0x0D && gPr[i+3]==0x0A)
				{
					seekoffset =offset+i;
					dictrows++;
					dictrow[dictrows]=seekoffset;
					seekoffset +=2;
					break;
				}
				if(gPr[i]==0x0A)
				{
					seekoffset =offset+i+1;
					break;
				}
				else if(gPr[i]==0x0D && gPr[i+1]==0x0A)
				{					
					seekoffset =offset+i+2;
					break;
				}
				
			}
			if (i ==len)
				seekoffset = offset +i-chflag;
			offset = seekoffset;
			gPr = dictp+seekoffset;
			if (i>0)
				dictrows++;
		}
	}		
}


