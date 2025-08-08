#ifdef WIN32
	#include "brain_win.h"
#else
	/* brain.h -- needed to interact with linbolo */
	#include "brain.h"
#endif
#include <stdlib.h>
#include <stdio.h>
#include "actionqueue.h"
#include "pointtank.h"
#include "travelto.h"
#include "attackpill.h"
#include "settings.h"
#include "laypill.h"
#include <math.h>
#include <time.h>



WORLD_X targetx;
WORLD_Y targety;
time_t basetime = 0;

extern WORLD_X basex;
extern WORLD_Y basey;

extern int speed;

int getpriorityBaseguard(const BrainInfo *info, int idnum) {
	extern setting opts;
	if (settings_getoptions(&opts, BASEGUARD_MODE)) {
		return 100000;
	}
	if (basetime != 0) {
		if (basetime + 2400 < time(NULL)) {
			printf("timereset\n");
			basetime = 0;
		}
		printf("return 1000000\n");
		return 1000000;
	}
	
	return BASEGUARD_PRI;
}


Boolean startBaseguard(const BrainInfo *info, int idnum) {
	/* Pick the best base */
	printf("Startbaseguard\n");
	if (basex == 0 && basey == 0) {
		printf("Pickbase\n");
		pickbase(info);
	}
	if (basex == 0 && basey == 0) {
		printf("SetBaseTime\n");
		basetime = time(NULL);
		return TRUE;
	}
	
	printf("Start BG\n");
	if (!getPath(info, basex >> 8, basey >> 8, info->tankx >> 8, info->tanky >> 8, 0, 0, 0, 0)) {
		printf("Danger -- No Path\n");
		return FALSE;
	}
	return TRUE;
}


Boolean middleBaseguard(const BrainInfo *info, int idnum) {
	if (basex == 0 && basey == 0) {
		return TRUE;
	}
	if ((int)hypot(abs((info->tankx >> 8) - (basex >> 8)), abs((info->tanky >> 8) - (basey >> 8))) < 6) {
		return TRUE;
	}
	return doTravel(info, basex >> 8, basey >> 8, TRUE, FALSE);
}


Boolean endBaseguard(const BrainInfo *info, int idnum) {
	if (basex == 0 && basey == 0) {
		return 2;
	}
	speed = 0;
	return FALSE;
}

Boolean addBaseguard(const BrainInfo *info, int idnum) {
	extern actionqueue nextaction;
	actionqueue_insert(info, &nextaction, &startBaseguard, &middleBaseguard, &endBaseguard, &getpriorityBaseguard, idnum);
	return TRUE;
}


