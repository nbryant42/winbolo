#ifdef WIN32
	#include "brain_win.h"
#else
	/* brain.h -- needed to interact with linbolo */
	#include "brain.h"
#endif
#include <stdlib.h>
#include <stdio.h>
#include "getman.h"
#include "actionqueue.h"
#include "pointtank.h"
#include "travelto.h"
#include <math.h>




WORLD_X targetx;
WORLD_Y targety;

int getpriorityGetMan(const BrainInfo *info, int idnum) {
	unsigned int pri;
	#ifdef SHOWPRI
		extern int getmanpri;
	#endif
	
	if (info->man_status == 0 || info->man_status == 1) {
		#ifdef SHOWPRI
			getmanpri = 100000;
		#endif	
		return 100000;
	}
	//pri = (int) hypot(abs((info->man_x >> 8) - (info->tankx >> 8)), abs((info->man_y >> 8) - (info->tanky >> 8)));
	
	//pri += (pri * 2) + 0;// + 100;
	if (info->man_x == 0 || info->man_y == 0) {
		return 100000;
	}
	
	pri = GETMAN_PRI;
	
	#ifdef SHOWPRI
		getmanpri = GETMAN_PRI;
	#endif
	return pri;
	
}


Boolean startGetMan(const BrainInfo *info, int idnum) {
	targetx = info->man_x;
	targety = info->man_y;
	printf("Start Get of Man: tx: %d, ty: %d, %d, %d\n", targetx, targety, info->tankx, info->tanky);

	if (targetx == 0 || targety == 0) {
		return FALSE;
	}
	if (!getPath(info, targetx >> 8, targety >> 8, info->tankx >> 8, info->tanky >> 8, 0, 0, 0, 0)) {
		printf("Danger -- No Path\n");
		return FALSE;
	}
	printf("Got path\n");
	return TRUE;
}



Boolean middleGetMan(const BrainInfo *info, int idnum) {
	extern int speed;
	if (info->man_status == 0 || info->man_status == 1) {
		return TRUE;
	}
	targetx = info->man_x;
	targety = info->man_y;
	if (info->tankx >> 8 == targetx >> 8 && info->tanky >> 8 == targety >> 8) {
		pointTank(info, info->man_x, info->man_y);
		speed = 10;
		return FALSE;
	}
	if (doTravel(info, targetx >> 8, targety >> 8, TRUE, FALSE)) {
		printf("were her\n");
		return TRUE;
	} else {
		printf("Travelin\n");
		return FALSE;
	}
}






Boolean endGetMan(const BrainInfo *info, int idnum) {
	return 2;
}





Boolean addGetMan(const BrainInfo *info, int idnum) {
	extern actionqueue nextaction;
	actionqueue_insert(info, &nextaction, &startGetMan, &middleGetMan, &endGetMan, &getpriorityGetMan, idnum);
	return TRUE;
}

