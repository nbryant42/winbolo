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
#include "pickup.h"
#include "settings.h"
#include <math.h>
#include "laypill.h"

WORLD_X targetx;
WORLD_Y targety;


int startpillcount;

extern WORLD_X basex;
extern WORLD_Y basey;
int lasttickcount = 0;



Boolean endPickup(const BrainInfo *info, int idnum);

Boolean pickupqueue[16] = {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
Boolean diedtryingtogetpill[16] = {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};


int getpriorityPickup(const BrainInfo *info, int idnum) {
	unsigned int pri;
	int j;
	extern ObjectInfo pillboxes[16];
	extern setting opts;
	extern Boolean (*endfunction)(const BrainInfo *info, int idnum);
	
	#ifdef SHOWPRI
		extern int pickuppri;
	#endif
	
	
	if (!settings_getoptions(&opts, PICKUP_PILLBOXES)) {
		#ifdef SHOWPRI
			pickuppri = 100000;
		#endif
		return 100000;
	}
	
	
	if (pillboxes[idnum].direction != 0) {
		#ifdef SHOWPRI
			pickuppri = 100000;
		#endif
		return 100000;
	}
	
	
	if (pillboxes[idnum].idnum == 20) {
		#ifdef SHOWPRI
			pickuppri = 100000;
		#endif
		return 100000;
		
	}
	
	
	for (j=0;j < 16;j++) {
		//printf("PBD %d\n", pillboxes[j].direction);
		if (pillboxes[j].direction != 0 && pillboxes[j].idnum != 20 && (/*pillboxes[idnum].info & OBJECT_NEUTRAL || */(pillboxes[j].info & OBJECT_HOSTILE || pillboxes[j].info & OBJECT_NEUTRAL))) {
			
			if ((int) hypot(abs((pillboxes[idnum].x >> 8) - (pillboxes[j].x >> 8)), abs((pillboxes[idnum].y >> 8) - (pillboxes[j].y >> 8))) < 7) {
				if (clearPath(info, pillboxes[j].x, pillboxes[j].y, pillboxes[idnum].x, pillboxes[idnum].y, TRUE, FALSE)) {
					if (diedtryingtogetpill[idnum]) {
						//printf("Cantgetthere\n");
						return 100000;
					}// else {
						//printf("Havent died yet\n");
					//}
				}
			}
		}
	}
	
	//if (info->inboat && (info->theWorld[(u_short)(pillboxes[idnum].y + 256) | (pillboxes[idnum].x >> 8) + 256] == DEEPSEA || info->theWorld[(u_short)(pillboxes[idnum].y - 256) | (pillboxes[idnum].x >> 8) + 256] == DEEPSEA || info->theWorld[(u_short)(pillboxes[idnum].y + 256) | (pillboxes[idnum].x >> 8) - 256] == DEEPSEA || info->theWorld[(u_short)(pillboxes[idnum].y - 256) | (pillboxes[idnum].x >> 8) - 256] == DEEPSEA)) {
	//	return 900;
	//}
	//printf("--");
	if (!settings_getoptions(&opts, BASEGUARD_MODE)) {
		if ((int) hypot(abs((pillboxes[idnum].x >> 8) - (basex >> 8)), abs((pillboxes[idnum].y >> 8) - (basey >> 8))) > 15) {
			return 100000;
		}
	}

	pri = (int) hypot(abs((pillboxes[idnum].x >> 8) - (info->tankx >> 8)), abs((pillboxes[idnum].y >> 8) - (info->tanky >> 8)));
	if (endfunction != endPickup) {
		if (pri > PICKUP_TO_FAR_DISTANCE) {
			pri += PICKUP_TO_FAR_DISTANCE_TO_ADD;
		}
	}
	//printf("--\n");
	pri += (pri * PICKUP_DISTANCE_MULTIPLY) + PICKUP_PRI;
	#ifdef SHOWPRI
		pickuppri = pri;
	#endif
	return pri;
	
//	return 100;
}


Boolean startPickup(const BrainInfo *info, int idnum) {
	extern ObjectInfo pillboxes[16];
	
	printf("Start pillpickup\n");
	printf("idnum: %d - x: %d  y: %d\n", idnum, pillboxes[idnum].x, pillboxes[idnum].y);
	targetx = pillboxes[idnum].x;
	targety = pillboxes[idnum].y;
	if (pillboxes[idnum].direction == 0) {
		if (!getPath(info, targetx >> 8, targety >> 8, info->tankx >> 8, info->tanky >> 8, 0, 0, 0, 0)) {
			printf("Danger -- No Path\n");
			return FALSE;
		}
	} else {
		extern Boolean (*endfunction)(const BrainInfo *info, int idnum);
		endfunction = NULL;
	}
	startpillcount = info->carriedpills;
	return TRUE;
	
	
	
	
	//return FALSE;
}


Boolean middlePickup(const BrainInfo *info, int idnum) {
	extern ObjectInfo pillboxes[16];
	int i;


	if (info->carriedpills - 1 == startpillcount) {
		return TRUE;
	}
	
	/* The following code is so that we don't go and *
	* pickup pillboxes that have already been picked *
	* up. */
	if ((int) hypot(abs((info->tankx >> 8) - (pillboxes[idnum].x >> 8)), abs((info->tanky >> 8) - (pillboxes[idnum].y >> 8))) < 11) {
		Boolean found;
		found = FALSE;
		for (i=0;i < info->num_objects;i++) {
			if (info->objects[i].object == OBJECT_PILLBOX && info->objects[i].idnum == idnum) {
				found = TRUE;
			}
		}
		if (!found) {
			pillboxes[idnum].idnum = 20;
			return TRUE;
		}
		
	}
	return doTravel(info, targetx >> 8, targety >> 8, TRUE, FALSE);
}




Boolean endPickup(const BrainInfo *info, int idnum) {
	extern ObjectInfo pillboxes[16];
	printf("Pillbox pickuped\n");
	
	pillboxes[idnum].idnum = 20;
	diedtryingtogetpill[idnum] = FALSE;
	pickupqueue[idnum] = FALSE;
	return TRUE;
}



Boolean addPickup(const BrainInfo *info, int idnum) {
	extern actionqueue nextaction;
	actionqueue_insert(info, &nextaction, &startPickup, &middlePickup, &endPickup, &getpriorityPickup, idnum);
	pickupqueue[idnum] = TRUE;
	printf("Added pickup action to queue.\n");
	return TRUE;
}


Boolean isPickupQueued(const BrainInfo *info, int idnum) {
	return pickupqueue[idnum];
}

