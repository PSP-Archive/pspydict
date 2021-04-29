//
//	This program is free software; you can redistribute it and/or
//	modify it under the terms of the GNU General Public License
//
//	Copyright (c) 2005 ZYM <yanming.zhang@gmail.com>
//
#include <pspctrl.h>
#include <stdio.h>
#include "main.h"

void disp_switchmode_page(int laststate);

void ydict_switchmode(int laststate)
{
	struct ydictctrl yctrl;

	while(1) {
		disp_switchmode_page(laststate);

		getcontrol(&yctrl);
		switch (yctrl.buttons)
		{
		case PSP_CTRL_TRIANGLE:
			// change dict
			gstate = STATE_SELECTDICT;
			return;
		case PSP_CTRL_CIRCLE:
			// switch_mode
			if (laststate == STATE_BROWSE) {
				gstate = STATE_RING_KEY;
				ydictconf.mode = RINGKEY_MODE;
				ydictconf.changed = 1;
			}
			else if (laststate == STATE_RING_KEY) {
				gstate = STATE_BROWSE;
				ydictconf.mode = BROWSE_MODE;
				ydictconf.changed = 1;
			}
			return;
		case PSP_CTRL_CROSS:
			// help 
			break;
		case PSP_CTRL_SQUARE:
			// ringkey game
			break;
		case PSP_CTRL_SELECT:
			gstate = laststate;
			return;
		default:
			break;
		}
		
	}
}

void disp_switchmode_page(int laststate)
{
	pgWaitV();
	pgFillvram(BG_COLOR);
	// draw the top line
	chDrawRec(0, 16, SCREEN_WIDTH, 4, FG_COLOR, 1);
	// draw bottom horizon line
	chDrawRec(0, 16+4+WORDLIST_SIZE*16, SCREEN_WIDTH, 2, FG_COLOR, 1);
	// draw version
	pgPrint(0, CMAX_Y-1, FG_COLOR, VERSION);

	// triangle menu
	chDrawString(MENU_X, MENU_Y, TRIANGLE_COLOR, TRIANGLE_MENU, 1, 1);
	chDrawString(MENU_X+3*16, MENU_Y,
		FG_COLOR, CHANGE_DICT_MENU, 1, 1);

	// circle menu
	chDrawString(MENU_X, MENU_Y+MENU_DELTA, CIRCLE_COLOR, CIRCLE_MENU, 1, 1);
	if (laststate == STATE_BROWSE)
		chDrawString(MENU_X+3*16, MENU_Y+MENU_DELTA,
			FG_COLOR, RINGKEY_MODE_MENU, 1, 1);
	else if (laststate == STATE_RING_KEY)
		chDrawString(MENU_X+3*16, MENU_Y+MENU_DELTA,
			FG_COLOR, BROWSE_MODE_MENU, 1, 1);
	
	// cross menu
	chDrawString(MENU_X, MENU_Y+MENU_DELTA*2, CROSS_COLOR, CROSS_MENU, 1, 1);
	chDrawString(MENU_X+3*16, MENU_Y+MENU_DELTA*2,
		GRAY_COLOR, HELP_MENU, 1, 1);

	// square menu
	chDrawString(MENU_X, MENU_Y+MENU_DELTA*3, SQUARE_COLOR, SQUARE_MENU, 1, 1);
	chDrawString(MENU_X+3*16, MENU_Y+MENU_DELTA*3,
		GRAY_COLOR, RINGKEY_GAME_MENU, 1, 1);

	pgScreenFlipV();
}
