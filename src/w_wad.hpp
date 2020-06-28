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
//	WAD I/O functions.
//


#ifndef __W_WAD__
#define __W_WAD__

#include <stdio.h>
#include <vector>

#include "doomtype.hpp"
#include "w_file.hpp"


//
// TYPES
//

//
// WADFILE I/O related stuff.
//

typedef int lumpindex_t;

struct lumpinfo_t
{
   lump_name_t	name;
   wad_file_t *wad_file{};
   int		position{};
   int		size{};
   void       *cache{};

    // Used for hash table lookups
   lumpindex_t next{};
};


extern std::vector<lumpinfo_t*> lumpinfo;
extern unsigned int numlumps;

wad_file_t *W_AddFile(const std::string &filename);
void W_Reload(void);

lumpindex_t W_CheckNumForName(const lump_name_t &name);
lumpindex_t W_GetNumForName(const lump_name_t &name);
lumpindex_t W_CheckNumForNameFromTo(const lump_name_t &name, int from, int to);

int W_LumpLength(lumpindex_t lump);
void W_ReadLump(lumpindex_t lump, void *dest);

void *W_CacheLumpNum(lumpindex_t lump, int tag);
void *W_CacheLumpName(const lump_name_t &name, int tag);

void W_GenerateHashTable(void);

extern unsigned int W_LumpNameHash(const char *s);

void W_ReleaseLumpNum(lumpindex_t lump);
void W_ReleaseLumpName(const lump_name_t &name);

const char *W_WadNameForLump(const lumpinfo_t *lump);
boolean W_IsIWADLump(const lumpinfo_t *lump);

#endif
