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

#define FRACBITS		16
#define FRACUNIT		(1<<FRACBITS)
#define FIXED2DOUBLE(x)		(x / static_cast<double>(FRACUNIT))

class fixed_t {
   int value;
public:
   fixed_t() = default;
   fixed_t(int _value) : value{ _value } {
   }
   operator int() const { return value; }
   operator int64_t() const { return value; }
   operator bool() const { return value != 0; }
   operator double() const { return (double)value; }
   operator short() const { return (short)value; }
   operator unsigned char() const { return (unsigned char)value; }
   operator unsigned int() const { return (unsigned int)value; }
   friend fixed_t operator-(fixed_t lhs) {
      return fixed_t(-lhs.value);
   }
   friend fixed_t operator+=(fixed_t &lhs, const fixed_t rhs) {
      return lhs.value += rhs.value;
   }
   friend unsigned int operator+=(unsigned int &lhs, const fixed_t rhs) {
      return lhs += rhs.value;
   }
   friend int64_t operator+=(int64_t &lhs, const fixed_t rhs) {
      return lhs += rhs.value;
   }
   friend short operator+=(short &lhs, const fixed_t rhs) {
      return lhs += rhs.value;
   }
   friend int operator+=(int &lhs, const fixed_t rhs) {
      return lhs += rhs.value;
   }
   friend fixed_t operator*=(fixed_t &lhs, const int rhs) {
      return lhs.value *= rhs;
   }
   friend fixed_t operator-=(fixed_t &lhs, const fixed_t rhs) {
      return lhs.value -= rhs.value;
   }
   friend unsigned int operator-(unsigned int lhs, const fixed_t rhs) {
      return lhs - rhs.value;
   }
   friend fixed_t operator^(fixed_t lhs, const fixed_t rhs) {
      return lhs.value ^ rhs.value;
   }
   friend fixed_t operator&(fixed_t lhs, const int rhs) {
      return lhs.value & rhs;
   }
   friend fixed_t operator&(fixed_t lhs, const unsigned int rhs) {
      return lhs.value & rhs;
   }
   friend fixed_t operator-(fixed_t lhs, const int rhs) {
      return lhs.value - rhs;
   }
   friend int operator-(int lhs, const fixed_t rhs) {
      return lhs - rhs.value;
   }
   friend int operator-=(int &lhs, const fixed_t rhs) {
      return lhs -= rhs.value;
   }
   friend short operator-=(short &lhs, const fixed_t rhs) {
      return lhs -= rhs.value;
   }
   friend fixed_t operator>>=(fixed_t &lhs, const int rhs) {
      return lhs.value >>= rhs;
   }
   friend fixed_t operator<<=(fixed_t &lhs, const int rhs) {
      return fixed_t(lhs.value <<= rhs);
   }
   friend fixed_t operator<<(fixed_t lhs, const int rhs) {
      return fixed_t(lhs.value << rhs);
   }
   friend int operator*(const fixed_t lhs, const int rhs) {
      return lhs.value * rhs;
   }
   friend int operator/(const fixed_t lhs, const int rhs) {
      return lhs.value / rhs;
   }
   friend double operator/(const fixed_t lhs, const double rhs) {
      return lhs.value / rhs;
   }
   friend int operator/(const int lhs, const fixed_t rhs) {
      return lhs / rhs.value;
   }
   friend int64_t operator/(const int64_t lhs, const fixed_t rhs) {
      return lhs / rhs.value;
   }
   friend int operator*(const fixed_t lhs, const fixed_t rhs) {
      return lhs.value * rhs.value;
   }
   friend int operator*(const int lhs, const fixed_t rhs) {
      return lhs * rhs.value;
   }
   friend int64_t operator*(const int64_t lhs, const fixed_t rhs) {
      return lhs * rhs.value;
   }
   friend fixed_t operator+(const fixed_t lhs, const fixed_t rhs) {
      return fixed_t(lhs.value + rhs.value);
   }
   friend double operator+(const double lhs, const fixed_t rhs) {
      return lhs + rhs.value;
   }
   friend int64_t operator+(const int64_t lhs, const fixed_t rhs) {
      return lhs + rhs.value;
   }
   friend int64_t operator-(const fixed_t lhs, const int64_t rhs) {
      return lhs.value - rhs;
   }
   friend fixed_t operator-(const fixed_t lhs, const unsigned rhs) {
      return fixed_t(lhs.value - rhs);
   }
   friend int64_t operator-(const fixed_t lhs, const fixed_t rhs) {
      return lhs.value - rhs.value;
   }
  friend int64_t operator-(const int64_t lhs, const fixed_t rhs) {
      return lhs - rhs.value;
   }
   friend fixed_t operator+(const fixed_t lhs, const int rhs) {
      return fixed_t(lhs.value + rhs);
   }
  friend unsigned int operator+(const unsigned int lhs, const fixed_t rhs) {
      return lhs + rhs.value;
   }
   friend fixed_t operator>>(const fixed_t lhs, const int rhs) {
      return fixed_t(lhs.value >> rhs);
   }
   friend fixed_t operator+(const int lhs, const fixed_t rhs) {
      return fixed_t(lhs + rhs.value);
   }
   friend bool operator<(const fixed_t lhs, const fixed_t rhs) {
      return lhs.value < rhs.value;
   }
   friend bool operator<(const fixed_t lhs, int64_t rhs) {
      return lhs.value < rhs;
   }
   friend bool operator<(const int lhs, const fixed_t rhs) {
      return lhs < rhs.value;
   }
    friend bool operator<(const fixed_t lhs, const int rhs) {
      return lhs.value < rhs;
   }
   friend bool operator<(const int64_t lhs, const fixed_t rhs) {
      return lhs < rhs.value;
   }
   friend bool operator>(const fixed_t lhs, const fixed_t rhs) {
      return lhs.value > rhs.value;
   }
   friend bool operator>(const int lhs, const fixed_t rhs) {
      return lhs > rhs.value;
   }
    friend bool operator>(const fixed_t lhs, const int rhs) {
      return lhs.value > rhs;
   }
   friend bool operator>(const int64_t lhs, const fixed_t rhs) {
      return lhs > rhs.value;
   }
   friend bool operator==(const fixed_t lhs, const fixed_t rhs) {
      return lhs.value == rhs.value;
   }
   friend bool operator==(const fixed_t lhs, const int rhs) {
      return lhs.value == rhs;
   }
   friend bool operator!=(const fixed_t lhs, const int rhs) {
      return lhs.value != rhs;
   }
   friend bool operator!=(const int64_t lhs, const fixed_t rhs) {
      return lhs != rhs.value;
   }
  friend bool operator<=(const fixed_t lhs, const fixed_t rhs) {
      return lhs.value <= rhs.value;
   }
  friend bool operator<=(const fixed_t lhs, const int rhs) {
      return lhs.value <= rhs;
   }
   friend bool operator>=(const fixed_t lhs, const fixed_t rhs) {
      return lhs.value >= rhs.value;
   }
   friend bool operator>=(const fixed_t lhs, const int rhs) {
      return lhs.value >= rhs;
   }
   friend bool operator>=(const int64_t lhs, const fixed_t rhs) {
      return lhs >= rhs.value;
   }
   fixed_t abs() {
      return value < 0 ? fixed_t(-value) : fixed_t(value);
   }
};

template <>
struct fmt::formatter<fixed_t> {
   constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

   template <typename FormatContext>
   auto format(const fixed_t& t, FormatContext& ctx) {
      return format_to(ctx.out(), "{}", (int)t );
   }
};

inline fixed_t abs(fixed_t a) {
   return a.abs();
}

fixed_t FixedMul	(fixed_t a, fixed_t b);
fixed_t FixedDiv	(fixed_t a, fixed_t b);



#endif
