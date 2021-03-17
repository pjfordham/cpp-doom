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
//	Fixed point arithemtics, implementation.
//


#ifndef __M_FIXED__
#define __M_FIXED__




//
// Fixed point, 32bit as 16.16.
//
#include <cstdint>
#include <fmt/core.h>

class fixed_t {
   int value;
public:
   fixed_t() = default;
   fixed_t(int _value) : value{ _value } {
   }
   operator int() const { return value; }
   explicit operator int64_t() const { return value; }
   explicit operator bool() const { return value != 0; }
   explicit operator double() const { return (double)value; }
   explicit operator short() const { return (short)value; }
   explicit operator unsigned char() const { return (unsigned char)value; }
   explicit operator unsigned int() const { return (unsigned int)value; }

   // Well defined operators

   // comparison
   friend bool operator<(fixed_t lhs, fixed_t rhs) {
      return lhs.value < rhs.value;
   }
   friend bool operator>(fixed_t lhs, fixed_t rhs) {
      return lhs.value > rhs.value;
   }
   friend bool operator==(fixed_t lhs, fixed_t rhs) {
      return lhs.value == rhs.value;
   }
   friend bool operator<=(fixed_t lhs, fixed_t rhs) {
      return lhs.value <= rhs.value;
   }
   friend bool operator>=(fixed_t lhs, fixed_t rhs) {
      return lhs.value >= rhs.value;
   }

   // unary
   friend fixed_t operator-(fixed_t lhs) {
      return fixed_t(-lhs.value);
   }

   // binary, fixed_t, fixed_t => fixed_t
   friend fixed_t operator+(fixed_t lhs, fixed_t rhs) {
      return fixed_t(lhs.value + rhs.value);
   }
   friend fixed_t operator-(fixed_t lhs, fixed_t rhs) {
      return fixed_t(lhs.value - rhs.value);
   }
   friend fixed_t operator^(fixed_t lhs, fixed_t rhs) {
      return fixed_t(lhs.value ^ rhs.value);
   }
   friend fixed_t operator&(fixed_t lhs, fixed_t rhs) {
      return fixed_t(lhs.value & rhs.value);
   }

   // binary, fixed_t, fixed_t => int
   friend int operator*(const fixed_t lhs, const fixed_t rhs) {
      // This is a weird one, logically wrong as you need
      // to shift the result right by 16/32-bits.....
      return lhs.value * rhs.value;
   }

   // binary, fixed_t, int => fixed_t
   friend fixed_t operator<<(fixed_t lhs, int rhs) {
      return fixed_t(lhs.value << rhs);
   }
   friend fixed_t operator>>(fixed_t lhs, int rhs) {
      return fixed_t(lhs.value >> rhs);
   }
   friend fixed_t operator*(const fixed_t lhs, int rhs) {
      return fixed_t(lhs.value * rhs);
   }
   friend fixed_t operator/(const fixed_t lhs, int rhs) {
      return fixed_t(lhs.value / rhs);
   }

   // updating versions of above
   friend fixed_t operator+=(fixed_t &lhs, fixed_t rhs) {
      return fixed_t(lhs.value += rhs.value);
   }
   friend fixed_t operator-=(fixed_t &lhs, fixed_t rhs) {
      return fixed_t(lhs.value -= rhs.value);
   }
   friend fixed_t operator*=(fixed_t &lhs, int rhs) {
      return fixed_t(lhs.value *= rhs);
   }
   friend fixed_t operator>>=(fixed_t &lhs, int rhs) {
      return fixed_t(lhs.value >>= rhs);
   }
   friend fixed_t operator<<=(fixed_t &lhs, int rhs) {
      return fixed_t(lhs.value <<= rhs);
   }



   // dodgy operators allowing implicit type conversions, should be factored out.
   friend unsigned int operator+=(unsigned int &lhs, const fixed_t rhs) {
      // angle_t related
      return lhs += rhs.value;
   }
   friend short operator+=(short &lhs, const fixed_t rhs) { 
      // angle_t related
      return lhs += rhs.value;
   }
   friend unsigned int operator-(unsigned int lhs, const fixed_t rhs) {
      // angle_t related
      return lhs - rhs.value;
   }
   friend fixed_t operator-(fixed_t lhs, const int rhs) {
      return lhs.value - rhs;
   }
   friend int operator-(int lhs, const fixed_t rhs) {
      return lhs - rhs.value;
   }
   friend short operator-=(short &lhs, const fixed_t rhs) {
      // angle_t related
      return lhs -= rhs.value;
   }
   friend int operator/(const fixed_t lhs, const fixed_t rhs) {
      return lhs.value / rhs.value;
   }
   friend double operator/(const fixed_t lhs, const double rhs) {
      return lhs.value / rhs;
   }
   friend int operator/(const int lhs, const fixed_t rhs) {
      return lhs / rhs.value;
   }
   friend int operator*(const int lhs, const fixed_t rhs) {
      return lhs * rhs.value;
   }
   friend double operator*(const double lhs, const fixed_t rhs) {
      return lhs * rhs.value;
   }
   friend double operator/(const double lhs, const fixed_t rhs) {
      return lhs / rhs.value;
   }
   friend double operator+(const double lhs, const fixed_t rhs) {
      return lhs + rhs.value;
   }
   friend fixed_t operator-(const fixed_t lhs, const unsigned rhs) {
      return fixed_t(lhs.value - rhs);
   }
   friend fixed_t operator+(const fixed_t lhs, const int rhs) {
      return fixed_t(lhs.value + rhs);
   }
   friend unsigned int operator+(const unsigned int lhs, const fixed_t rhs) {
      return lhs + rhs.value;
   }
   friend fixed_t operator+(const int lhs, const fixed_t rhs) {
      return fixed_t(lhs + rhs.value);
   }
   friend bool operator<(const int lhs, const fixed_t rhs) {
      return lhs < rhs.value;
   }
   friend bool operator<(const fixed_t lhs, const int rhs) {
      return lhs.value < rhs;
   }
   friend bool operator>(const int lhs, const fixed_t rhs) {
      return lhs > rhs.value;
   }
   friend bool operator>(const fixed_t lhs, const int rhs) {
      return lhs.value > rhs;
   }
   friend bool operator==(const fixed_t lhs, const int rhs) {
      return lhs.value == rhs;
   }
   friend bool operator<=(const fixed_t lhs, const int rhs) {
      return lhs.value <= rhs;
   }
   friend bool operator>=(const fixed_t lhs, const int rhs) {
      return lhs.value >= rhs;
   }

   // 64-bit weirdities, need to better understand
   friend int64_t operator+=(int64_t &lhs, const fixed_t rhs) {
      return lhs += rhs.value;
   }
   friend int64_t operator+(fixed_t lhs, const int64_t rhs) {
      return (int64_t)lhs.value + rhs;
   }
   friend int64_t operator/(const int64_t lhs, const fixed_t rhs) {
      return lhs / rhs.value;
   }
   friend int64_t operator%(const int64_t lhs, const fixed_t rhs) {
      return lhs % rhs.value;
   }
   friend int64_t operator*(const int64_t lhs, const fixed_t rhs) {
      return lhs * rhs.value;
   }
   friend int64_t operator+(const int64_t lhs, const fixed_t rhs) {
      return lhs + rhs.value;
   }
   friend int64_t operator-(const fixed_t lhs, const int64_t rhs) {
      return lhs.value - rhs;
   }
   friend int64_t operator-(const int64_t lhs, const fixed_t rhs) {
      return lhs - rhs.value;
   }
   friend bool operator<(const fixed_t lhs, int64_t rhs) {
      return lhs.value < rhs;
   }
   friend bool operator<(const int64_t lhs, const fixed_t rhs) {
      return lhs < rhs.value;
   }
   friend bool operator>(const int64_t lhs, const fixed_t rhs) {
      return lhs > rhs.value;
   }
   friend bool operator!=(const int64_t lhs, const fixed_t rhs) {
      return lhs != rhs.value;
   }
   friend bool operator>=(const int64_t lhs, const fixed_t rhs) {
      return lhs >= rhs.value;
   }

   fixed_t abs() {
      return value < 0 ? fixed_t(-value) : fixed_t(value);
   }
   friend struct fmt::formatter<fixed_t>;
   friend class fracbits_t;
   };

template <>
struct fmt::formatter<fixed_t> {
   constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

   template <typename FormatContext>
   auto format(const fixed_t& t, FormatContext& ctx) {
      return format_to(ctx.out(), "{}", t.value );
   }
};

inline fixed_t abs(fixed_t a) {
   return a.abs();
}

fixed_t FixedMul	(fixed_t a, fixed_t b);
fixed_t FixedDiv	(fixed_t a, fixed_t b);

class fracbits_t {
   friend fixed_t operator<<( int a, fracbits_t b) {
      return fixed_t( a << 16 );
   }
   friend int operator>>( fixed_t a, fracbits_t b) {
      return a.value >> 16;
   }
   // This is definitely dodgy
   friend int64_t operator<<( int64_t a, fracbits_t b) {
      return a << 16;
   }
   friend int64_t operator>>( int64_t a, fracbits_t b) {
      return a >> 16;
   }
   // these should be factored out
   friend int operator+( int a, fracbits_t b) {
      return a + 16;
   }
   friend int operator+( fracbits_t b, int a) {
      return a + 16;
   }
   friend int operator*( fracbits_t b, int a) {
      return a * 16;
   }
   friend int operator-( int a, fracbits_t b) {
      return a - 16;
   }
   friend int operator-( fracbits_t b, int a) {
      return 16 - a;
   }
};

const fracbits_t FRACBITS;
const fixed_t FRACUNIT{1<<FRACBITS};
inline double FIXED2DOUBLE( fixed_t x ) {
   return x / static_cast<double>(FRACUNIT);
}

#endif
