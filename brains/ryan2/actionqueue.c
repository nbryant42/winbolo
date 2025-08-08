/********************* Actionqueue.c ******************
* This file is used to prioritize the current actions *
* that the tank is to perform.  It is a basic queue   *
* with a quicksort algorithum to dynimacilly sort all *
* of the items.  The sort calles a getpriority        *
* function to see what the current priority is based  *
* on the location of the tank.                        *
*******************************************************/



#include "sort.h"
#ifdef WIN32
	#include "brain_win.h"
#else
	/* brain.h -- needed to interact with linbolo */
	#include "brain.h"
#endif
#include "scout.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>



#ifdef WIN32
	#include <stdarg.h>
	#include <windows.h>
	#include "resource.h"

#endif

#include "actionqueue.h" // must include after any system headers that define printf

const BrainInfo *outinfo;


static int compare_actions(const void *in1, const void *in2);


#ifdef WIN32

	extern HWND hDebugWindow;

	char ts[1024];
	char window[1024];

	int printdebug(const char *instr, ...) {
		/*if (IsWindowVisible(hDebugWindow)) {
			va_list list;
			va_start(list, instr);
			
			GetDlgItemText(hDebugWindow, IDC_EDIT_MAIN, window, 500);
			vsprintf(ts, instr, list);
			strcat(ts, "\r");
			strcat(ts, window);
			va_end(list);
			SetDlgItemText(hDebugWindow, IDC_EDIT_MAIN, window);
		}*/
		return 0;
	}
	
	
#endif



int actionqueue_init(actionqueue *actqueue) {
	/**************** actionqueue_init **************
	* This function sets initilizes the actionqueue *
	* specified by *actqueue                        *
	*************************************************/
	actqueue->act = NULL;
	actqueue->size = 0;
	return 1;
}

int actionqueue_insert(const BrainInfo *info, actionqueue *actqueue, Boolean (*startfunction)(const BrainInfo *info, int idnum), Boolean (*middlefunction)(const BrainInfo *info, int idnum), Boolean (*endfunction)(const BrainInfo *info, int idnum), int (*getpriority)(const BrainInfo *info, int idnum), int idnum) {
	/************************** actionqueue_insert **************
	* This function adds an item to the queue.  It accolates    *
	* the necessary memory and sets actqueue->act to that       *
	* location.  Then it copys the incoming values into that    *
	* queue space.                                              *
	************************************************************/
	
	if (actqueue->act == NULL) {
		actqueue->act = (action *) malloc(sizeof(action));
	} else {
		actqueue->act = (action *) realloc(actqueue->act, sizeof(action) * (actqueue->size + 1));
	}
	actqueue->act[actqueue->size].startfunction = startfunction;
	actqueue->act[actqueue->size].middlefunction = middlefunction;
	actqueue->act[actqueue->size].endfunction = endfunction;
	actqueue->act[actqueue->size].getpriority = getpriority;
	actqueue->act[actqueue->size].idnum = idnum;
	actqueue->size++;
	return 1;
}

static int compare_actions(const void *in1, const void *in2) {
	/* We call a compare function that ends up calling the *
	*  getpriority function to see if the priority has     *
	* changed.                                             */



	int ret1, ret2;
	const action *act1 = in1;
	const action *act2 = in2;
	
	
	
	ret1 = act1->getpriority(outinfo, act1->idnum);
	ret2 = act2->getpriority(outinfo, act2->idnum);
	if (ret1 > ret2) {
		return 1;
	} else if (ret1 < ret2) {
		return -1;
	} else {
		if (ret1 + act1->idnum > ret2 + act2->idnum) {
			return 1;
		} else if (ret1 + act1->idnum < ret2 + act2->idnum) {
			return -1;
		}
		return 0;
	}
}

int actionqueue_size(actionqueue *actqueue) {
	/* Humm, what does this do, too confusing.  No, can't understand, *
	*  oh yea, returns the size item in the queue.  */
	return actqueue->size;
}

int popAction(const BrainInfo *info, actionqueue *actqueue, action *outaction) {
	outinfo = info;

	if (actqueue->size > 0) {
		action *tmpact;
		memcpy(outaction, &actqueue->act[0], sizeof(action));
		tmpact = actqueue->act;
		actqueue->act = (action *) malloc(sizeof(action) * (actqueue->size - 1));
		memcpy(actqueue->act, &tmpact[1], sizeof(action) * (actqueue->size - 1));
		free(tmpact);
		actqueue->size--;
		return 1;
	} else {
		return 0;
	}
}


int peekAction(const BrainInfo *info, actionqueue *actqueue, action *outaction) {
	outinfo = info;
	if (qksort(actqueue->act, actqueue->size, sizeof(action), 0, actqueue->size - 1, compare_actions) != 0) {
		printf("Sort Error\n");
	}
	if (actqueue->size > 0) {
		memcpy(outaction, &actqueue->act[0], sizeof(action));
		/* Ok, I had to put the thing in below so that if any of them are over 1000, even *
		* thought they are still in the queue, they will not be run because over 1000,    *
		* and scout function takes over.                                                  */
		
		if (actqueue->act[0].getpriority(info, actqueue->act[0].idnum) > 1000) {
			return 0;
		} else {
			return 1;
		}
	} else {
		return 0;
	}
}
