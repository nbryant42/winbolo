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

Boolean attackqueue[16] = {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};


WORLD_X targetx;
WORLD_Y targety;

int getpriorityAttack(const BrainInfo *info, int idnum) {
	#ifdef SHOWPRI
		extern int attackpillpri;
	#endif
	
	unsigned int pri;
	extern ObjectInfo pillboxes[16];
	
	
	extern setting opts;
	
	
	if ((!(pillboxes[idnum].info & OBJECT_HOSTILE || pillboxes[idnum].info & OBJECT_NEUTRAL))) {
		#ifdef SHOWPRI
			attackpillpri = 100000;
		#endif
		return 100000;
	}
	if (!settings_getoptions(&opts, ATTACK_PILLBOXES)) {
		#ifdef SHOWPRI
			attackpillpri = 100000;
		#endif
		return 100000;
	}
	if (pillboxes[idnum].idnum == 20 || info->shells < 2) {
		#ifdef SHOWPRI
			attackpillpri = 100000;
		#endif
		return 100000;
	}
	if (pillboxes[idnum].direction == 0) {
		#ifdef SHOWPRI
			attackpillpri = 100000;
		#endif
		return 100000;
	}
	if (stillChecks(info, idnum)) {
		#ifdef SHOWPRI
			attackpillpri = 100000;
		#endif
		return 100000;
	}
	pri = (int) hypot(abs((pillboxes[idnum].x >> 8) - (info->tankx >> 8)), abs((pillboxes[idnum].y >> 8) - (info->tanky >> 8)));
	pri += (pri * ATTACKPILL_DISTANCE_MULTIPLY) + ATTACKPILL_PRI;
	/* add the pillboxes strength so we attack the weaker ones. */
	pri += (pillboxes[idnum].direction * 2);
	#ifdef SHOWPRI
		attackpillpri = pri;
	#endif
	return pri;
}


Boolean startAttack(const BrainInfo *info, int idnum) {
	extern ObjectInfo pillboxes[16];
	
	
	printf("Start attack\n");
	if (stillChecks(info, idnum)) {
		return 2;
	}
	targetx = pillboxes[idnum].x;
	targety = pillboxes[idnum].y;
	if (pillboxes[idnum].direction != 0) {
		if (!getPath(info, targetx >> 8, targety >> 8, info->tankx >> 8, info->tanky >> 8, 0, 0, 0, 0)) {
			printf("Danger - No path.\n");
			return FALSE;
		}
	} else {
		extern Boolean (*endfunction)(const BrainInfo *info, int idnum);
		endfunction = NULL;
	}
	return TRUE;
	
}


Boolean middleAttack(const BrainInfo *info, int idnum) {
	if (stillChecks(info, idnum)) {
		return 2;
	}
	if (abs((int)sqrt((((info->tankx) - (targetx)) * ((info->tankx) - (targetx))) + (((info->tanky) - (targety)) * ((info->tanky) - (targety))))) < 1700) {
		if (clearPath(info, info->tankx, info->tanky, targetx, targety, FALSE, TRUE)) {
			return TRUE;
		}
	}
	return doTravel(info, targetx >> 8, targety >> 8, TRUE, FALSE);
}


Boolean endAttack(const BrainInfo *info, int idnum) {
	extern int speed;
	int i, aimdir;
	Boolean isPointed;

	isPointed = TRUE;
	if (stillChecks(info, idnum)) {
		return 2;
	}
	
	
	
	*info->holdkeys = 0;
	aimdir = aim(info, targetx, targety);
	pointTank(info, targetx, targety);
	if (abs((int)sqrt(((info->tankx - targetx) * (info->tankx - targetx)) + ((info->tanky - targety) * (info->tanky - targety)))) > 1600) {
		speed = 64;
	} else {
		speed = 0;
	}

	if (abs(info->direction - aimdir) > 127) {
		if (info->direction > aimdir) {
			if ((abs(info->direction - 256) + aimdir) > 5) {
				isPointed = FALSE;
			}
		} else {
			if ((abs(aimdir - 256) + info->direction) > 5) {
				isPointed = FALSE;
			}
		}
	} else if (abs(info->direction - aimdir) > 5) {
		isPointed = FALSE;
	}
	
	
	if (isPointed) {
		setkey(*info->holdkeys, KEY_shoot);
	}
	for (i=0;i < info->num_objects;i++) {
		if (info->objects[i].idnum == idnum) {
			if (info->objects[i].direction == 0) {
				attackqueue[idnum] = FALSE;
				printf("Done attacking pillbox\n");
				return TRUE;
			} else {
				break;
			}
		}
	}			
	return FALSE;
}

Boolean stillChecks(const BrainInfo *info, int idnum) {

	if (info->shells < 10) {
		return TRUE;
	}
	if (info->armour < 4) {
		return TRUE;
	}
	return FALSE;
}

Boolean addAttack(const BrainInfo *info, int idnum) {
	extern actionqueue nextaction;
	actionqueue_insert(info, &nextaction, &startAttack, &middleAttack, &endAttack, &getpriorityAttack, idnum);
	attackqueue[idnum] = TRUE;
	printf("Added attack action to queue.\n");
	return TRUE;
}


Boolean isAttackQueued(const BrainInfo *info, int idnum) {
	return attackqueue[idnum];
}

