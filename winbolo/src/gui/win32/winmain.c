#define WINBOLO_EXE

#include <Windows.h>
#include <WinUser.h>
#include "win32_winbolo.h"
#include "..\winbolo.h"
#include "..\resource.h"
#include "..\gamefront.h"
#include "..\lang.h"


/*********************************************************
*NAME:          WinMain
*AUTHOR:        John Morrison
*CREATION DATE: 31/10/98
*LAST MODIFIED: 4/1/00
*PURPOSE:
*  Main Function. Creates the window and sets up
*  message handling
*
*ARGUMENTS:
*  hInst     - Handle to the app instance
*  hInstPrev - Handle to the prious App Instance
*  szCmdLine - String pointer to the command line
*  nCmdShow  - Window State on start up
*********************************************************/
int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR szCmdLine, int nCmdShow) {
    HACCEL hAccel; /* Accelerator table */
    int timerFlush; /* Used to flush the timer */


    appInst = hInst;
    winboloQuit = FALSE;

    /*time_t currTime; /* Current Time *
      time(&currTime);
      if (currTime > 943884002) {
        MessageBoxA(NULL, "This beta version of WinBolo has expired. Please download a more recent version", DIALOG_BOX_TITLE, MB_ICONEXCLAMATION);
        exit(0);
      } else {
        MessageBoxA(NULL, "NOTE: This beta version of WinBolo expires on the 30/11/99", DIALOG_BOX_TITLE, MB_ICONINFORMATION);
      }  */

    initWinboloTimer();

    hAccel = LoadAccelerators(hInst, MAKEINTRESOURCE(IDR_ACCELERATOR));
    if (clientMutexCreate() == FALSE) {
        MessageBoxA(NULL, langGetText(STR_WBERR_MUTEXCREATE), DIALOG_BOX_TITLE, MB_ICONEXCLAMATION);
        return 0;
    }

    appWnd = gameFrontStart(hInst, szCmdLine, nCmdShow, &keys, FALSE); /* Start Game */
    if (appWnd == NULL) {
        exit(0);
    }
    winboloQuit = FALSE;
    while (winboloQuit == FALSE) {
        isInMenu = FALSE;
        finishedLoop = FALSE;
        windowReCreate();
        windowApplyMenuChecks();
        inputActivate();

        /* Set up Timers */
        if (soundEffects == TRUE) {
            soundISASoundCard(isISASoundCard);
        }
        Sleep(500);
        oldTick = winboloTimer();
        oldFrameTick = oldTick;
        timerGameID = timeSetEvent(GAME_TICK_LENGTH, 10000, windowGameTimer, 0, TIME_PERIODIC);
        timerFrameID = timeSetEvent(frameRateTime, 10000, windowFrameRateTimer, 0, TIME_PERIODIC);
        winboloQuit = TRUE;
        finishedLoop = FALSE;
        gameFrontRun(hInst, appWnd, hAccel, nCmdShow);
        finishedLoop = TRUE;

        /* Kill Timers */
        timeKillEvent(timerGameID);
        timeKillEvent(timerFrameID);
        timerFlush = 0;
        while (timerFlush < 10000) {
            timerFlush++;
        }
        clientMutexRelease();
        gameFrontEnd(hInst, appWnd, &keys, TRUE, winboloQuit); /* Shutdown game */
        doingTutorial = FALSE;
        if (winboloQuit == FALSE) {
            gameFrontStart(hInst, szCmdLine, nCmdShow, &keys, TRUE); /* Start Game */
        }
    }

    endWinboloTimer();
    clientMutexDestroy();

    /*  dbg = _CrtDumpMemoryLeaks() ;
      if (dbg != FALSE) {
        dbg = 1;
      }    */
    return 0;
}
