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
//      Miscellaneous.
//    


#ifndef __M_MISC__
#define __M_MISC__

#include <stdio.h>
#include <stdarg.h>
#include <memory>

#include "doomtype.hpp"

boolean M_WriteFile(const std::string &name, const void *source, int length);
int M_ReadFile(const std::string &name, byte **buffer);
void M_MakeDirectory(const std::string &dir);
std::string M_TempFile(const std::string &s);
boolean M_FileExists(const std::string &file);
std::string M_FileCaseExists(const std::string &file);
long M_FileLength(FILE *handle);
boolean M_StrToInt(const std::string &str, int *result);
std::string M_DirName(const std::string_view &path);
const char *M_BaseName(const char *path);
void M_ExtractFileBase(const char *path, char *dest);
void M_ForceUppercase(char *text);
void M_ForceLowercase(char *text);
const char *M_StrCaseStr(const char *haystack, const char *needle);
char *M_StringDuplicate(const std::string_view &orig);
boolean M_StringCopy(char *dest, const char *src, size_t dest_size);
boolean M_StringConcat(char *dest, const char *src, size_t dest_size);
std::string M_StringReplace(const std::string_view &haystack, const std::string_view &needle,
                            const std::string_view &replacement);
char *M_StringJoin(const char *s, ...);
boolean M_StringStartsWith(const std::string_view &s, const std::string_view &prefix);
boolean M_StringEndsWith(const std::string_view &s, const std::string_view &suffix);
int M_vsnprintf(char *buf, size_t buf_len, const char *s, va_list args);
int M_snprintf(char *buf, size_t buf_len, const char *s, ...) PRINTF_ATTR(3, 4);

#endif

