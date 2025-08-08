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
#include <math.h>
#include <time.h>
#include "repair.h"


WORLD_X targetx;
WORLD_Y targety;

int rsubstep = 0;
int rstep = 0;
time_t starttime, curtime;


Boolean endRepair(const BrainInfo *info, int idnum);


int getpriorityRepair(const BrainInfo *info, int idnum) {
	unsigned int pri;
	int i;
	extern ObjectInfo pillboxes[16];
	extern ObjectInfo bases[16];
	extern Boolean (*endfunction)(const BrainInfo *info, int idnum);
	
	
	extern setting opts;
	
	if (pillboxes[idnum].idnum == 20) {
		return 100000;
	}
	
	if (info->man_status != 0) {
		return 100000;
	}
	
	
	
	if (((pillboxes[idnum].info & OBJECT_HOSTILE || pillboxes[idnum].info & OBJECT_NEUTRAL))) {
		return 100000;
	}
	if (!settings_getoptions(&opts, BUILD_PILLBOXES)) {
		return 100000;
	}
	if (pillboxes[idnum].idnum == 20 || info->trees < 4) {
		return 100000;
	}
	
	//printf("pillboxes[idnum].direction: %d\n", pillboxes[idnum].direction);
	if (pillboxes[idnum].direction == 0) {
		return 100000;
	}
	
	if (pillboxes[idnum].direction >= 15) {
		return 100000;
	}
	
	
	
	
	
	pri = (int) hypot(abs((pillboxes[idnum].x >> 8) - (info->tankx >> 8)), abs((pillboxes[idnum].y >> 8) - (info->tanky >> 8)));
	if (endfunction != endRepair) {
		if (pri > REPAIR_TO_FAR_DISTANCE) {
			pri += REPAIR_TO_FAR_DISTANCE_TO_ADD;
		}
	}
	pri += (pri * REPAIR_DISTANCE_MULTIPLY) + (pillboxes[idnum].direction * REPAIR_PILLBOX_STRENGTH_MULTIPLY) + REPAIR_PRI;
	for (i=0;i < 16;i++) {
		//printf("pi: %d\n", pillboxes[idnum].info);
		if (pillboxes[i].direction != 0 && pillboxes[i].idnum != 20 && (/*pillboxes[idnum].info & OBJECT_NEUTRAL || */(pillboxes[i].info & OBJECT_HOSTILE || pillboxes[i].info & OBJECT_NEUTRAL))) {
			if ((int)hypot(abs((pillboxes[i].x >> 8) - (bases[idnum].x >> 8)), abs((pillboxes[i].y >> 8) - (bases[idnum].y >> 8))) < 13) {
				if (clearPath(info, pillboxes[i].x, pillboxes[i].y, bases[idnum].x, bases[idnum].y, TRUE, FALSE)) {
					pri += REPAIR_PILLBOX_DANGER;
					break;
				}
			}
		}
	}
	
	for (i=0;i < info->num_objects;i++) {
		if (info->objects[i].object == OBJECT_SHOT) {
			if ((int)hypot(abs((info->objects[i].x >> 8) - (pillboxes[idnum].x >> 8)), abs((info->objects[i].y >> 8) - (pillboxes[idnum].y >> 8))) < 9) {
				pri += REPAIR_SHOT_DANGER;
				break;
			}
		}
	}
	return pri;
}


Boolean startRepair(const BrainInfo *info, int idnum) {
	extern ObjectInfo pillboxes[16];
	
	
	printf("Start Pillbox Repair idnum: %d, idnum: %d, %d\n", pillboxes[idnum].idnum, idnum, pillboxes[idnum].info);
	if (pillboxes[idnum].idnum == 20) {
		printf("Not a valid pill\n");
		return FALSE;
	}
	targetx = pillboxes[idnum].x;
	targety = pillboxes[idnum].y;
	if (!getPath(info, targetx >> 8, targety >> 8, info->tankx >> 8, info->tanky >> 8, 0, 0, 0, 0)) {
		printf("Danger - No path.\n");
		return FALSE;
	}
	rsubstep = 0;
	return TRUE;
	
}


Boolean middleRepair(const BrainInfo *info, int idnum) {
	pointTank(info, targetx, targety);

	if ((int)hypot(abs((info->tankx) - (targetx)), abs((info->tanky) - (targety))) < 330) {
		extern int speed;
		pointTank(info, targetx, targety);
		speed = 15;
		rstep = 0;
		return TRUE;
	}
	if (rsubstep == 0) {
		if (doTravel(info, targetx >> 8, targety >> 8, TRUE, TRUE)) {
			rsubstep = 1;
		}
	}

	return FALSE;
}


Boolean endRepair(const BrainInfo *info, int idnum) {
	extern int speed;


	if (rstep == 0) {
		//printf("Step2\n");
		pointTank(info, targetx, targety);
		info->build->y = targety >> 8;
		info->build->x = targetx >> 8;
		info->build->action = BUILDMODE_PBOX;
		printf("tkx: %d, tky: %d, sx: %d, sy: %d\n", info->tankx >> 8, info->tanky >> 8, targetx >> 8, targety >> 8);
		rstep = 1;
		*info->holdkeys = 0;
		speed = 15;
		time(&starttime);
	} else {
		pointTank(info, targetx, targety);
		*info->holdkeys = 0;
		speed = 15;
		time(&curtime);
		if (curtime >= starttime + 3) {
			return 2;
		}
		if (info->man_status == 0) {
			if (info->carriedpills == 0) {
				rstep = 0;
				return 2;
			} else {
				rstep = 0;
			}
			
		}
	}
	return FALSE;
}

Boolean addRepair(const BrainInfo *info, int idnum) {
	extern actionqueue nextaction;
	actionqueue_insert(info, &nextaction, &startRepair, &middleRepair, &endRepair, &getpriorityRepair, idnum);
	return TRUE;
}


