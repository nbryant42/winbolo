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
#include "attacktank.h"
#include "settings.h"
#include <math.h>




int getpriorityAttackTank(const BrainInfo *info, int idnum) {
	int i;
	extern setting opts;
	
	
	#ifdef SHOWPRI
		extern int attacktankpri;
	#endif


	/* Don't attack the tank if we only have 10 or less shells*/
	if (info->shells < 10) {
		#ifdef SHOWPRI
			attacktankpri = 100000;
		#endif
		return 100000;
	}
	
	
	if (!settings_getoptions(&opts, ATTACK_TANKS)) {
		#ifdef SHOWPRI
			attacktankpri = 100000;
		#endif
		return 100000;
	}
	
	for (i=0;i < info->num_objects;i++) {
		if (info->objects[i].object == OBJECT_TANK && (info->objects[i].info & OBJECT_HOSTILE)) {
			if (clearPath(info, info->tankx, info->tanky, info->objects[i].x, info->objects[i].y, FALSE, TRUE)) {
				#ifdef SHOWPRI
					attacktankpri = ATTACKTANK_PRI;
				#endif
				return ATTACKTANK_PRI;
			}
		}
	}
	#ifdef SHOWPRI
		attacktankpri = 100000;
	#endif
	return 100000;
	
}

Boolean addAttackTank(const BrainInfo *info, int idnum) {
	extern actionqueue nextaction;
	actionqueue_insert(info, &nextaction, &startAttackTank, &middleAttackTank, &endAttackTank, &getpriorityAttackTank, 0);
	printf("Added attack action to queue.\n");
	return TRUE;
}

Boolean startAttackTank(const BrainInfo *info, int idnum) {
	printf("Start Attack Tank\n");
	return TRUE;
	
}


Boolean middleAttackTank(const BrainInfo *info, int idnum) {
	WORLD_X tgx, tgy;
	int i;
	extern int speed;
	for (i=0;i < info->num_objects;i++) {
		if (info->objects[i].object == OBJECT_TANK && (info->objects[i].info & OBJECT_HOSTILE)) {
			
			speed = 64;
			tgx = info->objects[i].x;
			tgy = info->objects[i].y;
			goto gout;
		}
	}
	return FALSE;
	
	
	gout:
	if (abs((int)sqrt((((info->tankx) - (tgx)) * ((info->tankx) - (tgx))) + (((info->tanky) - (tgy)) * ((info->tanky) - (tgy))))) < 1700) {
		if (clearPath(info, info->tankx, info->tanky, tgx, tgy, FALSE, TRUE)) {
			return TRUE;
		}
	}
	return doTravel(info, tgx >> 8, tgy >> 8, FALSE, FALSE);
	
}


Boolean endAttackTank(const BrainInfo *info, int idnum) {
	int i;
	extern int speed;
	int dist;

	printf("AttackTank\n");
	for (i=0;i < info->num_objects;i++) {
		if (info->objects[i].object == OBJECT_TANK && (info->objects[i].info & OBJECT_HOSTILE)) {
			Boolean isPointed;
			int aimdir;
			
			isPointed = TRUE;
			
			speed = 64;
			aimdir = aim(info, info->objects[i].x, info->objects[i].y);
			pointTank(info, info->objects[i].x, info->objects[i].y);
			if (abs(info->direction - aimdir) > 127) {
				if (info->direction > aimdir) {
					if ((abs(info->direction - 256) + aimdir) > 30) {
						isPointed = FALSE;
					}
				} else {
					if ((abs(aimdir - 256) + info->direction) > 30) {
						isPointed = FALSE;
					}
				}
			} else if (abs(info->direction - aimdir) > 30) {
				isPointed = FALSE;
			}
			dist = (int) hypot(abs((info->objects[i].x >> 8) - (info->tankx >> 8)), abs((info->objects[i].y >> 8) - (info->tanky >> 8)));
			if (dist > 11) {
				return 2;
			} else if (dist < 4) {
				speed = 0;
			} else if (dist > 8) {
				speed = 64;
			}
			
			if (isPointed) {
				setkey(*info->holdkeys, KEY_shoot);
			}
			goto gout;
		}
	}
	return 2;
	//return TRUE;
	gout:
	return FALSE;
	
	
	
	
}

Boolean stillChecksTank(const BrainInfo *info, int idnum) {

	if (info->shells < 10) {
		return TRUE;
	}
	if (info->armour < 4) {
		return TRUE;
	}
	return FALSE;
}

