/********************* Actionqueue.h ******************
* This file is used to prioritize the current actions *
* that the tank is to perform.  It is a basic queue   *
* with a quicksort algorithum to dynimacilly sort all *
* of the items.  The sort calles a getpriority        *
* function to see what the current priority is based  *
* on the location of the tank.                        *
*******************************************************/


#ifndef ACTIONQUEUE_H
#define ACTIONQUEUE_H

#define VERSION "2.4.3"

#ifdef WIN32


	#define printf printdebug


	int printdebug(const char *instr, ...);


#endif


//#define SHOWPRI



/* The priority to attack a tank ---- Could get confused if more than one tank */
#define ATTACKTANK_PRI 1
/* The priority to wait at the base */
#define BASEGUARD_PRI 900
/* The priority to pickup the man */
#define GETMAN_PRI 0
/* The priority to get trees */
#define GETTREES_PRI 155
/* Add the following to pri before we are done */
#define PICKUP_PRI 100
/* The priority to lay a pillbox */
#define LAYPILL_PRI 150
/* The priority of the scout action */
#define SCOUT_PRI 10000
/* The priority to attack a pillbox */
#define ATTACKPILL_PRI 230
/* Add this to the priority */
#define REFUEL_PRI 300
/* Add the following to get a neutral refuel base*/
#define REFUEL_NEUTRAL_PRI 250
/* Add this to the pri */
#define REPAIR_PRI 250
/* What should we multiply the distance by before we add 
* ATTACKPILL_PRI */
#define ATTACKPILL_DISTANCE_MULTIPLY 2
/* At the following distance, we really don't want to pickup, so we 
* add an additional amount to the priority*/
#define PICKUP_TO_FAR_DISTANCE 40
/* This is the amount we add when it is too far. */
#define PICKUP_TO_FAR_DISTANCE_TO_ADD 6000
/* Multiply the distance by this amount before we add an amount */
#define PICKUP_DISTANCE_MULTIPLY 2
/* Add this amount eachtime there is pillbox danger around a base we are trying to refuel at */
#define REFUEL_ADD_IF_PILLBOX_DANGER 11000
/* Multiply the priority by this before we add something. */
#define REFUEL_DISTANCE_MULTIPLY 2
/* Once we have started refueling, to keep refueling we set the pri to the following */
#define REFUEL_CONTINUE_AT 20
/* At the following distance, we really don't want to repair, so we 
* add an additional amount to the priority */
#define REPAIR_TO_FAR_DISTANCE 40
/* THis is the amount we add wehn it is too far */
#define REPAIR_TO_FAR_DISTANCE_TO_ADD 200
/* Multiply the distance by this amount before we add an amount */
#define REPAIR_DISTANCE_MULTIPLY 2
/* We add the strength of the pilbox (0-16) multiplied by the following, to the priority */
#define REPAIR_PILLBOX_STRENGTH_MULTIPLY 3
/* Add the following if there is danger of being attacked by a pillbox while repairing. */
#define REPAIR_PILLBOX_DANGER 11000
/* Add the following if there is danger of being shot while repairing. */
#define REPAIR_SHOT_DANGER 10000






/* Gets the terrain value of a square. */
#define GETSQUARE(a,b) ((info->theWorld[a | b] & TERRAIN_MASK))


/* The structure of an action.  (A lot of function pointers */
typedef struct {
	int (*getpriority)(const BrainInfo *info, int idnum);
	int idnum;
	Boolean (*startfunction)(const BrainInfo *info, int idnum);
	Boolean (*middlefunction)(const BrainInfo *info, int idnum);
	Boolean (*endfunction)(const BrainInfo *info, int idnum);
} action;


/* A queue item. */
typedef struct {
	action *act;
	int size;
} actionqueue;



int actionqueue_insert(const BrainInfo *info, actionqueue *actqueue, Boolean (*startfunction)(const BrainInfo *info, int idnum), Boolean (*middlefunction)(const BrainInfo *info, int idnum), Boolean (*endfunction)(const BrainInfo *info, int idnum), int (*getpriority)(const BrainInfo *info, int idnum), int idnum);
int actionqueue_init(actionqueue *actqueue);
int actionqueue_size(actionqueue *actqueue);
int popAction(const BrainInfo *info, actionqueue *actqueue, action *outaction);
int peekAction(const BrainInfo *info, actionqueue *actqueue, action *outaction);



#endif
