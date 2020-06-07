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
//

#include <string>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL_stdinc.h"

#include "d_iwad.hpp"
#include "doomtype.hpp"
#include "i_system.hpp"
#include "m_misc.hpp"
#include "m_argv.hpp"  // haleyjd 20110212: warning fix

std::vector<std::string> myargv;

//
// M_CheckParm
// Checks for the given parameter
// in the program's command line arguments.
// Returns the argument number (1 to argc-1)
// or 0 if not present
//

int M_CheckParmWithArgs(const char *check, int num_args)
{
    int i;

    for (i = 1; i < myargv.size() - num_args; i++)
    {
        if (!strcasecmp(check, myargv[i].c_str()))
	    return i;
    }

    return 0;
}

//
// M_ParmExists
//
// Returns true if the given parameter exists in the program's command
// line arguments, false if not.
//

boolean M_ParmExists(const char *check)
{
    return M_CheckParm(check) != 0;
}

int M_CheckParm(const char *check)
{
    return M_CheckParmWithArgs(check, 0);
}

#define MAXARGVS        100

static void LoadResponseFile(int argv_index, const char *filename)
{
    FILE *handle;
    int size;
    char *infile;
    char *file;
    int i, k;

    // Read the response file into memory
    handle = fopen(filename, "rb");

    if (handle == NULL)
    {
        printf ("\nNo such response file!");
        exit(1);
    }

    printf("Found response file %s!\n", filename);

    size = M_FileLength(handle);

    // Read in the entire file
    // Allocate one byte extra - this is in case there is an argument
    // at the end of the response file, in which case a '\0' will be
    // needed.

    auto unique_file = std::make_unique<char[]>(size + 1);
    file = unique_file.get();

    i = 0;

    while (i < size)
    {
        k = fread(file + i, 1, size - i, handle);

        if (k < 0)
        {
            I_Error("Failed to read full contents of '%s'", filename);
        }

        i += k;
    }

    fclose(handle);

    infile = file;
    k = 0;

    // myargv.erase( myargv.begin() + argv_index );

    while(k < size)
    {
        // Skip past space characters to the next argument

        while(k < size && isspace(infile[k]))
        {
            ++k;
        }

        if (k >= size)
        {
            break;
        }

        // If the next argument is enclosed in quote marks, treat
        // the contents as a single argument.  This allows long filenames
        // to be specified.

        if (infile[k] == '\"')
        {
            // Skip the first character(")
            ++k;

            int start = k;

            // Read all characters between quotes

            while (k < size && infile[k] != '\"' && infile[k] != '\n')
            {
                ++k;
            }

            if (k >= size || infile[k] == '\n')
            {
                I_Error("Quotes unclosed in response file '%s'",
                        filename);
            }

            // Cut off the string at the closing quote

            infile[k] = '\0';
            myargv.emplace( myargv.begin() + argv_index, infile+start, infile+k );
            ++k;
        }
        else
        {
            // Read in the next argument until a space is reached

            int start = k;

            while(k < size && !isspace(infile[k]))
            {
                ++k;
            }

            // Cut off the end of the argument at the first space

            infile[k] = '\0';
            myargv.emplace( myargv.begin() + argv_index, infile+start, infile+k );
            ++k;

        }
    }


#if 0
    // Disabled - Vanilla Doom does not do this.
    // Display arguments

    printf("%d command-line args:\n", myargv.size());

    for (k=1; k<myargv.size(); k++)
    {
        printf("'%s'\n", myargv[k]);
    }
#endif
}

//
// Find a Response File
//

void M_FindResponseFile(void)
{
    int i;

    for (i = 1; i < myargv.size(); i++)
    {
        if (myargv[i][0] == '@')
        {
           LoadResponseFile(i, myargv[i].c_str() + 1);
        }
    }

    for (;;)
    {
        //!
        // @arg <filename>
        //
        // Load extra command line arguments from the given response file.
        // Arguments read from the file will be inserted into the command
        // line replacing this argument. A response file can also be loaded
        // using the abbreviated syntax '@filename.rsp'.
        //
        i = M_CheckParmWithArgs("-response", 1);
        if (i <= 0)
        {
            break;
        }
        // Replace the -response argument so that the next time through
        // the loop we'll ignore it. Since some parameters stop reading when
        // an argument beginning with a '-' is encountered, we keep something
        // that starts with a '-'.
        myargv[i] = "-_";
        LoadResponseFile(i + 1, myargv[i + 1].c_str());
    }
}

#if defined(_WIN32)
enum
{
    FILETYPE_UNKNOWN = 0x0,
    FILETYPE_IWAD =    0x2,
    FILETYPE_PWAD =    0x4,
    FILETYPE_DEH =     0x8,
};

static int GuessFileType(const char *name)
{
    int ret = FILETYPE_UNKNOWN;
    static boolean iwad_found = false;

    auto lower = std::string( M_BaseName(name) );
    std::transform(lower.begin(), lower.end(),
                   lower.begin(), ::tolower);

    // only ever add one argument to the -iwad parameter

    if (iwad_found == false && D_IsIWADName(lower.c_str()))
    {
        ret = FILETYPE_IWAD;
        iwad_found = true;
    }
    else if (M_StringEndsWith(lower, ".wad") ||
             M_StringEndsWith(lower, ".lmp"))
    {
        ret = FILETYPE_PWAD;
    }
    else if (M_StringEndsWith(lower, ".deh") ||
             M_StringEndsWith(lower, ".bex") || // [crispy] *.bex
             M_StringEndsWith(lower, ".hhe") ||
             M_StringEndsWith(lower, ".seh"))
    {
        ret = FILETYPE_DEH;
    }

    return ret;
}

typedef struct
{
    char *str;
    int type, stable;
} argument_t;

static int CompareByFileType(const void *a, const void *b)
{
    const argument_t *arg_a = (const argument_t *) a;
    const argument_t *arg_b = (const argument_t *) b;

    const int ret = arg_a->type - arg_b->type;

    return ret ? ret : (arg_a->stable - arg_b->stable);
}

void M_AddLooseFiles(void)
{
    int i, types = 0;
    char **newargv;
    argument_t *arguments;

    if (myargv.size() < 2)
    {
        return;
    }

    // allocate space for up to three additional regular parameters

    auto unique_arguments = std::make_unique<argument_t[]>( myargv.size() + 3 );
    arguments = unique_arguments.get();
    
    std::fill( arguments, arguments + (myargv.size() + 3), argument_t{} );

    // check the command line and make sure it does not already
    // contain any regular parameters or response files
    // but only fully-qualified LFS or UNC file paths

    for (i = 1; i < myargv.size(); i++)
    {
        char *arg = myargv[i];
        int type;

        if (strlen(arg) < 3 ||
            arg[0] == '-' ||
            arg[0] == '@' ||
            ((!isalpha(arg[0]) || arg[1] != ':' || arg[2] != '\\') &&
            (arg[0] != '\\' || arg[1] != '\\')))
        {
            return;
        }

        type = GuessFileType(arg);
        arguments[i].str = arg;
        arguments[i].type = type;
        arguments[i].stable = i;
        types |= type;
    }

    // add space for one additional regular parameter
    // for each discovered file type in the new argument  list
    // and sort parameters right before their corresponding file paths

    if (types & FILETYPE_IWAD)
    {
        arguments[myargc].str = "-iwad";
        arguments[myargc].type = FILETYPE_IWAD - 1;
        myargc++;
    }
    if (types & FILETYPE_PWAD)
    {
        arguments[myargc].str = "-merge";
        arguments[myargc].type = FILETYPE_PWAD - 1;
        myargc++;
    }
    if (types & FILETYPE_DEH)
    {
        arguments[myargc].str = "-deh";
        arguments[myargc].type = FILETYPE_DEH - 1;
        myargc++;
    }

#error
    // NEED TO REWRITE THIS IF I BOTHER WITH WINDOWS
    
    // sort the argument list by file type, except for the zeroth argument
    // which is the executable invocation itself

    SDL_qsort(arguments + 1, myargc - 1, sizeof(*arguments), CompareByFileType);

    newargv[0] = myargv[0];

    for (i = 1; i < myargc; i++)
    {
        newargv[i] = arguments[i].str;
    }

    myargv = newargv;
}
#endif

// Return the name of the executable used to start the program:

const char *M_GetExecutableName(void)
{
   return M_BaseName(myargv[0]).data();
}

