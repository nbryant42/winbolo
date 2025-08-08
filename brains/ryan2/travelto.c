#ifdef WIN32
	#include "brain_win.h"
#else
	/* brain.h -- needed to interact with linbolo */
	#include "brain.h"
#endif
#include "travelto.h"
#include "pointtank.h"
#include "pqueue.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "settings.h"
#include "actionqueue.h" // must include after any system headers that define printf

WORLD_X subtargetx, oldx, lastpositionx, lastshootx, lastbuildx = 0;
WORLD_Y subtargety, oldy, lastpositiony, lastshooty, lastbuildy = 0;
int iteration;
int subit = 0;
int lastspeed = 0;
Boolean lastshoot = FALSE;
int subtickcount = 0;
time_t starttime, curtime;
Boolean moving = FALSE;
Boolean all_stuck = FALSE;
extern setting opts;


/* mopen is our open list used for the A* algorithum. */
PQueue mopen;


points *pts = NULL;


/* the xarray and yarray are the locations of the surrounding squares to with relation *
* to the current square. */
const int xarray[] = {0, -1, 1, 0};
const int yarray[] = {-1, 0, 0, 1};


const int BORDERX[8] = {-1, 0, 1, -1, 0, -1, 0, 1};
const int BORDERY[8] = {-1, -1, -1, 0, 0, 1, 1, 1};




typedef struct {
	MAP_X x;
	MAP_Y y;
	void *parent;
	int cost; /* How much it costs to costs to cross this square */
	int totalcost; /* How much it costs to get to this square from the beginning parent->totalcost + cost */
	int f, h; /* used in a* algo */
	Boolean inwater; /* True - the square is water of some sort. */
	Boolean inboat; /* True - the path take has crossed over a boat, and thus we would be in a boat when we get to this square */
	int boatcost;
	Boolean closed; /* See docs on A*, this would usually be a linked list called closed, but in this case we can simplify it. */
	int shell; /* how many shells we have when we get to this square */
	int shellcost; /* how much shells it will take to cross (or shoot throught) this square. */
	int armour; /* How much armour we have at this square */
	BYTE armourcost; /* how much armour we will loose by crossing this square. */
	TERRAIN terrain;
} square;

square *map;



/* compare_int is used in the A* algorithm.  It helps what item has the lowest priority */
static int compare_int(const void *int1, const void *int2) {

	square *tmp1 = (square *)int1;
	square *tmp2 = (square *)int2;
	if (tmp1->f < tmp2->f) {
		return 1;
	} else if (tmp1->f > tmp2->f) {
		return -1;
	} else {
		return 0;
	}

}









Boolean clearPath(const BrainInfo *info, WORLD_X x1, WORLD_Y y1, WORLD_X x2, WORLD_Y y2, Boolean blocktrees, Boolean fromtank) {
	/* This is a fairly complex function that figures out of you can shoot from (startx, starty) to *
	* (endx, endy)  fromtank is if you are shooting from the tank...*/
	int x, y, left, top;


	int a, b, c, eq1a, eq1b, eq2a, eq2b, eq4a, eq4b, finx, finy;
	double eq1ang, eq2ang, eq3ang, d, eq4ang;
	
	//printf("Called Clear Path\n");
	if (x1 <= x2) {
		left = x1 >> 8;
	} else {
		left = x2 >> 8;
	}
	/* Now lets do the same for the top */
	if (y1 <= y2) {
		top = y1 >> 8;
	} else {
		top = y2 >> 8;
	}
	
	//printf("Startclear\n");
	
	for (y=0;y < abs((y1 >> 8) - (y2 >> 8));y++) {
		for (x=0;x < abs((x1 >> 8) - (x2 >> 8));x++) {
			int worldType;
			
			/* Ok, first lets find the distance on all of the sides  We do this with the *
			* distance formula.                                                          *
			*  d = sqrt((x1 - x2)^2 + (y1 + y2)^2) */
			worldType = GETSQUARE((u_short)((y + top) << 8), (x + left));

			worldType = worldType & TERRAIN_MASK;
			//printf("Clear Path\n");
			if (((info->inboat && fromtank) && (worldType == CRATER || worldType == RUBBLE || worldType == GRASS || worldType == ROAD || worldType == SWAMP || worldType == BUILDING || (blocktrees && worldType == FOREST) || worldType == HALFBUILDING)) || 
			(worldType == BUILDING || (blocktrees && worldType == FOREST) || worldType == HALFBUILDING )) {
			//printf("COol\n");
				a = (int)sqrt(((x1 - ((x + left) << 8) + 128) * (x1 - ((x + left) << 8) + 128)) + ((y1 - ((y + top) << 8) + 128) * (y1 - ((y + top) << 8) + 128)));
				b = (int)sqrt((((((x + left) << 8) + 128) - x2) * ((((x + left) << 8) + 128) - x2)) + (((((y + top) << 8) + 128) - y2) * ((((y + top) << 8) + 128) - y2)));
				c = (int)sqrt(((x1 - x2) * (x1 - x2)) + ((y1 - y2) * (y1 - y2)));
				
				eq1a = y2 - y1;
				eq1b = x2 - x1;
				
				eq1ang = atan(((double)eq1a / (double)eq1b));
				
				eq2a = (((y + top) << 8) + 128) - y1;
				eq2b = (((x + left) << 8) + 128) - x1;
				
				eq2ang = atan(((double)eq2a / (double)eq2b));
				
				eq3ang = eq2ang - eq1ang;
				
				d = (((double)sin(eq3ang)) * (double)eq2a);
				
				
				eq4ang = 90 - eq3ang - (90 - eq2ang);
				
				eq4a = (int)((double)d * cos(eq4ang));
				eq4b = (int)((double)d * sin(eq4ang));
				
				
				finx = (((x + left) << 8) + 128) - eq4b;
				finy = (((y + top) << 8) + 128) - eq4a;
				
				//d = asin((double)x * (double)y);
				
				if ((finx >> 8) == (((x + left) << 8) >> 8) && ((finy >> 8) == (((y + top) << 8) >> 8))) {
					//printf("return FALSE\n");
					return FALSE;
				}
				//printf("a: %d, b: %d, c: %d, d: %d\n", (((x + left) << 8) + 128), (((y + top) << 8) + 128), finx, finy);
				
			}
			
		}
	}
	
	
	
	//printf("Return TRUE!\n");
	return TRUE;
}






Boolean squareisbase(MAP_X x, MAP_Y y) {
	int i;
	extern ObjectInfo bases[16];
	for (i=0;i < 16;i++) {
		if (bases[i].x >> 8 == x && bases[i].y >> 8 == y && bases[i].info & OBJECT_HOSTILE) {
			return TRUE;
		}
	}
	return FALSE;
}


Boolean isLand(TERRAIN landtype) {
	if (landtype == BUILDING ||
	landtype == HALFBUILDING ||
	landtype == CRATER ||
	landtype == ROAD ||
	landtype == FOREST ||
	landtype == RUBBLE ||
	landtype == GRASS ||
	landtype == REFBASE_T ||
	landtype == PILLBOX_T) {
		return TRUE;
	} else {
		return FALSE;
	}
}

Boolean landAround(const BrainInfo *info) {
	if ((isLand((TERRAIN) GETSQUARE((info->tanky + (1 << 8)), ((info->tankx >> 8) + 0))) ||
	isLand((TERRAIN) GETSQUARE((info->tanky + (1 << 8)), ((info->tankx >> 8) - 1)) ||
	isLand((TERRAIN) GETSQUARE((info->tanky + (1 << 8)), ((info->tankx >> 8) + 1))) ||
	isLand((TERRAIN) GETSQUARE((info->tanky + (0 << 8)), ((info->tankx >> 8) + 1))) ||
	isLand((TERRAIN) GETSQUARE((info->tanky + (0 << 8)), ((info->tankx >> 8) - 1))) ||
	isLand((TERRAIN) GETSQUARE((info->tanky - (1 << 8)), ((info->tankx >> 8) + 0))) ||
	isLand((TERRAIN) GETSQUARE((info->tanky - (1 << 8)), ((info->tankx >> 8) + 1))) ||
	isLand((TERRAIN) GETSQUARE((info->tanky - (1 << 8)), ((info->tankx >> 8) - 1)))))) {
		return TRUE;
	} else {
		return FALSE;
	}
	
}







Boolean doTravel(const BrainInfo *info, MAP_X targetx, MAP_Y targety, Boolean stopatend, Boolean stoptwofromend) {
	int squarebuffer, i;
	extern int speed;
	int pointat;
	Boolean shellsinair;
	Boolean pointedatwall;
	Boolean lastinboat = FALSE;
	
	squarebuffer = 340;
	speed = 64;
	

	subtargetx = pts[iteration].x;
	subtargety = pts[iteration].y;
	
	//printf("%d\t%d\n%d\t%d\n", subtargetx >> 8, subtargety >> 8, info->tankx >> 8, info->tanky >> 8);
	
	shellsinair = FALSE;
	
	for (i=0;i < info->num_objects;i++) {
		if (info->objects[i].object == OBJECT_SHOT) {
			shellsinair = TRUE;
			break;
		}
	}

	
	//printf("tgx: %d, tgy: %d  itx: %d, ity: %d\n", targetx, targety, info->tankx, info->tanky);

	pointTank(info, subtargetx, subtargety);
	if (pts[iteration].x == 0 && pts[iteration].y == 0) {
		printf("Were there\n");
		return TRUE;
	}


	
	//printf("%d -- %d\n", pts[iteration].x >> 8, pts[iteration].y >> 8);
	
	
	/* ------------------ Shoot Logic ------------*
	* if the next square we are supposed to go to *
	* is some sort of wall, then the .shoot option*
	* will be turned on and we should shoot       *
	* through it.                                 */
	pointat = aim(info, subtargetx, subtargety);
	if (lastshoot) {
		//printf("Wall! pa: %d  id: %d\n", pointat, info->direction);
		pointedatwall = TRUE;
		if (abs(pointat - info->direction) > 127) {
			if (pointat > info->direction) {
				if ((abs(pointat - 256) + info->direction) > 10) {
					pointedatwall = FALSE;
				}
			} else {
				if ((abs(info->direction - 256) + pointat) > 10) {
					pointedatwall = FALSE;					
				}
			}
		} else if (abs(pointat - info->direction) > 10) {
			pointedatwall = FALSE;
		}
		if (pointat == info->direction) {
			pointedatwall = TRUE;
		}
	
		//printf("yl: %d %d il: %d %d\n", info->tankx >> 8, info->tanky >> 8, lastshootx >> 8, lastshooty >> 8);
		if (/*!info->reload && */pointedatwall) {
			if (GETSQUARE((u_short)lastshooty, (lastshootx >> 8)) == BUILDING || GETSQUARE((u_short)lastshooty, (lastshootx >> 8)) == HALFBUILDING || GETSQUARE((u_short)lastshooty, (lastshootx >> 8)) == TERRAIN_UNKNOWN || squareisbase(lastshootx >> 8, lastshooty >> 8)) {
				setkey(*info->tapkeys, KEY_shoot);
			}
		}
		
		
		
		
		/*int ntx, nty, d;
		ntx = lastshootx - info->tankx;
		nty = lastshooty - info->tanky;
		d = (ntx * sin(info->direction) - nty * cos(info->direction));
		ntx -= (d * sin(info->direction));
		nty += (d * cos(info->direction));
		printf("d: %d, sqrt: %d\n", d, (int)sqrt((ntx*ntx) + (nty*nty)));
		if (!info->reload && ((*//*d > 0 && d > 2560 &&*//* ((int)sqrt((ntx*ntx) + (nty*nty))) < 512))) {
			
			
			//printf("11: %d  x: %d, y: %d\n", info->theWorld[(u_short)(lastshooty | (lastshootx >> 8))], lastshooty >> 8, lastshootx >> 8);
			*//*if (info->theWorld[(u_short)lastshooty | (lastshootx >> 8)] == BUILDING || info->theWorld[(u_short)lastshooty | (lastshootx >> 8)] == HALFBUILDING || info->theWorld[(u_short)lastshooty | (lastshootx >> 8)] == TERRAIN_UNKNOWN || squareisbase(lastshootx >> 8, lastshooty >> 8)) {
				setkey(*info->tapkeys, KEY_shoot);
			}
		}*/
	}
	
	
	if ((iteration > 0 && pts[iteration - 1].shoot)) {
		lastshoot = TRUE;
		lastshootx = pts[iteration - 1].x;
		lastshooty = pts[iteration - 1].y;
	} else if (iteration == 0 && pts[iteration].shoot) {
		lastshoot = TRUE;
		lastshootx = pts[iteration].x;
		lastshooty = pts[iteration].y;
	} else if ((!(pts[iteration + 1].x == 0 && pts[iteration + 1].y == 0)) && pts[iteration + 1].shoot) {
		lastshoot = TRUE;
	} else {
		lastshoot = FALSE;
	}

	



	
	if (pts[iteration + 1].inboat) {
		if (!landAround(info)) {
			speed = 64;
		} else {
			squarebuffer = 30;
			speed = 50;
		}
		
	}
	
	/* ___________ ADDED_________________*/
	 else {
	 	//if (pts[iteration + 1].
	 	if ((info->armour < 2) && (!info->newtank) && info->trees > 20 && (pts[iteration].x != 0 && pts[iteration].y != 0) && (!shellsinair) && (!info->inboat) && info->man_status == 0 && (pts[iteration].terrain == RIVER || pts[iteration].terrain == RUBBLE || pts[iteration].terrain == CRATER /*|| pts[iteration].terrain == SWAMP*/)) {
	 		//if ((int)hypot(abs(info->tankx - pts[iteration].x), abs(info->tanky - pts[iteration].y)) < 255) {
				if (lastbuildx != pts[iteration].x && lastbuildy != pts[iteration].y && settings_getoptions(&opts, BUILD_ROADS)) {
					printf("build: %d, %d\n", pts[iteration].x, pts[iteration].y);
					info->build->y = pts[iteration].y >> 8;
					info->build->x = pts[iteration].x >> 8;
					info->build->action = BUILDMODE_ROAD;
					lastbuildx = pts[iteration].x;
					lastbuildy = pts[iteration].y;
				}
			//}
			printf("Done build\n");
		}

	 }
	 /* ___________ENDADDED_______________*/


	
	
	
	
	/*if (!info->inboat && pts[iteration].inboat) {
		int tdist;
		squarebuffer = 30;
		//speed = 20;
		tdist = (int)hypot(abs((info->tankx) - (pts[iteration].x)), abs((info->tanky) - (pts[iteration].y)));
		//printf("tdist: %d\n", tdist);
		if (tdist < 256) {
			//printf("Speed: 20\n");
			speed = 20;
		}
		
	}*/
	
	
	
	// ___REVMOE TRUE !!!!!!!!!!!!!!!!
	if (!shellsinair) {
		int slowdist;
		
//		printf("pa: %d  id: %d\n", pointat, info->direction);
		if (info->inboat || pts[iteration].inboat) {
			slowdist = 3;
		} else {
			slowdist = 40;
		}
		if (abs(pointat - info->direction) > 127) {
			if (pointat > info->direction) {
				if ((abs(pointat - 256) + info->direction) > slowdist) {
					//printf("S=0; is=%d\n", info->speed);
					if (info->inboat && !landAround(info)) {
						speed = 64;
					} else {
						speed = 0;
					}
				}
			} else {
				if ((abs(info->direction - 256) + pointat) > slowdist) {
					//printf("S=0; is=%d\n", info->speed);
					if (info->inboat && !landAround(info)) {
						speed = 64;
					} else {
						speed = 0;
					}
				}
			}
		} else if (abs(pointat - info->direction) > slowdist) {
			//printf("S=0; is=%d\n", info->speed);
			if (info->inboat && !landAround(info)) {
				speed = 64;
			} else {
				speed = 0;
			}
		}
	}
	


	if (stoptwofromend) {
		if (pts[iteration + 2].x == 0 && pts[iteration + 2].y == 0) {
			//printf("Stop!!\n");
			squarebuffer = 50;
			if (speed > 20) {
				speed = 20;
			}
			if (pts[iteration].shoot) {
				lastshoot = TRUE;
				lastshootx = pts[iteration].x;
				lastshooty = pts[iteration].y;
			} else {
				//lastshoot = FALSE;
			}
		}
	} else if (stopatend) {
		if (pts[iteration + 1].x == 0 && pts[iteration + 1].y == 0) {
			//printf("Stop!!\n");
			squarebuffer = 50;
			if (speed > 20) {
				speed = 20;
			}
			if (pts[iteration].shoot) {
				lastshoot = TRUE;
				lastshootx = pts[iteration].x;
				lastshooty = pts[iteration].y;
			} else {
				//lastshoot = FALSE;
			}
		}
	}


	
	if (pts[iteration + 1].shoot) {
		squarebuffer = 100;
	}
	
	if (info->inboat && !pts[iteration + 1].inboat) {
		//printf("Dock\n");
		speed = 64;
	}


	
	
	if (lastpositionx == info->tankx && lastpositiony == info->tanky) {
		//printf("Lastpointused\n");
		lastspeed += 5;
		speed = lastspeed;
	} else {
		lastspeed = speed;
	}
	lastpositionx = info->tankx;
	lastpositiony = info->tanky;
	
	
	if (all_stuck) {
		squarebuffer = 10;
	}

	
	//printf("Squarebuffer: %d -- Speed: %d\n", squarebuffer, speed);
	
	/* The following code needs to be last, it makes
	* sure that if the tank looses it's boat, it 
	* realizes it.  START:*/
	if ((!info->inboat)/* && lastinboat*/ && pts[iteration].inboat) {
		//printf("RESET\n");
		subit = 10;
	}
	
	if (pts[iteration].inboat) {
		lastinboat = TRUE;
	} else {
		lastinboat = FALSE;
	}
	/* END */

	for (i=0;i < info->num_objects;i++) {
		if (info->objects[i].object == OBJECT_TANK) {
			if ((int) hypot(abs((info->objects[i].x >> 8) - (info->tankx >> 8)), abs((info->objects[i].y >> 8) - (info->tanky >> 8))) < 3) {
				subit = 5;
			}
		}
	}

	//printf("subtx: %d %d\n", subtargetx >> 8, subtargety >> 8);
	if (subtargetx + squarebuffer >= info->tankx && subtargetx - squarebuffer <= info->tankx && subtargety + squarebuffer >= info->tanky && subtargety - squarebuffer <= info->tanky) {		
		iteration++;
		subit++;
		if (subit >= 5) {
			getPath(info, targetx, targety, info->tankx >> 8, info->tanky >> 8, 0, 0, 0, 0);
			subit = 0;
		}
	}
	
	if (subtickcount > 100) {
		if (targetx != 0) {
			if (oldx == info->tankx && oldy == info->tanky) {
				if (moving) {
					all_stuck = FALSE;
					moving = FALSE;
					time(&starttime);
				} else {
					time(&curtime);
					if (curtime >= starttime + BOARDTIME) {
						extern int speed;
						extern Boolean (*endfunction)(const BrainInfo *info, int idnum);
						printf("Stuck!\n");
						all_stuck = TRUE;
						endfunction = NULL;
						speed = 64;
						
						*info->holdkeys = 0;
						setkey(*info->holdkeys, KEY_turnright);
					} else {
						//extern int speed;
						//speed = 64;
						//*info->holdkeys = 0;
						//setkey(*info->holdkeys, KEY_turnleft);
					}
				}
			} else {
				all_stuck = FALSE;
				moving = TRUE;
				oldx = info->tankx;
				oldy = info->tanky;
				//moving = TRUE;
			}
		}
		subtickcount = 0;
	}
	subtickcount++;
	
	if (all_stuck) {
		*info->holdkeys = 0;
		setkey(*info->holdkeys, KEY_turnright);
	}
	
	
	return FALSE;
}

Boolean getPath(const BrainInfo *info, MAP_X targetx, MAP_Y targety, MAP_X startx, MAP_Y starty, BYTE shells, BYTE armour, int steps, int lastx) {
		/********************** getPath ***************************************
		* the get path function is responsible for figuring out the cost to   *
		* get from startx, starty to targetx, targety.  It uses a mofified A* *
		* algo.  There will be mroe detaled comments as we go through the code*
		***********************************************************************/
		
	int height, width, top, left; /* The height and width are for *map, and top/left are for the submap */
	int a, b, c, y, x, position, j, i, l, tempsquare, squarecost/*, pleft, pright*/;
	square *temp;
	square *tmp;
	void *temp_void;
	extern ObjectInfo bases[16];
	extern ObjectInfo pillboxes[16];
	int fvalue, subtractfromf, v;
	
	
	

	/* If pts has stuff in it, we need to free it. */
	if (pts != NULL) {
		free(pts);
	}

	
	if (shells > info->shells) {
		shells = info->shells;
	}
	if (armour > info->armour) {
		armour = info->armour;
	}
	
	width = abs(targetx - startx) + SEARCHPAD;
	height = abs(targety - starty) + SEARCHPAD;
	
	map = malloc(sizeof(square) * (height * width));
	
	
	/* Lets get the location of the submap into the big map */
	if ((startx) <= targetx) {
		left = (startx) - (SEARCHPAD / 2);
	} else {
		left = targetx - (SEARCHPAD / 2);
	}
	/* Now lets do the same for the top */
	if ((starty) <= targety) {
		top = (starty) - (SEARCHPAD / 2);
	} else {
		top = targety - (SEARCHPAD / 2);
	}

	//printf("hei: %i, wid: %i, lef: %i, top: %i, stx: %i, sty: %i, tgx: %i, tgy: %i\n", height, width, left, top, startx, starty, targetx, targety);
	for (y=0;y < height;y++) {
		for (x=0;x < width;x++) {
			
			/* We'll start out by declaring a varable called position *
			* this keeps track of exactally what square we are in     *
			* within the array.                                       */
			position = (y * width) + x;
			
			
			/* Next we set the x and y of the square we are working on */
			map[position].x = x;
			map[position].y = y;
			
			/* Set f to 0 */
			map[position].f = 0;
			
			
			/* Now we get the type of terrain this square is.  Then we *
			* use that to find the cost and other stuff                */
			squarecost = GETSQUARE(((u_short)(y + top) << 8), (x + left));
			
			map[position].terrain = squarecost;
			if ((squarecost & 0xF0) != 0) {
				printf("ok....rethink\n");
			}
			
			if (squarecost == ROAD) {
				map[position].cost = 1;
				map[position].boatcost = 1000;
				map[position].shellcost = 0;
				map[position].armourcost = 0;
				map[position].inboat = FALSE;
			} else if (squarecost == GRASS) {
				map[position].cost = 2;
				map[position].boatcost = 1000;
				map[position].shellcost = 0;
				map[position].armourcost = 0;
				map[position].inboat = FALSE;
			} else if (squarecost == FOREST) {
				map[position].cost = 3;
				map[position].boatcost = 1000;
				map[position].shellcost = 0;
				map[position].armourcost = 0;
				map[position].inboat = FALSE;
			} else if (squarecost == BUILDING) {
				map[position].cost = 20;
				map[position].boatcost = 1000;
				map[position].shellcost = 5;
				map[position].armourcost = 0;
				map[position].inboat = FALSE;
			} else if (squarecost == HALFBUILDING) {
				map[position].cost = 10;
				map[position].boatcost = 1000;
				map[position].shellcost = 4;
				map[position].armourcost = 0;
				map[position].inboat = FALSE;
			} else if (squarecost == TERRAIN_UNKNOWN) {
				map[position].cost = 3;
				map[position].boatcost = 1000;
				map[position].shellcost = 0;
				map[position].armourcost = 0;
				map[position].inboat = FALSE;
			} else if (squarecost == BOAT) {
				map[position].cost = 1;
				map[position].boatcost = 1;
				map[position].shellcost = 0;
				map[position].armourcost = 0;
				map[position].inboat = TRUE;
				//map[position].inboat = FALSE;
			} else if (squarecost == RIVER) {
				map[position].cost = 30;
				map[position].boatcost = 1;
				map[position].shellcost = 0;
				map[position].armourcost = 1;
				map[position].inboat = FALSE;
			} else if (squarecost == DEEPSEA) {
				map[position].cost = 10000;
				map[position].boatcost = 1;
				map[position].shellcost = 0;
				map[position].armourcost = 0;
				map[position].inboat = FALSE;
			} else if (squarecost == SWAMP) {
				map[position].cost = 7;
				map[position].boatcost = 1000;
				map[position].shellcost = 0;
				map[position].armourcost = 0;
				map[position].inboat = FALSE;
			} else if (squarecost == RUBBLE) {
				map[position].cost = 7;
				map[position].boatcost = 1000;
				map[position].shellcost = 0;
				map[position].armourcost = 0;
				map[position].inboat = FALSE;
			} else if (squarecost == CRATER) {
				map[position].cost = 7;
				map[position].boatcost = 1000;
				map[position].shellcost = 0;
				map[position].armourcost = 0;
				map[position].inboat = FALSE;
			} else {
				map[position].cost = 4;
				map[position].boatcost = 1000;
				map[position].shellcost = 0;
				map[position].armourcost = 0;
				map[position].inboat = FALSE;
			}
			
			if (squarecost & TERRAIN_MINE) {
				map[position].cost += 4000;
			}
			if (squarecost & TERRAIN_PILL_VIS) {
				map[position].cost += 4000;
			}
			/* The next bit of code finds the h value.  It does this by         *
			* basically making a triangle with a and b being the discance       *
			* of the x and y distances, then we calculate the top hypotinuse    *
			* with pythagorean therum.  (However you spell it)  a^2 + b^2 = c^2*/
			a = abs((targetx - left) - x);
			b = abs((targety - top) - y);
			if (a == 0) {a = 1;}
			if (b == 0) {b = 1;}
			c = (int)sqrt((a * a) + (b * b));
			map[position].h = c;
			
			/* Finally, set closed to false */
			map[position].closed = FALSE;
			
			
		}
	}
	
	
	
	/* ----- Make sure it makes a turn instead of trying to go backwards --*
	* This makes sure that we don't go the the square behind us or to the   *
	* left or right. This can be done by upping the cost of the square      *
	* right behind it.                                                      */
	/*if (info->speed > 30) {
			x = (info->tankx >> 8) - left;
			y = (info->tanky >> 8) - top;
			if (info->direction < 31 && info->direction > 223) {
				position = (y * width) + (x - 1);
				pleft = ((y - 1) * width) + x;
				pright = ((y + 1) * width) + x;
			} else if (info->direction >= 31 && info->direction < 95) {
				position = ((y - 1) * width) + x;
				pleft = (y * width) + (x + 1);
				pright = (y * width) + (x - 1);
			} else if (info->direction >= 95 && info->direction < 159) {
				position = (y * width) + (x + 1);
				pleft = ((y + 1) * width) + x;
				pright = ((y - 1) * width) + x;
			} else {
				position = ((y + 1) * width) + x;
				pleft = (y * width) + (x + 1);
				pright = (y * width) + (x - 1);
			}
			map[position].cost = map[position].cost + 10;
			map[pleft].cost = map[pleft].cost + 10;
			map[pright].cost = map[pright].cost + 10;
			map[position].boatcost = 1000000;
			map[pleft].boatcost = 1000000;
			map[pright].boatcost = 1000000;
		}*/



	/*if (all_stuck) {
		if ((info->tankx >> 8) > left && (info->tankx >> 8) < (left + width) && (info->tanky >> 8) > top && (info->tanky >> 8) < (top + height)) {
			int x1,x2,x3,y1,y2,y3,pos1,pos2,pos3,x,y;
			if (info->direction < 31 && info->direction > 223) {
				x1 = 0;
				x2 = -1;
				x3 = 1;
				y1 = -1;
				y2 = -1;
				y3 = -1;
			} else if (info->direction >= 31 && info->direction < 95) {
				x1 = 1;
				x2 = 1;
				x3 = 1;
				y1 = 0;
				y2 = -1;
				y3 = 1;
			} else if (info->direction >= 95 && info->direction < 159) {
				x1 = 0;
				x2 = -1;
				x3 = 1;
				y1 = 1;
				y2 = 1;
				y3 = 1;
			} else {
				x1 = -1;
				x2 = -1;
				x3 = -1;
				y1 = 0;
				y2 = -1;
				y3 = 1;
			}
			x = (info->tankx >> 8) - left;
			y = (info->tanky >> 8) - top;

			printf("Unsticking\n");
			printf("tnk: %d %d, pos1: %d %d   %d\n", (info->tankx >> 8) - left, (info->tanky >> 8) - top, ((info->tankx >> 8) + x1 - left), ((info->tanky >> 8) + y1 - top), top);
			printf("tnk: %d %d, pos1: %d %d   %d\n", (info->tankx >> 8) - left, (info->tanky >> 8) - top, ((info->tankx >> 8) + x2 - left), ((info->tanky >> 8) + y2 - top), top);
			printf("tnk: %d %d, pos1: %d %d   %d\n", (info->tankx >> 8) - left, (info->tanky >> 8) - top, ((info->tankx >> 8) + x3 - left), ((info->tanky >> 8) + y3 - top), top);
			pos1 = abs((y + y1) * width) + (x + x1);
			pos2 = abs((y + y2) * width) + (x + x2);
			pos3 = abs((y - y3) * width) + (x + x3);
			printf("Pos1: %d  Pos2: %d  Pos3: %d\n", pos1, pos2, pos3);
			map[pos1].cost += 5000;
			map[pos1].boatcost += 5000;
			map[pos2].cost += 5000;
			map[pos2].boatcost += 5000;
			map[pos3].cost += 5000;
			map[pos3].boatcost += 5000;
			//all_stuck = FALSE;
			
		}
	}*/
	//printf("Go TO:\n");

	/* -------- Add cost if it is with in range of a pillbox ---------*
	* Ok, I suck at geometry, and thus I am going to waste cpu..  oh, *
	* well.  This function goes through every square with in a 20x20  *
	* square around the pillbox.  Each time it calculates the distance*
	* and adds cost based on that.                                    */
	for (i=0;i < 16;i++) {
		if ((pillboxes[i].idnum != 20 && pillboxes[i].direction != 0)) {
			if ((pillboxes[i].x >> 8) > left && (pillboxes[i].x >> 8) < (left + width) && (pillboxes[i].y >> 8) > top && (pillboxes[i].y >> 8) < (top + height)) {
				int tx, ty, tempdist;
				//int tc, ta, tb;
				position = (((pillboxes[i].y >> 8) - top) * width) + ((pillboxes[i].x >> 8) - left);
				map[position].cost += 500;
				
				if (pillboxes[i].info & OBJECT_HOSTILE || pillboxes[i].info & OBJECT_NEUTRAL) {
					x = (pillboxes[i].x >> 8) - 7 - left;
					y = (pillboxes[i].y >> 8) - 7 - top;
					for (ty=0;ty < 14;ty++) {
						for (tx=0;tx < 14;tx++) {
							if ((tx + x) > 0 && (tx + x) < width && (ty + y) > 0 && (ty + y) < height) {
								if ((tempdist = abs((int)sqrt(((tx - 7) * (tx - 7)) + ((ty - 7) * (ty - 7))))) < 7) {
									if (clearPath(info, pillboxes[i].x, pillboxes[i].y, (left + tx + x) << 8, (top + ty + y) << 8, TRUE, FALSE)) {
										position = ((y + ty) * width) + (x + tx);
										//map[position].cost = (map[position].cost + 1) * (abs(8 - tempdist) + 1) * 3;
										//map[position].boatcost = (map[position].boatcost + 1) *(abs(8 - tempdist) + 1) * 3;
										
										if (map[position].boatcost == 1) {
											/* We know that this is water that will be passed in a boat */
											map[position].cost += 10;
											map[position].boatcost += 10;
										}											
										map[position].cost *= abs(8 - tempdist);
										map[position].boatcost *= abs(8 - tempdist) + 15;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	
	
	/* Make sure that we don't run into any other tanks. */
	
	for (i=0;i < info->num_objects;i++) {
		if ((info->objects[i].object == OBJECT_TANK)) {
			if ((info->objects[i].x >> 8) > left && (info->objects[i].x >> 8) < (left + width) && (info->objects[i].y >> 8) > top && (info->objects[i].y >> 8) < (top + height)) {
				int o,p;
				for (o=-1;o <= 1;o++) {
					for (p=-1;p <= 1;p++) {
						position = ((((info->objects[i].y >> 8) - top) + o) * width) + (((info->objects[i].x >> 8) - left) + p);
						map[position].cost += 50000;
						map[position].boatcost += 50000;
						
						if (o == 0 && p == 0) {
							map[position].cost += 500;
							map[position].boatcost += 500;
						}
					}
				}



				//position = (((info->objects[i].y >> 8) - top) * width) + ((info->objects[i].x >> 8) - left);
				//map[position].cost += 50000;
				//map[position].boatcost += 50000;
				
			}
		}
	}
	
	
	/* Make sure that bases we can't get through don't get in our way,
	*  and that we shoot through the ones we can shoot through. */
	
	for (i=0;i < 16;i++) {
		if ((bases[i].idnum != 20 && bases[i].direction != 0) && (bases[i].info & OBJECT_HOSTILE)) {
			if ((bases[i].x >> 8) > left && (bases[i].x >> 8) < (left + width) && (bases[i].y >> 8) > top && (bases[i].y >> 8) < (top + height)) {
				//int tc, ta, tb;
				position = (((bases[i].y >> 8) - top) * width) + ((bases[i].x >> 8) - left);
				if (info->shells > bases[i].direction + 2) {
					//printf("Add Shell Cost\n");
					map[position].shellcost += bases[i].direction;
				} else {
					map[position].cost += 50000;
					map[position].boatcost += 50000;
					map[position].shellcost += bases[i].direction + 5;
				}
			}
		}
	}
	
	
	//printf("Get MAP\n");

	/* First we need to setup the mopen queue */
	pqueue_init(&mopen, &compare_int, NULL);
	
	
	
	x = (startx) - left;
	y = (starty) - top;
	position = (y * width) + x;
	map[position].parent = NULL;
	map[position].totalcost = 0;
	map[position].shell = info->shells;
	//map[position].armour = info->armour * 10;
	if (info->inboat) {
		map[position].inboat = TRUE;
	}
	map[position].f = 1;
	pqueue_insert(&mopen, &map[position]);
	
	
	//printf("height: %d  width: %d\n", height, width);
	
	while (TRUE) {
		//printf("f");
		if (pqueue_extract(&mopen, &temp_void)) {
			printf("Ran out of squares to examine: tgx: %d, tgy: %d, stx: %d, sty: %d\n", targetx, targety, startx, starty);
			return FALSE;
		} else {
			temp = (square *) temp_void;
			for (j=0;j < 4;j++) {
				if (!(temp->y + yarray[j] < 0 || temp->y + yarray[j] >= height || temp->x + xarray[j] < 0 || temp->x + xarray[j] >= width)) {
					tempsquare = ((temp->y + yarray[j]) * width) + (temp->x + xarray[j]);
					
					
					if (!map[tempsquare].closed) {
						if (map[tempsquare].f == 0) {
				
							/* Check to see if the previous square was in a boat, if so *
							* we should make the next one in a boat if it is not land   *
							(100) then set the cost to 1                               */
							if (temp->inboat) {
								if (map[tempsquare].boatcost != 1000) {
									map[tempsquare].inboat = TRUE;
									//printf("Advance ");
									map[tempsquare].cost = 1;
									//map[tempsquare].boatcost = 1;
								}
							}
	
							map[tempsquare].shell = temp->shell - (int)map[tempsquare].shellcost;
							if ((map[tempsquare].shellcost > 1) && (map[tempsquare].shell < shells || map[tempsquare].shellcost >= info->shells)) {
								//printf("Cant got through wall %d %d %d\n", map[tempsquare].x, map[tempsquare].y, info->shells);
								map[tempsquare].cost += 400000;
							}
							//REMOUTif (map[tempsquare].shell < shells) {
								//map[tempsquare].cost += 20000;
								//goto leave;
							//REMOUT} else {
							
							/* TODO:map[tempsquare].armour = temp->armour - (int)map[tempsquare].armourcost;
							if (map[tempsquare].armour < armour) {
								map[tempsquare].cost += 20000;
								map[tempsquare].boatcost += 20000;
							}*/
	
							
							map[tempsquare].f = map[tempsquare].h + temp->totalcost + map[tempsquare].cost;
							map[tempsquare].totalcost = temp->totalcost + map[tempsquare].cost;
							map[tempsquare].closed = TRUE;
							map[tempsquare].parent = temp;
							pqueue_insert(&mopen, &map[tempsquare]);
							//leave:
							//REMOUT}
						} else {
							/*tempf = map[tempsquare].h + temp->totalcost + (int)map[tempsquare].cost;
							if (tempf < map[tempsquare].f) {
								map[tempsquare].f = map[tempsquare].h + temp->totalcost + (int)map[tempsquare].cost;
								map[tempsquare].totalcost = temp->totalcost + (int)map[tempsquare].cost;
								map[tempsquare].closed = TRUE;
								map[tempsquare].parent = temp;
								pqueue_reheap(&mopen, &map[tempsquare]);*/
								//printf("RH-");
						}
					}
				}
			}
		}
		//printf("f");
		if (temp->x == (targetx - left) && temp->y == (targety - top)) {
			break;
		}	
	}

	
	tmp = &map[(((targety - top) * width) + (targetx - left))];
	for (i=0;TRUE;i++) {
		if (tmp->parent == NULL) {
			break;
		} else {
			tmp = (square *) tmp->parent;
		}
	}

	pts = (points *) malloc(sizeof(points) * (i + 1));
	tmp = &map[(((targety - top) * width) + (targetx - left))];
	fvalue = tmp->totalcost;
	subtractfromf = 0;
	/*if (finalshells != NULL) {
		*finalshells = (int)tmp->shell;
	}
	if (finalarmour != NULL) {
		*finalarmour = (int)tmp->armour;
	}*/
	v = 0;
	for (l=i;l > 0;l--) {
		if (lastx > 0 && v == lastx) {
			subtractfromf += tmp->totalcost;
		}
		pts[l - 1].x = ((tmp->x + left) << 8) + 128;
		pts[l - 1].y = ((tmp->y + top) << 8) + 128;
		pts[l - 1].terrain = tmp->terrain;
		if (tmp->shellcost > 0) {
			pts[l - 1].shoot = TRUE;
		} else {
			pts[l - 1].shoot = FALSE;
		}
		pts[l - 1].inboat = tmp->inboat;
		tmp = (square *) tmp->parent;
		v++;
	}
	pts[i].x = 0;
	pts[i].y = 0;
	pts[i].shoot = FALSE;
	pqueue_destroy(&mopen);
	free(map);
	
	iteration = 0;
	subit = 0;

	//printf("stand back\n");
	fvalue -= subtractfromf;
	//printf("SubfromF: %d\n", subtractfromf);
	if (fvalue == 0) {
		fvalue++;
	}
	return fvalue;
}

