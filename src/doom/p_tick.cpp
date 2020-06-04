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


#include "z_zone.hpp"
#include "p_local.hpp"
#include "s_musinfo.hpp" // [crispy] T_MAPMusic()
#include <list>
#include <iostream>

#include "doomstat.hpp"
#include <functional>


int	leveltime;

//
// THINKERS
// All thinkers should be allocated by zone_malloc
// so they can be operated on uniformly.
// The actual structures will vary in size,
// but the first element must be thinker_t.
//

std::list<thinker_t *> thinkers;


//
// P_InitThinkers
//
void P_InitThinkers (void)
{
    thinkers.clear();
}




//
// P_AddThinker
// Adds a new thinker at the end of the list.
//
void P_AddThinker (thinker_t* thinker)
{
    thinkers.push_back( thinker );
}



//
// P_RemoveThinker
// Deallocation is lazy -- it will not actually be freed
// until its thinking turn comes up.
//
void P_RemoveThinker (thinker_t* thinker)
{
  // FIXME: NOP.
  thinker->function = think_t{-1};
}



//
// P_AllocateThinker
// Allocates memory and adds a new thinker at the end of the list.
//
void P_AllocateThinker (thinker_t*	thinker)
{
}


//
// P_VisitThinkers( function )
//

bool P_VisitThinkers( std::function<bool( thinker_t* )> visitor ) {

   for( auto i = thinkers.begin(); i != thinkers.end(); i++ ) {
      // Break out early if return true;
      if ( visitor( *i ) ) {
         return true;
      }
   }

   return false;
}

bool P_VisitMobjThinkers(std::function<bool(mobj_t *)> visitor) {
    return P_VisitThinkers( [&visitor]( thinker_t *thinker ) {
        if (thinker->function == P_MobjThinker)
        {
           return visitor( static_cast<mobj_t*>(thinker) );
        }
        return false;} );
}

//
// P_RunThinkers
//
void P_RunThinkers (void)
{
   auto i = thinkers.begin();
   do {
      while ( i != thinkers.end() && (*i)->function == think_t{-1} )
      {
         Z_Free(*i);
         i = thinkers.erase( i );
      }

      if ( i != thinkers.end() ) {
         (*i)->action();
         i++;
      }
      else
      {
         break;
      }
   } while ( true );

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
