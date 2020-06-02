//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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
//      Miscellaneous.
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <string>
#include <algorithm>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>
#ifdef _MSC_VER
#include <direct.h>
#endif
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "doomtype.hpp"

#include "deh_str.hpp"

#include "../utils/memory.hpp"
#include "i_swap.hpp"
#include "i_system.hpp"
#include "i_video.hpp"
#include "m_misc.hpp"
#include "v_video.hpp"
#include "w_wad.hpp"
#include "z_zone.hpp"

//
// Create a directory
//

void M_MakeDirectory(const std::string &path)
{
#ifdef _WIN32
    mkdir(path.c_str());
#else
    mkdir(path.c_str(), 0755);
#endif
}

// Check if a file exists

boolean M_FileExists(const std::string &filename)
{
    FILE *fstream;

    fstream = fopen(filename.c_str(), "r");

    if (fstream != NULL)
    {
        fclose(fstream);
        return true;
    }
    else
    {
        // If we can't open because the file is a directory, the 
        // "file" exists at least!

        return errno == EISDIR;
    }
}

// Check if a file exists by probing for common case variation of its filename.
// Returns a newly allocated string that the caller is responsible for freeing.

std::string M_FileCaseExists(const std::string &path)
{
    // 0: actual path
    if (M_FileExists(path))
    {
       return path;
    }

    // Make a modifiable string
    std::string path_dup = path;
    std::size_t pos = path_dup.rfind( DIR_SEPARATOR );

    auto filename_begin = pos == std::string::npos ? path_dup.begin() : path_dup.begin() + pos + 1;
    auto filename_end = path_dup.end();

    // 1: lowercase filename, e.g. doom2.wad
    std::transform(filename_begin, filename_end, filename_begin, ::tolower);

    if (M_FileExists( path_dup ) )
    {
          return path_dup;
    }

    // 2: uppercase filename, e.g. DOOM2.WAD
    std::transform(filename_begin, filename_end, filename_begin, ::toupper);

    if (M_FileExists(path_dup))
    {
        return path_dup;
    }

    // 3. uppercase basename with lowercase extension, e.g. DOOM2.wad
    pos = path_dup.rfind( "." );
    if ( pos != std::string::npos ) {
       std::transform(path_dup.begin() + pos, path_dup.end(), path_dup.begin() + pos, ::tolower);

       if (M_FileExists(path_dup))
       {
          return path_dup;
       }
    }

    // 4. lowercase filename with uppercase first letter, e.g. Doom2.wad
    if (path_dup.size() > 1)
    {
       std::transform(filename_begin + 1, filename_end, filename_begin, ::tolower);

       if (M_FileExists(path_dup))
       {
          return path_dup;
       }
    }

    // 5. no luck
    return {};
}

//
// Determine the length of an open file.
//

long M_FileLength(FILE *handle)
{ 
    long savedpos;
    long length;

    // save the current position in the file
    savedpos = ftell(handle);
    
    // jump to the end and find the length
    fseek(handle, 0, SEEK_END);
    length = ftell(handle);

    // go back to the old location
    fseek(handle, savedpos, SEEK_SET);

    return length;
}

//
// M_WriteFile
//

boolean M_WriteFile(const std::string &name, const void *source, int length)
{
    FILE *handle;
    int	count;
	
    handle = fopen(name.c_str(), "wb");

    if (handle == NULL)
	return false;

    count = fwrite(source, 1, length, handle);
    fclose(handle);
	
    if (count < length)
	return false;
		
    return true;
}


//
// M_ReadFile
//

int M_ReadFile(const std::string &name, byte **buffer)
{
    FILE *handle;
    int	count, length;
    byte *buf;
	
    handle = fopen(name.c_str(), "rb");
    if (handle == NULL)
        I_Error ("Couldn't read file %s", name.c_str());

    // find the size of the file by seeking to the end and
    // reading the current position

    length = M_FileLength(handle);
    
    buf = zone_malloc<byte>(PU_STATIC, length + 1);
    count = fread(buf, 1, length, handle);
    fclose (handle);
	
    if (count < length)
        I_Error ("Couldn't read file %s", name.c_str());
		
    buf[length] = '\0';
    *buffer = buf;
    return length;
}

// Returns the path to a temporary file of the given name, stored
// inside the system temporary directory.
//
// The returned value must be freed with Z_Free after use.

std::string M_TempFile(const std::string &s)
{
#ifdef _WIN32
    // Check the TEMP environment variable to find the location.
    const char *tempdir = getenv("TEMP");
    if (tempdir == NULL)
    {
        tempdir = ".";
    }
    return tempdir;
#else
    // In Unix, just use /tmp.

    return std::string( "/tmp" ) + DIR_SEPARATOR_S + s;
#endif

}

boolean M_StrToInt(const std::string &str, int *result)
{
   auto c_str = str.c_str();
   return sscanf(c_str, " 0x%x", result) == 1
        || sscanf(c_str, " 0X%x", result) == 1
        || sscanf(c_str, " 0%o", result) == 1
        || sscanf(c_str, " %d", result) == 1;
}

// Returns the directory portion of the given path, without the trailing
// slash separator character. If no directory is described in the path,
// the string "." is returned. In either case, the result is newly allocated
// and must be freed by the caller after use.
std::string M_DirName(const std::string_view &path)
{
    std::size_t p = path.rfind( DIR_SEPARATOR );
    if (p == std::string::npos)
    {
       return std::string(".");
    }
    else
    {
       return std::string( path.substr( p + 1 ) );
    }
}

// Returns the base filename described by the given path (without the
// directory name). The result points inside path and nothing new is
// allocated.
const char *M_BaseName(const char *path)
{
    const char *p;

    p = strrchr(path, DIR_SEPARATOR);
    if (p == NULL)
    {
        return path;
    }
    else
    {
        return p + 1;
    }
}

void M_ExtractFileBase(const char *path, char *dest)
{
    const char *src;
    const char *filename;
    int length;

    src = path + strlen(path) - 1;

    // back up until a \ or the start
    while (src != path && *(src - 1) != DIR_SEPARATOR)
    {
	src--;
    }

    filename = src;

    // Copy up to eight characters
    // Note: Vanilla Doom exits with an error if a filename is specified
    // with a base of more than eight characters.  To remove the 8.3
    // filename limit, instead we simply truncate the name.

    length = 0;
    std::fill(dest, dest + 8, 0);

    while (*src != '\0' && *src != '.')
    {
        if (length >= 8)
        {
            printf("Warning: Truncated '%s' lump name to '%.8s'.\n",
                   filename, dest);
            break;
        }

	dest[length++] = toupper((int)*src++);
    }
}

//---------------------------------------------------------------------------
//
// PROC M_ForceUppercase
//
// Change string to uppercase.
//
//---------------------------------------------------------------------------

void M_ForceUppercase(char *text)
{
    char *p;

    for (p = text; *p != '\0'; ++p)
    {
        *p = toupper(*p);
    }
}

//---------------------------------------------------------------------------
//
// PROC M_ForceLowercase
//
// Change string to lowercase.
//
//---------------------------------------------------------------------------

void M_ForceLowercase(char *text)
{
    char *p;

    for (p = text; *p != '\0'; ++p)
    {
        *p = tolower(*p);
    }
}

//
// M_StrCaseStr
//
// Case-insensitive version of strstr()
//

const char *M_StrCaseStr(const char *haystack, const char *needle)
{
    unsigned int haystack_len;
    unsigned int needle_len;
    unsigned int len;
    unsigned int i;

    haystack_len = strlen(haystack);
    needle_len = strlen(needle);

    if (haystack_len < needle_len)
    {
        return NULL;
    }

    len = haystack_len - needle_len;

    for (i = 0; i <= len; ++i)
    {
        if (!strncasecmp(haystack + i, needle, needle_len))
        {
            return haystack + i;
        }
    }

    return NULL;
}

//
// Safe version of strdup() that checks the string was successfully
// allocated.
//

char *M_StringDuplicate(const std::string_view &orig)
{
    char *result;

    result = strndup(orig.data(), orig.size() );

    if (result == NULL)
    {
        I_Error("Failed to duplicate string (length %" PRIuPTR ")\n",
                orig.size());
    }

    return result;
}

//
// String replace function.
//

std::string M_StringReplace(const std::string_view &haystack, const std::string_view &needle,
                            const std::string_view &replacement)
{
    std::string result;
    auto remainder = haystack;

    auto pos = remainder.find( needle );
    while ( pos != std::string::npos ) {
       result += remainder.substr( 0, pos );
       result += replacement;
       remainder = remainder.substr( pos + needle.length() );
       pos = remainder.find( needle );
    }

    result += remainder;
    return result;
}

// Safe string copy function that works like OpenBSD's strlcpy().
// Returns true if the string was not truncated.

boolean M_StringCopy(char *dest, const char *src, size_t dest_size)
{
    size_t len;

    if (dest_size >= 1)
    {
        dest[dest_size - 1] = '\0';
        strncpy(dest, src, dest_size - 1);
    }
    else
    {
        return false;
    }

    len = strlen(dest);
    return src[len] == '\0';
}

// Safe string concat function that works like OpenBSD's strlcat().
// Returns true if string not truncated.

boolean M_StringConcat(char *dest, const char *src, size_t dest_size)
{
    size_t offset;

    offset = strlen(dest);
    if (offset > dest_size)
    {
        offset = dest_size;
    }

    return M_StringCopy(dest + offset, src, dest_size - offset);
}

// Returns true if 's' begins with the specified prefix.

boolean M_StringStartsWith(const std::string_view &s, const std::string_view &prefix)
{
   return s.rfind( prefix, 0 ) == 0;
}

// Returns true if 's' ends with the specified suffix.

boolean M_StringEndsWith(const std::string_view &s, const std::string_view &suffix)
{
    if (suffix.size() > s.size()) return false;
    return std::equal(suffix.rbegin(), suffix.rend(), s.rbegin());

}

// Return a newly-malloced string with all the strings given as arguments
// concatenated together.

char *M_StringJoin(const char *s, ...)
{
    char *result;
    const char *v;
    va_list args;
    size_t result_len;

    result_len = strlen(s) + 1;

    va_start(args, s);
    for (;;)
    {
        v = va_arg(args, const char *);
        if (v == NULL)
        {
            break;
        }

        result_len += strlen(v);
    }
    va_end(args);

    result = static_cast<char *>(malloc(result_len));

    if (result == NULL)
    {
        I_Error("M_StringJoin: Failed to allocate new string.");
        return NULL;
    }

    M_StringCopy(result, s, result_len);

    va_start(args, s);
    for (;;)
    {
        v = va_arg(args, const char *);
        if (v == NULL)
        {
            break;
        }

        M_StringConcat(result, v, result_len);
    }
    va_end(args);

    return result;
}

// On Windows, vsnprintf() is _vsnprintf().
#ifdef _WIN32
#if _MSC_VER < 1400 /* not needed for Visual Studio 2008 */
#define vsnprintf _vsnprintf
#endif
#endif

// Safe, portable vsnprintf().
int M_vsnprintf(char *buf, size_t buf_len, const char *s, va_list args)
{
    int result;

    if (buf_len < 1)
    {
        return 0;
    }

    // Windows (and other OSes?) has a vsnprintf() that doesn't always
    // append a trailing \0. So we must do it, and write into a buffer
    // that is one byte shorter; otherwise this function is unsafe.
    result = vsnprintf(buf, buf_len, s, args);

    // If truncated, change the final char in the buffer to a \0.
    // A negative result indicates a truncated buffer on Windows.
    if (result < 0 || result >= buf_len)
    {
        buf[buf_len - 1] = '\0';
        result = buf_len - 1;
    }

    return result;
}

// Safe, portable snprintf().
int M_snprintf(char *buf, size_t buf_len, const char *s, ...)
{
    va_list args;
    int result;
    va_start(args, s);
    result = M_vsnprintf(buf, buf_len, s, args);
    va_end(args);
    return result;
}

#ifdef _WIN32

char *M_OEMToUTF8(const char *oem)
{
    unsigned int len = strlen(oem) + 1;
    wchar_t *tmp;
    char *result;

    tmp = malloc(len * sizeof(wchar_t));
    MultiByteToWideChar(CP_OEMCP, 0, oem, len, tmp, len);
    result = malloc(len * 4);
    WideCharToMultiByte(CP_UTF8, 0, tmp, len, result, len * 4, NULL, NULL);
    free(tmp);

    return result;
}

#endif

