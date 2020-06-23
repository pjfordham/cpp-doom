//
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
//
// Parses Text substitution sections in dehacked files
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <map>
#include <string>
#include <unordered_set>

#include "doomtype.hpp"
#include "deh_str.hpp"
#include "m_misc.hpp"

#include "../utils/memory.hpp"
#include "z_zone.hpp"
#include <iostream>

std::map<std::string, std::string> hash_table;
std::unordered_set<std::string> list;

// Look up a string to see if it has been replaced with something else
// This will be used throughout the program to substitute text

const std::string &DEH_String(const std::string &s)
{
   auto i = hash_table.find( s );

   if (i == hash_table.end()) {
      auto a = list.insert( s );
      //std::cerr << "Sub no " << s << " " <<  a.first->c_str() << "\n";
      return *a.first;
   } else {
      //std::cerr << "Sub yes " << s << " " <<  i->second.c_str() << "\n";
      return i->second;
  }
}

// [crispy] returns true if a string has been substituted

boolean DEH_HasStringReplacement(const std::string &s)
{
     auto i = hash_table.find( s );
     //std::cerr << "Has " << s << " " <<  (i != hash_table.end() ? "true" : "false") << "\n";
     return i != hash_table.end();
}

void DEH_AddStringReplacement(const std::string &from_text, const std::string &to_text)
{
   //std::cerr << "Add " << from_text << " " << to_text << "\n";
   hash_table[ from_text ] = to_text;
}

typedef enum
{
    FORMAT_ARG_INVALID,
    FORMAT_ARG_INT,
    FORMAT_ARG_FLOAT,
    FORMAT_ARG_CHAR,
    FORMAT_ARG_STRING,
    FORMAT_ARG_PTR,
    FORMAT_ARG_SAVE_POS
} format_arg_t;

// Get the type of a format argument.
// We can mix-and-match different format arguments as long as they
// are for the same data type.

static format_arg_t FormatArgumentType(char c)
{
    switch (c)
    {
        case 'd': case 'i': case 'o': case 'u': case 'x': case 'X':
            return FORMAT_ARG_INT;

        case 'e': case 'E': case 'f': case 'F': case 'g': case 'G':
        case 'a': case 'A':
            return FORMAT_ARG_FLOAT;

        case 'c': case 'C':
            return FORMAT_ARG_CHAR;

        case 's': case 'S':
            return FORMAT_ARG_STRING;

        case 'p':
            return FORMAT_ARG_PTR;

        case 'n':
            return FORMAT_ARG_SAVE_POS;

        default:
            return FORMAT_ARG_INVALID;
    }
}

// Given the specified string, get the type of the first format
// string encountered.

static format_arg_t NextFormatArgument(const char **str)
{
    format_arg_t argtype;

    // Search for the '%' starting the next string.

    while (**str != '\0')
    {
        if (**str == '%')
        {
            ++*str;

            // Don't stop for double-%s.

            if (**str != '%')
            {
                break;
            }
        }

        ++*str;
    }

    // Find the type of the format string.

    while (**str != '\0')
    {
        argtype = FormatArgumentType(**str);

        if (argtype != FORMAT_ARG_INVALID)
        {
            ++*str;

            return argtype;
        }

        ++*str;
    }

    // Stop searching, we have reached the end.

    *str = NULL;

    return FORMAT_ARG_INVALID;
}

// Check if the specified argument type is a valid replacement for
// the original.

static boolean ValidArgumentReplacement(format_arg_t original,
                                        format_arg_t replacement)
{
    // In general, the original and replacement types should be
    // identical.  However, there are some cases where the replacement
    // is valid and the types don't match.

    // Characters can be represented as ints.

    if (original == FORMAT_ARG_CHAR && replacement == FORMAT_ARG_INT)
    {
        return true;
    }

    // Strings are pointers.

    if (original == FORMAT_ARG_STRING && replacement == FORMAT_ARG_PTR)
    {
        return true;
    }

    return original == replacement;
}

// Return true if the specified string contains no format arguments.

static boolean ValidFormatReplacement(const char *original, const char *replacement)
{
    const char *rover1;
    const char *rover2;

    // Check each argument in turn and compare types.

    rover1 = original; rover2 = replacement;

    for (;;)
    {
        const auto argtype1 = NextFormatArgument(&rover1);
        const auto argtype2 = NextFormatArgument(&rover2);

        if (argtype2 == FORMAT_ARG_INVALID)
        {
            // No more arguments left to read from the replacement string.

            break;
        }
        else if (argtype1 == FORMAT_ARG_INVALID)
        {
            // Replacement string has more arguments than the original.

            return false;
        }
        else if (!ValidArgumentReplacement(argtype1, argtype2))
        {
            // Not a valid replacement argument.

            return false;
        }
    }

    return true;
}

// Get replacement format string, checking arguments.

static const char *FormatStringReplacement(const char *s)
{
    const char *repl;

    repl = DEH_String(s).c_str();

    if (!ValidFormatReplacement(s, repl))
    {
        printf("WARNING: Unsafe dehacked replacement provided for "
               "printf format string: %s\n", s);

        return s;
    }

    return repl;
}

// printf(), performing a replacement on the format string.

void DEH_printf(const char *fmt, ...)
{
    va_list args;
    const char *repl;

    repl = FormatStringReplacement(fmt);

    va_start(args, fmt);

    vprintf(repl, args);

    va_end(args);
}

// fprintf(), performing a replacement on the format string.

void DEH_fprintf(FILE *fstream, const char *fmt, ...)
{
    va_list args;
    const char *repl;

    repl = FormatStringReplacement(fmt);

    va_start(args, fmt);

    vfprintf(fstream, repl, args);

    va_end(args);
}

// snprintf(), performing a replacement on the format string.

void DEH_snprintf(char *buffer, size_t len, const char *fmt, ...)
{
    va_list args;
    const char *repl;

    repl = FormatStringReplacement(fmt);

    va_start(args, fmt);

    M_vsnprintf(buffer, len, repl, args);

    va_end(args);
}

