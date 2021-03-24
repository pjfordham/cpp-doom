//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	Play functions, animation, global header.
//


#ifndef __P_LOCAL__
#define __P_LOCAL__

#ifndef __R_LOCAL__
#include "r_local.hpp"
#endif

#define TOCENTER                -8
#define AFLAG_JUMP              0x80
const fixed_t FLOATSPEED = FRACUNIT*4;


#define MAXHEALTH		100
const fixed_t VIEWHEIGHT = FRACUNIT*41;


// player radius for movement checking
const fixed_t PLAYERRADIUS = 16*FRACUNIT;

// MAXRADIUS is for precalculated sector block boxes
// the spider demon is larger,
// but we do not have any moving sectors nearby
const fixed_t MAXRADIUS = 32*FRACUNIT;

const fixed_t GRAVITY = FRACUNIT;
const fixed_t MAXMOVE = 30*FRACUNIT;

const fixed_t USERANGE = 64*FRACUNIT;
const fixed_t MELEERANGE = 64*FRACUNIT;
const fixed_t MISSILERANGE = 32*64*FRACUNIT;

// follow a player exlusively for 3 seconds
#define	BASETHRESHOLD	 	100

#include <functional>
#include <variant>
#include <list>
#include "s_musinfo.hpp" // [crispy] T_MAPMusic()
#include "p_spec.hpp"
#include "z_zone.hpp"

//
// P_TICK
//

//
// THINKERS
// All thinkers should be allocated by zone_malloc
// so they can be operated on uniformly.

extern std::list<std::variant<fireflicker_t, lightflash_t, strobe_t, glow_t, plat_t, vldoor_t, ceiling_t, floormove_t, mobj_t>>
    static_thinkers;


void P_InitThinkers (void);
void P_RunThinkers (void);

// FIXME: Just for backward compatability
bool P_VisitMobjThinkers(std::function<bool(mobj_t *)> visitor);

//
// P_AddThinker
// Adds a new thinker at the end of the list.
//
template <typename Thinker>
Thinker *P_AddThinker()
{
   static_thinkers.emplace_back(std::in_place_type_t<Thinker>());
   return &std::get<Thinker>(static_thinkers.back());
}

//
// P_RemoveThinker
// Deallocation is lazy -- it will not actually be freed
// until its thinking turn comes up.
//
template <typename Thinker>
void P_RemoveThinker (Thinker* thinker)
{
   thinker->deleted = true;
}

//
// P_VisitThinkers( function )
//
template <typename Thinker>
bool P_VisitThinkers(std::function<bool(Thinker *)> visitor)
{
   for( auto i = static_thinkers.begin(); i != static_thinkers.end(); i++ ) {
      // Break out early if return true;
      if ( holds_alternative<Thinker>( *i ) && visitor( &std::get<Thinker>(*i) ) ) {
         return true;
      }
   }
   return false;
}

//
// P_PSPR
//
void P_SetupPsprites (player_t* curplayer);
void P_MovePsprites (player_t* curplayer);
void P_DropWeapon (player_t* player);


//
// P_USER
//
#define MLOOKUNIT	8
#define PLAYER_SLOPE(a)	((((a)->lookdir / MLOOKUNIT) << FRACBITS) / 173)
void	P_PlayerThink (player_t* player);


//
// P_MOBJ
//
const fixed_t ONFLOORZ = INT_MIN;
const fixed_t ONCEILINGZ = INT_MAX;

// Time interval for item respawning.
#define ITEMQUESIZE		128

extern mapthing_t	itemrespawnque[ITEMQUESIZE];
extern int		itemrespawntime[ITEMQUESIZE];
extern int		iquehead;
extern int		iquetail;


void P_RespawnSpecials (void);

mobj_t*
P_SpawnMobj
( fixed_t	x,
  fixed_t	y,
  fixed_t	z,
  mobjtype_t	type );

void 	P_RemoveMobj (mobj_t* th);
mobj_t* P_SubstNullMobj (mobj_t* th);
boolean	P_SetMobjState (mobj_t* mobj, statenum_t state);
void 	P_MobjThinker (mobj_t* mobj);
mobj_t *Crispy_PlayerSO (int p); // [crispy] weapon sound sources

void	P_SpawnPuff (fixed_t x, fixed_t y, fixed_t z);
void 	P_SpawnBlood (fixed_t x, fixed_t y, fixed_t z, int damage, mobj_t* target);
mobj_t* P_SpawnMissile (mobj_t* source, mobj_t* dest, mobjtype_t type);
void	P_SpawnPlayerMissile (mobj_t* source, mobjtype_t type);

void	P_SpawnPuffSafe (fixed_t x, fixed_t y, fixed_t z, boolean safe);

//
// P_ENEMY
//
void P_NoiseAlert (mobj_t* target, mobj_t* emmiter);


//
// P_MAPUTL
//
typedef struct
{
    fixed_t	x;
    fixed_t	y;
    fixed_t	dx;
    fixed_t	dy;
    
} divline_t;

typedef struct
{
    fixed_t	frac;		// along trace line
    boolean	isaline;
    union {
	mobj_t*	thing;
	line_t*	line;
    }			d;
} intercept_t;

// Extended MAXINTERCEPTS, to allow for intercepts overrun emulation.

#define MAXINTERCEPTS_ORIGINAL 128
#define MAXINTERCEPTS          (MAXINTERCEPTS_ORIGINAL + 61)

//extern intercept_t	intercepts[MAXINTERCEPTS]; // [crispy] remove INTERCEPTS limit
extern intercept_t*	intercept_p;

typedef boolean (*traverser_t) (intercept_t *in);

fixed_t P_AproxDistance (fixed_t dx, fixed_t dy);
int 	P_PointOnLineSide (fixed_t x, fixed_t y, line_t* line);
int 	P_PointOnDivlineSide (fixed_t x, fixed_t y, divline_t* line);
void 	P_MakeDivline (line_t* li, divline_t* dl);
fixed_t P_InterceptVector (divline_t* v2, divline_t* v1);
int 	P_BoxOnLineSide (fixed_t* tmbox, line_t* ld);

extern fixed_t		opentop;
extern fixed_t 		openbottom;
extern fixed_t		openrange;
extern fixed_t		lowfloor;

void 	P_LineOpening (line_t* linedef);

boolean P_BlockLinesIterator (map_block_t x, map_block_t y, boolean(*func)(line_t*) );
boolean P_BlockThingsIterator (map_block_t x, map_block_t y, boolean(*func)(mobj_t*) );

#define PT_ADDLINES		1
#define PT_ADDTHINGS	2
#define PT_EARLYOUT		4

extern divline_t	trace;

boolean
P_PathTraverse
( fixed_t	x1,
  fixed_t	y1,
  fixed_t	x2,
  fixed_t	y2,
  int		flags,
  boolean	(*trav) (intercept_t *));

void P_UnsetThingPosition (mobj_t* thing);
void P_SetThingPosition (mobj_t* thing);


//
// P_MAP
//

// If "floatok" true, move would be ok
// if within "tmfloorz - tmceilingz".
extern boolean		floatok;
extern fixed_t		tmfloorz;
extern fixed_t		tmceilingz;


extern	line_t*		ceilingline;

// fraggle: I have increased the size of this buffer.  In the original Doom,
// overrunning past this limit caused other bits of memory to be overwritten,
// affecting demo playback.  However, in doing so, the limit was still
// exceeded.  So we have to support more than 8 specials.
//
// We keep the original limit, to detect what variables in memory were
// overwritten (see SpechitOverrun())

#define MAXSPECIALCROSS 		20
#define MAXSPECIALCROSS_ORIGINAL	8

extern	line_t*	spechit[MAXSPECIALCROSS];
extern	int	numspechit;

boolean P_CheckPosition (mobj_t *thing, fixed_t x, fixed_t y);
boolean P_TryMove (mobj_t* thing, fixed_t x, fixed_t y);
boolean P_TeleportMove (mobj_t* thing, fixed_t x, fixed_t y);
void	P_SlideMove (mobj_t* mo);
boolean P_CheckSight (mobj_t* t1, mobj_t* t2);
void 	P_UseLines (player_t* player);

boolean P_ChangeSector (sector_t* sector, boolean crunch);

extern mobj_t*	linetarget;	// who got hit (or NULL)

fixed_t
P_AimLineAttack
( mobj_t*	t1,
  angle_t	angle,
  fixed_t	distance );

void
P_LineAttack
( mobj_t*	t1,
  angle_t	angle,
  fixed_t	distance,
  fixed_t	slope,
  int		damage );

void
P_RadiusAttack
( mobj_t*	spot,
  mobj_t*	source,
  fixed_t	damage );



//
// P_SETUP
//
extern byte*		rejectmatrix;	// for fast sight rejection
extern int32_t*	blockmaplump;	// offsets in blockmap are from here // [crispy] BLOCKMAP limit
extern map_block_t*	blockmap; // [crispy] BLOCKMAP limit
extern map_block_t	bmapwidth;
extern map_block_t	bmapheight;	// in mapblocks
extern fixed_t		bmaporgx;
extern fixed_t		bmaporgy;	// origin of block map
extern mobj_t**		blocklinks;	// for thing chains

// [crispy] factor out map lump name and number finding into a separate function
extern int P_GetNumForMap (int episode, int map, boolean critical);

// [crispy] blinking key or skull in the status bar
#define KEYBLINKMASK 0x8
#define KEYBLINKTICS (7*KEYBLINKMASK)
extern int st_keyorskull[3];

//
// P_INTER
//
extern int		maxammo[NUMAMMO];
extern int		clipammo[NUMAMMO];

void
P_TouchSpecialThing
( mobj_t*	special,
  mobj_t*	toucher );

void
P_DamageMobj
( mobj_t*	target,
  mobj_t*	inflictor,
  mobj_t*	source,
  int		damage );


//
// P_SPEC
//
#include "p_spec.hpp"


#endif	// __P_LOCAL__
