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
#include "laypill.h"
#include "settings.h"
#include <math.h>



WORLD_X targetx, nsubtargetx;
WORLD_Y targety, nsubtargety;
int step = 0;
int substep = 0;
extern WORLD_X basex;
extern WORLD_Y basey;



time_t starttime, curtime;

int getBaseRating(const BrainInfo *info, MAP_X tgx, MAP_Y tgy) {
	int sx, sy, left, top, clearsquares;
	TERRAIN squaret;
	
	left = (targetx >> 8) - 7;
	top = (targety >> 8) - 7;
	
	clearsquares = 0;
	
	
	
	for (sy=0;sy < 14;sy++) {
		for (sx=0;sx < 14;sx++) {
			squaret = GETSQUARE(((top + sy) << 8), (left + sx));
			if (squaret == ROAD) {
				clearsquares += 5;
			} else if (squaret == FOREST) {
				clearsquares += 5;
			} else if (squaret == RUBBLE) {
				clearsquares += 1;
			} else if (squaret == GRASS) {
				clearsquares += 5;
			} else if (squaret == CRATER) {
				clearsquares += 1;
			} else if (squaret == SWAMP) {
				clearsquares += 1;
			}

		}
	}
	return clearsquares;
	
	
}

int getpriorityLay(const BrainInfo *info, int idnum) {
	unsigned int pri;
	extern setting opts;
	extern ObjectInfo bases[16];
	int i;
	
	#ifdef SHOWPRI
		extern int laypillpri;
	#endif
	
	if (step != 0) {
		#ifdef SHOWPRI
			laypillpri = -10;
		#endif
		return -2000000;
	}
	
	if (basex == 0 && basey == 0) {
		return 100000;
	}

	if (info->carriedpills <= 0) {
		#ifdef SHOWPRI
			laypillpri = 100000;
		#endif
		return 100000;
	}
	if (info->trees < 4) {
		#ifdef SHOWPRI
			laypillpri = 100000;
		#endif
		return 100000;
	}
	if (!settings_getoptions(&opts, BUILD_PILLBOXES)) {
		#ifdef SHOWPRI
			laypillpri = 100000;
		#endif
		return 100000;
	}
	if (info->man_status != 0) {
		#ifdef SHOWPRI
			laypillpri = 100000;
		#endif
		return 100000;
	}
	for (i=0;i < 16;i++) {
		if (bases[i].idnum != 20) {
			goto found;
		}
	}
	#ifdef SHOWPRI
		laypillpri = 100000;
	#endif
	return 100000;
	
	
	found:
	pri = LAYPILL_PRI;
	#ifdef SHOWPRI
		laypillpri = pri;
	#endif

	
	return pri;
	
}



Boolean pickbase(const BrainInfo *info) {
	extern ObjectInfo bases[16];
	extern ObjectInfo pillboxes[16];
	
	int i, ndist, odist, j;
	odist = -100000000;
	for (i=0;i < 16;i++) {
		odist = getBaseRating(info, basex >> 8, basey >> 8);
		if (bases[i].idnum != 20) {
			Boolean goodbase = TRUE;
			ndist = getBaseRating(info, bases[i].x >> 8, bases[i].y >> 8);
			if (basex == 0) {
				odist = -100000000;
			}
			
			for (j=0;j < 16;j++) {
				if (pillboxes[j].idnum != 20 && pillboxes[j].direction != 0) {
					if (pillboxes[j].info & OBJECT_HOSTILE || pillboxes[j].info & OBJECT_NEUTRAL) {
						if ((int) hypot(abs((pillboxes[j].x >> 8) - (bases[i].x >> 8)), abs((pillboxes[j].y >> 8) - (bases[i].y >> 8))) < 11) {
							//if (clearPath(info, pillboxes[j].x, pillboxes[j].y, bases[i].x, bases[i].y, FALSE, FALSE)) {
								goodbase = FALSE;
								//printf("Pills\n");
								ndist -= 10000000;
							//}
						}
					} else {
						/* This is so that we will pick the base that already has *
						*  some of our pillboxes on it. */
						
						if ((int) hypot(abs((pillboxes[j].x >> 8) - (bases[i].x >> 8)), abs((pillboxes[j].y >> 8) - (bases[i].y >> 8))) < 11) {
							ndist *= 2;
						}
					}
				}
			}

			if (goodbase) {				
				if (ndist >= odist) {
					basex = bases[i].x;
					basey = bases[i].y;
					odist = ndist;
				}
			}
			
			
			//break;
		}
	}
	return TRUE;
}


Boolean startLay(const BrainInfo *info, int idnum) {
	
	
	/* if the basex choordinate == 0, then we know that we have to *
	* set our base.  Now this can be a very tricky thing to do, and*
	* there will be a way to move it at a later point in life.     */
	//if (basex == 0) {
		/* TODO: improve the base choosing algo.  */
	extern int speed;
	pickbase(info);
	
	targetx = basex;
	targety = basey;
	
	printf("Start lay\n");
	step = 0;
	printf("targetx: %d %d, targety: %d %d\n", targetx, targetx >> 8, targety, targety >> 8);
	if (targetx == 0 || targety == 0) {
		printf("Not valid\n");
		speed = 0;
		return FALSE;
	}
	if (!getPath(info, targetx >> 8, targety >> 8, info->tankx >> 8, info->tanky >> 8, 0, 0, 0, 0)) {
		printf("Danger -- No Path\n");
		return FALSE;
	}
	printf("MiddleLay\n");
	return TRUE;
	
	
	
	
	//return FALSE;
}


Boolean middleLay(const BrainInfo *info, int idnum) {
	if ((int)sqrt(((info->tankx - targetx) * (info->tankx - targetx)) + ((info->tanky - targety) * (info->tanky - targety))) < 2560) {
		return TRUE;
	}
	return doTravel(info, targetx >> 8, targety >> 8, TRUE, FALSE);
}


int getShootCount(const BrainInfo *info, MAP_X tgx, MAP_Y tgy) {
	
	
	int sx, sy, left, top, clearsquares;
	
	left = (targetx >> 8) - 7;
	top = (targety >> 8) - 7;
	
	clearsquares = 0;
	
	
	
	for (sy=0;sy < 14;sy++) {
		for (sx=0;sx < 14;sx++) {
			if (clearPath(info, (tgx << 8), (tgy << 8), ((left + sx) << 8), ((top + sy) << 8), FALSE, FALSE)) {
				clearsquares++;
			}
		}
	}
	return clearsquares;
	
}

Boolean endLay(const BrainInfo *info, int idnum) {
	//extern ObjectInfo pillboxes[16];
	extern int speed;
	int rndx, rndy, i;
	TERRAIN squarety;
	extern ObjectInfo pillboxes[16];
	extern ObjectInfo bases[16];

	
	speed = 0;
	
	/* Find a spot bulid.  This algo will definately improve over *
	* time, but for now this is going to be fairly simple.  */
	
	/* Ok, here is a duck tape solution for now... */
//	if (info->man_status == 1) {
//		return 2;
//	}
	if (step == 0) {
		
		int sx, sy, left, top, highestx, highesty, highestcount, tempshootcount, pilldist;
		
		left = (targetx >> 8) - 7;
		top = (targety >> 8) - 7;
		highestcount = -10000;
		highestx = 0;
		highesty = 0;
		
		printf("Startendlay\n");
		
		for (sy=0;sy < 14;sy++) {
			for (sx=0;sx < 14;sx++) {
				squarety = GETSQUARE(((sy + top) << 8), ((sx + left)));
				if (squarety == GRASS || squarety == SWAMP || squarety == CRATER || squarety == RUBBLE || squarety == ROAD || squarety == FOREST) {
					//printf("Check\n");
					tempshootcount = getShootCount(info, left + sx, top + sy);
					for (i=0;i < 16;i++) {
						if (pillboxes[i].idnum != 20) {
							if ((pilldist = (int)hypot(abs((pillboxes[i].x >> 8) - (left + sx)), abs((pillboxes[i].y >> 8) - (top + sy)))) < 7) {
								//printf("pd: %d\n", (abs(7 - pilldist) * 8));
								if (pilldist == 0) {
									printf("ThePILLLLLL\n");
									tempshootcount -= 3000;
								}
								tempshootcount -= (abs(7 - pilldist) * 8);
							}
						}
					}
					for (i=0;i < 16;i++) {
						if ((left + sx) == (bases[i].x >> 8) && (top + sy) == (bases[i].y >> 8)) {
							tempshootcount -= 3000;
						}
					}
					for (i=0;i < info->num_objects;i++) {
						if (info->objects[i].object == OBJECT_TANK) {
							if ((pilldist = (int)hypot(abs((info->objects[i].x >> 8) - (left + sx)), abs((info->objects[i].y >> 8) - (top + sy)))) < 1) {
								//printf("pd: %d\n", (abs(7 - pilldist) * 8));
								tempshootcount -= 3000;
							}
						}
					}
					if ((pilldist = (int)hypot(abs((info->tankx >> 8) - (left + sx)), abs((info->tanky >> 8) - (top + sy)))) < 1) {
								//printf("pd: %d\n", (abs(7 - pilldist) * 8));
								tempshootcount -= 3000;
							}
					if (tempshootcount > highestcount) {
						highestcount = tempshootcount;
						highestx = sx;
						highesty = sy;
					}
				}
			}
		}
		
		rndx = highestx + left;
		printf("hi: %d, top: %d\n", highesty, top);
		rndy = highesty + top;
		printf("Got: base: %d, %d, tar: %d, %d\n", targetx >> 8, targety >> 8, rndx, rndy);
		
		
		
		
		
		
		
		/*for (i=0;i < 500;i++) {
			
			
			#ifdef WIN32
				srand(58941548);
				rndx = (int)rand();
				rndy = (int)rand();
			#else
				rndx = (int)random();
				rndy = (int)random();
			#endif
			
			rndx = (rndx / 214748364) - 4;
			rndy = (rndy / 214748364) - 4;
			rndx = (rndx == 0) ? 3 : rndx;
			rndy = (rndy == 0) ? 3 : rndy;
			printf("rndx: %d, rndy: %d\n", rndx, rndy);
			squarety = GETSQUARE((u_short)((targety) + (rndy << 8)), ((targetx >> 8) + rndx));
			if (squarety == GRASS || squarety == SWAMP || squarety == CRATER || squarety == RUBBLE) {
				break;
			}
		}*/
		nsubtargetx = rndx << 8;//(targetx + (rndx << 8));
		nsubtargety = rndy << 8;//(targety + (rndy << 8));
		step = 1;
		printf("tx: %d, ty: %d, rx: %d, ry: %d, sx: %d, sy: %d\n", targetx >> 8, targety >> 8, rndx, rndy, nsubtargetx >> 8, nsubtargety >> 8);
		if (!getPath(info, nsubtargetx >> 8, nsubtargety >> 8, info->tankx >> 8, info->tanky >> 8, 0, 0, 0, 0)) {
			printf("Danger -- No Path\n");
		}
		substep = 0;
		//*info->holdkeys = 0;
		//speed = 0;
	} else if (step == 1) {
		//printf("Step1\n");
		pointTank(info, nsubtargetx, nsubtargety);
		if ((int)hypot(abs((info->tankx) - (nsubtargetx) - 128), abs((info->tanky) - (nsubtargety) - 128)) < 256) {
			step = 2;
			substep = 0;
			if (info->inboat) {
				speed = 64;
			} else {
				speed = 15;
			}
		} else {
			if (info->inboat) {
				speed = 64;
			} else {
				speed = 15;
			}
		}
		if (substep == 0) {
			if (doTravel(info, nsubtargetx >> 8, nsubtargety >> 8, TRUE, TRUE)) {
				substep = 1;
			}
		}
		//if ((int)sqrt(((info->tankx - nsubtargetx) * (info->tankx - nsubtargetx)) + ((info->tanky - nsubtargety) * (info->tanky - nsubtargety))) < 512) {
		//	speed = 15;
		//}

	} else if (step == 2) {
		//printf("Step2\n");
		pointTank(info, nsubtargetx, nsubtargety);
		info->build->y = nsubtargety >> 8;
		info->build->x = nsubtargetx >> 8;
		info->build->action = BUILDMODE_PBOX;
		printf("tkx: %d, tky: %d, sx: %d, sy: %d\n", info->tankx >> 8, info->tanky >> 8, nsubtargetx >> 8, nsubtargety >> 8);
		step = 3;
		*info->holdkeys = 0;
		speed = 15;
		time(&starttime);
	} else {
		*info->holdkeys = 0;
		pointTank(info, nsubtargetx, nsubtargety);
		speed = 15;
		time(&curtime);
		if (curtime >= starttime + 3) {
			step = 0;
			return 2;
		}
		if (info->man_status == 0) {
			if (info->carriedpills == 0) {
				step = 0;
				return 2;
			} else {
				step = 0;
			}
			
		}
		//step = 0;
		//return 2;
	}
	return FALSE;
}


Boolean addLay(const BrainInfo *info, int idnum) {
	extern actionqueue nextaction;
	actionqueue_insert(info, &nextaction, &startLay, &middleLay, &endLay, &getpriorityLay, idnum);
	printf("Added lay action to queue.\n");
	return TRUE;
}

