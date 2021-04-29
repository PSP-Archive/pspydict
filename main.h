//
//	This program is free software; you can redistribute it and/or
//	modify it under the terms of the GNU General Public License
//
//	Copyright (c) 2005 ZYM <yanming.zhang@gmail.com>
//

#ifndef _MAIN_H_
#define _MAIN_H_

#include "pg.h"
#include "ctrl.h"

// debug mode
//#define DEBUG_YDICT

#define VERSION "PSP YDICT ver0.2 by zym"

// mempool size 22M is ok, 23M fail
#define MEMPOOL_SIZE	(0x100000*22)
#define MEMPOOL_FREE_SIZE	0x20000
#define GZ_INBUF_SIZE		0x80000 //512k

enum {
	STATE_START,
	STATE_BROWSE,
	STATE_RING_KEY,
	STATE_VIEWWORD,
	STATE_SELECTDICT,
	STATE_SWITCHMODE,
};

// max len of word
#define MAXWORD	50
#define WORDSTACKSIZE	50
#define WORDLIST_SIZE	15
#define WORDLIST_WIDTH 16

#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT 272

#define KEYNUMBER	32
#define LETTERANGLE (360/KEYNUMBER)
#define KBCENTRE_X	(480/2)
#define KBCENTRE_Y	(272/2)
#define KBRADIUS	80

// the max word len of currange to display
#define RANGEWORDLEN	11
// the max row in dict interpretation 
#define MAX_ROW	(10*1024)

// col count in a raw in dict area
#define C_LEN (((SCREEN_WIDTH)-(WORDLIST_WIDTH*8))/8)
// raw count in dict area
#define R_LEN WORDLIST_SIZE


struct idxword {
	char word[MAXWORD];
	int index;
	long dictoffset;
	long dictlen;
};

struct idxrange {
	struct idxword first;
	struct idxword mid;
	struct idxword last;
};

struct wordstack {
	struct idxrange stack[WORDSTACKSIZE];
	unsigned int stacktop;
};

#define PATH_LEN	0x400
#define FILENAME_LEN	0x108
#define DICTNAME_LEN	30
#define CONFIG_FILE	"ydict.conf"
#define BROWSE_MODE	1
#define RINGKEY_MODE	2

struct configdata {
	char dirname[FILENAME_LEN];
	char dictfile[FILENAME_LEN];
	char idxfile[FILENAME_LEN];
	char dictename[DICTNAME_LEN];
	char dictcname[DICTNAME_LEN];
	unsigned char mode;
	unsigned char changed;
};
extern struct configdata ydictconf;

#define SELECT_DICT_STR		"请选择词典:"
#define NO_DICT_STR		"无可用词典! 按<HOME>键退出。"
#define TRIANGLE_MENU		"△"
#define CIRCLE_MENU		"○"
#define CROSS_MENU		"Ⅹ"
#define SQUARE_MENU		"□"

#define NODICT_MENU		"[无]"
#define CHANGE_DICT_MENU	"更换词典"
#define RINGKEY_MODE_MENU	"切换至键盘模式"
#define BROWSE_MODE_MENU	"切换至翻阅模式"
#define RINGKEY_GAME_MENU	"打字游戏"
#define HELP_MENU		"帮助"

#define	MENU_X		(16*3)
#define MENU_Y		(16*3)
#define MENU_DELTA	(16*2)

#define FG_COLOR	rgb2col(255,255,255)
#define BG_COLOR	rgb2col(0,0,0)
#define BLUE_COLOR	rgb2col(0, 0, 255)
#define RED_COLOR	rgb2col(255, 0, 0)
#define GREEN_COLOR	rgb2col(0, 255, 0)
#define TRIANGLE_COLOR	GREEN_COLOR
#define CIRCLE_COLOR	RED_COLOR
#define CROSS_COLOR	BLUE_COLOR
#define SQUARE_COLOR	rgb2col(255, 0, 255)

#define KEYS_COLOR 	FG_COLOR
#define CURRKEY_COLOR 	RED_COLOR
//#define GRAY_COLOR 	rgb2col(90,90,90)
//#define GRAY_COLOR 	rgb2col(75,75,75)
#define GRAY_COLOR 	rgb2col(60,60,60)

extern unsigned char errstr[200];
#define ERR_PRINT(arg...) {\
	pgWaitV();\
	pgFillvram(rgb2col(0,0,0));\
	sprintf(errstr,arg);\
	chDrawString(0, 0, FG_COLOR, errstr, 1, 1);\
	chDrawString(0, 32, FG_COLOR, "\n\nERROR OCCURRED. press <home> to exit.\n :-(", 1, 1);\
	pgScreenFlipV();\
	while(1) pgWaitV();}

#ifdef DEBUG_YDICT
extern unsigned char dbgstr[200];
extern void wait_button(void);
#define DBG_PRINT(arg...) {\
	pgWaitV();\
	pgFillvram(rgb2col(0,0,0));\
	sprintf(dbgstr,arg);\
	chDrawString(0, 0, FG_COLOR, dbgstr, 1, 1);\
	pgScreenFlipV();\
	wait_button();}
#else
#define DBG_PRINT
#endif

extern unsigned char mempool[MEMPOOL_SIZE];
extern unsigned int idxfilesize;
extern unsigned int dictfilesize;
extern unsigned char *idxfilep;
extern unsigned char *dictfilep;

extern int gstate;
extern unsigned char **wordarray;
extern unsigned int wordcount;
extern struct wordstack myidxstack;
extern struct idxrange currange;
extern struct idxword wordlist[WORDLIST_SIZE];
extern int curword;

extern unsigned int dictrow[MAX_ROW];
extern unsigned int dictrows;
extern int curpage;
extern int curwordrows;
extern int dictpages;

extern char psp_full_path[PATH_LEN];
extern char dictfilepath[PATH_LEN];
extern char idxfilepath[PATH_LEN];
extern char configfilepath[PATH_LEN];
extern char dictdirpath[PATH_LEN];


int updatelist(unsigned int idx);
void build_dictraw(struct idxword *idxp);


void ydict_browse(int laststate);
void ydict_ringkey(int laststate);
void ydict_selectdict(int laststate);
void ydict_switchmode(int laststate);

unsigned long ydict_unzip(unsigned long output_start, int infd,
							unsigned long inbuffer, unsigned long free_mem);

#endif

