#include <stdio.h>
#include "settings.h"





void settings_init(unsigned short *options) {
	*options = 0xFFFF;
}

void settings_setoptions(unsigned short *options, int optionsflag, int boolvalue) {
	if (boolvalue) {
		if (!(optionsflag & *options)) {
			*options = optionsflag | *options;
		}
	} else {
		if (optionsflag & *options) {
			*options = optionsflag ^ *options;
		}
	}
}


int settings_getoptions(unsigned short *options, int optionsflag) {
	return (optionsflag & *options);
}

