#include "../winbolo.h"

#ifdef WINBOLO_EXE
#define DLLDATA __declspec(dllimport)
#else
#define DLLDATA __declspec(dllexport)
#endif

/* Handle to the application Instance and window */
DLLDATA HINSTANCE appInst;
DLLDATA HWND appWnd;

/* Time since the game timer last went up */
DLLDATA DWORD oldTick;
DLLDATA DWORD oldFrameTick;

/* Time between frame updates based on Frame rate */
DLLDATA int frameRateTime;

/* Whether the sound effects are turn on or not */
DLLDATA bool soundEffects;

/* Is the sound card of the ISA variety */
DLLDATA bool isISASoundCard;

/* The timer for game updates */
DLLDATA unsigned int timerGameID;
DLLDATA unsigned int timerFrameID;

/* Key Settings */
DLLDATA keyItems keys;

/* Are we in a menu or not */
DLLDATA bool isInMenu;

DLLDATA bool doingTutorial;

/* Time to quit */
DLLDATA bool winboloQuit;
DLLDATA bool finishedLoop;
