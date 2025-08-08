

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
#include "gettrees.h"
#include <math.h>




WORLD_X targetx;
WORLD_Y targety;

int getpriorityGetTrees(const BrainInfo *info, int idnum) {
	
	if (info->man_status != 0) {
		return 100000;
	}

	if ((info->carriedpills > 0 && info->trees < 15) || (info->trees < 10)) {
		int x,y, left, top;
		
		left = (info->tankx >> 8) - 8;
		top = (info->tanky >> 8) - 8;
		for (y=0;y < 16;y++) {
			for (x=0;x < 16;x++) {
				if (GETSQUARE(((top + y) << 8), (left + x)) == FOREST) {
					goto found;
				}
			}
		}
		return 100000;
		
		found:
		return GETTREES_PRI;
	} else {
		return 100000;
	}
	
}


Boolean startGetTrees(const BrainInfo *info, int idnum) {
	extern ObjectInfo pillboxes[16];
	int x,y,i, treedist, oldtreedist, left, top, pilldist;
	
	printf("Start Get Tree\n");
	left = (info->tankx >> 8) - 8;
	top = (info->tanky >> 8) - 8;
	oldtreedist = 1000000;
	for (y=0;y < 16;y++) {
		for (x=0;x < 16;x++) {
			if (GETSQUARE(((top + y) << 8), (left + x)) == FOREST) {
				treedist = (int) hypot(abs((left + x) - (info->tankx >> 8)), abs((top + y) - (info->tanky >> 8)));
				for (i=0;i < 16;i++) {
					if (pillboxes[i].idnum != 20) {
						if (clearPath(info, pillboxes[i].x, pillboxes[i].y, (left + x) << 8, (top + y) << 8, TRUE, FALSE)) {
							if ((pilldist = (int)hypot(abs((pillboxes[i].x >> 8) - (left + x)), abs((pillboxes[i].y >> 8) - (top + y)))) < 7) {
								treedist += (abs(7 - pilldist) * 8);
							}
						}
					}
				}
				if (treedist < oldtreedist) {
					oldtreedist = treedist;
					targetx = (left + x) << 8;
					targety = (top + y) << 8;
				}
			}
		}
	}
	
	
	if (!getPath(info, targetx >> 8, targety >> 8, info->tankx >> 8, info->tanky >> 8, 0, 0, 0, 0)) {
		printf("Danger -- No Path\n");
		return FALSE;
	}
	
	return TRUE;
}



Boolean middleGetTrees(const BrainInfo *info, int idnum) {
	if (doTravel(info, targetx >> 8, targety >> 8, TRUE, FALSE)) {
		return TRUE;
	} else {
		return FALSE;
	}
}






Boolean endGetTrees(const BrainInfo *info, int idnum) {
	info->build->y = targety >> 8;
	info->build->x = targetx >> 8;
	info->build->action = BUILDMODE_FARM;
	
	return 2;

}





Boolean addGetTrees(const BrainInfo *info, int idnum) {
	extern actionqueue nextaction;
	actionqueue_insert(info, &nextaction, &startGetTrees, &middleGetTrees, &endGetTrees, &getpriorityGetTrees, idnum);
	return TRUE;
}

