/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


/*********************************************************
* Ryan2 -- ryan2 is a project that I decited to work on. *
* My first brain named "ryan" ended up having massive    *
* problems when the new version of bolo came out.  Plus  *
* there was bad pointers all over the place.  So I took  *
* some time off to read a few books on programming and   *
* do a few other things before I decited to go back to   *
* work on the new version.  I hope this brain will be as *
* good as the old one, and will be able to help me       *
* improve my programming and geometry skills.            *
* Ryan Stout - 05/27/01                                  *
*********************************************************/

/* Includes -- Ok, here we have some basic includes for dealing with the *
* the text and graphical interface.  man them if you need more info.     */


#ifdef _WIN32
	/* These are the includes we need to incorporate in order to *
	*  run it on windows.  resource.h holds information created  *
	*  by Visual C++, it can be edited however if you wish to    *
	*  create new menu's by hand. */
	#include <WinSock2.h>
	#include "resource.h"

	/* This funciton is used to tell the menu to check or uncheck *
	*  the menu items. */
	#define MENU_CHECK_WIN32(X) ((X==0) ? (MF_UNCHECKED) : (MF_CHECKED))
#else
	/* The linux interface includes.  The linux menu interface is *
	*  described in setgui.c */
	#include <glib.h>
	#include <gmodule.h>
	#include <gtk/gtk.h>
#endif

/* Either operating system includes. (We use glibc under windows) */
#include <math.h>
#include <stdlib.h>
#include <stdio.h>


/* My Includes */
#include "Ryan.h"
#include "actionqueue.h"
#include "pointtank.h"
#include "update.h"


/* Here is defines of what the tank can do..  */

#include "scout.h"
#include "refuel.h"
#include "pickup.h"
#include "attackpill.h"
#include "attacktank.h"
#include "laypill.h"
#include "getman.h"
#include "gettrees.h"
#include "repair.h"
#include "baseguard.h"

/* setgui -- adds specific menu options and stuff */
#include "setgui.h"
#include "settings.h"



#ifdef WIN32
	/* We define the windows menu interface options in Ryan.c, *
	*  these are also defined in setgui.c */
	extern Boolean opt[14];
	extern setting *stngs;
	
	int settingsarray[] = {0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080, 0x0100, 0x0200, 0x0400};

#endif



extern int change_tracker;
setting opts;
int priority[11] = {0,0,0,0,0,0,0,0,0,0,0};



/* The speed varable is uesd all over the program *
*  You can access it using an extern from anywhere*
*  in the program.  It will change the speed of   *
*  the tank.  It can go from 0-64.  */
int speed = 0;

/* How many times has the brainMain function been accessed. */
int tickcount = 0;



int lastshellcount = 40;


/* The action queue is a priority queue where we decide what *
*  the next action of the tank will be. */
action currentact;
actionqueue nextaction;



int idnum = 0;
int last_idnum = 0;
Boolean keepPrev = FALSE;



extern Boolean pickbase(const BrainInfo *info);
void doUpdate();


/* Used to keep track of what funciton we are on, after getting *
*  the function from the action queue.  If we have completed it *
*  it will be set to NULL.  More info above getNext() */
Boolean (*last_startfunction)(const BrainInfo *info, int idnum);
Boolean (*last_middlefunction)(const BrainInfo *info, int idnum);
Boolean (*last_endfunction)(const BrainInfo *info, int idnum);
Boolean (*startfunction)(const BrainInfo *info, int idnum);
Boolean (*middlefunction)(const BrainInfo *info, int idnum);
Boolean (*endfunction)(const BrainInfo *info, int idnum);




/* See notes */
extern Boolean endPickup(const BrainInfo *info, int idnum);
extern Boolean diedtryingtogetpill[16];



/* An array that keeps track of the bases and pillboxes without having *
*  to use the info->objects array.  */
ObjectInfo bases[16];
ObjectInfo pillboxes[16];


/* The location of our main base that we build all of the pillboxes around. */
WORLD_X basex = 0;
WORLD_Y basey = 0;


/* Used to output the priority of each of the functions. */
#ifdef SHOWPRI
	int attackpillpri = 999999;
	int attacktankpri = 999999;
	int getmanpri = 999999;
	int laypillpri = 999999;
	int pickuppri = 999999;
	int refuelpri = 999999;
	int scoutpri = 999999;
#endif





#ifdef WIN32
	
	/* More windows menu code. */
	HINSTANCE brainInst;
	HMENU brainMenu = NULL;
	HWND hAboutDialog = NULL;
	HWND hUpdateDialog = NULL;
	HWND hDebugWindow = NULL;
	
	/* When the user clicks the Ok button from the about box.*/
	BOOL CALLBACK aboutDialogCallback(HWND hWnd, unsigned uMsg, WPARAM wParam, LPARAM lParam) {
		if (uMsg == WM_COMMAND && LOWORD(wParam) == IDOK) {
			ShowWindow(hAboutDialog, SW_HIDE);
		} else if (uMsg == WM_COMMAND && LOWORD(wParam) == ID_CHANGE) {
			if (change_tracker == 0) {
				SetDlgItemText(hAboutDialog, ID_CHANGE, "View About Info");
				SetDlgItemText(hAboutDialog, IDC_EDIT_MAIN, CHANGELOG);
				change_tracker = 1;
			} else {
				SetDlgItemText(hAboutDialog, ID_CHANGE, "View Change Log");
				SetDlgItemText(hAboutDialog, IDC_EDIT_MAIN, ABOUTMESSAGE);
				change_tracker = 0;
			}
		}
		return FALSE;
	}

	BOOL CALLBACK updateDialogCallback(HWND hWnd, unsigned uMsg, WPARAM wParam, LPARAM lParam) {
		if (uMsg == WM_COMMAND && LOWORD(wParam) == IDOK) {
			ShowWindow(hUpdateDialog, SW_HIDE);
		}
		return FALSE;
	}
	
	/* When the user clicks the Ok button from the debug dialog. */
	BOOL CALLBACK debugDialogCallback(HWND hWnd, unsigned uMsg, WPARAM wParam, LPARAM lParam) {
		if (uMsg == WM_COMMAND && LOWORD(wParam) == IDOK) {
			ShowWindow(hDebugWindow, SW_HIDE);
		}
		return FALSE;
	}
	

#endif



/* Functions */






/*************************************************************
* getNext(const Brain *info)                                 *
* The getNext funciton retrieves the next value from the     *
* priority queue.  Each action has four functions.           *
* getpriorityFUNCTIONNAME - The function returns a number    *
*    value with the priority.  Anything over 1000, and Ryan2 *
*    will run scout instead.                                 *
* startFUNCTIONNAME - Usually sets up varables, or gets the  *
*     path so the tank knows where to go.                    *
* middleFUNCTIONNAME - Usually does the actual traveling,    *
*    usually involves calling the doTravel function.         *
* endFUNCTIONNAME - Usually does what ever the tank came     *
*    there to do.  Eg. deploy pill, attack pill....          *
*                                                            *
* Each function is called in that order.  getPriority is     *
* called from the priority queue.  After an aciton comes to  *
* the top, each action (start, middle, end) is run through in*
* that order, and the funcition is run until it returns TRUE *
* If the function returns TRUE on end, it is removed from the*
* queue.  However, it can return 2 to stay in the queue.     *
*************************************************************/
void getNext(const BrainInfo *info) {
	action tmpaction;
	tmpaction = currentact;
	if (!peekAction(info, &nextaction, &currentact)) {
		printf("Doing scout -- Nothing else left to do.\n");
		addScout(info);
		peekAction(info, &nextaction, &currentact);
		startfunction = currentact.startfunction;
		middlefunction = currentact.middlefunction;
		endfunction = currentact.endfunction;
		idnum = currentact.idnum;
	} else if (tmpaction.getpriority != currentact.getpriority || tmpaction.idnum != currentact.idnum) {
		startfunction = currentact.startfunction;
		middlefunction = currentact.middlefunction;
		endfunction = currentact.endfunction;
		idnum = currentact.idnum;
	} else {
		startfunction = currentact.startfunction;
		middlefunction = currentact.middlefunction;
		endfunction = currentact.endfunction;
		idnum = currentact.idnum;
	}
	keepPrev = FALSE;
}





Boolean brainThink(const BrainInfo *info) {
	int i, endout;

	*info->holdkeys = 0;
	


	/* If we just died, set endfunction to null so we can get a new action. */
	if (info->newtank) {
		printf("Just died :-<  Resetting values.\n");
		if (endfunction == &endPickup) {
			printf("Arg, died trying\n");
			diedtryingtogetpill[idnum] = TRUE;
		}
		startfunction = NULL;
		middlefunction = NULL;
		endfunction = NULL;
		last_idnum = 1000;
	}

	/* The next section calls each action functions. */
	if (endfunction == NULL) {
		getNext(info);
	} else {
		if (endfunction == last_endfunction && last_idnum != 1000 && idnum == last_idnum && keepPrev) {
			startfunction = last_startfunction;
			middlefunction = last_middlefunction;
			endfunction = last_endfunction;
		}
		if (startfunction != NULL) {
			if (startfunction(info, idnum)) {
				startfunction = NULL;
			}
		} else if (middlefunction != NULL) {
			if (middlefunction(info, idnum)) {
				middlefunction = NULL;
			}
		} else if (endfunction != NULL) {
			if ((endout = endfunction(info, idnum))) {
				action tmpact;
				if ((endout != 2)) {
					popAction(info, &nextaction, &tmpact);
				}
				endfunction = NULL;
			}
		}
		last_startfunction = startfunction;
		last_middlefunction = middlefunction;
		last_endfunction = endfunction;
		last_idnum = idnum;
	}

	
		#ifdef SHOWPRI
			/* If SHOWPRI is enabled, we print out the values of each of the getpri's *
			* from each action.  The values will be 9999999 if they have not yet been *
			* set, and may go to 0 if not in queue. */
	 		printf("attackpill:\t%d\nattacktank:\t%d\ngetman:\t\t%d\nlaypill:\t%d\npickup:\t\t%d\nrefuel:\t\t%d\nscout:\t\t%d\n\n", attackpillpri, attacktankpri, getmanpri, laypillpri, pickuppri, refuelpri, scoutpri);
		#endif
	
	/* Once we find an item, we want to queue it up.  So for each item that we find, it needs *
	* to be queued so that we can track it, and prioritize it.  Each time we need to get the  *
	* next action, the priority of the items we find is taken into acount.  We also only do   *
	* this every 100 loop arounds so we don't waste cpu busy polling.  Fairly simple, right   */
	if (tickcount > 50) {
		getNext(info);
		keepPrev = TRUE;

		for (i=0;i < info->num_objects;i++) {
			if (info->objects[i].object == OBJECT_PILLBOX) {
				pillboxes[info->objects[i].idnum] = info->objects[i];
				if (info->objects[i].direction == 0) {
					if (!isPickupQueued(info, info->objects[i].idnum)) {
						addPickup(info, (int)info->objects[i].idnum);
						if (endfunction != NULL) {
							getNext(info);
						}
					}
				} else {
					if (!isAttackQueued(info, info->objects[i].idnum) && (info->objects[i].info & OBJECT_HOSTILE || info->objects[i].info & OBJECT_NEUTRAL)) {
						addAttack(info, (int)info->objects[i].idnum);
						if (endfunction != NULL) {
							getNext(info);
						}
					}
				}
			} else if (info->objects[i].object == OBJECT_REFBASE) {
				bases[info->objects[i].idnum] = info->objects[i];
				if (!isRefuelQueued(info, info->objects[i].idnum)) {
					addRefuel(info, (int)info->objects[i].idnum);
					if (endfunction != NULL) {
						getNext(info);
					}
				}
			} else if (info->objects[i].object == OBJECT_TANK) {
				if (endfunction != NULL) {
					getNext(info);
				}
			}
		}
		
		
		if (lastshellcount > 10 && info->shells <= 10) {
			getNext(info);
		}
		lastshellcount = info->shells;
		if (basex == 0 && basey == 0) {
			pickbase(info);
		}
		tickcount = 0;
	}
	tickcount++;


	/* The following code sets the speed of the tank to the value in GETSPEED*/
	if (info->speed < GETSPEED()) {
		setkey(*info->holdkeys, KEY_faster);
	} else if (info->speed > GETSPEED()) {
		setkey(*info->holdkeys, KEY_slower);
	}
	
	return TRUE;
}



/* We define the brainOpen function two ways depending on the operating system. */
#ifdef WIN32
	Boolean brainOpen() {
#else
	Boolean brainOpen(GtkWidget *menu_bar) {
#endif
	Boolean returnValue;
	int i;



	#ifdef WIN32
	/* More menu handeling code. */
	HWND hMainWnd;
	HMENU hMenu;
	
	
	
	returnValue = TRUE;
	brainMenu = NULL;
	hMainWnd = GetActiveWindow();
	hMenu = GetMenu(hMainWnd);
	

	
	/* Check for good handles. */
	if (hMenu == NULL || hMainWnd == NULL) {
		returnValue = FALSE;
	}
	/* Load Resources. */
	if (returnValue == TRUE) {
		brainMenu = LoadMenu(brainInst, MAKEINTRESOURCE(IDR_BRAIN_MENU));
		if (brainMenu == NULL) {
			returnValue = FALSE;
		}
	}
	
	/* Add our menu. */
	if (returnValue == TRUE) {
		AppendMenu(hMenu, MF_POPUP, (UINT) brainMenu, BRAIN_MENU_NAME);
		DrawMenuBar(hMainWnd);
	}
	
	/* Load Debug Window */
	if (returnValue == TRUE) {
		hDebugWindow = CreateDialog(brainInst, MAKEINTRESOURCE(IDD_DEBUGWINDOW), NULL, debugDialogCallback);
		if (hDebugWindow == NULL) {
			returnValue = FALSE;
		}
	}

	/* Load About Box */
	if (returnValue == TRUE) {
		hAboutDialog = CreateDialog(brainInst, MAKEINTRESOURCE(IDD_ABOUT), NULL, aboutDialogCallback);
		if (hAboutDialog == NULL) {
			returnValue = FALSE;
		}
	}


	/* Load Update Dialog */
	if (returnValue == TRUE) {
		hUpdateDialog = CreateDialog(brainInst, MAKEINTRESOURCE(IDD_DIALOGUPDATE), NULL, updateDialogCallback);
		if (hUpdateDialog == NULL) {
			returnValue = FALSE;
		}
	}
	SetDlgItemText(hDebugWindow, IDC_EDIT_MAIN, "Not Yet Implememnted...");
	SetDlgItemText(hAboutDialog, IDC_EDIT_MAIN, ABOUTMESSAGE);

	/* Initilize the settings. */
	settings_init(&opts);
	setsettings(&opts);

	
	
	/* Set all of the opt(ions) to 0. */
	for (i=0;i < 14;i++) {
		opt[i] = 0;
	}
	

	

	
	#else
		/* Load the menu if we are in linux. */
		returnValue = TRUE;
		setGui(menu_bar);
		settings_init(&opts);
		setsettings(&opts);
	#endif
	/* Ok, so the brain worked right so far.  Lets let people know via the explain macro what is going on.*/
	
	printf("Welcome to Ryans Brain.  In this area you will see the stats on what is happening in the game, and what the brain is doing.  This brain was compiled on ");
	printf(__DATE__);
	printf("\n");
	startfunction = NULL;
	middlefunction = NULL;
	endfunction = NULL;
	if (!actionqueue_init(&nextaction)) {
		printf("The action queue didn't init right\n");
	} else {
		printf("Action Queue Started\n");
	}
	
	for (i=0;i < 16;i++) {
		pillboxes[i].idnum = 20;
	}
	for (i=0;i < 16;i++) {
		bases[i].idnum = 20;
	}
	currentact.getpriority = NULL;
	currentact.idnum = 0;
	currentact.startfunction = NULL;
	currentact.middlefunction = NULL;
	currentact.endfunction = NULL;
	
	
	/* We add some things to the queue right away. */
	addGetMan(NULL, 0);
	addGetTrees(NULL, 0);
	addLay(NULL, 0);
	addAttackTank(NULL, 0);
	addBaseguard(NULL, 0);
	
	for (i=0;i < 16;i++) {
		addRepair(NULL, i);
	}
	
	return returnValue;
}

void brainClose(void) {
	/* Remove menu */
	#ifdef WIN32
		HWND hMainWnd;
		HMENU hMenu;
		
		if (brainMenu != NULL) {
			/* This code removes the brain menu */
			hMainWnd = GetActiveWindow();
			hMenu = GetMenu(hMainWnd);
			DestroyMenu(brainMenu);
			RemoveMenu(hMenu, GetMenuItemCount(hMenu)-1, MF_BYPOSITION);
			DrawMenuBar(hMainWnd);
			brainMenu = NULL;
		}
		
		if (hAboutDialog != NULL) {
			/* Remove the about Dialog */
			EndDialog(hAboutDialog, TRUE);
			DestroyWindow(hAboutDialog);
		}

		if (hUpdateDialog != NULL) {
			/* Remove the update Dialog */
			EndDialog(hUpdateDialog, TRUE);
			DestroyWindow(hUpdateDialog);
		}

		if (hDebugWindow != NULL) {
			/* Remove the Debug Window */
			EndDialog(hDebugWindow, TRUE);
			DestroyWindow(hDebugWindow);
		}

	
	#else
		/* Remove window in linux (setgui.c) */
		menu_destroy();
	#endif
	/* Add any brain specific shutdown code here... */
}



#ifdef WIN32
/* Handle the brain menu under windows. */

void setCheckOpts(int optnumber, UINT menuItem) {
	/* This changes the various settings after something *
	*  is clicked from the meun. */
	opt[optnumber] ^= 1;
	if (opt[optnumber]) {
		settings_setoptions(stngs, settingsarray[optnumber], 0);
	} else {
		settings_setoptions(stngs, settingsarray[optnumber], 1);
	}
	CheckMenuItem(brainMenu, menuItem, MENU_CHECK_WIN32(opt[optnumber]));


	
}
void brainMenuHandler(const BrainInfo *brainInfo) {
	/* The following code handles the windows menu clicks. */
	switch (brainInfo->menu_item) {
		case ID_BRAINTEMPLATE_ABOUT:
			ShowWindow(hAboutDialog, SW_SHOW);
			SetForegroundWindow(hAboutDialog);
			break;
		case ID_DEBUGWINDOW:
			ShowWindow(hDebugWindow, SW_SHOW);
			SetForegroundWindow(hDebugWindow);
			break;
		case ID_RESTRICTIONS_ATTACKPILLBOXES:
			//printf("%d\n", settings_getoptions(stngs, ATTACK_PILLBOXES));
			//settings_setoptions(stngs, ATTACK_PILLBOXES, 0);
			//opts = 0x0000;
			setCheckOpts(0, ID_RESTRICTIONS_ATTACKPILLBOXES);
			break;
		case ID_RESTRICTIONS_BUILDPILLBOXES:
			setCheckOpts(1, ID_RESTRICTIONS_BUILDPILLBOXES);
			break;
		case ID_RESTRICTIONS_PICKUPPILLBOXES:
			setCheckOpts(2, ID_RESTRICTIONS_PICKUPPILLBOXES);
			break;
		case ID_RESTRICTIONS_LAYMINES:
			setCheckOpts(3, ID_RESTRICTIONS_LAYMINES);
			break;
		case ID_RESTRICTIONS_CLEARMINES:
			setCheckOpts(4, ID_RESTRICTIONS_CLEARMINES);
			break;
		case ID_RESTRICTIONS_ATTACKTANKS:
			setCheckOpts(5, ID_RESTRICTIONS_ATTACKTANKS);
			break;
		case ID_ASSASIN:
			setCheckOpts(6, ID_ASSASIN);
			break;
		case ID_BASEGUARD:
			setCheckOpts(7, ID_BASEGUARD);
			if (opt[7]) {
				settings_setoptions(stngs, ATTACK_PILLBOXES, 0);
				settings_setoptions(stngs, EXPLORER, 0);
			} else {
				settings_setoptions(stngs, ATTACK_PILLBOXES, 1);
				settings_setoptions(stngs, EXPLORER, 1);
			}
			break;
		case ID_ARCHITECH:
			setCheckOpts(9, ID_ARCHITECH);
			break;
		case ID_PILLBOXHUNTER:
			setCheckOpts(8, ID_PILLBOXHUNTER);
			break;
		case ID_EXPLORER:
			setCheckOpts(10, ID_EXPLORER);
			break;
		case ID_CHECKFORUPDATE:
			doUpdate();
			break;
	}
}

void doUpdate() {
	  
	char outmessage[5000];
	if (doUpdateCheck(outmessage, 5000) == 1) {
		SetDlgItemText(hUpdateDialog, IDC_MAIN, outmessage);
	} else {
		SetDlgItemText(hUpdateDialog, IDC_MAIN, "An error occoured when trying to connect to the server.");
	}
	ShowWindow(hUpdateDialog, SW_SHOW);
	SetForegroundWindow(hUpdateDialog);

	
}



#endif



#ifdef WIN32
	/* Here is the windows BrainMain first line. */
	__declspec( dllexport ) short BrainMain(const BrainInfo *brainInfo) {

#else
	/* If we are running linux, this is what we export. */
	G_MODULE_EXPORT short BrainMain(const BrainInfo *brainInfo) {
#endif
	short returnValue; /* Value to return */
	
	returnValue = 0;
	if (brainInfo->InfoVersion != CURRENT_BRAININFO_VERSION) {
				returnValue = BRAIN_ERROR;
	} else {
		switch (brainInfo->operation) {
			case BRAIN_OPEN:
				/* Do brain startup. */
				#ifdef WIN32
					if (brainOpen() == FALSE) {
				#else
					if (brainOpen((GtkWidget *) brainInfo->userdata) == FALSE) {
				#endif
				
					returnValue = BRAIN_ERROR;
				}
				break;
			case BRAIN_CLOSE:
				/* Do brain shutdown. */
				brainClose();
				break;
			case BRAIN_THINK:
				/* Do brain think call */
				if (brainThink(brainInfo) == FALSE) {
					returnValue = BRAIN_ERROR;
				}
				break;
			case BRAIN_MENU:
				/* Do brain menu action. */
				/* Not required under linux/GTK */
				#ifdef WIN32
					brainMenuHandler(brainInfo);
				
				#endif
				
				break;
			default:
				/* This should never be reached so it must be an unexpected error.
				Shutdown and return an error. */
			returnValue = BRAIN_ERROR;
		}
	}

	if (returnValue != 0) {
		/* We must of encounted an error so shutdown */
		brainClose();
	}
	return returnValue;
}



#ifdef WIN32
	/* Here is the win32 dll main entry point. */
	BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved) {
		brainInst = hInst;
		return TRUE;
	}

#endif






