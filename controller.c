#include <pspctrl.h>
#include "main.h"


#define REPEAT_TIME 0x40000
static unsigned long control_bef_ctl  = 0;
static unsigned long control_bef_tick = 0;

unsigned long Read_Key(void) {
	SceCtrlData pad;

	sceCtrlReadBufferPositive(&pad,1);
	if (pad.Buttons == control_bef_ctl) {
		if ((pad.TimeStamp- control_bef_tick) > REPEAT_TIME) {
			return control_bef_ctl;
		}
		return 0;
	}
	control_bef_ctl  = pad.Buttons;
	control_bef_tick = pad.TimeStamp;
	return control_bef_ctl;
}

int Control(void) {
	unsigned long key;

	// wait key
	while(1) {
		key = Read_Key();
		if (key != 0) break;
		pgWaitV();
	}
#if 0
if (key & CTRL_SQUARE)   {return 1;}  
if (key & CTRL_TRIANGLE) {return 2;} 
if (key & CTRL_CIRCLE)   {return 3;}   
if (key & CTRL_CROSS)    {return 4;}
if (key & CTRL_UP)       {return 5;}       
if (key & CTRL_DOWN)     {return 6;}     
if (key & CTRL_LEFT)     {return 7;}     
if (key & CTRL_RIGHT)    {return 8;}    
if (key & CTRL_START)    {return 9;}    
if (key & CTRL_SELECT)   {return 10;}   
if (key & CTRL_LTRIGGER) {return 11;} 
if (key & CTRL_RTRIGGER) {return 12;} 
#endif

	return key;
}

void wait_button(void) {
	/*ctrl_data_t ctl;
	int btn;

	btn=1;
	while(btn!=0){
		sceCtrlRead(&ctl,1);
		btn = ((ctl.buttons & 0xF000) != 0);
	}
	btn=0;
	while(btn==0){
		sceCtrlRead(&ctl,1);
		btn = ((ctl.buttons & 0xF000) != 0);
	}
	*/
	while(Control()==0);
}

