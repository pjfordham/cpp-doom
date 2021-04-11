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

#include "fixed.hpp"
#include <fmt/core.h>

class fixed_t : public ffixed_t<int,16> {
public:
   fixed_t() = default;

   constexpr explicit fixed_t(int _value) : ffixed_t<int,16>( _value ) {   }
   constexpr fixed_t(ffixed_t<int,16> _value) : ffixed_t<int,16>( _value ) {   }
   constexpr explicit fixed_t(ffixed_t<int64_t,16> _value) : ffixed_t<int,16>( _value.value ) {   }

   explicit operator int64_t() const { return value; }
   explicit operator double() const { return static_cast<double>(value); }
   explicit operator short() const { return static_cast<short>(value); }
   explicit operator unsigned char() const { return static_cast<unsigned char>(value); }

   ffixed_t<int64_t,16> to_64() const {
      return ffixed_t<int64_t,16>(value);
   }

   fixed_t fractional_part() { // Come up with something better for this.
      return fixed_t( value & 0xFFFF);
   }
   fixed_t inverse() {
      return fixed_t( 0xffffffffu / static_cast<unsigned>(value) );
   }

   friend struct fmt::formatter<fixed_t>;
};

class fixed64_t : public ffixed_t<int64_t,16> {
public:
   fixed64_t() = default;

   explicit fixed64_t(int64_t _value) : ffixed_t<int64_t,16>{ _value } {   }
   fixed64_t(fixed_t _value) : ffixed_t<int64_t,16>{ static_cast<int>(_value) } {   }
   fixed64_t(ffixed_t<int64_t,16> _value) : ffixed_t<int64_t,16>( _value ) {   }

   fixed64_t& operator=(fixed_t other) {
      value = other.value;
      return *this;
   }

   explicit operator fixed_t() const { return fixed_t(value); }
   explicit operator double() const { return static_cast<double>(value); }

   friend struct fmt::formatter<fixed64_t>;
};

constexpr inline fixed_t operator"" _fix ( unsigned long long n ) {
   return fixed_t{static_cast<int>(n)};
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
   return static_cast<int>(FixedMul(fixed_t(a),b));
}

inline int FixedMul	(fixed_t a, int b) {
   return static_cast<int>(FixedMul(a,fixed_t(b)));
}


template <int SIZE>
class fracbits_t {
public:
   static const int size = SIZE;
   constexpr friend fixed_t operator<<( int a, fracbits_t b) {
      return fixed_t( a << size);
   }
   constexpr friend int operator>>( fixed_t a, fracbits_t b) {
      return a.value >> size;
   }
   constexpr friend fixed64_t operator<<( int64_t a, fracbits_t b) {
      return fixed64_t( a << size);
   }
   constexpr friend int64_t operator>>( fixed64_t a, fracbits_t b) {
      return a.value >> size;
   }

   template <typename Integer, int Precision,
             typename = std::enable_if_t<std::is_integral_v<Integer>, bool>>
   constexpr friend auto operator<<( ffixed_t<Integer,Precision> a, fracbits_t b) {
      return ffixed_t<Integer,Precision + size>( a.value << size);
   }

   template <typename Integer, int Precision,
             typename = std::enable_if_t<std::is_integral_v<Integer>, bool>>
   constexpr friend auto operator>>( ffixed_t<Integer,Precision> a, fracbits_t b) {
      return ffixed_t<Integer,Precision - size>( a.value >> size);
   }
};

constexpr fracbits_t<16> FRACBITS;
constexpr fixed_t FRACUNIT{1<<FRACBITS};
inline double FIXED2DOUBLE( fixed_t x ) {
   return static_cast<double>(x) / static_cast<double>(FRACUNIT);
}

inline fixed64_t
FixedMul
( fixed64_t	a,
  fixed64_t	b )
{
   return  ( a * b ) >> FRACBITS;
}

#endif
