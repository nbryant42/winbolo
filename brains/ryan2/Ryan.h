
#ifndef RYAN_H
#define RYAN_H
/* This is the header file for Ryan.c */

#ifdef WIN32
	#include "brain_win.h"
#else
	/* brain.h -- needed to interact with linbolo */
	#include "brain.h"
#endif

/* Defines */
#define EXPLAIN printf




/* Priority is an array with how much each action is weighted.  These actions *
* are weighted based on many factors.  What kinds of options has a lot to do  *
* with it, but in later version, I hope to have them also weighted based on   *
* how well it does with the setting.										  */

enum {
	ATTACK_PILL	= 0,
	BUILD_PILL, PICKUP_PILL, LAY_MINE, CLEAR_MINE, CAPTURE_BASE,
	ATTACK_TANK, REPAIR_BASE, REPAIR_PILL, ATTACK_BASE, EXPLORE
};

/* Use this macro to get the priority of one of the functions above */
#define GETPRIORITY(x) (priority[x])

/* Speed is a varable I use to keep track of how fast the tank "should" be going. *
* this varable can be changed from anywhere in the program with the SETSPEED      *
* function.  use GETSPEED to get the value of speed.                              */

#define SETSPEED(x) (speed = x)
#define GETSPEED() (speed)



/* Generic brain error */
#define BRAIN_ERROR -1

#define BRAIN_MENU_NAME "Ryan2"


#endif

