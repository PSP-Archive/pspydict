//
//	This program is free software; you can redistribute it and/or
//	modify it under the terms of the GNU General Public License
//
//	Copyright (c) 2005 ZYM <yanming.zhang@gmail.com>
//
#include <pspkernel.h>
#include <pspctrl.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "chinesedraw.h"

static void disp_selectdict_page(void);

struct dictinfo {
	char dirname[FILENAME_LEN];
	char dictfile[FILENAME_LEN];
	char idxfile[FILENAME_LEN];
	char dictcname[DICTNAME_LEN];
	char dictename[DICTNAME_LEN];
};

#define	MAXDICTNUM	20
struct dictinfo dictlist[MAXDICTNUM];
static int dictnum;
static int dictindex;
struct SceIoDirent direntry, fileentry;

static void getdictslist(void)
{
	int fd, dictdirfd, ifofd;
	int readsize;
	unsigned char ifo[100];
	char *p, *p2;
	
	dictnum=0;
	fd = sceIoDopen(psp_full_path);
	if (fd <0) {
		ERR_PRINT("Open dir [%s] fail.", psp_full_path);
	}
	while (dictnum<MAXDICTNUM) {
		memset(&dictlist[dictnum], 0, sizeof(struct dictinfo));
		memset(&direntry, 0, sizeof(struct SceIoDirent));
		if (sceIoDread(fd, &direntry) <= 0) {
			DBG_PRINT("no more dict dir in %s, dictnum=%d",psp_full_path, dictnum);
			break;
		}
		else {
			DBG_PRINT("dictnum %d", dictnum);
		}
		DBG_PRINT("dir attr %x, dirname %s",direntry.d_stat.st_attr, direntry.d_name);
		if (direntry.d_stat.st_attr & FIO_SO_IFDIR) {
			if (!strcmp(direntry.d_name, "."))
				continue;
			if (!strcmp(direntry.d_name, ".."))
				continue;
			strcpy(dictdirpath, psp_full_path);
			strcat(dictdirpath, direntry.d_name);
			strcat(dictdirpath, "/");
			dictdirfd = sceIoDopen(dictdirpath);
			DBG_PRINT("dictdir %s", dictdirpath);
			while (sceIoDread(dictdirfd, &fileentry) >0) {
				DBG_PRINT("filename %s", fileentry.d_name);
				if (fileentry.d_stat.st_attr & FIO_SO_IFREG) {
					if (strstr(fileentry.d_name, "dict.pspz")) {
						// get dict file
						strcpy(dictlist[dictnum].dictfile, fileentry.d_name);
					}
					else if (strstr(fileentry.d_name, "idx.pspz")) {
						// get idx file
						strcpy(dictlist[dictnum].idxfile, fileentry.d_name);
					}
					else if (strstr(fileentry.d_name, "ifo.psp")) {
						// get ifo file
						strcat(dictdirpath, fileentry.d_name);
						ifofd = sceIoOpen(dictdirpath, PSP_O_RDONLY, 0777);
						if (!ifofd){
							ERR_PRINT("open file[%s] fail.", fileentry.d_name);
						}
						else {
							readsize = sceIoRead(ifofd, ifo, 100);
							ifo[readsize]='\0';
							DBG_PRINT("%s ifo %s",direntry.d_name, ifo);
							p = strstr(ifo,"pspename=");
							p += strlen("pspename=");
							p2 = strchr(p, '\n');
							strncpy(dictlist[dictnum].dictename, p, p2-p);
							p = strstr(p2+1, "pspcname=");
							p += strlen("pspcname=");
							p2 = strchr(p, '\n');
							strncpy(dictlist[dictnum].dictcname, p, p2-p);
						}
						sceIoClose(ifofd);
					}
				}
			}
			if ((strlen(dictlist[dictnum].dictfile)>0) &&
				(strlen(dictlist[dictnum].idxfile)>0) &&
				(strlen(dictlist[dictnum].dictename)>0) &&
				(strlen(dictlist[dictnum].dictcname)>0))
				strcpy(dictlist[dictnum].dirname, direntry.d_name);
			dictnum++;
			sceIoDclose(dictdirfd);
		}		
	}
	sceIoDclose(fd);
}

// return 0 fail, 1 ok.
//
static int loaddictfile(void)
{
	int dictfd, idxfd;
	unsigned long gzfilesize;
	unsigned char *p;

	memset(dictfilepath, 0, PATH_LEN);
	memset(idxfilepath, 0, PATH_LEN);
	// load dict file
	strcpy(dictfilepath, psp_full_path);
	strcat(dictfilepath, ydictconf.dirname);
	strcat(dictfilepath, "/");
	strcat(dictfilepath, ydictconf.dictfile);
	dictfd = sceIoOpen(dictfilepath, PSP_O_RDONLY, 0777);
	if (dictfd<0) {
		// must delete config file first
		memset(configfilepath, 0, PATH_LEN);
		strcpy(configfilepath, psp_full_path);
		strcat(configfilepath, CONFIG_FILE);
		sceIoRemove(configfilepath);
		DBG_PRINT("open file [%s] fail.", ydictconf.dictfile);
		return 0;
	}
	else {
		gzfilesize = (unsigned long)sceIoLseek32(dictfd, 0, PSP_SEEK_END);
		sceIoLseek32(dictfd, 0, PSP_SEEK_SET);
		if (gzfilesize > MEMPOOL_SIZE) {
			ERR_PRINT("file [%s] is too big.", ydictconf.dictfile);
		}
		dictfilesize = ydict_unzip(mempool, dictfd,
				mempool+MEMPOOL_SIZE-MEMPOOL_FREE_SIZE-GZ_INBUF_SIZE,
				mempool+MEMPOOL_SIZE-MEMPOOL_FREE_SIZE);
		dictfilep = mempool;
		sceIoClose(dictfd);
	}
	// load idxfile
	strcpy(idxfilepath, psp_full_path);
	strcat(idxfilepath, ydictconf.dirname);
	strcat(idxfilepath, "/");
	strcat(idxfilepath, ydictconf.idxfile);
	idxfd = sceIoOpen(idxfilepath, PSP_O_RDONLY, 0777);
	if (idxfd<0) {
		// must delete config file first
		memset(configfilepath, 0, PATH_LEN);
		strcpy(configfilepath, psp_full_path);
		strcat(configfilepath, CONFIG_FILE);
		sceIoRemove(configfilepath);
		DBG_PRINT("open file [%s] fail.", ydictconf.idxfile);
		return 0;
	}
	else {
		gzfilesize = (unsigned long)sceIoLseek(idxfd, 0, PSP_SEEK_END);
		sceIoLseek(idxfd, 0, PSP_SEEK_SET);
		idxfilep = mempool + ((dictfilesize +0xFFF) & 0xFFFFF000);
		idxfilesize = ydict_unzip(idxfilep, idxfd,
				mempool+MEMPOOL_SIZE-MEMPOOL_FREE_SIZE-GZ_INBUF_SIZE,
				mempool+MEMPOOL_SIZE-MEMPOOL_FREE_SIZE);
		sceIoClose(idxfd);
	}

	DBG_PRINT("load dict&idx ok");
	// build the word offset array
	wordarray = (char **)(((int)idxfilep + idxfilesize+ 0xFFF) & 0xFFFFF000);
	wordcount = 0;
	p= idxfilep;
	while (p <(idxfilep + idxfilesize)) {
		*(wordarray+wordcount)=p;
		p+=strlen((char *)p);
		p+=9;
		wordcount++;
	}
	DBG_PRINT("build wordcount %d ok", wordcount);

	return 1;
}

void ydict_selectdict(int laststate)
{
	int cfgfd;
	struct ydictctrl yctrl;
	int menuindex;
	
	if (laststate == STATE_START) {
		// ydict start, read config file
		memset(configfilepath, 0, PATH_LEN);
		strcpy(configfilepath, psp_full_path);
		strcat(configfilepath, CONFIG_FILE);
		cfgfd = sceIoOpen(configfilepath, PSP_O_RDONLY, 0777);
		if (cfgfd>=0) {
			sceIoRead(cfgfd, &ydictconf, sizeof(struct configdata));
			sceIoClose(cfgfd);
			if (loaddictfile() ==0)
				return;
			if (ydictconf.mode == BROWSE_MODE)
				gstate = STATE_BROWSE;
			else if (ydictconf.mode == RINGKEY_MODE)
				gstate = STATE_RING_KEY;
			return ;
		}
		else {
			// no config file
			DBG_PRINT("no config file");
		}
	}
	DBG_PRINT("before getdictslist\n");
	getdictslist();
	dictindex=0;
	while (1) {
		disp_selectdict_page();
		
		getcontrol(&yctrl);
		switch (yctrl.buttons)
		{
			case PSP_CTRL_TRIANGLE:
			case PSP_CTRL_CIRCLE:
			case PSP_CTRL_CROSS:
			case PSP_CTRL_SQUARE:
				if (yctrl.buttons == PSP_CTRL_TRIANGLE)
					menuindex = 0;
				else if (yctrl.buttons == PSP_CTRL_CIRCLE)
					menuindex = 1;
				else if (yctrl.buttons == PSP_CTRL_CROSS)
					menuindex = 2;
				else if (yctrl.buttons == PSP_CTRL_SQUARE)
					menuindex = 3;
				dictindex +=menuindex;
				if ((dictindex<dictnum) &&(strlen(dictlist[dictindex].dictfile)>0) &&
					(strcmp(ydictconf.dictfile, dictlist[dictindex].dictfile))) {
					strcpy(ydictconf.dirname, dictlist[dictindex].dirname);
					strcpy(ydictconf.dictfile, dictlist[dictindex].dictfile);
					strcpy(ydictconf.idxfile, dictlist[dictindex].idxfile);
					strcpy(ydictconf.dictename, dictlist[dictindex].dictename);
					strcpy(ydictconf.dictcname, dictlist[dictindex].dictcname);
					if (loaddictfile()==0)
						return;
					if (ydictconf.mode == BROWSE_MODE)
						gstate = STATE_BROWSE;
					else if (ydictconf.mode == RINGKEY_MODE)
						gstate = STATE_RING_KEY;
					else {
						ydictconf.mode = BROWSE_MODE;
						gstate = STATE_BROWSE;
					}
					ydictconf.changed = 1;
					return;
				}
				else if (dictindex<dictnum){
					if (ydictconf.mode == BROWSE_MODE)
						gstate = STATE_BROWSE;
					else if (ydictconf.mode == RINGKEY_MODE)
						gstate = STATE_RING_KEY;
					return;
				}
				break;
			case PSP_CTRL_LEFT:
				if ((dictindex-4)>=0)
					dictindex -=4;
				break;
			case PSP_CTRL_RIGHT:
				if ((dictindex+4)<dictnum)
					dictindex +=4;
				break;
			default:
				break;
		}
	}
}

static void disp_selectdict_page(void)
{

	pgWaitV();
	pgFillvram(rgb2col(0,0,0));

	// draw select dict
	chDrawString(16*3, 0, FG_COLOR, SELECT_DICT_STR, 1, 1);
	// draw the top line
	chDrawRec(0, 16, SCREEN_WIDTH, 4, FG_COLOR, 1);
	// draw bottom horizon line
	chDrawRec(0, 16+4+WORDLIST_SIZE*16, SCREEN_WIDTH, 2, FG_COLOR, 1);
	// draw version
	pgPrint(0, CMAX_Y-1, FG_COLOR, VERSION);

	if (dictnum == 0) {
		chDrawString(SCREEN_WIDTH/2 - strlen(NO_DICT_STR)/2*8, 
			SCREEN_HEIGHT/2 -8, FG_COLOR, NO_DICT_STR, 1, 1);
	}
	else {
		if (dictindex<dictnum) {
			// triangle menu
			chDrawString(MENU_X, MENU_Y, TRIANGLE_COLOR, TRIANGLE_MENU, 1, 1);
			chDrawString(MENU_X+3*16, MENU_Y,
				FG_COLOR, dictlist[dictindex].dictcname, 1, 1);
		}
		if ((dictindex+1)<dictnum) {
			// circle menu
			chDrawString(MENU_X, MENU_Y+MENU_DELTA, CIRCLE_COLOR, CIRCLE_MENU, 1, 1);
			chDrawString(MENU_X+3*16, MENU_Y+MENU_DELTA,
				FG_COLOR, dictlist[dictindex+1].dictcname, 1, 1);
		}
		if ((dictindex+2)<dictnum) {
			// cross menu
			chDrawString(MENU_X, MENU_Y+MENU_DELTA*2, CROSS_COLOR, CROSS_MENU, 1, 1);
			chDrawString(MENU_X+3*16, MENU_Y+MENU_DELTA*2,
				FG_COLOR, dictlist[dictindex+2].dictcname, 1, 1);
		}
		if ((dictindex+3)<dictnum) {
			// square menu
			chDrawString(MENU_X, MENU_Y+MENU_DELTA*3, SQUARE_COLOR, SQUARE_MENU, 1, 1);
			chDrawString(MENU_X+3*16, MENU_Y+MENU_DELTA*3,
				FG_COLOR, dictlist[dictindex+3].dictcname, 1, 1);
		}
		// next page
		// if ()
	}
	pgScreenFlipV();

}
