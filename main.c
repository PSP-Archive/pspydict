//
//	This program is free software; you can redistribute it and/or
//	modify it under the terms of the GNU General Public License
//
//	Copyright (c) 2005 ZYM <yanming.zhang@gmail.com>
//
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "chinesedraw.h"

/* Define the module info section */
PSP_MODULE_INFO("PSPYDICT", 0, 0, 2);

/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

void dump_threadstatus(void);

int done = 0;

/* Exit callback */
int exit_callback(void)
{
	int fd;
	if (ydictconf.changed == 1) {
		ydictconf.changed = 0;
		memset(configfilepath, 0, PATH_LEN);
		strcpy(configfilepath, psp_full_path);
		strcat(configfilepath, CONFIG_FILE);
		fd = sceIoOpen(configfilepath, PSP_O_WRONLY|PSP_O_CREAT, 0777);
		if (fd) {
			sceIoWrite(fd, &ydictconf, sizeof(struct configdata));
			DBG_PRINT("write cfg %s", psp_full_path);
		}
		else {
			ERR_PRINT("Open file [%s] fail.", configfilepath);
		}
		sceIoClose(fd);
	}
	sceKernelExitGame();
	done = 1;
	return 0;
}

/* Callback thread */
int CallbackThread(SceSize args, void *argp)
{
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);
	sceKernelSleepThreadCB();

	return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void)
{
	int thid = 0;

	thid = sceKernelCreateThread("update_thread", CallbackThread,
				     0x11, 0xFA0, 0, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}

	return thid;
}


////// global data in ydict  /////////////////
char psp_full_path[PATH_LEN];
char dictfilepath[PATH_LEN];
char idxfilepath[PATH_LEN];
char configfilepath[PATH_LEN];
char dictdirpath[PATH_LEN];


unsigned char mempool[MEMPOOL_SIZE];

unsigned int idxfilesize;
unsigned int dictfilesize;
unsigned char *idxfilep;
unsigned char *dictfilep;
unsigned char **wordarray;
unsigned int wordcount;

struct configdata ydictconf;
struct idxrange currange;
struct idxword wordlist[WORDLIST_SIZE];
struct wordstack myidxstack;

int curword = -1;
// the max dict word in oxford-gb is 'go', size 234731
unsigned int dictrow[MAX_ROW];
unsigned int dictrows;
int curpage;
// some word maybe longer than the dict area width, curwordrows = 1 or 2
int curwordrows;
int dictpages;

unsigned char errstr[200];

#ifdef DEBUG_YDICT
unsigned char dbgstr[200];
int dbgrow=0;
#endif


int gstate;


int main(int argc, char **argv)
{
	char *psp_eboot_path;
	int laststate;

	pgInit();
	SetupCallbacks();
	pgScreenFrame(2,0);

	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

	// get the ydict pbp path
	strncpy(psp_full_path, argv[0], sizeof(psp_full_path) - 1);
	psp_full_path[sizeof(psp_full_path) - 1] = '\0';
 	psp_eboot_path = strrchr(psp_full_path, '/');
  	if (psp_eboot_path != NULL)
  	{
		*(psp_eboot_path+1) = '\0';
	} 
	memset((void *)&ydictconf, 0, sizeof(struct configdata));
	
	laststate=STATE_START;
	gstate=STATE_SELECTDICT;
	
	while(1)
	{
		switch(gstate)
		{
			case STATE_BROWSE:
				ydict_browse(laststate);
				laststate = STATE_BROWSE;
				break;
			case STATE_RING_KEY:
				ydict_ringkey(laststate);
				laststate = STATE_RING_KEY;
				break;
			case STATE_SELECTDICT:
				ydict_selectdict(laststate);
				laststate = STATE_SELECTDICT;
				break;
			case STATE_SWITCHMODE:
				ydict_switchmode(laststate);
				laststate = STATE_SWITCHMODE;
				break;
			default:
				ERR_PRINT("unknown state occur.");
				break;
		}
	}
	ERR_PRINT("program exit.");
	return 0;
}

