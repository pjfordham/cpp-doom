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
//   Functions and definitions relating to the game type and operational
//   mode.
//

#ifndef __D_MODE__
#define __D_MODE__

#include "doomtype.hpp"

// The "mission" controls what game we are playing.

typedef enum
{
    doom,            // Doom 1
    doom2,           // Doom 2
    pack_tnt,        // Final Doom: TNT: Evilution
    pack_plut,       // Final Doom: The Plutonia Experiment
    pack_chex,       // Chex Quest (modded doom)
    pack_hacx,       // Hacx (modded doom2)
    heretic,         // Heretic
    hexen,           // Hexen
    strife,          // Strife
    pack_nerve,      // Doom 2: No Rest For The Living
    pack_master,     // Master Levels for Doom 2

    none
} GameMission_t;

// The "mode" allows more accurate specification of the game mode we are
// in: eg. shareware vs. registered.  So doom1.wad and doom.wad are the
// same mission, but a different mode.

typedef enum
{
    shareware,       // Doom/Heretic shareware
    registered,      // Doom/Heretic registered
    commercial,      // Doom II/Hexen
    retail,          // Ultimate Doom
    indetermined     // Unknown.
} GameMode_t;

// What version are we emulating?

typedef enum
{
    exe_doom_1_2,    // Doom 1.2: shareware and registered
    exe_doom_1_666,  // Doom 1.666: for shareware, registered and commercial
    exe_doom_1_7,    // Doom 1.7/1.7a: "
    exe_doom_1_8,    // Doom 1.8: "
    exe_doom_1_9,    // Doom 1.9: "
    exe_hacx,        // Hacx
    exe_ultimate,    // Ultimate Doom (retail)
    exe_final,       // Final Doom
    exe_final2,      // Final Doom (alternate exe)
    exe_chex,        // Chex Quest executable (based on Final Doom)

    exe_heretic_1_3, // Heretic 1.3

    exe_hexen_1_1,   // Hexen 1.1
    exe_strife_1_2,  // Strife v1.2
    exe_strife_1_31  // Strife v1.31
} GameVersion_t;

// What IWAD variant are we using?

typedef enum
{
    vanilla,    // Vanilla Doom
    freedoom,   // FreeDoom: Phase 1 + 2
    freedm,     // FreeDM
    bfgedition, // Doom Classic (Doom 3: BFG Edition)
} GameVariant_t;

// Skill level.

class skill_t {
   char skill;
public:
   skill_t() : skill(0) {}
   explicit skill_t(char _skill) : skill(_skill) {}
   char get_value() const { return skill; }
   static skill_t noitems() { return skill_t(-1); }
   static skill_t baby() { return skill_t(0); }
   static skill_t easy() { return skill_t(1); }
   static skill_t medium() { return skill_t(2); }
   static skill_t hard() { return skill_t(3); }
   static skill_t nightmare() { return skill_t(4); }

   std::string to_string() const {
      switch (skill) {
      case -1: return "Nothing";  // the "-skill 0" hack
      case 0: return "Baby";
      case 1: return "Easy";
      case 2: return "Normal";
      case 3: return"Hard";
      case 4: return "Nightmare";
      }
      return "Invalid skill level";
   }
   friend bool operator> (const skill_t &c1, const skill_t &c2);
   friend bool operator< (const skill_t &c1, const skill_t &c2);
   friend bool operator== (const skill_t &c1, const skill_t &c2);
   friend bool operator<= (const skill_t &c1, const skill_t &c2);
};

template <>
struct fmt::formatter<skill_t> {
  constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const skill_t& t, FormatContext& ctx) {
     std::string out = t.to_string();;
     return format_to(ctx.out(), "{}", out);
  }
};

inline std::ostream& operator<<(std::ostream& os, const skill_t& dt)
{
   os << dt.to_string();
   return os;
}

inline bool operator> (const skill_t &c1, const skill_t &c2)
{
   return c1.skill > c2.skill;
}
inline bool operator< (const skill_t &c1, const skill_t &c2)
{
   return c1.skill < c2.skill;
}
inline bool operator<= (const skill_t &c1, const skill_t &c2)
{
   return c1.skill <= c2.skill;
}
inline bool operator== (const skill_t &c1, const skill_t &c2)
{
   return c1.skill == c2.skill;
}

boolean D_ValidGameMode(int mission, int mode);
boolean D_ValidGameVersion(GameMission_t mission, GameVersion_t version);
boolean D_ValidEpisodeMap(GameMission_t mission, GameMode_t mode,
                          int episode, int map);
int D_GetNumEpisodes(GameMission_t mission, GameMode_t mode);
boolean D_IsEpisodeMap(GameMission_t mission);
const char *D_GameMissionString(GameMission_t mission);
const char *D_GameModeString(GameMode_t mode);

#endif /* #ifndef __D_MODE__ */

