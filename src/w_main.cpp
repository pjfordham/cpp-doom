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
//     Common code to parse command line, identifying WAD files to load.
//

#include <stdlib.h>
#include <fmt/core.h>

#include "config.h"
#include "d_iwad.hpp"
#include "i_glob.hpp"
#include "i_system.hpp"
#include "m_argv.hpp"
#include "w_main.hpp"
#include "w_merge.hpp"
#include "w_wad.hpp"
#include "z_zone.hpp"

// Parse the command line, merging WAD files that are sppecified.
// Returns true if at least one file was added.
boolean W_ParseCommandLine(void)
{
    boolean modifiedgame = false;
    int p;

    // Merged PWADs are loaded first, because they are supposed to be 
    // modified IWADs.

    //!
    // @arg <files>
    // @category mod
    //
    // Simulates the behavior of deutex's -merge option, merging a PWAD
    // into the main IWAD.  Multiple files may be specified.
    //

    p = M_CheckParmWithArgs("-merge", 1);

    if (p > 0)
    {
       for (p = p + 1; p<myargv.size() && myargv[p][0] != '-'; ++p)
        {
            modifiedgame = true;
            auto filename = D_TryFindWADByName(myargv[p]);
            fmt::print(" merging {}\n", filename);
            W_MergeFile(filename);
        }
    }

    // NWT-style merging:

    // NWT's -merge option:

    //!
    // @arg <files>
    // @category mod
    //
    // Simulates the behavior of NWT's -merge option.  Multiple files
    // may be specified.

    p = M_CheckParmWithArgs("-nwtmerge", 1);

    if (p > 0)
    {
       for (p = p + 1; p<myargv.size() && myargv[p][0] != '-'; ++p)
        {
            modifiedgame = true;
            auto filename = D_TryFindWADByName(myargv[p]);
            fmt::print(" performing NWT-style merge of {}\n", filename);
            W_NWTDashMerge(filename);
        }
    }
    
    // Add flats

    //!
    // @arg <files>
    // @category mod
    //
    // Simulates the behavior of NWT's -af option, merging flats into
    // the main IWAD directory.  Multiple files may be specified.
    //

    p = M_CheckParmWithArgs("-af", 1);

    if (p > 0)
    {
       for (p = p + 1; p<myargv.size() && myargv[p][0] != '-'; ++p)
        {
            modifiedgame = true;
            auto filename = D_TryFindWADByName(myargv[p]);
            fmt::print(" merging flats from {}\n", filename);
            W_NWTMergeFile(filename, W_NWT_MERGE_FLATS);
        }
    }

    //!
    // @arg <files>
    // @category mod
    //
    // Simulates the behavior of NWT's -as option, merging sprites
    // into the main IWAD directory.  Multiple files may be specified.
    //

    p = M_CheckParmWithArgs("-as", 1);

    if (p > 0)
    {
       for (p = p + 1; p<myargv.size() && myargv[p][0] != '-'; ++p)
        {
            modifiedgame = true;
            auto filename = D_TryFindWADByName(myargv[p]);
            fmt::print(" merging sprites from {}\n", filename);
            W_NWTMergeFile(filename, W_NWT_MERGE_SPRITES);
        }
    }

    //!
    // @arg <files>
    // @category mod
    //
    // Equivalent to "-af <files> -as <files>".
    //

    p = M_CheckParmWithArgs("-aa", 1);

    if (p > 0)
    {
       for (p = p + 1; p<myargv.size() && myargv[p][0] != '-'; ++p)
        {
            modifiedgame = true;
            auto filename = D_TryFindWADByName(myargv[p]);
            fmt::print(" merging sprites and flats from {}\n", filename);
            W_NWTMergeFile(filename, W_NWT_MERGE_SPRITES | W_NWT_MERGE_FLATS);
        }
    }

    //!
    // @arg <files>
    // @vanilla
    //
    // Load the specified PWAD files.
    //

    p = M_CheckParmWithArgs ("-file", 1);
    if (p)
    {
	// the parms after p are wadfile/lump names,
	// until end of parms or another - preceded parm
	modifiedgame = true;            // homebrew levels
	while (++p != myargv.size() && myargv[p][0] != '-')
        {
            auto filename = D_TryFindWADByName(myargv[p]);
            // [crispy] always merge arguments of "-file" parameter
            fmt::print(" merging {} !\n", filename);
	    W_MergeFile(filename);
        }
    }

//    W_PrintDirectory();

    return modifiedgame;
}

// Load all WAD files from the given directory.
void W_AutoLoadWADs(const std::string &path)
{
    glob_t *glob;
    std::string filename;

    std::vector<std::string> globs = { "*.wad", "*.lmp" };
    glob = I_StartMultiGlob(path, GLOB_FLAG_NOCASE|GLOB_FLAG_SORTED,
                            globs);
    for (;;)
    {
        filename = I_NextGlob(glob);
        if (filename.empty())
        {
            break;
        }
        fmt::print(" [autoload] merging {}\n", filename);
        W_MergeFile(filename);
    }

    I_EndGlob(glob);
}

// Lump names that are unique to particular game types. This lets us check
// the user is not trying to play with the wrong executable, eg.
// chocolate-doom -iwad hexen.wad.
static const struct
{
    GameMission_t mission;
    const char *lumpname;
} unique_lumps[] = {
    { doom,    "POSSA1" },
    { heretic, "IMPXA1" },
    { hexen,   "ETTNA1" },
    { strife,  "AGRDA1" },
};

void W_CheckCorrectIWAD(GameMission_t mission)
{
    int i;
    lumpindex_t lumpnum;

    for (i = 0; i < arrlen(unique_lumps); ++i)
    {
        if (mission != unique_lumps[i].mission)
        {
            lumpnum = W_CheckNumForName(unique_lumps[i].lumpname);

            if (lumpnum >= 0)
            {
                I_Error("\nYou are trying to use a %s IWAD file with "
                        "the %s%s binary.\nThis isn't going to work.\n"
                        "You probably want to use the %s%s binary.",
                        D_SuggestGameName(unique_lumps[i].mission,
                                          indetermined),
                        PROGRAM_PREFIX,
                        D_GameMissionString(mission),
                        PROGRAM_PREFIX,
                        D_GameMissionString(unique_lumps[i].mission));
            }
        }
    }
}

