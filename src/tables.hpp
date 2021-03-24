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
//	Lookup tables.
//	Do not try to look them up :-).
//	In the order of appearance: 
//
//	int finetangent[4096]	- Tangens LUT.
//	 Should work with BAM fairly well (12 of 16bit,
//      effectively, by shifting).
//
//	int finesine[10240]		- Sine lookup.
//	 Guess what, serves as cosine, too.
//	 Remarkable thing is, how to use BAMs with this? 
//
//	int tantoangle[2049]	- ArcTan LUT,
//	  maps tan(angle) to angle fast. Gotta search.	
//    


#ifndef __TABLES__
#define __TABLES__

#include "doomtype.hpp"

#include "m_fixed.hpp"
	
#define FINEANGLES		8192
#define FINEMASK		(FINEANGLES-1)

// Effective size is 10240.
extern const fixed_t finesine[5*FINEANGLES/4];

// Re-use data, is just PI/2 pahse shift.
extern const fixed_t *finecosine;


// Effective size is 4096.
extern const fixed_t finetangent[FINEANGLES/2];

// Gamma correction tables.
extern const byte gammatable[5][256];

// Binary Angle Measument, BAM.

class angle_t {
   unsigned int value;
public:
   angle_t() = default;
   explicit angle_t( unsigned int _value ) : value(_value) {
   }

   explicit operator unsigned int() const { return value;}
   explicit operator bool() const { return value != 0; }

   // comparison
   friend bool operator<(angle_t lhs, angle_t rhs) {
      return lhs.value < rhs.value;
   }
   friend bool operator>(angle_t lhs, angle_t rhs) {
      return lhs.value > rhs.value;
   }
   friend bool operator==(angle_t lhs, angle_t rhs) {
      return lhs.value == rhs.value;
   }
   friend bool operator<=(angle_t lhs, angle_t rhs) {
      return lhs.value <= rhs.value;
   }
   friend bool operator>=(angle_t lhs, angle_t rhs) {
      return lhs.value >= rhs.value;
   }

   // unary
   friend angle_t operator-(angle_t lhs) {
      return angle_t(-lhs.value);
   }

   // binary, angle_t, angle_t => angle_t
   friend angle_t operator+(angle_t lhs, angle_t rhs) {
      return angle_t(lhs.value + rhs.value);
   }
   friend angle_t operator-(angle_t lhs, angle_t rhs) {
      return angle_t(lhs.value - rhs.value);
   }

   // updating versions of +/-
   friend angle_t operator+=(angle_t &lhs, angle_t rhs) {
      return angle_t(lhs.value += rhs.value);
   }
   friend angle_t operator-=(angle_t &lhs, angle_t rhs) {
      return angle_t(lhs.value -= rhs.value);
   }

   friend angle_t operator*(angle_t lhs, unsigned int rhs) {
      return angle_t(lhs.value * rhs);
   }
   friend angle_t operator*(unsigned int lhs, angle_t rhs) {
      return angle_t(lhs * rhs.value);
   }
   friend angle_t operator/(angle_t lhs, unsigned int rhs) {
      return angle_t(lhs.value / rhs);
   }
   friend unsigned int operator/(angle_t lhs, angle_t rhs) {
      return lhs.value / rhs.value;
   }

   angle_t snap_to_8ths() {
      return angle_t( value & (0x7 << 29));
   }

   template <unsigned int shift> friend class angleshift_t;
   friend struct fmt::formatter<angle_t>;

};

// Is this a template too far?
// Angle_t's can be converted back and forward to unsigned ints
// in a few different ways with different shift. Try to caputre
// those cases and make them typesafe.
template <unsigned int shift> class angleshift_t {
   friend auto operator>>( angle_t a, angleshift_t b) {
      return a.value >> shift;
   }
   friend auto operator<<( decltype(angle_t::value) a, angleshift_t b) {
      return angle_t(a << shift);
   }
};

const angleshift_t<19> ANGLETOFINESHIFT;
const angleshift_t<29> ANGLETOMOVEDIRSHIFT;
const angleshift_t<16> ANGLETURNBITS;

template <>
struct fmt::formatter<angle_t> {
   constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

   template <typename FormatContext>
   auto format(const angle_t& t, FormatContext& ctx) {
      return format_to(ctx.out(), "{}", t.value >> 24 );
   }
};

const angle_t ANG0 = angle_t(0x00000000u);
const angle_t ANG45 = angle_t(0x20000000u);
const angle_t ANG90 = angle_t(0x40000000u);
const angle_t ANG180 = angle_t(0x80000000u);
const angle_t ANG270 = angle_t(0xc0000000u);

const angle_t ANG1 = (ANG45 / 45);
const angle_t ANG60 = (ANG180 / 3);

// Heretic code uses this definition as though it represents one 
// degree, but it is not!  This is actually ~1.40 degrees.

const angle_t ANG1_X = angle_t(0x01000000u);

#define SLOPERANGE		2048
#define SLOPEBITS		11
#define DBITS			(FRACBITS.size()-SLOPEBITS)

// Effective size is 2049;
// The +1 size is to handle the case when x==y
//  without additional checking.
extern const angle_t *tantoangle;


// Utility function,
//  called by R_PointToAngle.
int SlopeDiv(unsigned int num, unsigned int den);
int SlopeDivCrispy(unsigned int num, unsigned int den);

inline fixed_t sin(angle_t t) {
   return finesine[ t >> ANGLETOFINESHIFT ];
}
inline fixed_t cos(angle_t t) {
   return finecosine[ t >> ANGLETOFINESHIFT ];
}
inline fixed_t tan(angle_t t) {
   return finetangent[ t >> ANGLETOFINESHIFT ];
}

inline angle_t arctan(fixed_t t) {
   return tantoangle[ (int)(t >> DBITS) ];
}

#endif

