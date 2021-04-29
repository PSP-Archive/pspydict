//
//	This program is free software; you can redistribute it and/or
//	modify it under the terms of the GNU General Public License
//
//	Copyright (c) 2005 ZYM <yanming.zhang@gmail.com>
//
#include <pspdisplay.h>
#include <pspctrl.h>
#include "main.h"

SceCtrlData lastctrldata = {0, 0, 0, 0};

int getcontrol(struct ydictctrl *yctrl)
{
	SceCtrlData pad;

	// dispatch control key
	while (1) {
		yctrl->buttons=0;
		yctrl->x=yctrl->y=(MAXANALOG_X/2);
		sceCtrlReadBufferPositive(&pad, 1); 
		if (pad.Buttons == lastctrldata.Buttons) {
			if ((pad.TimeStamp-lastctrldata.TimeStamp) >REPEAT_TIME) {
				yctrl->buttons = pad.Buttons;
			}
			else
				yctrl->buttons = 0;
		}
		else {
			lastctrldata.Buttons = pad.Buttons;
			lastctrldata.TimeStamp = pad.TimeStamp;
			yctrl->buttons = pad.Buttons;
		}
		if ((yctrl->buttons!=0)||(lastctrldata.Lx !=pad.Lx)||(lastctrldata.Ly !=pad.Lx)) {
			yctrl->x=lastctrldata.Lx=pad.Lx;
			yctrl->y=lastctrldata.Ly=pad.Ly;
			break;
		}
		sceDisplayWaitVblankStart();
	}
}
