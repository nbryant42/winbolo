#ifndef SETTINGS_H
#define SETTINGS_H


typedef unsigned short setting;


/* These bits are used to set restrictions.  If one of these bits is false, we can't do that operation.  */
enum {
	ATTACK_PILLBOXES = 0x0001,
	BUILD_PILLBOXES  = 0x0002,
	PICKUP_PILLBOXES = 0x0004,
	LAY_MINES		= 0x0008,
	BUILD_ROADS	  = 0x0010,
	ATTACK_TANKS	 = 0x0020,
	ASSASIN_MODE	 = 0x0040,
	BASEGUARD_MODE   = 0x0080,
	PILLBOX_HUNTER   = 0x0100,
	ARCHITECT		= 0x0200,
	EXPLORER		 = 0x0400
};



void settings_init(unsigned short *options);
void settings_setoptions(unsigned short *options, int optionsflag, int boolvalue);
int settings_getoptions(unsigned short *options, int optionsflag);






#endif

