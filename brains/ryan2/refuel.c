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
#include "refuel.h"
#include <math.h>
#include <time.h>

WORLD_X targetx, currentx;
WORLD_Y targety, currenty;
Boolean cmoving = TRUE;

Boolean queued[16] = {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
Boolean refuelempty[16] = {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
time_t refueltimes[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int getpriorityRefuel(const BrainInfo *info, int idnum) {
	extern ObjectInfo bases[16];
	extern ObjectInfo pillboxes[16];
	
	#ifdef SHOWPRI
		extern int refuelpri;
	#endif
	
	
	int pri, i;
	pri = 0;
	
	//#define DEBUG_REF
	
	if (bases[idnum].idnum == 20) {
		return 100000;
	}
	/* After 4 minutes (240 seconds) reset the empty setting. */
	if (refueltimes[idnum] != 0 && time(NULL) - refueltimes[idnum] > 240) {
		refueltimes[idnum] = 0;
		refuelempty[idnum] = FALSE;
		printf("Resetting refueltime\n");
	}
	
	#ifdef DEBUG_REF
		//printf("base[%d].info = %d\n", idnum, bases[idnum].info);
	#endif
	if (refuelempty[idnum]) {
		#ifdef DEBUG_REF
			printf("ref: empty\n");
		#endif
		
		
		#ifdef SHOWPRI
			refuelpri = 100000;
		#endif
		return 100000;
	}
	
	for (i=0;i < info->num_objects;i++) {
		/* Don't go to it if there is a tank on the base */
		if (info->objects[i].object == OBJECT_TANK && ((info->objects[i].x >> 8) == (bases[idnum].x >> 8) && (info->objects[i].y >> 8) == (bases[idnum].y >> 8))) {
			return 100000;
		}
	}

	
	
	if (bases[idnum].info & OBJECT_HOSTILE && (info->shells <= bases[idnum].direction)) {
		#ifdef DEBUG_REF
			printf("ref: Can't Shoot Through: id: %d, info: %d, direction: %d, shells: %d\n", idnum, bases[idnum].info, bases[idnum].direction, info->shells);
		#endif
		
		#ifdef SHOWPRI
			refuelpri = 100000;
		#endif
		return 100000;
	}
	
	if (bases[idnum].info & OBJECT_NEUTRAL) {
		int tmpri;
		tmpri = 0;
		tmpri = (int) hypot(abs((bases[idnum].x >> 8) - (info->tankx >> 8)), abs((bases[idnum].y >> 8) - (info->tanky >> 8)));
		tmpri = (tmpri * REFUEL_DISTANCE_MULTIPLY) + REFUEL_NEUTRAL_PRI;
		for (i=0;i < 16;i++) {
			if (pillboxes[i].direction != 0 && pillboxes[i].idnum != 20 && (/*pillboxes[idnum].info & OBJECT_NEUTRAL || */(pillboxes[i].info & OBJECT_HOSTILE || pillboxes[i].info & OBJECT_NEUTRAL))) {
				if ((int)hypot(abs((pillboxes[i].x >> 8) - (bases[idnum].x >> 8)), abs((pillboxes[i].y >> 8) - (bases[idnum].y >> 8))) < 13) {
					if (clearPath(info, pillboxes[i].x, pillboxes[i].y, bases[idnum].x, bases[idnum].y, TRUE, FALSE)) {
						goto leavenow;
					}
				}
			}
		}
		return tmpri + idnum;
	}
	
	leavenow:
	
	if (/*(info->shells >= 18 && info->armour >= 4) && */(info->shells < 39 || info->armour < 8)) {
		if ((int) hypot(abs((bases[idnum].x >> 8) - (info->tankx >> 8)), abs((bases[idnum].y >> 8) - (info->tanky >> 8))) < 2 /*(((bases[idnum].x >> 8) == (info->tankx >> 8)) && ((bases[idnum].y) == (info->tanky >> 8)))*/) {
			#ifdef SHOWPRI
				refuelpri = 20;
			#endif
			pri = 20;
			goto addpillcount;
			//return 20;
		} else {
			if ((info->shells < 18 || info->armour < 4)) {
				goto next;
			}
			if (bases[idnum].info != 0) {
				goto next;
			} else {
				#ifdef DEBUG_REF
					printf("ref: Full - everything\n");
				#endif
				
				
				#ifdef SHOWPRI
					refuelpri = 100000;
				#endif
				return 100000;
			}
		}
	}
	
	
	next:
	if ((info->shells < 18 || info->armour < 4)) {
		//int ts;
		//float tsf;
		pri = (int) hypot(abs((bases[idnum].x >> 8) - (info->tankx >> 8)), abs((bases[idnum].y >> 8) - (info->tanky >> 8)));
		pri = (pri * REFUEL_DISTANCE_MULTIPLY) + REFUEL_PRI;
		addpillcount:
		for (i=0;i < 16;i++) {
			//printf("pi: %d\n", pillboxes[idnum].info);
			if (pillboxes[i].direction != 0 && pillboxes[i].idnum != 20 && (/*pillboxes[idnum].info & OBJECT_NEUTRAL || */(pillboxes[i].info & OBJECT_HOSTILE || pillboxes[i].info & OBJECT_NEUTRAL))) {
				if ((int)hypot(abs((pillboxes[i].x >> 8) - (bases[idnum].x >> 8)), abs((pillboxes[i].y >> 8) - (bases[idnum].y >> 8))) < 13) {
					if (clearPath(info, pillboxes[i].x, pillboxes[i].y, bases[idnum].x, bases[idnum].y, TRUE, FALSE)) {
						pri += REFUEL_ADD_IF_PILLBOX_DANGER;
						goto getout;
					}
				}
			}
		}
		//printf("Prev: %d\n", pri);
		if (pri != 20) {
			//pri = (int)((float)pri * ((((float)60 + (float)info->shells) + ((float)60 + (5 * (float)info->armour)) / 2.0) * (float).01));
		}
		//printf("After: %d\n", pri);
		getout:
		#ifdef DEBUG_REF
			printf("ref: Priority: %d\n", pri);
		#endif
		
		
		#ifdef SHOWPRI
			refuelpri = pri;
		#endif
		return pri;
	}
	
	#ifdef DEBUG_REF
		printf("ref: 3000\n");
	#endif
	
	
	#ifdef SHOWPRI
		refuelpri = 100000;
	#endif
	return 100000;

}

Boolean startRefuel(const BrainInfo *info, int idnum) {
	extern ObjectInfo bases[16];
	
	printf("Start refuel\n");
	targetx = bases[idnum].x;
	targety = bases[idnum].y;
	if (!getPath(info, targetx >> 8, targety >> 8, info->tankx >> 8, info->tanky >> 8, 0, 0, 0, 0)) {
		printf("Danger Will Robinson\n");
		refuelempty[idnum] = TRUE;
		refueltimes[idnum] = time(NULL);
		return FALSE;
	}
	cmoving = TRUE;
	return TRUE;
	
	
	
	
	//return FALSE;
}


Boolean middleRefuel(const BrainInfo *info, int idnum) {
	if (cmoving) {
		if (doTravel(info, targetx >> 8, targety >> 8, TRUE, FALSE)) {
			currentx = info->tankx;
			currenty = info->tanky;
			if (info->speed == 0) {
				return TRUE;
			} else {
				cmoving = FALSE;
				return TRUE;
			}
		} else {
			return FALSE;
		}
	} else {
		if (info->speed == 0) {
			cmoving = TRUE;
			return TRUE;
		} else {
			return FALSE;
		}
	}
}


Boolean endRefuel(const BrainInfo *info, int idnum) {
	extern int speed;
	*(info->holdkeys) = 0;
	*(info->tapkeys) = 0;
	speed = 0;
	
	if (((info->shells >= 40 || info->base_shells < 1) && (info->armour >= 8 || info->base_armour < 2)) || (abs((int)sqrt(((info->tankx - targetx) * (info->tankx - targetx)) + ((info->tanky - targety) * (info->tanky - targety)))) > 120)) {
		if (info->base_shells == 0 || info->base_armour <= 1) {
			refuelempty[idnum] = TRUE;
			refueltimes[idnum] = time(NULL);
		}
		queued[idnum] = FALSE;
		return TRUE;
	}
	
	//currentx = info->tankx;
	//currenty = info->tanky;
	//printf("We have reached the refueling base\n");
	
	
	return FALSE;
}

Boolean addRefuel(const BrainInfo *info, int idnum) {
	extern actionqueue nextaction;
	actionqueue_insert(info, &nextaction, &startRefuel, &middleRefuel, &endRefuel, &getpriorityRefuel, idnum);
	queued[idnum] = TRUE;
	printf("Added refuel action to queue.\n");
	return TRUE;
}


Boolean isRefuelQueued(const BrainInfo *info, int idnum) {
	return queued[idnum];
}


