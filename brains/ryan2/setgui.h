#ifndef SETGUI_H
#define SETGUI_H


#include <stdio.h>
#ifdef WIN32
	#include "brain_win.h"
#else
	/* brain.h -- needed to interact with linbolo */
	#include "brain.h"
#endif

#ifndef WIN32
	#include <glib.h>
	#include <gmodule.h>
	#include <gtk/gtk.h>
#endif
#include "settings.h"


#ifndef WIN32
	void setGui(GtkWidget *menu_bar);
	void menu_destroy(void);
#endif

void setsettings(setting *stn);

#ifdef WIN32
	#define BR "\r\n"
#else
	#define BR "\n"
#endif
#define ABOUTMESSAGE "Ryan2   2.4.3" BR "http://ryan2.servehttp.com" BR "" BR "	I am currently trying to keep up with my studies at Montana State University.  Hopefully I should get some time to work on the project over christmas break, but we'll just have to see." BR "	Please e-mail me with any bugs you may find in the program. (rstout@cybernet1.com)  Please send me details about what causes these bugs and if possible how to duplicate it.  Thanks for your help." BR "" BR "	Ryan2 is a project that I started about a year ago.  Programming is something that I really enjoy doing, and I also enjoy bolo, so I knew that this would be a fun project to do." BR "     At the moment, the project is still in development, and you may knotice some bugs.  The autopilot uses many different techniques to achieve its task.  The basic path finding algorithum it uses is called an A" BR " (star) algorithum.  It has been somewhat modified, but works quite well.  The autopilot also uses a very advanced priority queue system to determine what action to take next.  At this moment, the tank can:" BR "- Attack Pills" BR "- Attack Tanks" BR "- Retrieve Lost Builders" BR "- Lay Pills/Build Bases" BR "- Pickup Pills" BR "- Refuel" BR "- Explore" BR "- Repair Pillboxes" BR "- Get Trees" BR "- Build Roads in some cases" BR "     In the next version, the tank should be able to build roads a little better, build walls, and it will have an improved priority system.  I am also working on trying to get all of the bugs out of the basic traveling algorithum.  Ryan2 was origionally written in Linux using the gcc compiler.  However recently I have managed to port it to windows.  It was actually a fairly easy port.  I use Visual C++ 6 to compile the brain on Windows.  If you are interested in the source, it can be downloaded from http://ryan2.servehttp.com" BR "" BR "     About Me:  I have enjoyed programming for what seems like forever.  I also enjoy snowboarding, playing guitar and drums, and putting on retreats.  I just finished with a year of traveling retreat ministry with the catholoc church.  It was an amazing and incredible experence, from which I learned a lot."
#define CHANGELOG "Change Log" BR "" BR "Version 2.4.3" BR "" BR "" BR "" BR " Ok, so I got a little free time over thanksgiving break to work on a new versoin, expect a better version over christmas break." BR "11/29/02 - Fixed the bug where tank drives off after getting guy" BR "11/29/02 - Added the ability to build roads" BR "11/29/02 - Rebuild priority system and changed many values" BR "11/29/02 - Made pillbox pickup more aggresive" BR "11/29/02 - Fixed the update function" BR "" BR "" BR "Version 2.4.2" BR "08/29/02 - Fixed get man only once bug." BR "08/29/02 - Added Base Guard function." BR "08/29/02 - Fixed Menu options in windows." BR "08/28/02 - Added Repair Pillbox function." BR "08/28/02 - Fixed (somewhat) tank flondering when getting on boats." BR "08/28/02 - Fixed tank killing allies bug." BR "08/28/02 - Added function to get trees." BR "08/28/02 - Fixed Base Laying Algorithum." BR "08/28/02 - Added Check for Update feature." BR "08/28/02 - Changed About Box - now includes changelog under windows" BR "08/28/02 - Improved Base Building algorithum.  Will now pick base to build pills around based on how much grass/road, if you already have pills there, and if a non-ally has pills there." BR "08/28/02 - Fixed bug that caused tank to get stuck on walls along water."



#endif

