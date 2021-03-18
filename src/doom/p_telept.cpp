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
//	Teleportation.
//




#include "doomdef.hpp"
#include "doomstat.hpp"

#include "s_sound.hpp"

#include "p_local.hpp"


// Data.
#include "sounds.hpp"

// State.
#include "r_state.hpp"



//
// TELEPORTATION
//
void
EV_Teleport
( line_t*	line,
  int		side,
  mobj_t*	thing )
{
    // don't teleport missiles
    if (thing->flags & MF_MISSILE)
        return;

    // Don't teleport if hit back of line,
    //  so you can get out of teleporter.
    if (side == 1)
	return;

    int tag = line->tag;
    for (int i = 0; i < numsectors; i++)
    {
	if (sectors[ i ].tag == tag )
	{
           P_VisitMobjThinkers([i,&thing](mobj_t *m) {

		// not a teleportman
		if (m->type != MT_TELEPORTMAN )
                   return false;

		sector_t *sector = m->subsector->sector;
		// wrong sector
		if (sector-sectors != i )
                   return false;

		  fixed_t oldx = thing->x;
		  fixed_t oldy = thing->y;
                  fixed_t oldz = thing->z;

		if (!P_TeleportMove (thing, m->x, m->y))
		    return true;

                // The first Final Doom executable does not set thing->z
                // when teleporting. This quirk is unique to this
                // particular version; the later version included in
                // some versions of the Id Anthology fixed this.

                if (gameversion != exe_final)
		    thing->z = thing->floorz;

		if (thing->player)
		{
		    thing->player->viewz = thing->z+thing->player->viewheight;
		    // [crispy] center view after teleporting
		    thing->player->centering = true;
		}

		// spawn teleport fog at source and destination
                mobj_t*	fog;
		fog = P_SpawnMobj (oldx, oldy, oldz, MT_TFOG);
		S_StartSound (fog, sfx_telept);
		fog = P_SpawnMobj (m->x+20*cos(m->angle), m->y+20*sin(m->angle)
				   , thing->z, MT_TFOG);

		// emit sound, where?
		S_StartSound (fog, sfx_telept);

		// don't move for a bit
		if (thing->player)
		    thing->reactiontime = 18;

		thing->angle = m->angle;
		thing->momx = thing->momy = thing->momz = 0;
		return true;
              } );
	}
    }
    return;
}

