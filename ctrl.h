//
//	This program is free software; you can redistribute it and/or
//	modify it under the terms of the GNU General Public License
//
//	Copyright (c) 2005 ZYM <yanming.zhang@gmail.com>
//

#ifndef _CTRL_H_
#define _CTRL_H_

#define MAXANALOG_X	255
#define MAXANALOG_Y	255
#define ANALOG_IGNORE	40

#define REPEAT_TIME 0x40000

#define LASTINPUT_X_LEFT		1
#define LASTINPUT_X_RIGHT		2
#define LASTINPUT_Y_UP			3
#define LASTINPUT_Y_DOWN		4
#define LASTINPUT_ANALOGNULL	5
#define LASTINPUT_KEY			6

struct ydictctrl {
	unsigned int buttons;
	unsigned char x;
	unsigned char y;
};

int getcontrol(struct ydictctrl *yctrl);

#endif
