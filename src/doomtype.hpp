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
//	Simple basic typedefs, isolated here to make it easier
//	 separating modules.
//    


#ifndef __DOOMTYPE__
#define __DOOMTYPE__

#include "config.h"

#if defined(_MSC_VER) && !defined(__cplusplus)
#define inline __inline
#endif

// #define macros to provide functions missing in Windows.
// Outside Windows, we use strings.h for str[n]casecmp.


#if !HAVE_DECL_STRCASECMP || !HAVE_DECL_STRNCASECMP

#include <string.h>
#if !HAVE_DECL_STRCASECMP
#define strcasecmp stricmp
#endif
#if !HAVE_DECL_STRNCASECMP
#define strncasecmp strnicmp
#endif

#else

#include <strings.h>

#endif


//
// The packed attribute forces structures to be packed into the minimum 
// space necessary.  If this is not done, the compiler may align structure
// fields differently to optimize memory access, inflating the overall
// structure size.  It is important to use the packed attribute on certain
// structures where alignment is important, particularly data read/written
// to disk.
//

#ifdef __GNUC__

#if defined(_WIN32) && !defined(__clang__)
#define PACKEDATTR __attribute__((packed,gcc_struct))
#else
#define PACKEDATTR __attribute__((packed))
#endif

#define PRINTF_ATTR(fmt, first) __attribute__((format(printf, fmt, first)))
#define PRINTF_ARG_ATTR(x) __attribute__((format_arg(x)))
#define NORETURN __attribute__((noreturn))

#else
#if defined(_MSC_VER)
#define PACKEDATTR __pragma(pack(pop))
#else
#define PACKEDATTR
#endif
#define PRINTF_ATTR(fmt, first)
#define PRINTF_ARG_ATTR(x)
#define NORETURN
#endif

#ifdef __WATCOMC__
#define PACKEDPREFIX _Packed
#elif defined(_MSC_VER)
#define PACKEDPREFIX __pragma(pack(push,1))
#else
#define PACKEDPREFIX
#endif

#define PACKED_STRUCT(...) PACKEDPREFIX struct __VA_ARGS__ PACKEDATTR

// C99 integer types; with gcc we just use this.  Other compilers
// should add conditional statements that define the C99 types.

// What is really wanted here is stdint.h; however, some old versions
// of Solaris don't have stdint.h and only have inttypes.h (the 
// pre-standardisation version).  inttypes.h is also in the C99 
// standard and defined to include stdint.h, so include this. 

#include <inttypes.h>

#if defined(__cplusplus) || defined(__bool_true_false_are_defined)

// Use builtin bool type with C++.

typedef bool boolean;

#else

typedef enum 
{
    false, 
    true
} boolean;

#endif

typedef uint8_t byte;
#ifndef CRISPY_TRUECOLOR
typedef uint8_t pixel_t;
typedef int16_t dpixel_t;
#else
typedef uint32_t pixel_t;
typedef int64_t dpixel_t;
#endif

#include <limits.h>

#ifdef _WIN32

#define DIR_SEPARATOR '\\'
#define DIR_SEPARATOR_S "\\"
#define PATH_SEPARATOR ';'

#else

#define DIR_SEPARATOR '/'
#define DIR_SEPARATOR_S "/"
#define PATH_SEPARATOR ':'

#endif

#define arrlen(array) (sizeof(array) / sizeof(*array))

#include <iterator>
#include <fmt/core.h>
#include <string_view>

struct lump_name_t {
   char name[8];
   lump_name_t(const std::string &_name) {
      std::fill(std::begin(name), std::end(name), 0);
      std::copy_n(std::begin(_name), std::min<std::size_t>(8,_name.length()), std::begin(name));
   }
   lump_name_t(const std::string_view _name) {
      std::fill(std::begin(name), std::end(name), 0);
      std::copy_n(std::begin(_name), std::min<std::size_t>(8,_name.length()), std::begin(name));
   }
   lump_name_t(const char *_name) {
      std::fill(std::begin(name), std::end(name), 0);
        strncpy( name, _name, 8);

      }
   lump_name_t() {
      std::fill(std::begin(name), std::end(name), 0);
      name[0] = '-';
   }

   int length() const {
      for( int i = 0; i < 8; ++i ) {
         if (name[i] == 0)
            return i;
      }
      return 8;
   }

   std::string to_string() const {
      return std::string( name, length() );
   }

   char &operator[](std::size_t t) {
      return name[t];
   }
   const char &operator[](std::size_t t) const {
      return name[t];
   }

   bool operator==( const lump_name_t &that ) const {
      return std::equal( std::begin(name), std::end(name),
                         std::begin(that.name), std::end(that.name),
                         [](char a, char b) {
                            return tolower(a) == tolower(b);
                         });
   }

   bool no_texture() const { return name[0] == '-'; }
};

template <>
struct fmt::formatter<lump_name_t> {
  constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const lump_name_t& t, FormatContext& ctx) {
     std::size_t i = 0;
     std::string out;
     while( i < 8 && t.name[i] != 0 ) {
        out.push_back( t.name[i++] );
     }
     return format_to(ctx.out(), "{}", out);
  }
};

#endif
