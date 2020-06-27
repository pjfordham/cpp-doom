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
//	Handles WAD file header, directory, lump I/O.
//




#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <fmt/core.h>

#include "doomtype.hpp"

#include "i_swap.hpp"
#include "i_system.hpp"
#include "i_video.hpp"
#include "m_misc.hpp"
#include "v_diskicon.hpp"
#include "z_zone.hpp"

#include "../utils/memory.hpp"
#include "w_wad.hpp"

typedef PACKED_STRUCT (
{
    // Should be "IWAD" or "PWAD".
    char		identification[4];
    int			numlumps;
    int			infotableofs;
}) wadinfo_t;


typedef PACKED_STRUCT (
{
    int			filepos;
    int			size;
    char		name[8];
}) filelump_t;

//
// GLOBALS
//

// Location of each lump on disk.
std::vector<lumpinfo_t*> lumpinfo;
unsigned int numlumps = 0;

// Hash table for fast lookups
static lumpindex_t *lumphash;

// Variables for the reload hack: filename of the PWAD to reload, and the
// lumps from WADs before the reload file, so we can resent numlumps and
// load the file again.
static wad_file_t *reloadhandle = NULL;
static std::vector<lumpinfo_t> reloadlumps;
static std::string reloadname;
static int reloadlump = -1;

// Hash function used for lump names.
unsigned int W_LumpNameHash(const char *s)
{
    // This is the djb2 string hash function, modded to work on strings
    // that have a maximum length of 8.

    unsigned int result = 5381;
    unsigned int i;

    for (i=0; i < 8 && s[i] != '\0'; ++i)
    {
        result = ((result << 5) ^ result ) ^ toupper(s[i]);
    }

    return result;
}

//
// LUMP BASED ROUTINES.
//

//
// W_AddFile
// All files are optional, but at least one file must be
//  found (PWAD, if all required lumps are present).
// Files with a .wad extension are wadlink files
//  with multiple lumps.
// Other files are single lumps with the base filename
//  for the lump name.

std::vector<lumpinfo_t> filelumps;

wad_file_t *W_AddFile (const std::string &_filename)
{
    std::string filename = _filename;
    wadinfo_t header;
    lumpindex_t i;
    wad_file_t *wad_file;
    int length;
    int startlump;
    filelump_t *fileinfo;
    filelump_t *filerover;
    int numfilelumps;

    // If the filename begins with a ~, it indicates that we should use the
    // reload hack.
    if (filename[0] == '~')
    {
        if (!reloadname.empty())
        {
            I_Error("Prefixing a WAD filename with '~' indicates that the "
                    "WAD should be reloaded\n"
                    "on each level restart, for use by level authors for "
                    "rapid development. You\n"
                    "can only reload one WAD file, and it must be the last "
                    "file in the -file list.");
        }

        reloadname = filename;
        reloadlump = numlumps;
        filename = filename.substr(1);
    }

    // Open the file and add to directory
    wad_file = W_OpenFile(filename);

    if (wad_file == NULL)
    {
       fmt::print(stdout, " couldn't open {}\n", filename);
       return NULL;
    }

    if (!iequals( filename.substr(filename.size() - std::min((std::size_t)3, filename.size())), "wad" ) )
    {
	// single lump file

        // fraggle: Swap the filepos and size here.  The WAD directory
        // parsing code expects a little-endian directory, so will swap
        // them back.  Effectively we're constructing a "fake WAD directory"
        // here, as it would appear on disk.

	fileinfo = Z_New<filelump_t>(PU_STATIC);
	fileinfo->filepos = LONG(0);
	fileinfo->size = LONG(wad_file->length);

        // Name the lump after the base of the filename (without the
        // extension).

	M_ExtractFileBase (filename, fileinfo->name);
	numfilelumps = 1;
    }
    else
    {
	// WAD file
        W_Read(wad_file, 0, &header, sizeof(header));

	if (strncmp(header.identification,"IWAD",4))
	{
	    // Homebrew levels?
	    if (strncmp(header.identification,"PWAD",4))
	    {
		W_CloseFile(wad_file);
		I_Error ("Wad file %s doesn't have IWAD "
			 "or PWAD id\n", filename);
	    }

	    // ???modifiedgame = true;
	}

	header.numlumps = LONG(header.numlumps);

         // Vanilla Doom doesn't like WADs with more than 4046 lumps
         // https://www.doomworld.com/vb/post/1010985
         // [crispy] disable PWAD lump number limit
         if (!strncmp(header.identification,"PWAD",4) && header.numlumps > 4046 && false)
         {
                 W_CloseFile(wad_file);
                 I_Error ("Error: Vanilla limit for lumps in a WAD is 4046, "
                          "PWAD %s has %d", filename, header.numlumps);
         }

	header.infotableofs = LONG(header.infotableofs);
	length = header.numlumps*sizeof(filelump_t);
	fileinfo = Z_New<filelump_t>(PU_STATIC, header.numlumps);

        W_Read(wad_file, header.infotableofs, fileinfo, length);
	numfilelumps = header.numlumps;
    }

    // Increase size of numlumps array to accomodate the new file.
    filelumps.clear();
    filelumps.resize( numfilelumps );

    // FIXME review this, it seems suspicious
    startlump = numlumps;
    numlumps += numfilelumps;
    lumpinfo.resize( numlumps );
    filerover = fileinfo;

    for (i = startlump; i < numlumps; ++i)
    {
        lumpinfo_t *lump_p = &filelumps[i - startlump];
        lump_p->wad_file = wad_file;
        lump_p->position = LONG(filerover->filepos);
        lump_p->size = LONG(filerover->size);
        lump_p->cache = NULL;
        strncpy(lump_p->name, filerover->name, 8);
        lumpinfo[i] = lump_p;

        ++filerover;
    }

    Z_Delete(fileinfo);

    if (lumphash != NULL)
    {
        Z_Delete(lumphash);
        lumphash = NULL;
    }

    // If this is the reload file, we need to save some details about the
    // file so that we can close it later on when we do a reload.
    if (!reloadname.empty())
    {
        reloadhandle = wad_file;
        reloadlumps = filelumps;
    }

    return wad_file;
}



//
// W_NumLumps
//
int W_NumLumps (void)
{
    return numlumps;
}



//
// W_CheckNumForName
// Returns -1 if name not found.
//

lumpindex_t W_CheckNumForName(const char *name)
{
    lumpindex_t i;

    // Do we have a hash table yet?

    if (lumphash != NULL)
    {
        int hash;

        // We do! Excellent.

        hash = W_LumpNameHash(name) % numlumps;

        for (i = lumphash[hash]; i != -1; i = lumpinfo[i]->next)
        {
            if (!strncasecmp(lumpinfo[i]->name, name, 8))
            {
                return i;
            }
        }
    }
    else
    {
        // We don't have a hash table generate yet. Linear search :-(
        //
        // scan backwards so patch lump files take precedence

        for (i = numlumps - 1; i >= 0; --i)
        {
            if (!strncasecmp(lumpinfo[i]->name, name, 8))
            {
                return i;
            }
        }
    }

    // TFB. Not found.

    return -1;
}




//
// W_GetNumForName
// Calls W_CheckNumForName, but bombs out if not found.
//
lumpindex_t W_GetNumForName(const char *name)
{
    lumpindex_t i;

    i = W_CheckNumForName (name);

    if (i < 0)
    {
        I_Error ("W_GetNumForName: %s not found!", name);
    }
 
    return i;
}

lumpindex_t W_CheckNumForNameFromTo(const char *name, int from, int to)
{
    lumpindex_t i;

    for (i = from; i >= to; i--)
    {
        if (!strncasecmp(lumpinfo[i]->name, name, 8))
        {
            return i;
        }
    }

    return -1;
}

//
// W_LumpLength
// Returns the buffer size needed to load the given lump.
//
int W_LumpLength(lumpindex_t lump)
{
    if (lump >= numlumps)
    {
	I_Error ("W_LumpLength: %i >= numlumps", lump);
    }

    return lumpinfo[lump]->size;
}



//
// W_ReadLump
// Loads the lump into the given buffer,
//  which must be >= W_LumpLength().
//
void W_ReadLump(lumpindex_t lump, void *dest)
{
    int c;
    lumpinfo_t *l;

    if (lump >= numlumps)
    {
        I_Error ("W_ReadLump: %i >= numlumps", lump);
    }

    l = lumpinfo[lump];

    V_BeginRead(l->size);

    c = W_Read(l->wad_file, l->position, dest, l->size);

    if (c < l->size)
    {
        I_Error("W_ReadLump: only read %i of %i on lump %i",
                c, l->size, lump);
    }
}




//
// W_CacheLumpNum
//
// Load a lump into memory and return a pointer to a buffer containing
// the lump data.
//
// 'tag' is the type of zone memory buffer to allocate for the lump
// (usually PU_STATIC or PU_CACHE).  If the lump is loaded as 
// PU_STATIC, it should be released back using W_ReleaseLumpNum
// when no longer needed (do not use Z_ChangeTag).
//

void *W_CacheLumpNum(lumpindex_t lumpnum, int tag)
{
    void *result;
    lumpinfo_t *lump;

    if ((unsigned)lumpnum >= numlumps)
    {
	I_Error ("W_CacheLumpNum: %i >= numlumps", lumpnum);
    }

    lump = lumpinfo[lumpnum];

    // Get the pointer to return.  If the lump is in a memory-mapped
    // file, we can just return a pointer to within the memory-mapped
    // region.  If the lump is in an ordinary file, we may already
    // have it cached; otherwise, load it into memory.

    if (lump->wad_file->mapped != NULL)
    {
        // Memory mapped file, return from the mmapped region.

        result = lump->wad_file->mapped + lump->position;
    }
    else if (lump->cache != NULL)
    {
        // Already cached, so just switch the zone tag.

        result = lump->cache;
        Z_ChangeTag(lump->cache, tag);
    }
    else
    {
        // Not yet loaded, so load it now

       lump->cache = static_cast<void*>(Z_New<char>(tag, W_LumpLength(lumpnum),
                                                    reinterpret_cast<char**>(&lump->cache)));
	W_ReadLump (lumpnum, lump->cache);
        result = lump->cache;
    }
	
    return result;
}



//
// W_CacheLumpName
//
void *W_CacheLumpName(const char *name, int tag)
{
    return W_CacheLumpNum(W_GetNumForName(name), tag);
}

// 
// Release a lump back to the cache, so that it can be reused later 
// without having to read from disk again, or alternatively, discarded
// if we run out of memory.
//
// Back in Vanilla Doom, this was just done using Z_ChangeTag 
// directly, but now that we have WAD mmap, things are a bit more
// complicated ...
//

void W_ReleaseLumpNum(lumpindex_t lumpnum)
{
    lumpinfo_t *lump;

    if ((unsigned)lumpnum >= numlumps)
    {
	I_Error ("W_ReleaseLumpNum: %i >= numlumps", lumpnum);
    }

    lump = lumpinfo[lumpnum];

    if (lump->wad_file->mapped != NULL)
    {
        // Memory-mapped file, so nothing needs to be done here.
    }
    else
    {
        Z_ChangeTag(lump->cache, PU_CACHE);
    }
}

void W_ReleaseLumpName(const char *name)
{
    W_ReleaseLumpNum(W_GetNumForName(name));
}

#if 0

//
// W_Profile
//
int		info[2500][10];
int		profilecount;

void W_Profile (void)
{
    int		i;
    memblock_t*	block;
    void*	ptr;
    char	ch;
    FILE*	f;
    int		j;
    char	name[9];
	
	
    for (i=0 ; i<numlumps ; i++)
    {	
	ptr = lumpinfo[i].cache;
	if (!ptr)
	{
	    ch = ' ';
	    continue;
	}
	else
	{
	    block = (memblock_t *) ( (byte *)ptr - sizeof(memblock_t));
	    if (block->tag < PU_PURGELEVEL)
		ch = 'S';
	    else
		ch = 'P';
	}
	info[i][profilecount] = ch;
    }
    profilecount++;
	
    f = fopen ("waddump.txt","w");
    name[8] = 0;

    for (i=0 ; i<numlumps ; i++)
    {
       std::copy( lumpinfo[i].name, lumpinfo[i].name + 8, name );

	for (j=0 ; j<8 ; j++)
	    if (!name[j])
		break;

	for ( ; j<8 ; j++)
	    name[j] = ' ';

	fprintf (f,"%s ",name);

	for (j=0 ; j<profilecount ; j++)
	    fprintf (f,"    %c",info[i][j]);

	fprintf (f,"\n");
    }
    fclose (f);
}


#endif

// Generate a hash table for fast lookups

void W_GenerateHashTable(void)
{
    lumpindex_t i;

    // Free the old hash table, if there is one:
    if (lumphash != NULL)
    {
        Z_Delete(lumphash);
    }

    // Generate hash table
    if (numlumps > 0)
    {
        lumphash = Z_New<int>(PU_STATIC, numlumps);

        for (i = 0; i < numlumps; ++i)
        {
            lumphash[i] = -1;
        }

        for (i = 0; i < numlumps; ++i)
        {
            unsigned int hash;

            hash = W_LumpNameHash(lumpinfo[i]->name) % numlumps;

            // Hook into the hash table

            lumpinfo[i]->next = lumphash[hash];
            lumphash[hash] = i;
        }
    }

    // All done!
}

// The Doom reload hack. The idea here is that if you give a WAD file to -file
// prefixed with the ~ hack, that WAD file will be reloaded each time a new
// level is loaded. This lets you use a level editor in parallel and make
// incremental changes to the level you're working on without having to restart
// the game after every change.
// But: the reload feature is a fragile hack...
void W_Reload(void)
{
    lumpindex_t i;

    if (reloadname.empty())
    {
        return;
    }

    // We must free any lumps being cached from the PWAD we're about to reload:
    for (i = reloadlump; i < numlumps; ++i)
    {
        if (lumpinfo[i]->cache != NULL)
        {
            Z_Delete(lumpinfo[i]->cache);
        }
    }

    // Reset numlumps to remove the reload WAD file:
    numlumps = reloadlump;

    // Now reload the WAD file.
    auto filename = reloadname;

    W_CloseFile(reloadhandle);

    reloadname.clear();
    reloadlump = -1;
    reloadhandle = NULL;
    W_AddFile(filename);

    // The WAD directory has changed, so we have to regenerate the
    // fast lookup hashtable:
    W_GenerateHashTable();
}

const char *W_WadNameForLump(const lumpinfo_t *lump)
{
   return M_BaseName(lump->wad_file->path).data();
}

boolean W_IsIWADLump(const lumpinfo_t *lump)
{
	return lump->wad_file == lumpinfo[0]->wad_file;
}

// [crispy] dump lump data into a new LMP file
int W_LumpDump (const std::string &lumpname)
{
    const auto i = W_CheckNumForName(lumpname);

    if (i < 0 || !lumpinfo[i]->size)
    {
	return -1;
    }

    // [crispy] open file for writing
    std::string filename = lumpname + ".lmp";
    std::transform(filename.begin(), filename.end(),
                   filename.begin(), ::tolower);
    FILE *fp = fopen(filename, "wb");
    if (!fp)
    {
	I_Error("W_LumpDump: Failed writing to file '%s'!", filename);
    }

    std::string lump_p(lumpinfo[i]->size, 0);
    W_ReadLump(i, lump_p.data());
    fwrite(lump_p.data(), 1, lumpinfo[i]->size, fp);

    fclose(fp);

    return i;
}
