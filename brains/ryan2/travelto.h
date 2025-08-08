#ifndef TRAVELTO_H
#define TRAVELTO_H

#define SEARCHPAD 40

#define BOARDTIME 1

typedef struct {
	WORLD_X x;
	WORLD_Y y;
	Boolean shoot;
	Boolean inboat;
	TERRAIN terrain;
	} points;


Boolean doTravel(const BrainInfo *info, MAP_X targetx, MAP_Y targety, Boolean stopatend, Boolean stoptwofromend);
Boolean getPath(const BrainInfo *info, MAP_X targetx, MAP_Y targety, MAP_X startx, MAP_Y starty, BYTE shells, BYTE armour, int steps, int lastx);
Boolean clearPath(const BrainInfo *info, WORLD_X startx, WORLD_Y starty, WORLD_X endx, WORLD_Y endy, Boolean blocktrees, Boolean fromtank);
Boolean isLand(TERRAIN landtype);
Boolean landAround(const BrainInfo *info);

#endif
