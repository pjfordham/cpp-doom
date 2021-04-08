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
//  MapObj data. Map Objects or mobjs are actors, entities,
//  thinker, take-your-pick... anything that moves, acts, or
//  suffers state changes of more or less violent nature.
//


#ifndef __D_THINK__
#define __D_THINK__

#include <tuple>
#include <cassert>


//
// Experimental stuff.
// To compile this as "ANSI C with classes"
//  we will need to handle the various
//  action functions cleanly.
//

struct mobj_t;
struct player_t;
struct pspdef_t;

struct actionf_t {
  typedef void (*m1)(mobj_t *mo);
  typedef void (*p3)(mobj_t *mo, player_t *player, pspdef_t *psp);

  constexpr actionf_t() = default;

  explicit constexpr actionf_t(const p3 p)
  {
    std::get<p3>(data) = p;
  }

  explicit constexpr actionf_t(const m1 p)
  {
    std::get<m1>(data) = p;
  }

  constexpr actionf_t &operator=(p3 p) {
    data = actionf_t{p}.data;
    return *this;
  }

  constexpr actionf_t &operator=(m1 p) {
    data = actionf_t{p}.data;
    return *this;
  }

  [[nodiscard]] constexpr bool operator==(p3 p) const {
    return std::get<decltype(p)>(data) == p;
  }

  [[nodiscard]] constexpr bool operator==(m1 p) const {
    return std::get<decltype(p)>(data) == p;
  }

  constexpr bool call_if( mobj_t *mo, player_t *player, pspdef_t *psp ){
    const auto func = std::get<p3>(data);
    if (func) {
       func(mo,player,psp);
      return true;
    } else {
       assert(*this == actionf_t{});
       return false;
    }
  }

   constexpr bool call_if(mobj_t *mo) {
    const auto func = std::get<m1>(data);
    if (func) {
      func(mo);
      return true;
    } else {
       assert(*this == actionf_t{});
       return false;
    }
  }

   constexpr explicit operator bool() const {
    return *this != actionf_t{};
  }

  constexpr bool operator==(const actionf_t &) const = default;

private:
   std::tuple<m1, p3> data{};
};

#endif
