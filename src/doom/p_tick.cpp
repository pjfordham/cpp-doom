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
//	Archiving: SaveGame I/O.
//	Thinker, Ticker.
//


#include "p_local.hpp"

#include "doomstat.hpp"

int	leveltime;

#include <tuple>
#include <list>

std::tuple<std::list<fireflicker_t>, std::list<lightflash_t>,
           std::list<strobe_t>, std::list<glow_t>, std::list<plat_t>,
           std::list<vldoor_t>, std::list<ceiling_t>,
           std::list<floormove_t>, std::list<mobj_t>>
    static_thinkers;

//
// P_InitThinkers
//
void P_InitThinkers (void)
{
    std::apply([](auto&&... args) {((args.clear()), ...);}, static_thinkers);
}

bool P_VisitMobjThinkers(std::function<bool(mobj_t *)> visitor) {
   return P_VisitThinkers<mobj_t>(visitor);
}

//
// P_RunThinkers
//
void P_RunThinkers (void)
{
   std::apply([](auto&&... args) {((P_RunThinkers(args)), ...);}, static_thinkers);

   // [crispy] support MUSINFO lump (dynamic music changing)
   T_MusInfo();
}

//
// P_Ticker
//

void P_Ticker (void)
{
    int		i;
    
    // run the tic
    if (paused)
	return;
		
    // pause if in menu and at least one tic has been run
    if ( !netgame
	 && menuactive
	 && !demoplayback
	 && players[consoleplayer].viewz != 1)
    {
	return;
    }
    
		
    for (i=0 ; i<MAXPLAYERS ; i++)
	if (playeringame[i])
	    P_PlayerThink (&players[i]);
			
    P_RunThinkers ();
    P_UpdateSpecials ();
    P_RespawnSpecials ();

    // for par times
    leveltime++;	
}
