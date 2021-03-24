//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1999 id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2017 Fabian Greffrath
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
//	[crispy] Create Blockmap
//

#include "../../utils/memory.hpp"
#include "i_system.hpp"
#include "p_local.hpp"
#include "z_zone.hpp"
#include <stdlib.h>
#include <vector>

// [crispy] taken from mbfsrc/P_SETUP.C:547-707, slightly adapted

void P_CreateBlockMap(void)
{
  int i;
  int minx = INT_MAX, miny = INT_MAX, maxx = INT_MIN, maxy = INT_MIN;

  // First find limits of map

  for (i=0; i<numvertexes; i++)
    {
      if ((vertexes[i].x >> FRACBITS) < minx)
	minx = vertexes[i].x >> FRACBITS;
      else
        if ((vertexes[i].x >> FRACBITS) > maxx)
	  maxx = vertexes[i].x >> FRACBITS;
        if ((vertexes[i].y >> FRACBITS) < miny)
	miny = vertexes[i].y >> FRACBITS;
      else
        if ((vertexes[i].y >> FRACBITS) > maxy)
	  maxy = vertexes[i].y >> FRACBITS;
    }

  // [crispy] doombsp/DRAWING.M:175-178
  minx -= 8; miny -= 8;
  maxx += 8; maxy += 8;

  // Save blockmap parameters

  bmaporgx = minx << FRACBITS;
  bmaporgy = miny << FRACBITS;
  bmapwidth  = ((maxx-minx) >> MAPBTOFRAC) + map_block_t(1);
  bmapheight = ((maxy-miny) >> MAPBTOFRAC) + map_block_t(1);

  // Compute blockmap, which is stored as a 2d array of variable-sized lists.
  //
  // Pseudocode:
  //
  // For each linedef:
  //
  //   Map the starting and ending vertices to blocks.
  //
  //   Starting in the starting vertex's block, do:
  //
  //     Add linedef to current block's list, dynamically resizing it.
  //
  //     If current block is the same as the ending vertex's block, exit loop.
  //
  //     Move to an adjacent block by moving towards the ending block in
  //     either the x or y direction, to the block which contains the linedef.

  {
    // blocklist structure
    struct bmap_t {
       int n;
       int nalloc;
       std::vector<int> list;
       bmap_t() : n{ 0 }, nalloc{ 0 } {}
    };

    unsigned tot = (bmapwidth * bmapheight).get();            // size of blockmap
    auto bmap = new bmap_t[tot];         // array of blocklists

    for (i=0; i < numlines; i++) {

  	// starting coordinates
	int x = (lines[i].v1->x >> FRACBITS) - minx;
	int y = (lines[i].v1->y >> FRACBITS) - miny;

	// x-y deltas
	int adx = lines[i].dx >> FRACBITS;
	int ady = lines[i].dy >> FRACBITS;

 	// delta for pointer when moving across y
        map_block_t dx = map_block_t(adx < 0 ? -1 : 1);
        map_block_t dy = map_block_t(ady < 0 ? -1 : 1) * bmapwidth;

        // difference in preferring to move across y (>0) instead of x (<0)
        int diff;
        if (!adx) {
           diff = 1;
        } else if (!ady) {
           diff = -1;
        } else {
           int dx = adx < 0 ? -1 : 1;
           int dy = ady < 0 ? -1 : 1;
           ady = abs(ady);
           adx = abs(adx);
           int _x = (x >> MAPBTOFRAC) << MAPBTOFRAC;
           int _y = (y >> MAPBTOFRAC) << MAPBTOFRAC;
           int stepx = (dx > 0 ? MAPBLOCKUNITS-1 : 0);
           int stepy = (dy > 0 ? MAPBLOCKUNITS-1 : 0);
           diff =
              (_x + stepx - x) * ady * dx -
              (_y + stepy - y) * adx * dy;
        }

        // starting block, and pointer to its blocklist structure
	map_block_t b = (y >> MAPBTOFRAC)*bmapwidth + (x >> MAPBTOFRAC);

	// ending block
	map_block_t bend = (((lines[i].v2->y >> FRACBITS) - miny) >> MAPBTOFRAC) *
	    bmapwidth + (((lines[i].v2->x >> FRACBITS) - minx) >> MAPBTOFRAC);

	// deltas for diff inside the loop
	const int iadx = adx << 7;
	const int iady = ady << 7;

	// Now we simply iterate block-by-block until we reach the end block.
	while (b.get() < tot) {   // failsafe -- should ALWAYS be true
           auto &block = bmap[b.get()];
          // Increase size of allocated list if necessary
             if (block.n >= block.nalloc) {
                block.nalloc = block.nalloc ? block.nalloc * 2 : 8;
                block.list.resize( block.nalloc );
             }

	    // Add linedef to end of list
	    block.list[block.n++] = i;

	    // If we have reached the last block, exit
	    if (b == bend)
	      break;

	    // Move in either the x or y direction to the next block
	    if (diff < 0) {
               diff += iady;
               b += dx;
            } else {
               diff -= iadx;
               b += dy;
            }
	  }
      }

    // Compute the total size of the blockmap.
    //
    // Compression of empty blocks is performed by reserving two offset words
    // at tot and tot+1.
    //
    // 4 words, unused if this routine is called, are reserved at the start.

    {
      int count = tot+6;  // we need at least 1 word per block, plus reserved's

      for (i = 0; i < tot; i++)
	if (bmap[i].n)
	  count += bmap[i].n + 2; // 1 header word + 1 trailer word + blocklist

      // Allocate blockmap lump with computed count
      blockmaplump = Z_New<int>(PU_LEVEL, count);
    }

    // Now compress the blockmap.
    {
      int ndx = tot += 4;         // Advance index to start of linedef lists
      bmap_t *bp = bmap;          // Start of uncompressed blockmap

      blockmaplump[ndx++] = 0;    // Store an empty blockmap list at start
      blockmaplump[ndx++] = -1;   // (Used for compression)

      for (i = 4; i < tot; i++, bp++)
	if (bp->n)                                      // Non-empty blocklist
	  {
	    blockmaplump[blockmaplump[i] = ndx++] = 0;  // Store index & header
	    do
	      blockmaplump[ndx++] = bp->list[--bp->n];  // Copy linedef list
	    while (bp->n);
	    blockmaplump[ndx++] = -1;                   // Store trailer
	  }
	else            // Empty blocklist: point to reserved empty blocklist
	  blockmaplump[i] = tot;

      delete [] bmap;    // Free uncompressed blockmap
    }
  }

  // [crispy] copied over from P_LoadBlockMap()
  {
    unsigned tot = (bmapwidth * bmapheight).get();            // size of blockmap
    blocklinks = Z_New<mobj_t*>(PU_LEVEL, tot);
    std::fill( blocklinks, blocklinks + tot, nullptr );
    blockmap = (map_block_t*)blockmaplump+4;
  }

  fprintf(stderr, "+BLOCKMAP)\n");
}
