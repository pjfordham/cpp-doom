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
   explicit fixed_t(int _value) : value{ _value } {   }

   explicit operator int() const { return value; }
   explicit operator int64_t() const { return value; }
   explicit operator bool() const { return value != 0; }
   explicit operator double() const { return (double)value; }
   explicit operator short() const { return (short)value; }
   explicit operator unsigned char() const { return (unsigned char)value; }

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
   friend int operator*(fixed_t lhs, fixed_t rhs) {
      // This is a weird one, logically wrong as you need
      // to shift the result right by 16/32-bits.....
      return lhs.value * rhs.value;
   }
   friend int operator/(fixed_t lhs, fixed_t rhs) {
      // Also weird but mostly OK, the ration of a fixed
      // to a fixed is logically an int.
      return lhs.value / rhs.value;
   }

   // binary, fixed_t, int => fixed_t
   friend fixed_t operator<<(fixed_t lhs, int rhs) {
      return fixed_t(lhs.value << rhs);
   }
   friend fixed_t operator>>(fixed_t lhs, int rhs) {
      return fixed_t(lhs.value >> rhs);
   }
   friend fixed_t operator*(fixed_t lhs, int rhs) {
      return fixed_t(lhs.value * rhs);
   }
   friend fixed_t operator/(fixed_t lhs, int rhs) {
      return fixed_t(lhs.value / rhs);
   }

   // binary, int, fixed_t => fixed_t
   friend fixed_t operator*(int lhs, fixed_t rhs) {
      return fixed_t(lhs * rhs.value);
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


   // Expliciity delete a bunch of int64_t operators to avoid narrowing
   // issues. Maybe do this properly with a template.
   friend int64_t operator+=(int64_t &lhs, fixed_t rhs) = delete;
   friend int64_t operator+(int64_t lhs, fixed_t rhs) = delete;
   friend int64_t operator*(int64_t lhs, fixed_t rhs)  = delete;
   friend int64_t operator-(int64_t lhs, fixed_t rhs) = delete;
   friend int64_t operator-(fixed_t lhs, int64_t rhs) = delete ;
   friend bool operator<(const fixed_t lhs, int64_t rhs) = delete;
   friend bool operator<(const int64_t lhs, fixed_t rhs) = delete;
   friend bool operator>(const int64_t lhs, fixed_t rhs) = delete;
   friend int64_t operator%(int64_t lhs, fixed_t rhs) = delete;

   fixed_t abs() {
      return value < 0 ? fixed_t(-value) : fixed_t(value);
   }
   fixed_t fractional_part() { // Come up with something better for this.
      return fixed_t( value & 0xFFFF);
   }
   fixed_t inverse() {
      return fixed_t( 0xffffffffu / (unsigned)value );
   }

   friend struct fmt::formatter<fixed_t>;
   friend class fracbits_t;

};

class shint64_t {
   int64_t value;
public:
   explicit operator int64_t() const { return value; }
   explicit shint64_t( int64_t _value ) : value(_value) {}
   friend shint64_t operator-(shint64_t lhs, shint64_t rhs) {
      return shint64_t(lhs.value - rhs.value);
   }
   friend shint64_t operator+(shint64_t lhs, shint64_t rhs) {
      return shint64_t(lhs.value + rhs.value);
   }
   friend bool operator<(shint64_t lhs, shint64_t rhs) {
      return lhs.value < rhs.value;
   }
   friend bool operator>(shint64_t lhs, shint64_t rhs) {
      return lhs.value > rhs.value;
   }
    friend shint64_t operator/(shint64_t lhs, int rhs) {
      return shint64_t(lhs.value / rhs);
   }
   friend fixed_t operator/(shint64_t lhs, fixed_t rhs) {
      return fixed_t(lhs.value / (int)rhs);
   }
   friend shint64_t operator*(shint64_t lhs, int rhs) {
      return shint64_t(lhs.value * rhs);
   }
   friend shint64_t operator<<(shint64_t lhs, int rhs) {
      return shint64_t(lhs.value << rhs);
   }
   friend shint64_t operator>>(shint64_t lhs, int rhs) {
      return shint64_t(lhs.value >> rhs);
   }
 
  
  friend class fracbits_t;
};

class fixed64_t {
   int64_t value;
public:
   fixed64_t() = default;
   explicit fixed64_t(int64_t _value) : value{ _value } {   }
   fixed64_t(fixed_t _value) : value{ (int) _value } {   }

   explicit operator bool() const { return value != 0; }
   explicit operator fixed_t() const { return fixed_t(value); }
   explicit operator long() const { return value; }
   explicit operator double() const { return (double)value; }

   // Well defined operators

   // comparison
   friend bool operator<(fixed64_t lhs, fixed64_t rhs) {
      return lhs.value < rhs.value;
   }
   friend bool operator>(fixed64_t lhs, fixed64_t rhs) {
      return lhs.value > rhs.value;
   }
   friend bool operator==(fixed64_t lhs, fixed64_t rhs) {
      return lhs.value == rhs.value;
   }
   friend bool operator<=(fixed64_t lhs, fixed64_t rhs) {
      return lhs.value <= rhs.value;
   }
   friend bool operator>=(fixed64_t lhs, fixed64_t rhs) {
      return lhs.value >= rhs.value;
   }

   // unary
   friend fixed64_t operator-(fixed64_t lhs) {
      return fixed64_t(-lhs.value);
   }

   // binary, fixed64_t, fixed64_t => fixed64_t
   friend fixed64_t operator+(fixed64_t lhs, fixed64_t rhs) {
      return fixed64_t(lhs.value + rhs.value);
   }
   friend fixed64_t operator-(fixed64_t lhs, fixed64_t rhs) {
      return fixed64_t(lhs.value - rhs.value);
   }
   friend fixed64_t operator^(fixed64_t lhs, fixed64_t rhs) {
      return fixed64_t(lhs.value ^ rhs.value);
   }
   friend fixed64_t operator&(fixed64_t lhs, fixed64_t rhs) {
      return fixed64_t(lhs.value & rhs.value);
   }

   // binary, fixed64_t, fixed64_t => int
   friend shint64_t operator*(fixed64_t lhs, fixed64_t rhs) {
      // This is a weird one, logically wrong as you need
      // to shift the result right by 16/32-bits.....
      return shint64_t(lhs.value * rhs.value);
   }
   friend int64_t operator/(fixed64_t lhs, fixed64_t rhs) {
      // Also weird but mostly OK, the ration of a fixed
      // to a fixed is logically an int.
      return lhs.value / rhs.value;
   }

   // binary, fixed64_t, int64_t => fixed64_t
   friend fixed64_t operator<<(fixed64_t lhs, int64_t rhs) {
      return fixed64_t(lhs.value << rhs);
   }
   friend fixed64_t operator>>(fixed64_t lhs, int64_t rhs) {
      return fixed64_t(lhs.value >> rhs);
   }
   friend fixed64_t operator*(fixed64_t lhs, int64_t rhs) {
      return fixed64_t(lhs.value * rhs);
   }
   friend fixed64_t operator/(fixed64_t lhs, int64_t rhs) {
      return fixed64_t(lhs.value / rhs);
   }
   // REVIEW THIS
   friend fixed64_t operator%(fixed64_t lhs, fixed64_t rhs) {
      return fixed64_t(lhs.value % rhs.value);
   }

   // binary, int, fixed64_t => fixed64_t
   friend fixed64_t operator*(int64_t lhs, fixed64_t rhs) {
      return fixed64_t(lhs * rhs.value);
   }

   // updating versions of above
   friend fixed64_t operator+=(fixed64_t &lhs, fixed64_t rhs) {
      return fixed64_t(lhs.value += rhs.value);
   }
   friend fixed64_t operator-=(fixed64_t &lhs, fixed64_t rhs) {
      return fixed64_t(lhs.value -= rhs.value);
   }
   friend fixed64_t operator*=(fixed64_t &lhs, int64_t rhs) {
      return fixed64_t(lhs.value *= rhs);
   }
   friend fixed64_t operator>>=(fixed64_t &lhs, int64_t rhs) {
      return fixed64_t(lhs.value >>= rhs);
   }
   friend fixed64_t operator<<=(fixed64_t &lhs, int64_t rhs) {
      return fixed64_t(lhs.value <<= rhs);
   }

   friend struct fmt::formatter<fixed64_t>;
   friend class fracbits_t;
   friend fixed64_t FixedMul( fixed64_t	a,  fixed64_t	b );
};

inline fixed_t operator"" _fix ( unsigned long long n ) {
   return fixed_t{(int)n};
}

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


inline int FixedMul	(int a, fixed_t b) {
   return (int)FixedMul(fixed_t(a),b);
}

inline int FixedMul	(fixed_t a, int b) {
   return (int)FixedMul(a,fixed_t(b));
}


class fracbits_t {
public:
   static int size() { return 16; }
   friend fixed_t operator<<( int a, fracbits_t b) {
      return fixed_t( a << b.size());
   }
   friend int operator>>( fixed_t a, fracbits_t b) {
      return a.value >> b.size();
   }
   friend fixed64_t operator<<( int64_t a, fracbits_t b) {
      return fixed64_t( a << b.size());
   }
   friend int64_t operator>>( fixed64_t a, fracbits_t b) {
      return a.value >> b.size();
   }
   friend shint64_t operator<<( fixed64_t a, fracbits_t b) {
      return shint64_t( a.value << b.size());
   }
   friend fixed64_t operator>>( shint64_t a, fracbits_t b) {
      return fixed64_t(a.value >> b.size());
   }
};

const fracbits_t FRACBITS;
const fixed_t FRACUNIT{1<<FRACBITS};
inline double FIXED2DOUBLE( fixed_t x ) {
   return (double)x / static_cast<double>(FRACUNIT);
}

inline fixed64_t
FixedMul
( fixed64_t	a,
  fixed64_t	b )
{
   return  ( a * b ) >> FRACBITS;
}

#endif
