#ifdef WIN32
	#include "brain_win.h"
#else
	/* brain.h -- needed to interact with linbolo */
	#include "brain.h"
#endif
#include <math.h>
#include <stdlib.h>



int aim(const BrainInfo *info, WORLD_X x, WORLD_Y y) {
	/*********************** aim ***************
	* The aim function returns the direction   *
	* you would need to go to reach x, y       *
	*******************************************/
	double tmp;
	tmp = atan2(x - info->tankx, -1 * (y - info->tanky));
	tmp = tmp * 127.5 / 3.14;
	if (tmp < 0) {
		tmp = 255 + tmp;
	}
	return (int)tmp;
}


void pointTank(const BrainInfo *info, WORLD_X x, WORLD_Y y) {
	/********************** pointTank **********************
	* The pointTank function is fairly simple, even though *
	* it took me forever to figure out.  What it does it   *
	* point the tank at a world choordinate                *
	*******************************************************/
	int pointat;
	
	pointat = aim(info, x, y);
	if (info->direction != pointat) {
		if (info->direction > pointat) {
			if (abs(info->direction - pointat) > 127) {
				if (!testkey(*info->holdkeys, KEY_turnright)) {
					*(info->holdkeys) = 0;
					setkey(*info->holdkeys, KEY_turnright);
				}
			} else {
				if (!testkey(*info->holdkeys, KEY_turnleft)) {
					*(info->holdkeys) = 0;
					setkey(*info->holdkeys, KEY_turnleft);
				}
			}
		} else {
			if (abs(info->direction - pointat) > 127) {
				if (!testkey(*info->holdkeys, KEY_turnleft)) {
					*(info->holdkeys) = 0;
					setkey(*info->holdkeys, KEY_turnleft);
				}
			} else {
				if (!testkey(*info->holdkeys, KEY_turnright)) {
					*(info->holdkeys) = 0;
					setkey(*info->holdkeys, KEY_turnright);
				}
			}
		}
	} else {
		*(info->holdkeys) = 0;
	}
}

