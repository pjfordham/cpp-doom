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

#include <variant>
#include <list>

std::list<std::variant<fireflicker_t, lightflash_t, strobe_t, glow_t, plat_t, vldoor_t, ceiling_t, floormove_t, mobj_t>> static_thinkers;

//
// P_InitThinkers
//
void P_InitThinkers (void)
{
   static_thinkers.clear();
}

bool P_VisitMobjThinkers(std::function<bool(mobj_t *)> visitor) {
   return P_VisitThinkers<mobj_t>(visitor);
}

struct {
   template <typename Thinker>
   void operator()( Thinker &&i) { i.action(); }
} action;

struct {
   template <typename Thinker>
   bool operator()( Thinker &&i) { return i.deleted; }
} deleted;

//
// P_RunThinkers
//
void P_RunThinkers (void)
{
   auto i = static_thinkers.begin();

   while ( i != static_thinkers.end() ) {
      if ( std::visit( deleted, *i ) ) {
         i = static_thinkers.erase( i );
      } else {
         std::visit( action, *(i++) );
      }
   }

   // [crispy] support MUSINFO lump (dynamic music changing)
   T_MusInfo();
}

//
// P_Ticker
//

void P_Ticker (void)
{
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


    for (int i=0 ; i<MAXPLAYERS ; i++)
	if (playeringame[i])
	    P_PlayerThink (&players[i]);

    P_RunThinkers ();
    P_UpdateSpecials ();
    P_RespawnSpecials ();

    // for par times
    leveltime++;
}
