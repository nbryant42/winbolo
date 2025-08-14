// ****************************************************************************
//
// Bolo standard Autopilot Brain
// (C) 1993-1995 Stuart Cheshire <cheshire@cs.stanford.edu>
// I make no claims that this is good code. It is provided solely as
// simple example code to assist in writing Bolo plug-in Brain modules.
// I do not have the time to tidy it up and make it elegant at all.
// It's poorly structured and it uses far too many global variables.
//
// Version history:
// 0.9w4 12th Aug 2025 improve A* gradient descent and base-ignore logic
// 0.9w3 11th Aug 2025 fix trigger-happy behavior in mazes and along walls
// 0.9w2 11th Aug 2025 various fixes
// 0.9w1 9th Aug 2025 ported to WinBolo (Menus currently disabled, so s_default settings only)
//       - Nathan Bryant nbryant nospamdeletethis at optonline dot net
// 0.9 12th May 1995. Updated to new version 3 Brain interface.
//     Made it prefer building slightly ahead, rather than under the tank,
//     when moving at a reasonable speed (speeds >= forest speed)
// 0.8 22nd Nove 1993. Added "Do Building" menu option, and moved
//     decide_building(nearx, neary) call to inside "if (correction…" statement
// 0.7 6th Nov 1993. Converted to A-Star heuristic, added route algorithm
//     debugging window and made tanks seek out boats for transportation.
//     Record time for Everard Island 18 min 30 secs
// 0.6 19th Oct 1993. Made it not try to pick up pillboxes which are sitting in deep sea.
// 0.5 13th Oct 1993. Major improvements to route finding and decision making. Pillbox
//     firing areas used, clear path routines, revised progress/boredom routines.
// 0.4 7th Oct 1993. Route finding switched to multi-parameter decision.
// 0.3 1st Oct 1993. New menu options, including "assassin" mode.
// 0.2 26th May 1993. Released with Bolo 0.99.
// 0.1 21st Feb 1993. Built into Bolo 0.98 as "Autopilot"

#define IMPLEMENTED 0 // #if IMPLEMENTED for things that are not yet ported from MacOS, mainly menus
#define AUTO_DEBUG 0 // also now being used to disable debug windows that are not ported and do not compile
#define VERBOSE 0

// ****************************************************************************

// Define an integer square root routine
#include "FixMath.h"
#include <math.h>

// ****************************************************************************

//#include "vsprintf.h"
#include "brain.h"

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

// wrappers/stubs/fakes for quick'n'dirty MacOS porting
typedef void* Ptr;
typedef size_t Size;
typedef uint32_t UInt32;

static inline Ptr NewPtr(Size byteCount) {
	return calloc(byteCount, 1);
}

static inline void DisposPtr(Ptr p) {
	free(p);
}

static inline void BlockMove(void* src, void* dest, size_t n) {
	memmove(dest, src, n);
}

// 60 ticks / second, i.e. 1 tick = 16.666... ms
static inline UInt32 TickCount(void)
{
	static ULONGLONG s_base_ms = 0;
	ULONGLONG now_ms = GetTickCount64();
	if (s_base_ms == 0) {
		srand((unsigned int)now_ms);
		s_base_ms = now_ms - rand() - rand();
	}

	ULONGLONG rel_ms = now_ms - s_base_ms;

	// exact integer conversion: ticks = floor(ms * 60 / 1000) == floor(ms * 3 / 50)
	// use 64-bit intermediates, return 32-bit (wraps after ~828 days of relative time)
	return (UInt32)((rel_ms * 3) / 50);
}

static inline void SysBeep(int _ignored) {
	MessageBeep(0xFFFFFFFF);
}

static inline void Alert(int _ignored, void *_ignored2) {
	MessageBeep(0xFFFFFFFF);
}

// ****************************************************************************

//#define abs(X) (((X) < 0 ) ? -(X) : (X))
//#define max(X,Y) ((X) > (Y) ? (X) : (Y))
//#define min(X,Y) ((X) < (Y) ? (X) : (Y))

// pillpickup_x and y are used to record the location of the last
// pillbox picked up in case it turns out to be fatal deep sea underneath

local const BrainInfo *info;
local long TimeNow;
local MAP_X nearx, pillpickup_x, fatal_x;
local MAP_Y neary, pillpickup_y, fatal_y;
local MAP_X tankleftx, tankrightx;
local MAP_Y tanktopy, tankbottomy;
local Boolean tank_close_to_mine;
local Boolean still_on_boat;	// Set until tank gets off boat
local BYTE land_direction;		// Initial guess at where the land is

// If cannot reach a target, get bored and ignore it after 20 seconds
#define ATTENTION_SPAN (60*20)
#define MAX_VIS_RANGE 0x7FFF
#define DEFENSIVE_RANGE 0xA00

typedef struct
	{
	u_long remaining_progress;	// Current 'best' point so far
	long last_progress;			// Last time progress was made
	long ignore_until;			// when we are bored with something
	} ProgressInfo;
#define MAX_PROGRESS 0xFFFFFFFF
local long boredomtime = ATTENTION_SPAN;
local ProgressInfo* tankprogress, * pillprogress;
local ProgressInfo (*baseprogress)[256]; // base idnums are unstable on WinBolo, so we track by [x][y]
local ProgressInfo scoutingprogress, manrescueprogress;
//local u_long previous_closest_dist = MAX_VIS_RANGE;
//local long current_attempt_time;

// Can place pillbox on swamp, crater, road, rubble and grass
static Boolean can_build_pillbox[NUM_TERRAINS] = { 0,0,1,1, 1,0,1,1, 0,0,0,0, 0,0 };

// Keyboard Control variables
local u_long keys, taps, last_keys, last_taps;

// Menu handling variables and mode settings
#define MyMenuID 1000
#if IMPLEMENTED
local MenuHandle MyMenu;
#endif
#if AUTO_DEBUG
local WindowPtr debugwindow, routewindow;
#endif

local Boolean do_explain   = 1;
local Boolean do_showroute = AUTO_DEBUG;

enum
	{
	m_about = 1,
	m_sep1, m_default, m_assassin, m_pillhunter, m_basehunter, m_defensive,
	m_sep2, m_explore, m_laymines, m_clearmines, m_dobuilding,
	m_attacktanks, m_attackpills, m_attackbases, m_repairpills, m_placepills, m_ppoffensive,
	m_sep3, m_explain, m_route
	};

typedef struct
	{
	Boolean explore, laymines, clearmines, dobuilding;
	Boolean attacktanks, attackpills, attackbases, repairpills, placepills, ppoffensive;
	} SETTINGS;

// Flags are:                         ex lm cm db at ap cb rp pp po
local const SETTINGS s_default    = { 1, 0, 1, 1, 1, 1, 1, 1, 1, 0 };
local const SETTINGS s_assassin   = { 1, 0, 1, 1, 1, 0, 0, 1, 1, 1 };
local const SETTINGS s_defensive  = { 0, 0, 1, 1, 1, 1, 0, 1, 1, 0 };
local const SETTINGS s_pillhunter = { 1, 0, 1, 1, 0, 1, 0, 1, 1, 0 };
local const SETTINGS s_basehunter = { 1, 0, 1, 1, 0, 0, 1, 1, 1, 0 };
local SETTINGS s;

// Thinking variables
local Boolean shootit, shootsoon, runaway;
#define SHOOT_NOTHING 0xFFFF
local OBJECT shootwhat;
local Boolean *shoot_path_costs;
local BYTE chosen_gunrange;		// Desired gun range, for shooting mines
local BYTE shoot_direction;		// to aim the gun accurately
local BYTE shoot_dir_vote;		// shoot_direction converted to range 0-15
local BYTE chosen_direction;
local BYTE flying_shells;		// total number of shells in flight at the moment
local BYTE incoming_shells;		// total number of shells heading towards us
local long direction_votes[16];
local u_long target_distances[16];
local BYTE direction_maxspeeds[16];

local MAP_X scoutx;
local MAP_Y scouty;
#define CAN_DO_BUILDING (info->build->action == 0 && info->man_status == 0 && \
						!info->inboat && flying_shells == 0)
#define CAN_BUILD_PILLBOX (CAN_DO_BUILDING && \
						s.placepills && info->carriedpills && info->trees >= 4)

typedef struct { char x; char y; } charpair;

typedef union
	{
	charpair s25[25];
	struct { charpair s1[1]; charpair s8[8]; charpair s16[16]; } s;
	} SURROUNDING_SQUARES;

local const SURROUNDING_SQUARES surrounding =
	{
	0,0,
	0,-1, 1,-1, 1,0, 1,1, 0,1, -1,1, -1,0, -1,-1,
	0,-2, 1,-2, 2,-2, 2,-1, 2,0, 2,1, 2,2, 1,2,
	0,2, -1,2, -2,2, -2,1, -2,0, -2,-1, -2,-2, -1, -2
	};

// ****************************************************************************

// Debugging message routines

#define EXPLAIN_WITH_MESSAGES 0
#define EXPLAIN_WITH_DEBUG 1

local void sendmessage(u_char *msg)
	{
	int i;	// Setting player bits to all zero makes this a debugger message
	for (i=0; i<(info->max_players+31)/32; i++) info->messagedest[i] = 0;
	for (i=0; i<=msg[0]; i++) info->sendmessage[i] = msg[i];
	}

local void debug(char *format, ...)
	{
	unsigned char buffer[256];
	va_list ptr;
	va_start(ptr,format);
	buffer[0] = vsprintf((char *)buffer+1, format, ptr);
	va_end(ptr);
	DebugStr(buffer);
	}

local Boolean SameString(const unsigned char *a, const unsigned char *b)
	{
	int i;
	for (i=0; i<=a[0]; i++) if (a[i] != b[i]) return(FALSE);
	return(TRUE);
	}

local void explain(char *format, ...)
	{
	if (do_explain)
		{
		static unsigned char old_buffer[256];
		unsigned char buffer[256];
		va_list ptr;
		va_start(ptr,format);
		buffer[0] = vsprintf((char *)buffer+1, format, ptr);
		va_end(ptr);
		
#if EXPLAIN_WITH_MESSAGES
		if (info->sendmessage[0] == 0 && !SameString(buffer, old_buffer))
			{ sendmessage(buffer); BlockMove(buffer, old_buffer, 1+buffer[0]); }
#elif EXPLAIN_WITH_DEBUG
		if (info->sendmessage[0] == 0 && !SameString(buffer, old_buffer))
		{
			OutputDebugString(buffer+1); OutputDebugString("\n"); BlockMove(buffer, old_buffer, 1 + buffer[0]);
		}
#else
		if (!SameString(buffer, old_buffer))
			{
			GrafPtr old;
			RgnHandle updateRgn = NewRgn();
			GetPort(&old);
			SetPort(debugwindow);
			TextFont(geneva);
			TextSize(10);
			ScrollRect(&debugwindow->portRect, 0, -12, updateRgn);
			DisposeRgn(updateRgn);
			MoveTo(debugwindow->portRect.left + 6, debugwindow->portRect.bottom - 6);
			DrawString(buffer);
			SetPort(old);
			BlockMove(buffer, old_buffer, 1+buffer[0]);
			}
#endif
		}
	}

// ****************************************************************************

// General utility routines

local TERRAIN raw_getmapcell(MAP_X x, MAP_Y y)
	{
	// This was the old way, when Brains were just given a small square of the map
/*	if (x < info->view_left || y < info->view_top) return(DEEPSEA);*/
/*	y -= info->view_top;*/
/*	x -= info->view_left;*/
/*	if (x >= info->view_width || y >= info->view_height) return(DEEPSEA);*/
/*	return(info->viewdata[y*info->view_width + x]);*/

	// This is the new way, now that Brains get a pointer to the whole world
	return(info->theWorld[(u_short)y << 8 | x]);
	}

// These two read the terrain / check for mine at MAP coordinates X,Y
#define getmapcellM(X,Y) (raw_getmapcell((X),(Y)) & TERRAIN_MASK)
#define checkmineM(X,Y)  (raw_getmapcell((X),(Y)) & TERRAIN_MINE)

// These two read the terrain / check for mine at WORLD coordinates X,Y
#define getmapcellW(X,Y) getmapcellM((X)>>8, (Y)>>8)
#define checkmineW(X,Y)  checkmineM((X)>>8, (Y)>>8)

local u_long findrange(WORLD_X x1, WORLD_Y y1, WORLD_X x2, WORLD_Y y2)
	{
	register long xdiff = (long)x1 - (long)x2;
	register long ydiff = (long)y1 - (long)y2;
	return(sqrt(xdiff*xdiff + ydiff*ydiff));
	}

#define center(w) ( ((w) & 0xFF00) | 0x80 )

local inline u_long objectrange_to_me(ObjectInfo* OB) {
	if (OB->object == OBJECT_PILLBOX || OB->object == OBJECT_REFBASE) {
		// these are always centered, but the brain.h convention appears to differ from MacBolo
		return findrange(center((OB)->x), center((OB)->y), info->tankx, info->tanky);
	}
	return findrange((OB)->x, (OB)->y, info->tankx, info->tanky);
}

// For Mac fixed point trig routines, full circle is 2 * PI * 0x10000
// We want full circle to be 0x100, so divide by (2 * PI * 0x10000 / 0x100)
// which is 1608 in decimal
#define aim(X,Y) ((FixATan2(-(Y),(X))/1608) & 0xFF)

// sin and cos routines are adjusted to take angles in the range 0-255
// and return return values in the range +/-128
#define sin(X) ((short)(FracSin((u_char)(X)*1608L) >> 23))
#define cos(X) ((short)(FracCos((u_char)(X)*1608L) >> 23))

local ObjectInfo *find_object(OBJECT obtype, MAP_X x, MAP_Y y)
	{
	ObjectInfo *ob;
	for (ob=&info->objects[0]; ob<&info->objects[info->num_objects]; ob++)
		if (ob->object == obtype && (ob->x >> 8) == x && (ob->y >> 8) == y) return(ob);
	return(NULL);
	}

local inline Boolean has_live_pill_at(MAP_X x, MAP_Y y)
{
	ObjectInfo* pb = find_object(OBJECT_PILLBOX, x, y);
	return pb && pb->pillbox_strength > 0;
}

local inline Boolean has_base_at(MAP_X x, MAP_Y y)
{
	return getmapcellM(x, y) == REFBASE_T;
}

static WORLD_X last_tx = -1;
static WORLD_Y last_ty = -1;

static inline Boolean moving(void) {
	return info->tankx != last_tx || info->tanky != last_ty;
}

static inline Boolean facing_diagonally(void) {
	int diff = info->direction & 0x3F;
	return diff >= 16 && diff <= 48;
}

// angles are [0..255], not [0..360), this rounds to the nearest 64 ticks=90 degrees,
// so clamps to the nearest N/S/East/West
static inline u_char clamp_to_cardinal(u_char a) {
	// round to nearest multiple of 64 (add 32 then zero lower 6 bits)
	return (u_char)((a + 32) & 0xC0);
}

static inline void cardinal_step(u_char dir, int* dx, int* dy) {
	switch (clamp_to_cardinal(dir)) {
	case 0x00: *dx = 0; *dy = -1; break; // N
	case 0x40: *dx = 1; *dy = 0; break; // E
	case 0x80: *dx = 0; *dy = 1; break; // S
	default: *dx = -1; *dy = 0; break; // W
	}
}

static inline Boolean is_blastable_obstruction(MAP_X x, MAP_Y y) {
	TERRAIN t = getmapcellM(x, y);
	//explain("Blastable? %d", t);
	return (t == BUILDING || t == HALFBUILDING);
}

static inline Boolean solid_ahead(void) {
	int dx, dy; cardinal_step(info->direction, &dx, &dy);
	int x = (int)nearx + dx;
	int y = (int)neary + dy;
	if (x < 0 || x > 0xFF || y < 0 || y > 0xFF) return FALSE;
	return is_blastable_obstruction((MAP_X)x, (MAP_Y)y);
}

// ****************************************************************************

// Shots can't pass through buildings, half buildings or pillboxes
// If we are chasing tank, also discourage wasting shots shooting forest
// Men cannot run through buildings, half buildings, pillboxes, refbases or water/deepsea
local BYTE stationary_target_path[NUM_TERRAINS] = { 5,  0,  0,  0,  0,  1,  0,  0,  4,  0,  0,  0, 16, 99 };
local BYTE moving_target_path    [NUM_TERRAINS] = { 5,  0,  0,  0,  0,  4,  0,  0,  4,  0,  0,  0, 16, 99 };
local BYTE running_man_path      [NUM_TERRAINS] = { 99, 99, 4,  4,  1,  2,  4,  1, 99,  1, 99, 99, 99, 99 };

// returns zero if there is a clear straight line path, from x,y, across terrain
// specified by terraincosts, to the target at tx,ty, in at most maxsteps steps
// (a step is half a map square)
// If there is not a clear path, then it returns the cost,
// measured in shots for shooting paths, and in time for man paths
local short path_cost(WORLD_X x, WORLD_Y y, Boolean *terraincosts,
	WORLD_X tx, WORLD_Y ty, WORD maxsteps)
	{
	BYTE direction = aim(tx - (long)x, ty - (long)y);
	MAP_X lastx = 0, targx = tx>>8;
	MAP_Y lasty = 0, targy = ty>>8;
	short steps = findrange(tx, ty, x, y) >> 7;	// count in half-square steps
	//short closest_step = 0;
	short cost = 0;
	if (steps > maxsteps) return(0x7FFF);

	// This first step is to make sure we skip over the initial square,
	// and don't count a cost for it
	x += sin(direction);
	y -= cos(direction);

	while (--steps > 0)
		{
		x += sin(direction);
		y -= cos(direction);
		if ((x>>8)==targx && (y>>8)==targy) break;		// OK, reached target
		if ((x>>8)==lastx && (y>>8)==lasty) continue;	// same square as last time -- skip it
		cost += terraincosts[getmapcellW(x,y)];
		}
	return(cost);
	}

// Man will not run over more than 5 squares of rubble, or 20 squares of road
#define MAN_PATH(X,Y) (path_cost(info->tankx,info->tanky,running_man_path,(X),(Y),0x7FFF) < 20)

// Note, must assume shots CAN travel over unknown terrain, otherwise,
// as soon as we leave the vicinity of an unsafe base, its terrain becomes
// unknown, and we would immediately return, assuming it safe 
local Boolean safe_base(ObjectInfo *base)
	{
	ObjectInfo *ob;
	for (ob=&info->objects[0]; ob<&info->objects[info->num_objects]; ob++)
		if (ob->object == OBJECT_PILLBOX && ob->pillbox_strength > 0 && (ob->info & OBJECT_HOSTILE))
			if (path_cost(ob->x, ob->y, stationary_target_path, base->x, base->y, 16) == 0)
				return(FALSE);
	return(TRUE);
	}

// ****************************************************************************

// Route finding routines

#define ARMOUR_UNIT_COST 8	// The armour cost scale counts eighths of a unit of tank armour
#define MAX_TIME_COST   0xFFFF
#define MAX_SHELL_COST  0xFF
#define MAX_ARMOUR_COST 0xFF
#define UNKNOWN_COST    0xFFFF
#define MAX_COST (UNKNOWN_COST-1)

// Notes:
// The 'squarecost' array holds the costs to reach the target from the given square --
// it forms a field for the tank to follow the steepest gradient downhill to its target.
// The 'cost' values in the CostPoints are this cost to reach the destination, PLUS
// a (mostly) conservative heuristic estimating the minimum cost for the tank to get
// to this square from its current position. This heuristic helps guide the search
// preferentially towards where the tank is now, rather then in fruitless directions
// away from it.

typedef struct			// Used to find best route through cost array
	{
	u_char x;			// Location in array (NOT in MAP coordinates)
	u_char y;
	u_short time;		// Time to reach destination from this square
	u_char shells;		// Shells required to reach destination from this square
	u_char armour;		// Armour required to reach destination from this square (scaled)
	u_char trees;
	u_char spare;
	u_char water;		// number of squares to be spent crossing water
	u_char deepsea;		// set if path will cross deep sea
	u_short cost;		// Overall 'cost' estimate to reach destination via this square
	} CostPoint;

#define COST_SEARCH_RANGE 14
// Tank can see the square it is on, and up to COST_SEARCH_RANGE
// above, below and to either side of it

#define COST_ARRAY_SIZE (COST_SEARCH_RANGE + 1 + COST_SEARCH_RANGE)
#define IN_COSTARRAY(X) ((unsigned)(X) < (unsigned)(COST_ARRAY_SIZE))

typedef u_char     u_charN   [COST_ARRAY_SIZE];				// Row of COST_ARRAY_SIZE unsigned char values
typedef u_short    u_shortN  [COST_ARRAY_SIZE];				// Row of COST_ARRAY_SIZE unsigned short values

local u_charN  pillcoverage[COST_ARRAY_SIZE];	// Array of rows of pillbox fire coverage
local u_shortN squarecost  [COST_ARRAY_SIZE];	// Array of rows of cost values
local u_shortN CPCost      [COST_ARRAY_SIZE];

local long costarray_time;		// time array was created
// position of array on map, position of target on map, and
// subtarget (target, or closest point if target is outside costarray)
local MAP_X costarray_left, costtarget_x, subtarget_x;
local MAP_Y costarray_top,  costtarget_y, subtarget_y;
local BYTE costarray_boat;		// Whether costarray calculated for boat or tank
local OBJECT costarray_targ;
local u_char costarray_tankhits;
// How many pillbox hits the tank's current position scores. Normally a two-hit square
// is bad news, but if we are already sitting on a three-hit square, then two suddenly
// looks quite desirable
local BYTE ca_tankx, ca_tanky;	// Position of tank in costarray coordinates

#define MAX_HEAP_SIZE (COST_ARRAY_SIZE * COST_ARRAY_SIZE >> 2)
local CostPoint cost_heap[MAX_HEAP_SIZE];	// Sorted heap of CostPoints
local int heap_size=0;

local void showroute(CostPoint c, u_short colour, u_short shotpoint)
	{
#if AUTO_DEBUG
	RGBColor col = { 0, 0, 0 };
	Rect r;
	u_short lowbits = (colour & 0x7F) * 0x180 + 0x4000;
	if (colour + 0x80 & 0x080) col.red   = lowbits;
	if (colour + 0x80 & 0x100) col.green = lowbits;
	if (colour + 0x80 & 0x200) col.blue  = lowbits;
	RGBForeColor(&col);
	r.top    = c.y * 8;
	r.left   = c.x * 8;
	r.bottom = r.top  + 8;
	r.right  = r.left + 8;
	if (shotpoint) InsetRect(&r, 1, 1);
	PaintRect(&r);
	//debug("%d,%d Cost %X Realcost %X", c.x, c.y, c.cost, colour);
#endif
	}

local void addtoheap(int place, CostPoint new)
	{
	int parent = (place-1)>>1;
	cost_heap[place]=new;
	while (place>0 && cost_heap[parent].cost > cost_heap[place].cost)
		{
		CostPoint temp    = cost_heap[place ];
		cost_heap[place ] = cost_heap[parent];
		cost_heap[parent] = temp;
		place=parent;
		parent=(place-1)>>1;
		}
	}

local CostPoint removefromheap(void)
	{
	CostPoint top = cost_heap[0];			// extract top element
	int gap   = 0;							// top is now empty
	int left  = 1;							// Left child is element 1
	int right = 2;							// Right child is element 2
	heap_size--;							// We have removed one element
	while(left<=heap_size)					// move gap to bottom
		{
		if(right>heap_size || cost_heap[right].cost > cost_heap[left].cost)
			{ cost_heap[gap]=cost_heap[left ]; gap=left;  }
		else{ cost_heap[gap]=cost_heap[right]; gap=right; }
		left  = (gap<<1)+1;
		right = (gap<<1)+2;
		}
	if(gap != heap_size) addtoheap(gap,cost_heap[heap_size]);
	return(top);							// return extracted element
	}

// Note: the cost in the CostPoint is biased with the search heuristic
// realcost is the actual cost to reach the destination from this point
local void setcost(CostPoint c, u_short realcost)
	{
	int i;
	Boolean already_present = (CPCost[c.y][c.x] < UNKNOWN_COST);
	CPCost[c.y][c.x]     = c.cost;
	squarecost[c.y][c.x] = realcost;

	// If adding a maxcost (which by definition cannot already be present
	// because there is no pre-existing cost > MAX_COST) then don't bother
	// putting it onto the heap (no point -- it cannot lead anywhere)
	if (c.cost == MAX_COST) return;

	// If already present, try to find and replace the element in place in the heap
	if (already_present)
		for (i=0; i<heap_size; i++)
			if (cost_heap[i].x == c.x && cost_heap[i].y == c.y)
				{ addtoheap(i, c); return; }

	// If heap already full nuke last element (shouldn't happen too often)
	if (heap_size >= MAX_HEAP_SIZE) heap_size--;
	addtoheap(heap_size++, c);
	}

local u_short route_square_time_costs[NUM_TERRAINS] =
	{
	10, 1, 8, 8,	// BUILDING, RIVER, SWAMP, CRATER
	 1, 3, 8, 2,	// ROAD, FOREST, RUBBLE, GRASS,
	 4, 1, 1, 1,	// HALFBUILDING, BOAT, DEEPSEA, REFBASE_T
	1000, 1000		// PILLBOX_T, TERRAIN_UNKNOWN
	};

// examine square in CostPoint c. The cost parameters have already been
// seeded up to this point -- the current square remains to be added.
local void examine(CostPoint c)
	{
	if (IN_COSTARRAY(c.x) && IN_COSTARRAY(c.y))
		{
		//Boolean examining_tanksquare = (c.x == ca_tankx && c.y == ca_tanky);
		MAP_X x = costarray_left + c.x;
		MAP_Y y = costarray_top  + c.y;
		u_long dx = abs((long)x-(long)nearx);
		u_long dy = abs((long)y-(long)neary);
		u_long cpcost, heuristic = max(dx,dy) + min(dx,dy) / 2;
		TERRAIN raw = raw_getmapcell(x,y);
		TERRAIN t   = raw & TERRAIN_MASK;
		short tankshells = info->shells;
		short tankarmour = info->armour * ARMOUR_UNIT_COST;
		short hits       = pillcoverage[c.y][c.x];
		long  new_timecost   = c.time + route_square_time_costs[t];
		short new_shellcost  = c.shells;
		short new_armourcost = c.armour;
		long newcost = 0;
		Boolean count_cost = TRUE;
		
		// The following part of the cost evaluation is skipped if we are:
		// (1) shooting something, and
		// (2) this is a not boat costarray or this square is not water, and
		// (3) this square has a clear path to the target
		// because once we have a vantage point to shoot
		// from, the cost of going further is irrelevant
		if (costarray_targ != SHOOT_NOTHING && (!costarray_boat || !is_wet(t)))
			{
			long cost = path_cost((WORLD_X)x<<8,(WORLD_Y)y<<8,shoot_path_costs,
									(WORLD_X)costtarget_x<<8,(WORLD_Y)costtarget_y<<8,15);
			if (cost == 0 || c.shells + 2 * cost < info->shells) count_cost = FALSE;
			}
		if (count_cost)
			{
			new_armourcost += route_square_time_costs[t] * (hits - costarray_tankhits);
			if (new_armourcost < 0) new_armourcost = 0;
			
			if (t == DEEPSEA) c.deepsea = TRUE;
			else if (t == RIVER) c.water++;
			else if (t == BUILDING)
				{
				if (tankshells < new_shellcost+8) newcost = MAX_COST;
				else new_shellcost += 5;
				}
			else if (t == HALFBUILDING)
				{
				if (tankshells < new_shellcost+4) newcost = MAX_COST;
				else new_shellcost += 4;
				}
			else if (t == REFBASE_T)
				{
				ObjectInfo *ob = find_object(OBJECT_REFBASE,x,y);
				if (ob && ob->info & OBJECT_HOSTILE) newcost = MAX_COST;
				}
			else if (has_live_pill_at(x, y))
				{
				// Live pillboxes are solid regardless of allegiance.
				newcost = MAX_COST;
				}
		
			if (raw & TERRAIN_MINE)
				{
				// If clearing mines and we have enough shells free,
				// count some time and one shell, else count one armour hit
				if (s.clearmines && tankshells > new_shellcost + 4)
					{ new_timecost += 8; new_shellcost++; }
				else new_armourcost += 3 * ARMOUR_UNIT_COST;
				}
			
			// Note: subtle stuff here. The 'cost' of water is carried unresolved,
			// like a quantum superposition of two different states, until the search
			// passes back onto land, where the cost is determined depending on whether
			// the path from the land is via a boat or not, like a quantum wave collapse.
			// The wave collapse can also be caused by the tank getting shot.
			// If the tank is already in water, then we pretend it is on a little square
			// of land, in order to precipitate that wave collapse and get the right answer
			if (c.x == ca_tankx && c.y == ca_tanky && !costarray_boat) t = ROAD;
			if (t==BOAT) c.water = c.deepsea = 0;			// on boat at this point, no cost
			else if (hits || (t != RIVER && t != DEEPSEA))	// no boat, so count cost
				{
				if (c.deepsea) newcost = MAX_COST;
				else { new_timecost += 7 * c.water; new_shellcost += 6 * c.water; }
				c.water = c.deepsea = 0;	// tentative cost now paid, so zero counters
				}
			}
		//else debug("%d, %d can shoot %d, %d", x, y, costtarget_x, costtarget_y);
		
		if (new_timecost   > MAX_TIME_COST)   new_timecost   = MAX_TIME_COST;
		if (new_shellcost  > MAX_SHELL_COST)  new_shellcost  = MAX_SHELL_COST;
		if (new_armourcost > MAX_ARMOUR_COST) new_armourcost = MAX_ARMOUR_COST;
	
		// If this route consumes more armour than we have, then it is unreachable
		if (tankarmour < new_armourcost) newcost = MAX_COST;
		
		// if newcost has not already been set to MAX_COST (ie impossible) then calculate it
		if (newcost < MAX_COST)
			{
			// Make sure we don't divide by zero
			if (tankshells > new_shellcost ) tankshells -= new_shellcost;  else tankshells = 1;
			if (tankarmour > new_armourcost) tankarmour -= new_armourcost; else tankarmour = 1;
			newcost = new_timecost + 100/tankshells + 200/tankarmour;
			if (newcost > MAX_COST) newcost = MAX_COST;
			}
		
		cpcost = newcost + heuristic;
		if (cpcost > MAX_COST) cpcost = MAX_COST;

		if (CPCost[c.y][c.x] > cpcost)	
			{
			c.time   = new_timecost;
			c.shells = new_shellcost;
			c.armour = new_armourcost;
			c.cost   = cpcost;
			
			// If we are not calculating a boat costarray, then the cost field generated
			// should include the water costs so that the tank is guided in the right
			// direction (which is NOT necessarily the same direction the search is taking).
			if (newcost < MAX_COST && !costarray_boat)
				{
				if (c.deepsea) newcost = MAX_COST;
				else
					{
					if (tankshells > 6 * c.water) tankshells -= 6 * c.water;
					else tankshells = 1;
					new_timecost  += 7 * c.water;
					newcost = new_timecost + 100/tankshells + 200/tankarmour;
					if (newcost > MAX_COST) newcost = MAX_COST;
					}
				}
			setcost(c,newcost);
			if (do_showroute) showroute(c, newcost, !count_cost);
			}
		}
	}

// ****************************************************************************

typedef void PAINT_FUNCTION(int x1, int x2, u_charN row);

local void p_increment(int  x1, int  x2, u_charN row)
	{
	if (x1 > COST_ARRAY_SIZE-1 || x2 < 0) return;
	if (x1 < 0                ) x1 = 0;					// clip to left edge
	if (x2 > COST_ARRAY_SIZE-1) x2 = COST_ARRAY_SIZE-1;	// clip to right edge
	while (x1<=x2) row[x1++]++;							// increment hit counts in this row
	}

#if 0
local void p_clear(int x1, int x2, u_charN row)
	{
	if (x1 > COST_ARRAY_SIZE-1 || x2 < 0) return;
	if (x1 < 0                ) x1 = 0;					// clip to left edge
	if (x2 > COST_ARRAY_SIZE-1) x2 = COST_ARRAY_SIZE-1;	// clip to right edge
	while (x1<=x2) row[x1++] = 0;						// clear hit counts in this row
	}
#endif

local void paint_circle(PAINT_FUNCTION p, BYTE cx, BYTE cy, BYTE radius)
	{
	BYTE x = radius, y = 0, y2 = radius;
	short d = 1-radius;
	if (IN_COSTARRAY(cy)) p(cx - x, cx + x, pillcoverage[cy]);
	while (x>y)
		{
		if (d<0) d += 2*y+3; else { d += 2*(y-x)+5; x--; }
		y++;
		if (IN_COSTARRAY(cy+y)) p(cx - x, cx + x, pillcoverage[(BYTE)(cy + y)]);
		if (IN_COSTARRAY(cy-y)) p(cx - x, cx + x, pillcoverage[(BYTE)(cy - y)]);
		if (x<y2 && y<y2)
			{
			BYTE x2 = y-1;
			if (IN_COSTARRAY(cy+y2)) p(cx - x2, cx + x2, pillcoverage[(BYTE)(cy + y2)]);
			if (IN_COSTARRAY(cy-y2)) p(cx - x2, cx + x2, pillcoverage[(BYTE)(cy - y2)]);
			y2--;
			}
		}
	}

local void scan_boundary(CostPoint c, int direction)
	{
	int i;
	BYTE *x = &c.x, *y = &c.y;
	if (direction<0) { x = &c.y; y = &c.x; }
	for (i=0; i<COST_SEARCH_RANGE; i++)
		{
		if ((*x) == 0 && (*y) > 0) (*y)--;
		else if ((*y) == COST_ARRAY_SIZE-1) (*x)--;
		else if ((*x) == COST_ARRAY_SIZE-1) (*y)++;
		else if ((*y) == 0) (*x)++;
		setcost(c, 0);
		if (do_showroute) showroute(c, 0, 0);
		}
	}

// ****************************************************************************

local void constrain_subtarget(MAP_X *tx, MAP_Y *ty)
	{
	MAP_X costarray_rightedge  = costarray_left + COST_ARRAY_SIZE-1;
	MAP_Y costarray_bottomedge = costarray_top  + COST_ARRAY_SIZE-1;
	if      (*tx < costarray_left      ) *tx = costarray_left;
	else if (*tx > costarray_rightedge ) *tx = costarray_rightedge;
	if      (*ty < costarray_top       ) *ty = costarray_top;
	else if (*ty > costarray_bottomedge) *ty = costarray_bottomedge;
	}

// If we are *shooting* at something, ttype will indicate what it is, so that we
// know we don't need to actually get there -- just get close enough to shoot it.
local u_short make_costarray(OBJECT ttype, MAP_X tx, MAP_Y ty, BYTE shells, BYTE armour)
	{
	CostPoint subtarget;
	ObjectInfo *ob;
	int x,y;
	
	// This calculation might take half a second, so better stop the tank while we think
	*(info->holdkeys) = *(info->tapkeys) = 0;
	// (For the astute reader who says "Hey, you can't do that!" the answer is
	// YES, these controls are 'live' -- they take effect immediately

	// Record info about this cost array
	costarray_time = TickCount();
	costarray_left = info->view_left;
	costarray_top  = info->view_top;
	costtarget_x   = subtarget_x = tx;
	costtarget_y   = subtarget_y = ty;
	costarray_boat = info->inboat;
	costarray_targ = ttype;
	constrain_subtarget(&subtarget_x, &subtarget_y);
	
	// Update tank local coordinates
	ca_tankx = nearx - costarray_left;
	ca_tanky = neary - costarray_top;

	// Check that tank falls within our visible region
	// (eg, on pillbox view, tank may not be within view)
	if (!IN_COSTARRAY(ca_tankx) || !IN_COSTARRAY(ca_tanky)) return(UNKNOWN_COST);

	// Initialize arrays
	for (y=0; y<COST_ARRAY_SIZE; y++)
		for (x=0; x<COST_ARRAY_SIZE; x++)
			{
			pillcoverage[y][x] = 0;
			squarecost[y][x] = CPCost[y][x] = UNKNOWN_COST;
			}
	
	for (ob=&info->objects[0]; ob<&info->objects[info->num_objects]; ob++)
		if (ob->object == OBJECT_PILLBOX && ob->pillbox_strength > 0 &&
			(ob->info & OBJECT_HOSTILE))
				{
				MAP_X x = ob->x >> 8;
				MAP_Y y = ob->y >> 8;
				paint_circle(p_increment, x-costarray_left, y-costarray_top, 8);
				paint_circle(p_increment, x-costarray_left, y-costarray_top, 6);
				paint_circle(p_increment, x-costarray_left, y-costarray_top, 4);
				}

	costarray_tankhits = pillcoverage[ca_tanky][ca_tankx];

// This code used to erase the target fire radius, before clear path finding was put
// into the examine routine
/*	if (ttype == OBJECT_PILLBOX || ttype == OBJECT_REFBASE)*/
/*		paint_circle(p_clear, tx-costarray_left, ty-costarray_top, 7);*/

	if (do_showroute && 0)		// This is to show the pillbox fire coverage array
		{
#if AUTO_DEBUG
		static const Pattern grey = { 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA };
		static const Rect tanklocation = { 112, 112, 120, 120 };
		CostPoint c;
		ForeColor(blackColor);
		for (c.y=0; c.y<COST_ARRAY_SIZE; c.y++)
			for (c.x=0; c.x<COST_ARRAY_SIZE; c.x++)
				showroute(c, pillcoverage[c.y][c.x] * 16, 0);
		Debugger();
		ForeColor(blackColor);
		FillRect(&routewindow->portRect, &qd.gray);
		PaintOval(&tanklocation);
#endif
		}

	// Start from (sub)target(s), with zero cost
	subtarget.x       = subtarget_x - costarray_left;
	subtarget.y       = subtarget_y - costarray_top;
	subtarget.time    = 0;			// (when we arrive, we are there)
	subtarget.shells  = shells;		// amount of shells and armour we want to arrive with
	subtarget.armour  = armour * ARMOUR_UNIT_COST;
	subtarget.water   = 0;
	subtarget.deepsea = 0;
	subtarget.cost    = 0;
	
	heap_size = 0;
	setcost(subtarget, 0);
	if (subtarget.x == 0 || subtarget.x == COST_ARRAY_SIZE-1 ||
		subtarget.y == 0 || subtarget.y == COST_ARRAY_SIZE-1)
			{ scan_boundary(subtarget, 1); scan_boundary(subtarget, -1); }
	while (heap_size)
		{
		CostPoint c = removefromheap();
		// Exit when we find a path to the tank
		if (c.x == ca_tankx && c.y == ca_tanky) return(c.cost);
		// otherwise, continue the search outwards
		c.y--; examine(c); c.y++;
		c.x++; examine(c); c.x--;
		c.y++; examine(c); c.y--;
		c.x--; examine(c);
		}
	explain("No route from tank to %d, %d", nearx, neary);
	//debug("No route from tank to %d, %d", nearx, neary);
	return(MAX_COST);
	}

// tx, ty say where we want to get to
// shells and armour say how much weponry we would like to arrive with
// returns FALSE if no route to the destination could be found
// NOTE: If the tanks current armour is less than the requested arrival armour,
// then ALL routes are failures. (The same hard rule does not apply to shells.)
// The tank will of course attempt to preserve armour, even if the hard minimum is zero
local Boolean find_best_route(OBJECT ttype, MAP_X tx, MAP_Y ty, BYTE shells, BYTE armour)
	{
	MAP_X subtarg_x = tx;
	MAP_Y subtarg_y = ty;
	int i, step = 1;
	long bestx = -1, besty;
	u_short bestcost = MAX_COST;
	TERRAIN best_terrain;
	static MAP_X nodiagx;		// Don't consider diagonal movement
	static MAP_Y nodiagy;		// when on this square
	static long max_costarray_age = 120;	// start with two seconds
	
	if (info->armour < armour) { explain("Insufficient armour for this objective"); return(FALSE); }
	
	// If tank stuck in maze, or very close to a mine, then
	// cutting corners with diagonal travel is not a good idea.
	// Should proceed very cautiously, slowly, from square to square
	if (info->tankobstructed || tank_close_to_mine || info->speed <= 0x18)
		{ nodiagx = nearx; nodiagy = neary; }
	if (nodiagx == nearx && nodiagy == neary) step = 2;

	ca_tankx = nearx - costarray_left;
	ca_tanky = neary - costarray_top;
	
	// If array is old, or for the wrong target, or wrong type of cost
	// array, or tank has moved too close to the edge, make a new one
	constrain_subtarget(&subtarg_x, &subtarg_y);
	if (TimeNow - costarray_time > max_costarray_age ||
		subtarget_x != subtarg_x || subtarget_y != subtarg_y ||
		costarray_boat != info->inboat || costarray_targ != ttype ||
		ca_tankx < 7 || ca_tankx >= COST_ARRAY_SIZE-7 ||
		ca_tanky < 7 || ca_tanky >= COST_ARRAY_SIZE-7)
		{
#if AUTO_DEBUG
		static const Pattern grey = { 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA };
		static const Rect tanklocation = { 112, 112, 120, 120 };
#endif
		u_short cost;
#if AUTO_DEBUG
		GrafPtr old;
#endif
		if (do_showroute)
			{
#if AUTO_DEBUG
			GetPort(&old);
			SetPort(routewindow);
			ForeColor(blackColor);
			FillRect(&routewindow->portRect, &qd.gray);
			PaintOval(&tanklocation);
#endif
			}
		cost = make_costarray(ttype,tx,ty,shells,armour);
		if (do_showroute)
			{
#if AUTO_DEBUG
			if (costarray_boat) ForeColor(whiteColor);
			else ForeColor(blackColor);
			PaintOval(&tanklocation);
			SetPort(old);
#endif
			}
		// Don't want to spend more than 5% of our time calculating cost arrays
		max_costarray_age = (TickCount() - costarray_time) * 20;
		if (max_costarray_age < 60 ) max_costarray_age = 60;
		if (max_costarray_age > 500) max_costarray_age = 500;
		if (cost == MAX_COST) return(FALSE);
		}

	for (i=0; i<8; i+=step)
		{
		BYTE x = ca_tankx+surrounding.s.s8[i].x;
		BYTE y = ca_tanky+surrounding.s.s8[i].y;
		if (IN_COSTARRAY(x) && IN_COSTARRAY(y) && bestcost > squarecost[y][x])
				{
				bestcost = squarecost[y][x];
				bestx = ((WORLD_X)(x + costarray_left)<<8) + 0x80;
				besty = ((WORLD_Y)(y + costarray_top )<<8) + 0x80;
				}
		}
	if (bestx == -1) {
		//explain("BUG? find_best_route fell through");
		return FALSE;
	}
	i = aim(bestx - (long)info->tankx, besty - (long)info->tanky);
	i = i + 8 >> 4 & 0xF;
	direction_votes[i] += 100;
	best_terrain = getmapcellW(bestx,besty);
	if (!info->inboat)
		{
		if (best_terrain == BOAT) direction_maxspeeds[i] = 32;
		// if (best_terrain == RIVER && info->trees > ?) build a bridge, or a boat??
		}
	return(TRUE);
	}

// ****************************************************************************

// Code to think about objects on the screen

// Calling reset_progress unconditionally resets 'progress' count
// and ignores object for 'ignoretime' ticks. (ignoretime==0 means *don't* ignore)
local void reset_progress(ProgressInfo *p, long ignoretime)
	{
	p->remaining_progress = MAX_PROGRESS;
	// Must forward set last_progress time, or we risk having it
	// marked 'boring' the moment the ingore timer expires
	p->last_progress = TimeNow + ignoretime;
	p->ignore_until  = TimeNow + ignoretime;
	}

// returns TRUE if still making progress
local Boolean register_progress(ProgressInfo *p, u_long remaining)
	{
	// if we are still making progress, or this is a stale record that
	// we haven't touched for a long time, then update it
	if (remaining < p->remaining_progress || TimeNow - p->last_progress > ATTENTION_SPAN * 2)
		{
		p->remaining_progress = remaining;
		p->last_progress = TimeNow;
		}
	else if (TimeNow - p->last_progress > ATTENTION_SPAN)
		{
		explain("No progress");
		reset_progress(p, boredomtime);
		return(FALSE);
		}
	return(TRUE);
	}

local void decide_shooting(OBJECT what, u_long dist, long tx, long ty, short strength)
	{
	if (info->shells)
		{
		shootwhat = what;
		shoot_path_costs = moving_target_path;
		if      (what == OBJECT_PILLBOX) shoot_path_costs = stationary_target_path;
		else if (what == OBJECT_REFBASE) shoot_path_costs = stationary_target_path;
		// Shootsoon is a safety so that the man does not
		// leave the tank just as we are about to engage an enemy
		if (dist < 0x900) shootsoon = TRUE;
		if (dist < 0x200) { shootit = TRUE; /*explain("shooting; line 1018");*/ }
		else if (dist < 0x780)
			{
			short cost = path_cost(info->tankx, info->tanky, shoot_path_costs, tx, ty, 0x7FFF);
			if (cost == 0 || strength + 2 * cost < info->shells)
				{
				shootit = TRUE;
				//explain("shooting; line 1025");
				}
			}
		}
	}

local inline ProgressInfo* getBaseProgress(ObjectInfo* ob) {
	return &baseprogress[ob->x >> 8][ob->y >> 8];
}

// Note: mustn't set "shells" (the target requirement) greater than 20 (wanted_shells)
// or the tank may decide that it doesn't want to refuel because it has > 20 shells
// but decides it cannot attack its target because it has < (say) 30 shells

local u_long check_objects(void)
	{
	int i;
	long tx, ty;
	BYTE shells = 10, armour = 0;
	ProgressInfo dummy;
	ProgressInfo *progresstarget = &dummy;
	ObjectInfo *ob;
	ObjectInfo *nearest_tank = NULL;
	ObjectInfo *nearest_pill = NULL, *nearest_frnd   = NULL;
	ObjectInfo *nearest_base = NULL, *nearest_refuel = NULL;
	u_long dist;
	u_long max_dist = (s.explore || s.attacktanks || s.attackpills || s.attackbases)
		? MAX_VIS_RANGE : DEFENSIVE_RANGE;
	u_long tankd = max_dist, pilld = max_dist, frndd = max_dist;
	u_long based = max_dist, refueld = max_dist;
	u_long limit_dist = max_dist;
	Boolean decided = FALSE;
	BYTE wanted_shells = 30, useful_shells = 6;
	static BYTE last_armour_value = 0;
	
	// If we are on a refuelling base then we want full shells, and we wait
	// until we have got every last shell from the base, otherwise we
	// will put up with as few as 20 for general trundling around the map
	// and not be attracted by bases that have less than 6 shells to give.
	if (getmapcellW(info->tankx, info->tanky) == REFBASE_T)
		{ wanted_shells = 40; useful_shells = 1; }
	
	// If we have some information about a nearby base (which we are not ignoring)
	if (info->base && TimeNow - getBaseProgress(info->base)->ignore_until >= 0)
		{
		dist = objectrange_to_me(info->base);

		// The following two apply to bases we are sitting on
		if (dist < 0x80)
			{
			// If we are losing armour (being shot somehow) then ditch this base
			if (info->armour < last_armour_value)
				{
				reset_progress(getBaseProgress(info->base), boredomtime);
				explain("Abandon base! %d, %d", info->armour, last_armour_value);
				}
			// If base has just fully refuelled us, mark it is "not boring"
			if (info->shells == 40 && info->armour == 8)
				reset_progress(getBaseProgress(info->base), 0);
			}
		
		// Do we WANT to refuel?
		if ((info->shells < wanted_shells || info->armour < 8) && safe_base(info->base))
			{
			// OK, if base can supply the requirements, make it the "nearest_refuel"
			Boolean get_shells = (info->shells < wanted_shells && info->base_shells >= useful_shells);
			Boolean get_armour = (info->armour < 8             && info->base_armour > MIN_BASE_ARMOUR);
			if (get_shells || get_armour) { nearest_refuel = info->base; refueld = dist; }
			else
				{
				explain("Refuelling base exhausted");
				reset_progress(getBaseProgress(info->base), boredomtime);
				}
			// else if base can't supply the requirements, mark it boring, so that
			// after we go far enough away to lose status reporting from it,
			// we still know not to bother coming back here for a while
			}
		}

	last_armour_value = info->armour;

// bug fix for 0.99.2b
#define GOODTANK (ob->x > 0x1000 && ob->x < 0xF000 && ob->y > 0x1000 && ob->y < 0xF000)

	for (ob=&info->objects[0]; ob<&info->objects[info->num_objects]; ob++)
		{
		switch (ob->object)
			{
			case OBJECT_TANK:
				if (GOODTANK && TimeNow - tankprogress[ob->idnum].ignore_until >= 0)
					if (ob->info & OBJECT_HOSTILE)
						if ((dist = objectrange_to_me(ob)) < tankd)
							{ nearest_tank = ob; tankd = dist; }
				break;
			case OBJECT_PILLBOX:
				if (ob->pillbox_strength > 0 &&
					pillprogress[ob->idnum].ignore_until - TimeNow > boredomtime)
						reset_progress(&pillprogress[ob->idnum], boredomtime);
				if (TimeNow - pillprogress[ob->idnum].ignore_until >= 0)
					{
					short strength = ob->pillbox_strength;
					dist = objectrange_to_me(ob);
					if (strength == 0)						// Dead pillboxes
						{
						dist += 16*256;
						if (fatal_x == ob->x >> 8 && fatal_y == ob->y >> 8)
							reset_progress(&pillprogress[ob->idnum], 6*60*60*60);
						else if (dist < pilld) { nearest_pill = ob; pilld = dist; }
						}
					else if (ob->info & OBJECT_HOSTILE || ob->info & OBJECT_NEUTRAL)	// Hostile pillboxes
						{
						dist += strength*256;
						if (dist < pilld) { nearest_pill = ob; pilld = dist; }
						}
					else if (strength < 15)					// Friendly damaged pillbox
						{
						if ((dist = objectrange_to_me(ob)) < frndd)
							{ nearest_frnd = ob; frndd = dist; }
						}
					}
				break;
			case OBJECT_REFBASE:
				if (TimeNow - getBaseProgress(ob)->ignore_until >= 0 && ob != info->base)
					{
					if (ob->info & OBJECT_HOSTILE)
						{
						if (s.attackbases && info->shells)
							if ((dist = objectrange_to_me(ob)) < based)
								{ nearest_base = ob; based = dist; }
						}
					else
						{
						if (info->shells < wanted_shells || info->armour < 8)
							if ((dist = objectrange_to_me(ob)) < refueld && safe_base(ob))
								{ nearest_refuel = ob; refueld = dist; }
						}
					}
				break;
			}
		}
	
	// pilld was weighted to account for pillbox strength -- fix it now
	if (nearest_pill) pilld = objectrange_to_me(nearest_pill);
	// (We prefer to attack weak pillboxes instad of strong ones, except dead
	// pillboxes get left till last because they pose no immediate threat.)

	// If damaged pillbox nearby, fix it
	if (s.repairpills && nearest_frnd && frndd < DEFENSIVE_RANGE &&
		info->trees && CAN_DO_BUILDING && MAN_PATH(nearest_frnd->x, nearest_frnd->y))
		{
		info->build->x = nearest_frnd->x >> 8;
		info->build->y = nearest_frnd->y >> 8;
		info->build->action = BUILDMODE_PBOX;
		}

	// If carrying a pillbox, and we have a hostile tank nearby, use it?
	if (CAN_BUILD_PILLBOX && s.ppoffensive && tankd < 0xD00)
		{
		MAP_X x = (info->tankx + nearest_tank->x) >> 9;
		MAP_Y y = (info->tanky + nearest_tank->y) >> 9;
		TERRAIN raw = raw_getmapcell(x, y);
		if (!(raw & TERRAIN_MINE) && can_build_pillbox[raw & TERRAIN_MASK] &&
			MAN_PATH((WORLD_X)x<<8, (WORLD_Y)y<<8))
			{
			info->build->x = x;
			info->build->y = y;
			info->build->action = BUILDMODE_PBOX;
			}
		}

	if (limit_dist > based  ) limit_dist = based;
	if (limit_dist > refueld) limit_dist = refueld;
	limit_dist = (limit_dist * 2) + DEFENSIVE_RANGE;
	if (limit_dist > max_dist) limit_dist = max_dist;
	// limit_dist is to limit our search, so we don't go to
	// the other side of the map for a dead pillbox when
	// there is something more important right next to us.

	// Special priority: don't leave man outside tank
	if (TimeNow - manrescueprogress.ignore_until >= 0 && info->man_status > 1)
		{
		tx = info->man_x;
		ty = info->man_y;
		dist = findrange(tx, ty, info->tankx, info->tanky);
		progresstarget = &manrescueprogress;
		if (info->manobstructed || dist > 0x200) { decided = TRUE; explain("Rescuing Man"); }
		}

	// First priority: hostile tanks/pillboxes which are within shooting range
	
	// If we have a nearby tank, but we're not attacking tanks, cancel it
	if (tankd < limit_dist && !s.attacktanks) tankd = limit_dist;

	// If we have a nearby pillbox, but we're not attacking pillboxes, cancel it
	// (unless it is a complete pushover)
	if (pilld < limit_dist && !s.attackpills)
		if (nearest_pill->pillbox_strength > info->armour ||
			nearest_pill->pillbox_strength > info->shells * 2) pilld = limit_dist;

	if (!decided && (tankd < limit_dist || pilld < limit_dist))
		{
		if (tankd < pilld)	// We have a hostile tank approaching
			{
			BYTE required_armour = (tankd > DEFENSIVE_RANGE) ? 6 : incoming_shells;
			Boolean wants_fight = info->shells > 5 && info->armour > required_armour;
			if (tankd < 0x800 || wants_fight)
				{
				dist = tankd;
				tx = nearest_tank->x;
				ty = nearest_tank->y;
				progresstarget = &tankprogress[nearest_tank->idnum];
				// No special register_progress -- just be content to close distance
				decided = TRUE;
				if (wants_fight)
					{
					shells = 20; armour = required_armour;
#if VERBOSE
					explain("Attacking tank: %lu, %lu (%ld, %ld)",
						tx>>8, ty>>8, (tx-info->tankx)>>8, (ty-info->tanky)>>8);
#else
					explain("Attacking tank: %lu, %lu", tx >> 8, ty >> 8);
#endif
					decide_shooting(OBJECT_TANK, dist, tx, ty, 9);
					}
				else
					{
#if VERBOSE
					explain("Fleeing tank: %lu, %lu (%ld, %ld)",
						tx>>8, ty>>8, (tx-info->tankx)>>8, (ty-info->tanky)>>8);
#else
					explain("Fleeing tank: %lu, %lu", tx >> 8, ty >> 8);
#endif
					runaway = TRUE;
					}
				}
			}
		else				// Nearest thing is a hostile pillbox
			{
			BYTE strength = nearest_pill->pillbox_strength;
			BYTE required_shells = strength + strength/2;
			BYTE required_armour = incoming_shells + strength ? (2 + (strength+2)/3) : 0;
			
			// ???? required_armour USED to say this:
			// (pilld > DEFENSIVE_RANGE) ? 6 : incoming_shells + 1 + strength/4;
			// but this made it afraid to pick up dead pillboxes if tank had no armour!
			
			Boolean wants_fight = info->shells >= strength && info->armour >= required_armour;
			
			// Consider pillbox if any of:
			// 1. It is dangerously close (do I still need this?)
			// 2. Want to fight it
			// 3. It's fighting us
			// 4. It is dead and waiting to be picked up
			if (/*pilld < 0x400 || */wants_fight || incoming_shells > 1 || strength == 0)
				{
				dist = pilld;
				tx = nearest_pill->x;
				ty = nearest_pill->y;
				progresstarget = &pillprogress[nearest_pill->idnum];
				if (pilld < 0x180 && strength == 0)
					{ pillpickup_x = tx>>8; pillpickup_y = ty>>8; }
				decided = TRUE;
				if (wants_fight)
					{
					shells = 20; armour = required_armour;
#if VERBOSE
					explain("Attacking pill: %lu, %lu (%ld, %ld)",
						tx>>8, ty>>8, (tx-info->tankx)>>8, (ty-info->tanky)>>8);
#else
					explain("Attacking pill: %lu, %lu", tx >> 8, ty >> 8);
#endif
					if (strength > 0) decide_shooting(OBJECT_PILLBOX, dist, tx, ty, strength);
					if (shootit) register_progress(progresstarget, strength);
					// If we are going to shoot the pillbox, then our criterion of
					// making progress is whether we are inflicting any damage
					}
				else
					{
#if VERBOSE
					explain("Fleeing pill: %lu, %lu (%ld, %ld)",
						tx>>8, ty>>8, (tx-info->tankx)>>8, (ty-info->tanky)>>8);
#else
					explain("Fleeing pill: %lu, %lu", tx >> 8, ty >> 8);
#endif
					runaway = TRUE;
					}
				}
			}
		}

	// Second priority: Attack some enemy bases
	if (!decided && based < limit_dist)
		{
		dist = based;
		tx = nearest_base->x;
		ty = nearest_base->y;
		progresstarget = getBaseProgress(nearest_base);
		decided = TRUE;
#if VERBOSE
		explain("Attack Base: %lu, %lu (%ld, %ld)",
			tx>>8, ty>>8, (tx-info->tankx)>>8, (ty-info->tanky)>>8);
#else
		explain("Attack Base: %lu, %lu", tx >> 8, ty >> 8);
#endif
		shells = 20; armour = 0;	// Don't need armour, but do need lots of shells
		if (nearest_base->refbase_strength) decide_shooting(OBJECT_REFBASE, dist, tx, ty, 30);
		}

	// Third priority: Do we need to refuel?
	if (!decided && refueld < limit_dist)
		{
		dist = refueld;
		tx = center(nearest_refuel->x);
		ty = center(nearest_refuel->y);
		progresstarget = getBaseProgress(nearest_refuel);
		// If we're on base; don't get bored
		if (dist < 0x80)
			{
			register_progress(progresstarget, 80 - info->shells - info->armour*5);
			for (i=0; i<16; i++) target_distances[i] = 0;
			}
		decided = TRUE;
#if VERBOSE
		explain("Refuel: %lu, %lu (%ld, %ld)",
			tx>>8, ty>>8, (tx-info->tankx)>>8, (ty-info->tanky)>>8);
#else
		explain("Refuel: %lu, %lu", tx >> 8, ty >> 8);
#endif
		shells = 0; armour = 0;	// Don't care what we arrive with, just get us there
		}

	// If not decided on anything, then there is nothing to be bored with
	if (!decided)
		{
		if (!s.explore) for (i=0; i<16; i++) target_distances[i] = 0;
		return(MAX_VIS_RANGE);
		}
	
	register_progress(progresstarget, dist);
	
	shoot_direction = aim(tx - (long)info->tankx, ty - (long)info->tanky);
	shoot_dir_vote  = shoot_direction + 8 >> 4 & 0xF;
	if (runaway)
		{
		reset_progress(progresstarget, 0);	// Don't get bored while running away!
		for (i=-4; i<=4; i++) direction_votes[shoot_dir_vote+i & 0xF] -= 20;
		for (i=5; i<=11; i++) direction_votes[shoot_dir_vote+i & 0xF] += 20;
		}
	else
		{
		target_distances[shoot_dir_vote] = dist;
		// If shooting or VERY close to target then lock on directly
		if (shootit || dist < 0x100) direction_votes[shoot_dir_vote] += 100;
		else
			{
			// find best route to target, if possible
			if (!find_best_route(shootwhat, tx>>8, ty>>8, shells, armour))
				{
				explain("No route");
				reset_progress(progresstarget, boredomtime);
				}
			// Planned successfully; suppress terrain this tick
			else return(0);
			}
		}
	return(dist);
	}

// ****************************************************************************

// Code to actively explore the map for things to do

local void setscout(void)
	{
	// Pick a new location to head towards
	scoutx = 53 + 10 * (TimeNow & 15);
	scouty = 53 + 10 * (TimeNow>>4 & 15);
	explain("Scouting %d,%d", scoutx, scouty);
	reset_progress(&scoutingprogress, 0);
	}

local void scout(void)
	{
	u_long dist = findrange((WORLD_X)scoutx << 8, (WORLD_Y)scouty << 8,
												info->tankx, info->tanky);

	if (!register_progress(&scoutingprogress, dist)) setscout();
	
	// if at scout target, pick another one
	if (nearx == scoutx && neary == scouty)
		{
		explain("Arrived (Scouting %d,%d)", scoutx, scouty);
		setscout();
		}

	// If failed to find route, or route goes into deep sea, pick another scout target
	if (!find_best_route(-1, scoutx, scouty, 10, 2) ||
		getmapcellM(subtarget_x, subtarget_y) == DEEPSEA)
			{
			explain("No route (Scouting %d,%d)", scoutx, scouty);
			setscout();
			}
	}

// ****************************************************************************

// Code to think about what kind of terrain we should try to get/stay on
// Simple kind of insect behaviour -- just look at the nearby terrain and
// decide which is nicest

local long terrain_desirability[NUM_TERRAINS] =
	{
	-10, -2, -1, -1,	// BUILDING, RIVER, SWAMP, CRATER
	5, 0, -1, 0,		// ROAD, FOREST, RUBBLE, GRASS,
	-10, 1, -100, 0,	// HALFBUILDING, BOAT, DEEPSEA, REFBASE_T,
	-10, -10			// PILLBOX_T, TERRAIN_UNKNOWN
	};

#define desirability(X,Y) \
(terrain_desirability[getmapcellM((X),(Y))] + (checkmineM((X),(Y)) ? -500 : 0))

local void check_terrain(void)
	{
	int i;
	long des = desirability(nearx, neary);
	terrain_desirability[RIVER  ] = info->inboat ? 0 : -2;
	terrain_desirability[DEEPSEA] = info->inboat ? -10 : -100;
	for (i=0; i<8; i++)
		{
		long val = desirability(nearx+surrounding.s.s8[i].x,
								neary+surrounding.s.s8[i].y) - des;
		direction_votes[i*2-2 & 0xF] += val;
		direction_votes[i*2-1 & 0xF] += val*2;
		direction_votes[i*2        ] += val*5;
		direction_votes[i*2+1      ] += val*2;
		direction_votes[i*2+2 & 0xF] += val;
		}

	for (i=0; i<16; i++)
		{
		long val = desirability(nearx+surrounding.s.s16[i].x,
								neary+surrounding.s.s16[i].y) - des;
		direction_votes[i-1 & 0xF] += val>>1;
		direction_votes[i        ] += val;
		direction_votes[i+1 & 0xF] += val>>1;
		}

	// If on road, try to keep tank centred.
	if (getmapcellM(nearx,neary) == ROAD)
		{
		Boolean go_down  = getmapcellW(info->tankx, info->tanky - 0x60) != ROAD;
		Boolean go_up    = getmapcellW(info->tankx, info->tanky + 0x60) != ROAD;
		Boolean go_right = getmapcellW(info->tankx - 0x60, info->tanky) != ROAD;
		Boolean go_left  = getmapcellW(info->tankx + 0x60, info->tanky) != ROAD;
		if (go_down + go_up + go_right + go_left == 1)
			{
			BYTE dir = go_right * 4 + go_down * 8 + go_left * 12;
			for (i=-3; i<=3; i++) direction_votes[dir+i & 0xF] += 10;
			}
		}
	}

// ****************************************************************************

// Code to organize voting about which direction we should be going in

local void cast_votes(void)
	{
	u_long target_distance;
	BYTE d = info->direction + 8 >> 4 & 0xF;

	// If we are clearing mines, and we are dangerously close
	// to a mine right now, should just try to get off it so
	// that we get a chance for a safe shot at it.
	if (s.clearmines && info->shells && tank_close_to_mine)
		{ check_terrain(); return; }

	// Bias towards continuing in the same direction
	direction_votes[d-1 & 0xF] += 1;
	direction_votes[d+0 & 0xF] += 2;
	direction_votes[d+1 & 0xF] += 1;
	direction_votes[d+7 & 0xF] -= 5;
	direction_votes[d+8 & 0xF] -=10;
	direction_votes[d+9 & 0xF] -= 5;

	target_distance = check_objects();
	
	// If only one square between us and the hostile,
	// then terrain doesn't matter any more
	if (target_distance < 0x180) return;
	
	// If nothing within sight, then explore?
	if (target_distance == MAX_VIS_RANGE && s.explore) scout();
	else			// else just make local decision
		{
		if (still_on_boat) direction_votes[land_direction>>4] += 20;
		check_terrain();
		}
	}

// ****************************************************************************

// Code to build bridges etc where necessary. Returns TRUE if decided to build

local Boolean decide_building(MAP_X x, MAP_Y y)
	{
	TERRAIN raw = raw_getmapcell(x, y);
	if (raw & TERRAIN_MINE) return FALSE;
	info->build->x = x;
	info->build->y = y;
	switch (raw & TERRAIN_MASK)
		{
		case RIVER    :
		case SWAMP    :
		case CRATER   :
		case RUBBLE   : if (info->trees >= 2)
							info->build->action = BUILDMODE_ROAD;
						break;
		case FOREST   : if (info->trees < 40)
							info->build->action = BUILDMODE_FARM;
						break;
		}
	return(info->build->action != 0);
	}

// ****************************************************************************

// Code to clear mines from in front of the tank

local MAP_X shotminex;		// Mine we have just shot at -- so ignore it
local MAP_Y shotminey;		// when looking for other mines to shoot
local long shotminetime;	// because it will be gone in a moment.

local MAP_X currentminex;	// The mine we plan to shoot at next
local MAP_Y currentminey;

// Don't even consider trying to hit mines more than 0x60 from the centre.
#define MINE_HIT_ERROR_LIMIT 0x60

local void minesweep(Boolean *foundmine, Boolean *doshoot)
	{
	char a, best_a;
	short r, best_r = 0;
	
	// Want to try to hit mines as close to the centre as possible.
	BYTE best_hit = MINE_HIT_ERROR_LIMIT;
	
	// If we already have something to shoot at, only worry about nearby mines
	short max_range = shootit ? 6 : 12;
	
	if (TimeNow - shotminetime > 60) shotminex = 0;
	// After shooting at a mine, if, for some unknown reason, it is not
	// gone within one second, then we should take another shot at it
	
	for (a = -32; a<=32; a+=8)	// sweep left and right of where tank is going
		{
		Boolean through_forest = FALSE;
		for (r=2; r<max_range; r++)	// check up to max_range in front of tank
			{
			WORLD_X wx = info->tankx + sin(chosen_direction + a) * r;
			WORLD_Y wy = info->tanky - cos(chosen_direction + a) * r;
			MAP_X x = wx >> 8;		// x,y are square we are checking
			MAP_Y y = wy >> 8;
			TERRAIN raw = raw_getmapcell(x, y);
			TERRAIN t = raw & TERRAIN_MASK;
			
			// Don't try to shoot through buildings or pillboxes
			if (t == BUILDING || t == HALFBUILDING || has_live_pill_at(x, y)) break;
			
			if (raw & TERRAIN_MINE)		// If there is a mine on this square...
				{
				// Calculate how far from centre of square the shell will land
				BYTE dx = wx & 0xFF;	// dx,dy indicate position within square
				BYTE dy = wy & 0xFF;
				if (dx > 0x80) dx = dx-0x80; else dx = 0x80-dx;
				if (dy > 0x80) dy = dy-0x80; else dy = 0x80-dy;
				if (dx < dy) dx = dy;
				
				if (dx < MINE_HIT_ERROR_LIMIT)	// We want to shoot this
					{
					// Don't shoot the mine if we are so close it would damage us
					if ((x == tankleftx || x == tankrightx) &&
						(y == tanktopy  || y == tankbottomy)) continue;
					
					// If there is a mine very close to us, then take note of it
					// even if we can't (or don't want to) shoot it immediately.
					if (r<4) *foundmine = TRUE;
					
					// If we have already shot at the mine, ignore it (for now).
					if (x == shotminex && y == shotminey) continue;
					
					// See if this is our best chance at a clean hit
					if (best_hit > dx)
						{
						best_hit = dx;
						best_r = r;
						best_a = a;
						// If firing through forest, we need multiple shots
						// so don't assume shot will hit the mine first time
						if (through_forest) currentminex = 0;
						else { currentminex = x; currentminey = y; }
						}
					}
				}

			// If shot would pass through forest, make a note of the fact
			if (t == FOREST) through_forest = TRUE;
			
			// If we are on a boat, we can't shoot past the shore
			if (info->inboat && t != RIVER && t != DEEPSEA) break;
			}
		}

	if (best_r)
		{
		chosen_gunrange = best_r;
		chosen_direction = chosen_direction + best_a;
		*foundmine = *doshoot = TRUE;
		}
	}

// ****************************************************************************

// A bit of geometry, to see if man lies close to path of shot

local Boolean man_clear_of_shot(void)
	{
	long x = (long)info->man_x - (long)info->tankx;		// displacement of man relative to tank
	long y = (long)info->man_y - (long)info->tanky;
	// Now do scalar product with tank's (unit) direction vector to find
	// the point on the line of the shot path which is closest to the man
	long d = (x * sin(info->direction) - y * cos(info->direction)) >> 7;
	//debug("Man at %ld, %ld, range %ld, error %ld;g", x, y, d);
	// Now make x and y be the displacement of the man relative to that
	// closest point (calculated by d times the unit direction vector).
	x -= (d * sin(info->direction)) >> 7;
	y += (d * cos(info->direction)) >> 7;	// (plus because x=sin, y=-cos)
	//debug("Man range %ld, error %ld;g", d, sqrt(x*x + y*y));
	// Man is safe if he is:
	// 1. Somewhere behind the tank
	// 2. More than ten squares in front of the tank
	// 3. More than two squares either side of the path of the shot
	return(d < 0 || d > 2560 || sqrt(x*x + y*y) > 512);
	}

// Code to count the votes and decide what action to take

local void count_votes(void)
	{
	int i, best = 0;
	u_long target_distance;
	u_long desired_dist = 0x20;	// Approach bases gently, and don't overshoot
	BYTE max_speed;
	char correction = 0;
	Boolean panic = (getmapcellM(nearx,neary) == RIVER || /*info->inboat || */runaway);
	Boolean gofaster = FALSE, goslower = FALSE;
	Boolean fullstop = FALSE, killmine = FALSE;
	static u_long lastFuelTick = 0;

	for (i=1; i<16; i++) if (direction_votes[best] < direction_votes[i]) best = i;

	if (best == shoot_dir_vote) chosen_direction = shoot_direction;
	else { chosen_direction = best<<4; shootit = FALSE; }
	target_distance = target_distances[best];
	max_speed = direction_maxspeeds[best];

	// If we have some shells, and we are not on a mine,
	// then think about clearing mines
	if (s.clearmines && info->shells && !checkmineM(nearx,neary))
		minesweep(&killmine, &shootit);

	if (target_distance > 0x20 || killmine)
		correction = (char)(info->direction - chosen_direction);
	
	// Decide whether to turn, or possibly shoot
	if      (correction < -9) setkey(keys, KEY_turnright);
	else if (correction >  9) setkey(keys, KEY_turnleft);
	else if (correction < -1) setkey(taps, KEY_turnright);
	else if (correction >  1) setkey(taps, KEY_turnleft);
	else if (shootit && info->gunrange == chosen_gunrange)
		{		// OK, we are on target, and we want to shoot
		shootsoon = TRUE;
		// Check man either inside tank, or safely away from the shot
		if (info->man_status == 0 || man_clear_of_shot())
			{
			if (killmine)		// If clearing mine, do this:
				{
				// Want to make sure tank is not turning, adjusting range etc.
				// to be sure we will hit the mine and not waste a shell
				u_long testkeys = 1<<KEY_turnleft  | 1<<KEY_turnright
								| 1<<KEY_morerange | 1<<KEY_lessrange
								| 1<<KEY_shoot;
				if ((testkeys & (last_keys | last_taps)) == 0)
					{
					// Tap shoot key, to fire ONE shot at the mine
					setkey(taps, KEY_shoot);
					//explain("shooting; line 1721");
					shotminex = currentminex;
					shotminey = currentminey;
					shotminetime = TimeNow;
					}
				}
			else				// else shooting tank or pillbox; do this:
				{
				setkey(keys, KEY_shoot);
				//explain("shooting; line 1730");
				if (shootwhat == OBJECT_PILLBOX) desired_dist = 0x700;
				else desired_dist = 0x400;
				// Get slightly closer when shooting a tank, in case it tries to run away
				}
			}
	}

	// 1. If tank facing in wrong direction by more than +/- 45 degrees,
	//    or if speed is too high, then slow down.
	//    Also, consider building road or bridge under the tank, if necessary
	if (correction < -32 || correction > 32 || info->speed > max_speed)
		{
		goslower = TRUE;
		// Decide whether we should be building or farming here
		if (s.dobuilding && CAN_DO_BUILDING && !shootsoon) decide_building(nearx,neary);
		}
	else
		{
		// 2. If tank is facing in roughly the right direction, consider
		//    speeding up or slowing down, according to distance to target
		MAP_X examinex = info->tankx + sin(info->direction) >> 8;
		MAP_Y examiney = info->tanky - cos(info->direction) >> 8;
		TERRAIN t = getmapcellM(examinex, examiney);
		if (!info->inboat && t == DEEPSEA) fullstop = TRUE;
		
		if (info->inboat) gofaster = TRUE;
		else
			{
			if (target_distance > desired_dist + (info->speed<<4)) gofaster = TRUE;
			if (target_distance < desired_dist + (info->speed<<3)) goslower = TRUE;
			}

		// 3a. If we want to go, but we are obstructed, then shoot our way out (avoid shooting friendly pills)
		if (correction > -8 && correction < 8 && !info->inboat && info->tankobstructed
			&& !moving() && !facing_diagonally() && solid_ahead())
		{
			// prevent building/etc while blasting
			shootsoon = TRUE;

			if (info->man_status == 0 || man_clear_of_shot())
			{
				setkey(taps, KEY_shoot);
				//explain("yup, shooting.");
			}
		}

		// 3b. If tank is facing in the correct direction, and moving forwards,
		//     see if it would be helpful to build road or bridge in front of us
		if (correction > -8 && correction < 8 && (gofaster || panic))
			{
			if (s.dobuilding && CAN_DO_BUILDING && !shootsoon)
				{
				int closest = 1, farthest = 3;
				if (info->speed < 0x18) { closest = 2; farthest = 4; }
				for (i=closest; i<=farthest; i++)
					{
					examinex = info->tankx + sin(info->direction)*i >> 8;
					examiney = info->tanky - cos(info->direction)*i >> 8;
					if (decide_building(examinex,examiney)) break;
					}
				}
	
			// If not in offensive pillbox mode, just drop pillboxes behind the tank
			// Note: the info->shells thing is a hack to make sure we don't drop a
			// pillbox somewhere and then find we have blocked ourself in somewhere
			if (CAN_BUILD_PILLBOX && !s.ppoffensive && info->shells > 20)
				{
				examinex = info->tankx - sin(info->direction)*2 >> 8;
				examiney = info->tanky + cos(info->direction)*2 >> 8;
				t = raw_getmapcell(examinex, examiney);
				if (!(t & TERRAIN_MINE) && can_build_pillbox[t & TERRAIN_MASK])
					{
					info->build->x = examinex;
					info->build->y = examiney;
					info->build->action = BUILDMODE_PBOX;
					}
				}
			}
		}

	// And act on that decision
	if (fullstop || killmine) setkey(keys, KEY_slower);
	else if (panic) setkey(keys, KEY_faster);
	else
		{
		if (gofaster) setkey(keys, KEY_faster);
		if (goslower) setkey(keys, KEY_slower);
		}

	// Be vandalistic when running away?
	if (runaway && s.laymines) setkey(keys, KEY_dropmine);

	// Decide whether to adjust gun range
	if      (info->gunrange > chosen_gunrange+2) setkey(keys, KEY_lessrange);
	else if (info->gunrange < chosen_gunrange-2) setkey(keys, KEY_morerange);
	else if (info->gunrange > chosen_gunrange  ) setkey(taps, KEY_lessrange);
	else if (info->gunrange < chosen_gunrange  ) setkey(taps, KEY_morerange);
	}

// ****************************************************************************

// Interface code to communicate with Bolo

local void brain_think(void)
	{
	long thinktime;
	int i;
	ObjectInfo *ob;
	
	// Map square tank is on now
	nearx = info->tankx >> 8;
	neary = info->tanky >> 8;
	
	// Map squares tank is touching
	tankleftx   = info->tankx - 0x80 >> 8;
	tankrightx  = info->tankx + 0x80 >> 8;
	tanktopy    = info->tanky - 0x80 >> 8;
	tankbottomy = info->tanky + 0x80 >> 8;
	
	// See if tank is dangerously close to a mine
	tank_close_to_mine =checkmineM(tankleftx,  tanktopy   ) ||
						checkmineM(tankrightx, tanktopy   ) ||
						checkmineM(tankleftx,  tankbottomy) ||
						checkmineM(tankrightx, tankbottomy);

	if (info->newtank)
		{
		explain("");
		explain("New tank");
		// Danger, Will Robinson! brains.h falsely claims that indexes are 0..n-1!
		for (i=0; i<=info->max_players;   i++) reset_progress(&tankprogress[i], 0);
		for (i=0; i<=info->max_pillboxes; i++) reset_progress(&pillprogress[i], 0);
		for (int x = 0; x <= 255; x++) for (int y=0; y<=255; y++) reset_progress(&baseprogress[x][y], 0);
		reset_progress(&manrescueprogress, 0);
		if (s.explore) setscout();
		still_on_boat  = TRUE;					// Tank starts on boat
		land_direction = info->direction;
		// See if we picked up a pillbox at x,y which killed us
		if (pillpickup_x) { fatal_x = pillpickup_x; fatal_y = pillpickup_y; }
		}
	pillpickup_x = pillpickup_y = 0;
	
	if (getmapcellM(nearx, neary) == DEEPSEA) { fatal_x = nearx; fatal_y = neary; }
	
	if (!info->inboat) still_on_boat = FALSE;	// Tank not on boat any more

	last_keys = keys;
	last_taps = taps;
	keys = taps = 0;
	shootit = shootsoon = runaway = FALSE;
	shootwhat = SHOOT_NOTHING;
	chosen_gunrange = MAXRANGE;

	// Count all shells, and count how many are coming towards us.
	incoming_shells = flying_shells = 0;
	for (ob=&info->objects[0]; ob<&info->objects[info->num_objects]; ob++)
		if (ob->object == OBJECT_SHOT)
			{
			char dev = ob->direction - aim( (long)info->tankx - (long)ob->x,
											(long)info->tanky - (long)ob->y );
			if (dev > -32 && dev < 32) incoming_shells++;
			flying_shells++;
			}
	
	for (i=0; i<16; i++)
		{
		direction_votes[i] = 0;
		target_distances[i] = 0xFFFFFFFF;
		direction_maxspeeds[i] = 0xFF;
		}

	cast_votes();
	count_votes();

	*(info->holdkeys) = keys;
	*(info->tapkeys ) = taps;

	last_tx = info->tankx;
	last_ty = info->tanky;
	
	thinktime = TickCount() - TimeNow;
	if (thinktime > 30)
		{
		explain("Warning: thought took %ld ticks", thinktime);
		if (do_explain) SysBeep(10);
		}
	}

local void set_brain_menu(short majormode)
	{
#if IMPLEMENTED
	CheckItem(MyMenu, m_default,    (majormode == m_default   ));
	CheckItem(MyMenu, m_assassin,   (majormode == m_assassin  ));
	CheckItem(MyMenu, m_pillhunter, (majormode == m_pillhunter));
	CheckItem(MyMenu, m_basehunter, (majormode == m_basehunter));
	CheckItem(MyMenu, m_defensive,  (majormode == m_defensive ));
	
	CheckItem(MyMenu, m_explore,      s.explore     );
	CheckItem(MyMenu, m_laymines,     s.laymines    );
	CheckItem(MyMenu, m_clearmines,   s.clearmines  );
	CheckItem(MyMenu, m_dobuilding,   s.dobuilding  );
	CheckItem(MyMenu, m_attacktanks,  s.attacktanks );
	CheckItem(MyMenu, m_attackpills,  s.attackpills );
	CheckItem(MyMenu, m_attackbases,  s.attackbases );
	CheckItem(MyMenu, m_repairpills,  s.repairpills );
	CheckItem(MyMenu, m_placepills,   s.placepills  );
	CheckItem(MyMenu, m_ppoffensive,  s.ppoffensive );

	CheckItem(MyMenu, m_explain,      do_explain    );
	CheckItem(MyMenu, m_route,        do_showroute  );
#endif
	}

local short brain_menu(short item)
	{
	switch (item)
		{
		case m_about      : Alert(1000, NULL); return(0);
		
		case m_default    : s = s_default;    break;
		case m_assassin   : s = s_assassin;   break;
		case m_pillhunter : s = s_pillhunter; break;
		case m_basehunter : s = s_basehunter; break;
		case m_defensive  : s = s_defensive;  break;
		
		case m_explore    : s.explore      ^= 1; break;
		case m_laymines   : s.laymines     ^= 1; break;
		case m_clearmines : s.clearmines   ^= 1; break;
		case m_dobuilding : s.dobuilding   ^= 1; break;
		case m_attacktanks: s.attacktanks  ^= 1; break;
		case m_attackpills: s.attackpills  ^= 1; break;
		case m_attackbases: s.attackbases  ^= 1; break;
		case m_repairpills: s.repairpills  ^= 1; break;
		case m_placepills : s.placepills   ^= 1; break;
		case m_ppoffensive: s.ppoffensive  ^= 1; break;
		
		case m_explain    : do_explain     ^= 1;
#if AUTO_DEBUG
							if (do_explain) ShowWindow(debugwindow);
							else HideWindow(debugwindow);
#endif
							break;
		
		case m_route      : do_showroute   ^= 1;
#if AUTO_DEBUG
							if (do_showroute) ShowWindow(routewindow);
							else HideWindow(routewindow);
#endif
							break;
		
		default: return(-1);
		}
	set_brain_menu(item);
	return(0);
	}

local Boolean brain_open(void)
	{
#if AUTO_DEBUG
	static Rect debugrect = { 40, 4, 340, 200 };
	static Rect routerect = { 40, 220, 40+COST_ARRAY_SIZE*8, 220+COST_ARRAY_SIZE*8 };
#endif
	Boolean ok;
	// Danger, Will Robinson! brains.h falsely claims that indexes are 0..n-1!
	tankprogress = NewPtr(sizeof(ProgressInfo) * (info->max_players + 1));
	pillprogress = NewPtr(sizeof(ProgressInfo) * (info->max_pillboxes + 1));
	baseprogress = NewPtr(sizeof(ProgressInfo) * 256 * 256);

#if IMPLEMENTED
	MyMenu = GetMenu(MyMenuID);
	InsertMenu(MyMenu, 0);
#endif
	s = s_default;

	set_brain_menu(m_default);
#if IMPLEMENTED
	DrawMenuBar();
#endif

#if AUTO_DEBUG
	debugwindow = NewWindow(NULL, &debugrect, info->playernames[info->player_number]->c,
								FALSE, documentProc, (WindowPtr)(-1), FALSE, 0);
	
	routewindow = NewCWindow(NULL, &routerect, info->playernames[info->player_number]->c,
								FALSE, documentProc, (WindowPtr)(-1), FALSE, 0);
	
	ok = tankprogress && pillprogress && baseprogress && debugwindow && routewindow;

	if (ok)
		{
		SysEnvRec sysenvirons;
		SysEnvirons(1, &sysenvirons);
		if (!sysenvirons.hasColorQD) { do_showroute = FALSE; DisableItem(MyMenu, m_route); }
		if (do_explain  ) ShowWindow(debugwindow);
		if (do_showroute) ShowWindow(routewindow);
		}
	return(ok);
#else
	return tankprogress && pillprogress && baseprogress;
#endif
	}

local void brain_close(void)
	{
	if (tankprogress) { DisposPtr((Ptr)tankprogress); tankprogress = NULL; }
	if (pillprogress) { DisposPtr((Ptr)pillprogress); pillprogress = NULL; }
	if (baseprogress) { DisposPtr((Ptr)baseprogress); baseprogress = NULL; }
#if AUTO_DEBUG
	if (debugwindow)  { DisposeWindow(debugwindow);   debugwindow  = NULL; } 
	if (routewindow)  { DisposeWindow(routewindow);   routewindow  = NULL; }
#endif
#if IMPLEMENTED
	DeleteMenu(MyMenuID);
	ReleaseResource((Handle)MyMenu);
	DrawMenuBar();
#endif
	}

__declspec(dllexport) short BrainMain(const BrainInfo* brainInfo)
	{
	if (brainInfo->InfoVersion != CURRENT_BRAININFO_VERSION) return(-1);

	info = brainInfo;			// copy parameter into global variable
	TimeNow = TickCount();		// so we know how long we have been in this call

	switch (info->operation)
		{
		case BRAIN_OPEN  :	if (brain_open()) return(0);
							else { brain_close(); return(-1); }
		case BRAIN_CLOSE :	brain_close(); return(0);
		case BRAIN_THINK :	brain_think(); return(0);
		case MyMenuID    :  return(brain_menu(info->menu_item));
		default          :  return(-1);
		}
	}
