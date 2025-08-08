#ifdef WIN32
	#include "brain_win.h"
#else
	/* brain.h -- needed to interact with linbolo */
	#include "brain.h"
#endif
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "actionqueue.h"
#include "pointtank.h"
#include "travelto.h"
#include "scout.h"
#include "settings.h"


WORLD_X targetx, subtargetx;
WORLD_Y targety, subtargety;



int getpriorityScout(const BrainInfo *info, int idnum) {
	extern setting opts;
	#ifdef SHOWPRI
		extern int scoutpri;
	#endif
	
	
	if (!settings_getoptions(&opts, EXPLORER)) {
		#ifdef SHOWPRI
			scoutpri = 100000;
		#endif
		return 100000;
	}
	
	#ifdef SHOWPRI
		scoutpri = SCOUT_PRI;
	#endif
	return SCOUT_PRI;
}


Boolean startScout(const BrainInfo *info, int idnum) {
	int rndx, rndy, i;
	WORLD_X tempx;
	WORLD_Y tempy;
	Boolean broke;
	
	startover:
	printf("Start Scout\n");

	broke = FALSE;
	for (i=0;i < 1000;i++) {
		/*rndx = (int)(random() / 8388608);
		rndy = (int)(random() / 8388608);
		if (rndx < 5) {rndx = 5;}
		if (rndy < 5) {rndy = 5;}
		tempx = rndx << 8;
		tempy = rndy << 8;*/
		
		#ifdef WIN32
			srand(98432189);
			rndx = (int)rand();
			rndy = (int)rand();
		
		#else
			rndx = (int)random();
			rndy = (int)random();
		
		#endif
		rndx = rndx >> 18;
		rndy = rndy >> 18;
		//if (rndx < 4000) {rndx = 4001;}
		//if (rndy < 4000) {rndy = 4001;}
		tempx = info->tankx + ((rndx - 4000) * 2);
		tempy = info->tanky + ((rndy - 4000) * 2);
		// If the target it first picks is the wrong type of terrain, go somewhere else.
		if (GETSQUARE((u_short)tempy, tempx >> 8) == GRASS || 
		GETSQUARE((u_short)tempy, tempx >> 8) == ROAD || 
		GETSQUARE((u_short)tempy, tempx >> 8) == SWAMP || 
		GETSQUARE((u_short)tempy, tempx >> 8) == CRATER || 
		GETSQUARE((u_short)tempy, tempx >> 8) == RUBBLE/* || GETSQUARE((u_short)tempy, tempx >> 8) == TERRAIN_UNKNOWN*/) {
			broke = TRUE;
			//printf("land type: %i\n", GETSQUARE((u_short)tempy, tempx >> 8));
			//printf("x: %i, y: %i\n", tempx, tempy);
			break;
		}
	}

	if (!broke) {
		printf("No Broke!!!!\n");
	}
	targetx = tempx;
	targety = tempy;
	if (!getPath(info, targetx >> 8, targety >> 8, info->tankx >> 8, info->tanky >> 8, 0, 0, 0, 0)) {
		printf("Danger -- No Path\n");
		targetx = info->tankx - 2560;
		targety = info->tanky - 2560;
		if (!getPath(info, targetx >> 8, targety >> 8, info->tankx >> 8, info->tanky >> 8, 0, 0, 0, 0)) {
			printf("Danger -- No Path2\n");
			goto startover;
			return FALSE;
		}

	}
	return TRUE;
}

Boolean middleScout(const BrainInfo *info, int idnum) {
	return doTravel(info, targetx >> 8, targety >> 8, FALSE, FALSE);
}


Boolean endScout(const BrainInfo *info, int idnum) {
	
	return TRUE;
}

Boolean addScout(const BrainInfo *info) {
	extern actionqueue nextaction;
	actionqueue_insert(info, &nextaction, &startScout, &middleScout, &endScout, &getpriorityScout, 0);
	printf("Added scout action to queue.\n");
	return TRUE;
}

