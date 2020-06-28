//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2013-2017 Brad Harding
// Copyright(C) 2017 Fabian Greffrath
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	Brightmaps for wall textures
//	Adapted from doomretro/src/r_data.c:97-209
//

#include "doomtype.hpp"
#include "doomstat.hpp"
#include "r_data.hpp"
#include "w_wad.hpp"

// [crispy] brightmap data

static byte nobrightmap[256] = {0};

static byte notgray[256] =
{
	0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

static byte notgrayorbrown[256] =
{
	0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

static byte redonly[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static byte greenonly1[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static byte greenonly2[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static byte greenonly3[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static byte yellowonly[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
};

static byte redandgreen[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static byte blueandgreen[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static byte brighttan[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0,
	1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1,
	0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

// [crispy] Chex Quest's "locked" door switches

static byte chexred[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

// [crispy] Chex Quest's "open" door switches

static byte chexgreen[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

// [crispy] Chex Quest's "lock"/"open" knobs

static byte chexredgreen[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static byte hacxlightning[256] =
{
	0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

byte *dc_brightmap = nobrightmap;

// [crispy] brightmaps for textures

enum
{
	DOOM1AND2,
	DOOM1ONLY,
	DOOM2ONLY,
};

typedef struct
{
	const lump_name_t texture;
	const int game;
	byte *colormask;
} fullbright_t;

static const fullbright_t fullbright_doom[] = {
	// [crispy] common textures
        {lump_name_t("COMP2"),    DOOM1AND2, blueandgreen},
	{lump_name_t("COMPSTA1"), DOOM1AND2, notgray},
	{lump_name_t("COMPSTA2"), DOOM1AND2, notgray},
	{lump_name_t("COMPUTE1"), DOOM1AND2, notgrayorbrown},
	{lump_name_t("COMPUTE2"), DOOM1AND2, notgrayorbrown},
	{lump_name_t("COMPUTE3"), DOOM1AND2, notgrayorbrown},
	{lump_name_t("EXITSIGN"), DOOM1AND2, notgray},
	{lump_name_t("EXITSTON"), DOOM1AND2, redonly},
	{lump_name_t("PLANET1"),  DOOM1AND2, notgray},
        {lump_name_t("SILVER2"),  DOOM1AND2, notgray},
        {lump_name_t("SILVER3"),  DOOM1AND2, notgrayorbrown},
        {lump_name_t("SLADSKUL"), DOOM1AND2, redonly},
        {lump_name_t("SW1BRCOM"), DOOM1AND2, redonly},
        {lump_name_t("SW1BRIK"),  DOOM1AND2, redonly},
        {lump_name_t("SW1BRN1"),  DOOM2ONLY, redonly},
        {lump_name_t("SW1COMM"),  DOOM1AND2, redonly},
        {lump_name_t("SW1DIRT"),  DOOM1AND2, redonly},
        {lump_name_t("SW1MET2"),  DOOM1AND2, redonly},
        {lump_name_t("SW1STARG"), DOOM2ONLY, redonly},
        {lump_name_t("SW1STON1"), DOOM1AND2, redonly},
        {lump_name_t("SW1STON2"), DOOM2ONLY, redonly},
        {lump_name_t("SW1STONE"), DOOM1AND2, redonly},
        {lump_name_t("SW1STRTN"), DOOM1AND2, redonly},
        {lump_name_t("SW2BLUE"),  DOOM1AND2, redonly},
        {lump_name_t("SW2BRCOM"), DOOM1AND2, greenonly2},
        {lump_name_t("SW2BRIK"),  DOOM1AND2, greenonly1},
        {lump_name_t("SW2BRN1"),  DOOM1AND2, greenonly2},
        {lump_name_t("SW2BRN2"),  DOOM1AND2, greenonly1},
        {lump_name_t("SW2BRNGN"), DOOM1AND2, greenonly3},
        {lump_name_t("SW2COMM"),  DOOM1AND2, greenonly1},
        {lump_name_t("SW2COMP"),  DOOM1AND2, redonly},
        {lump_name_t("SW2DIRT"),  DOOM1AND2, greenonly2},
        {lump_name_t("SW2EXIT"),  DOOM1AND2, notgray},
        {lump_name_t("SW2GRAY"),  DOOM1AND2, notgray},
        {lump_name_t("SW2GRAY1"), DOOM1AND2, notgray},
        {lump_name_t("SW2GSTON"), DOOM1AND2, redonly},
        {lump_name_t("SW2MARB"),  DOOM2ONLY, redonly},
        {lump_name_t("SW2MET2"),  DOOM1AND2, greenonly1},
        {lump_name_t("SW2METAL"), DOOM1AND2, greenonly3},
        {lump_name_t("SW2MOD1"),  DOOM1AND2, greenonly1},
        {lump_name_t("SW2PANEL"), DOOM1AND2, redonly},
        {lump_name_t("SW2ROCK"),  DOOM1AND2, redonly},
        {lump_name_t("SW2SLAD"),  DOOM1AND2, redonly},
        {lump_name_t("SW2STARG"), DOOM2ONLY, greenonly2},
        {lump_name_t("SW2STON1"), DOOM1AND2, greenonly3},
	// [crispy] beware!
        {lump_name_t("SW2STON2"), DOOM1ONLY, redonly},
        {lump_name_t("SW2STON2"), DOOM2ONLY, greenonly2},
        {lump_name_t("SW2STON6"), DOOM1AND2, redonly},
        {lump_name_t("SW2STONE"), DOOM1AND2, greenonly2},
        {lump_name_t("SW2STRTN"), DOOM1AND2, greenonly1},
        {lump_name_t("SW2TEK"),   DOOM1AND2, greenonly1},
        {lump_name_t("SW2VINE"),  DOOM1AND2, greenonly1},
        {lump_name_t("SW2WOOD"),  DOOM1AND2, redonly},
        {lump_name_t("SW2ZIM"),   DOOM1AND2, redonly},
        {lump_name_t("WOOD4"),    DOOM1AND2, redonly},
        {lump_name_t("WOODGARG"), DOOM1AND2, redonly},
        {lump_name_t("WOODSKUL"), DOOM1AND2, redonly},
//	{lump_name_t("ZELDOOR",  DOOM1AND2, redonly},
        {lump_name_t("LITEBLU1"), DOOM1AND2, notgray},
        {lump_name_t("LITEBLU2"), DOOM1AND2, notgray},
        {lump_name_t("SPCDOOR3"), DOOM2ONLY, greenonly1},
        {lump_name_t("PIPEWAL1"), DOOM2ONLY, greenonly1},
        {lump_name_t("TEKLITE2"), DOOM2ONLY, greenonly1},
        {lump_name_t("TEKBRON2"), DOOM2ONLY, yellowonly},
//	{lump_name_t("SW2SKULL", DOOM2ONLY, greenonly2},
        {lump_name_t("SW2SATYR"), DOOM1AND2, brighttan},
        {lump_name_t("SW2LION"),  DOOM1AND2, brighttan},
        {lump_name_t("SW2GARG"),  DOOM1AND2, brighttan},
	// [crispy] Final Doom textures
	// TNT - Evilution exclusive
        {lump_name_t("PNK4EXIT"), DOOM2ONLY, redonly},
        {lump_name_t("SLAD2"),    DOOM2ONLY, notgrayorbrown},
        {lump_name_t("SLAD3"),    DOOM2ONLY, notgrayorbrown},
        {lump_name_t("SLAD4"),    DOOM2ONLY, notgrayorbrown},
        {lump_name_t("SLAD5"),    DOOM2ONLY, notgrayorbrown},
        {lump_name_t("SLAD6"),    DOOM2ONLY, notgrayorbrown},
        {lump_name_t("SLAD7"),    DOOM2ONLY, notgrayorbrown},
        {lump_name_t("SLAD8"),    DOOM2ONLY, notgrayorbrown},
        {lump_name_t("SLAD9"),    DOOM2ONLY, notgrayorbrown},
        {lump_name_t("SLAD10"),   DOOM2ONLY, notgrayorbrown},
        {lump_name_t("SLAD11"),   DOOM2ONLY, notgrayorbrown},
        {lump_name_t("SLADRIP1"), DOOM2ONLY, notgrayorbrown},
        {lump_name_t("SLADRIP3"), DOOM2ONLY, notgrayorbrown},
        {lump_name_t("M_TEC"),    DOOM2ONLY, greenonly2},
        {lump_name_t("LITERED2"), DOOM2ONLY, redonly},
        {lump_name_t("BTNTMETL"), DOOM2ONLY, notgrayorbrown},
        {lump_name_t("BTNTSLVR"), DOOM2ONLY, notgrayorbrown},
        {lump_name_t("LITEYEL2"),DOOM2ONLY, yellowonly},
        {lump_name_t("LITEYEL3"), DOOM2ONLY, yellowonly},
        {lump_name_t("YELMETAL"), DOOM2ONLY, yellowonly},
	// Plutonia exclusive
//	{lump_name_t("SW2SKULL", DOOM2ONLY, redonly},
};

static const fullbright_t fullbright_chex[] = {
   {lump_name_t("BIGDOOR1"), DOOM1AND2, greenonly3},
//	{"BIGDOOR4", DOOM1AND2, greenonly3}, // C1: some stray green pixels, C2: many stray green pixels
//	{"BRNBIGL",  DOOM1AND2, greenonly3},
//	{"BRNBIGR",  DOOM1AND2, greenonly3}, // C1, C2: one stray green pixel
//	{"BRNSMAL2", DOOM1AND2, greenonly3}, // C1, C2: many stray green pixels
   {lump_name_t("COMP2"),    DOOM1AND2, notgray},
//	{"COMPTALL", DOOM1ONLY, notgray},
//	{"COMPTALL", DOOM2ONLY, greenonly3}, // C2: many stray green pixels
   {lump_name_t("COMPUTE2"), DOOM1AND2, notgray},
   {lump_name_t("LITE5"),    DOOM1ONLY, greenonly2},
   {lump_name_t("STARTAN3"), DOOM1AND2, greenonly2},
   {lump_name_t("SW1BRCOM"), DOOM1AND2, chexred},
   {lump_name_t("SW1BRN1"),  DOOM1AND2, chexgreen},
   {lump_name_t("SW1BRN2"),  DOOM1AND2, chexred},
   {lump_name_t("SW1BRNGN"), DOOM1AND2, chexred},
   {lump_name_t("SW1BROWN"), DOOM1AND2, chexred},
   {lump_name_t("SW1COMM"),  DOOM1AND2, chexred},
   {lump_name_t("SW1COMP"),  DOOM1AND2, chexred},
   {lump_name_t("SW1DIRT"),  DOOM1AND2, chexgreen},
   {lump_name_t("SW1METAL"), DOOM1AND2, chexredgreen},
   {lump_name_t("SW1PIPE"),  DOOM1AND2, chexgreen},
   {lump_name_t("SW1STARG"), DOOM1AND2, chexred},
   {lump_name_t("SW1STON1"), DOOM1AND2, chexred},
   {lump_name_t("SW1STRTN"), DOOM1AND2, chexred},
   {lump_name_t("SW2BRCOM"), DOOM1AND2, chexgreen},
   {lump_name_t("SW2BRN1"),  DOOM1AND2, chexred},
   {lump_name_t("SW2BRN2"),  DOOM1AND2, chexgreen},
   {lump_name_t("SW2BRNGN"), DOOM1AND2, chexgreen},
   {lump_name_t("SW2BROWN"), DOOM1AND2, chexgreen},
   {lump_name_t("SW2COMM"),  DOOM1AND2, chexgreen},
   {lump_name_t("SW2COMP"),  DOOM1AND2, chexgreen},
   {lump_name_t("SW2DIRT"),  DOOM1AND2, chexred},
   {lump_name_t("SW2METAL"), DOOM1AND2, chexredgreen},
   {lump_name_t("SW2PIPE"),  DOOM1AND2, chexred},
   {lump_name_t("SW2STARG"), DOOM1AND2, chexgreen},
   {lump_name_t("SW2STON1"), DOOM1AND2, chexgreen},
   {lump_name_t("SW2STONE"), DOOM1AND2, chexgreen},
   {lump_name_t("SW2STRTN"), DOOM1AND2, chexgreen},
//	{"BIGDOOR5", DOOM1AND2, greenonly1}, // C1, C2: some stray green pixels
//	{"BIGDOOR6", DOOM1AND2, greenonly1}, // C1, C2: some stray green pixels
   {lump_name_t("CEMENT3"),  DOOM1AND2, greenonly3},
   {lump_name_t("SKINFACE"), DOOM1AND2, greenonly1},
   {lump_name_t("SKINTEK1"), DOOM1ONLY, greenonly1},
   {lump_name_t("SKSPINE2"), DOOM1AND2, greenonly3},
   {lump_name_t("SW1BLUE"),  DOOM1AND2, chexgreen},
   {lump_name_t("SW1HOT"),   DOOM1AND2, chexgreen},
   {lump_name_t("SW1SKIN"),  DOOM1AND2, chexgreen},
   {lump_name_t("SW1VINE"),  DOOM1ONLY, chexgreen}, // C1: some stray green pixels in the vines
   {lump_name_t("SW1WOOD"),  DOOM1AND2, chexgreen},
   {lump_name_t("SW2BLUE"),  DOOM1AND2, chexred},
   {lump_name_t("SW2CMT"),   DOOM1AND2, chexgreen},
   {lump_name_t("SW2GSTON"), DOOM1AND2, chexred},
   {lump_name_t("SW2HOT"),   DOOM1AND2, chexred},
   {lump_name_t("SW2SKIN"),  DOOM1AND2, chexred},
   {lump_name_t("SW2VINE"),  DOOM1ONLY, chexred},
   {lump_name_t("SW2WOOD"),  DOOM1AND2, chexred},
   {lump_name_t("WOOD4"),    DOOM1AND2, chexredgreen},
   {lump_name_t("WOODGARG"), DOOM1AND2, chexred},
   {lump_name_t("WOODSKUL"), DOOM1AND2, chexredgreen},
};

static const fullbright_t fullbright_hacx[] = {
//	{"BFALL1",   DOOM2ONLY, redandgreen},
//	{"BFALL2",   DOOM2ONLY, redandgreen},
//	{"BFALL3",   DOOM2ONLY, redandgreen},
//	{"BFALL4",   DOOM2ONLY, redandgreen},
   {lump_name_t("BRNSMALR"), DOOM2ONLY, greenonly1},
         {lump_name_t("DOORRED"),  DOOM2ONLY, redandgreen},
         {lump_name_t("SLADWALL"), DOOM2ONLY, chexred},
//	{"SW1BRCOM", DOOM2ONLY, redonly},
//	{"SW1BRN1",  DOOM2ONLY, redandgreen},
         {lump_name_t("SW1BRN2"),  DOOM2ONLY, notgrayorbrown},
         {lump_name_t("SW1BRNGN"), DOOM2ONLY, notgrayorbrown},
//	{"SW1BROWN", DOOM2ONLY, notgrayorbrown},
//	{"SW2BRCOM", DOOM2ONLY, greenonly1},
//	{"SW2BRN1",  DOOM2ONLY, redandgreen},
         {lump_name_t("SW2BRN2"),  DOOM2ONLY, notgrayorbrown},
//	{"SW2BROWN", DOOM2ONLY, notgrayorbrown},
         {lump_name_t("COMPSPAN"), DOOM2ONLY, greenonly1},
         {lump_name_t("COMPSTA1"), DOOM2ONLY, notgrayorbrown},
//	{"COMPSTA2", DOOM2ONLY, notgrayorbrown},
         {lump_name_t("HD5"),      DOOM2ONLY, redandgreen},
//	{"HD8",      DOOM2ONLY, redandgreen},
//	{"HD9",      DOOM2ONLY, redandgreen},
         {lump_name_t("BLAKWAL2"), DOOM2ONLY, redandgreen},
         {lump_name_t("CEMENT7"),  DOOM2ONLY, greenonly1},
         {lump_name_t("ROCK4"),    DOOM2ONLY, redonly},
//	{"SLOPPY1",  DOOM2ONLY, notgrayorbrown},
//	{"SPCDOOR4", DOOM2ONLY, notgrayorbrown},
         {lump_name_t("ZZZFACE1"), DOOM2ONLY, greenonly1},
         {lump_name_t("ZZZFACE2"), DOOM2ONLY, redandgreen},
         {lump_name_t("HW166"),    DOOM2ONLY, redandgreen},
         {lump_name_t("HW510"),    DOOM2ONLY, notgrayorbrown},
         {lump_name_t("HW511"),    DOOM2ONLY, notgrayorbrown},
         {lump_name_t("HW512"),    DOOM2ONLY, notgrayorbrown},
};

static byte *R_BrightmapForTexName_Doom (const lump_name_t &texname)
{
	int i;

	for (i = 0; i < arrlen(fullbright_doom); i++)
	{
		const fullbright_t *fullbright = &fullbright_doom[i];

		if ((gamemission == doom && fullbright->game == DOOM2ONLY) ||
		    (gamemission != doom && fullbright->game == DOOM1ONLY))
		{
			continue;
		}

		if ( fullbright->texture == texname )
		{
			return fullbright->colormask;
		}
	}

	return nobrightmap;
}

static boolean chex2 = false;

static byte *R_BrightmapForTexName_Chex (const lump_name_t &texname)
{
	int i;

	for (i = 0; i < arrlen(fullbright_chex); i++)
	{
		const fullbright_t *fullbright = &fullbright_chex[i];

		if ((chex2 && fullbright->game == DOOM1ONLY) ||
		    (!chex2 && fullbright->game == DOOM2ONLY))
		{
			continue;
		}

		if (fullbright->texture == texname)
		{
			return fullbright->colormask;
		}
	}

	return nobrightmap;
}

static byte *R_BrightmapForTexName_Hacx (const lump_name_t &texname)
{
	int i;

	for (i = 0; i < arrlen(fullbright_hacx); i++)
	{
		const fullbright_t *fullbright = &fullbright_hacx[i];

		if (fullbright->texture == texname)
		{
			return fullbright->colormask;
		}
	}

	return nobrightmap;
}

// [crispy] brightmaps for sprites

// [crispy] adapted from russian-doom/src/doom/r_things.c:617-639
static byte *R_BrightmapForSprite_Doom (const int type)
{
	if (crispy->brightmaps & BRIGHTMAPS_SPRITES)
	{
		switch (type)
		{
			// Armor Bonus
			case SPR_BON2:
			// Cell Charge
			case SPR_CELL:
			{
				return greenonly1;
				break;
			}
			// Barrel
			case SPR_BAR1:
			{
				return greenonly3;
				break;
			}
			// Cell Charge Pack
			case SPR_CELP:
			{
				return yellowonly;
				break;
			}
			// BFG9000
			case SPR_BFUG:
			// Plasmagun
			case SPR_PLAS:
			{
				return redonly;
				break;
			}
		}
	}

	return nobrightmap;
}

static byte *R_BrightmapForSprite_Chex (const int type)
{
	// [crispy] TODO
	/*
	if (crispy->brightmaps & BRIGHTMAPS_SPRITES)
	{
		switch (type)
		{
			// Chainsaw
			case SPR_CSAW:
			// Shotgun
			case SPR_SHOT:
			// Chaingun
			case SPR_MGUN:
			// Rocket launcher
			case SPR_LAUN:
			// Plasmagun
			case SPR_PLAS:
			// BFG9000
			case SPR_BFUG:
			{
				return redandgreen;
				break;
			}
		}
	}
	*/
	return nobrightmap;
}

static byte *R_BrightmapForSprite_Hacx (const int type)
{
	if (crispy->brightmaps & BRIGHTMAPS_SPRITES)
	{
		switch (type)
		{
			// Chainsaw
			case SPR_CSAW:
			// Plasmagun
			case SPR_PLAS:
			// Cell Charge
			case SPR_CELL:
			// Cell Charge Pack
			case SPR_CELP:
			{
				return redonly;
				break;
			}
			// Rocket launcher
			case SPR_LAUN:
			// Medikit
			case SPR_MEDI:
			{
				return redandgreen;
				break;
			}
			// Rocket
			case SPR_ROCK:
			// Box of rockets
			case SPR_BROK:
			{
				return greenonly1;
				break;
			}
			// Health Bonus
			case SPR_BON1:
			// Stimpack
			case SPR_STIM:
			{
				return notgrayorbrown;
				break;
			}
		}
	}

	return nobrightmap;
}

// [crispy] brightmaps for flats

static int bmapflatnum[12];

static byte *R_BrightmapForFlatNum_Doom (const int num)
{
	if (crispy->brightmaps & BRIGHTMAPS_TEXTURES)
	{
		if (num == bmapflatnum[0] ||
		    num == bmapflatnum[1] ||
		    num == bmapflatnum[2])
		{
			return notgrayorbrown;
		}
	}

	return nobrightmap;
}

static byte *R_BrightmapForFlatNum_Hacx (const int num)
{
	if (crispy->brightmaps & BRIGHTMAPS_TEXTURES)
	{
		if (num == bmapflatnum[0] ||
		    num == bmapflatnum[1] ||
		    num == bmapflatnum[2] ||
		    num == bmapflatnum[3] ||
		    num == bmapflatnum[4] ||
		    num == bmapflatnum[5] ||
		    num == bmapflatnum[9] ||
		    num == bmapflatnum[10] ||
		    num == bmapflatnum[11])
		{
			return notgrayorbrown;
		}

		if (num == bmapflatnum[6] ||
		    num == bmapflatnum[7] ||
		    num == bmapflatnum[8])
		{
			return greenonly1;
		}
	}

	return nobrightmap;
}

static byte *R_BrightmapForFlatNum_None (const int num)
{
	return nobrightmap;
}

// [crispy] brightmaps for states

static byte *R_BrightmapForState_Doom (const int state)
{
	if (crispy->brightmaps & BRIGHTMAPS_SPRITES)
	{
		switch (state)
		{
			case S_BFG1:
			case S_BFG2:
			case S_BFG3:
			case S_BFG4:
			{
				return redonly;
				break;
			}
		}
	}

	return nobrightmap;
}

static byte *R_BrightmapForState_Hacx (const int state)
{
	if (crispy->brightmaps & BRIGHTMAPS_SPRITES)
	{
		switch (state)
		{
			case S_SAW2:
			case S_SAW3:
			{
				return hacxlightning;
				break;
			}
			case S_MISSILE:
			{
				return redandgreen;
				break;
			}
			case S_SAW:
			case S_SAWB:
			case S_PLASMA:
			case S_PLASMA2:
			{
				return redonly;
				break;
			}
		}
	}

	return nobrightmap;
}

static byte *R_BrightmapForState_None (const int state)
{
	return nobrightmap;
}

// [crispy] initialize brightmaps

byte *(*R_BrightmapForTexName) (const lump_name_t &texname);
byte *(*R_BrightmapForSprite) (const int type);
byte *(*R_BrightmapForFlatNum) (const int num);
byte *(*R_BrightmapForState) (const int state);

void R_InitBrightmaps ()
{
	if (gameversion == exe_hacx)
	{
           bmapflatnum[0] = R_FlatNumForName(lump_name_t("FLOOR1_1"));
           bmapflatnum[1] = R_FlatNumForName(lump_name_t("FLOOR1_7"));
           bmapflatnum[2] = R_FlatNumForName(lump_name_t("FLOOR3_3"));
           bmapflatnum[3] = R_FlatNumForName(lump_name_t("NUKAGE1"));
           bmapflatnum[4] = R_FlatNumForName(lump_name_t("NUKAGE2"));
           bmapflatnum[5] = R_FlatNumForName(lump_name_t("NUKAGE3"));
           bmapflatnum[6] = R_FlatNumForName(lump_name_t("BLOOD1"));
           bmapflatnum[7] = R_FlatNumForName(lump_name_t("BLOOD2"));
           bmapflatnum[8] = R_FlatNumForName(lump_name_t("BLOOD2"));
           bmapflatnum[9] = R_FlatNumForName(lump_name_t("SLIME13"));
           bmapflatnum[10] = R_FlatNumForName(lump_name_t("SLIME14"));
           bmapflatnum[11] = R_FlatNumForName(lump_name_t("SLIME15"));

		R_BrightmapForTexName = R_BrightmapForTexName_Hacx;
		R_BrightmapForSprite = R_BrightmapForSprite_Hacx;
		R_BrightmapForFlatNum = R_BrightmapForFlatNum_Hacx;
		R_BrightmapForState = R_BrightmapForState_Hacx;
	}
	else
	if (gameversion == exe_chex)
	{
		int lump;

		// [crispy] detect Chex Quest 2
		lump = W_CheckNumForName("INTERPIC");
		if (!strcasecmp(W_WadNameForLump(lumpinfo[lump]), "chex2.wad"))
		{
			chex2 = true;
		}

		R_BrightmapForTexName = R_BrightmapForTexName_Chex;
		R_BrightmapForSprite = R_BrightmapForSprite_Chex;
		R_BrightmapForFlatNum = R_BrightmapForFlatNum_None;
		R_BrightmapForState = R_BrightmapForState_None;
	}
	else
	{
		// [crispy] only three select brightmapped flats
           bmapflatnum[0] = R_FlatNumForName(lump_name_t("CONS1_1"));
           bmapflatnum[1] = R_FlatNumForName(lump_name_t("CONS1_5"));
           bmapflatnum[2] = R_FlatNumForName(lump_name_t("CONS1_7"));

		R_BrightmapForTexName = R_BrightmapForTexName_Doom;
		R_BrightmapForSprite = R_BrightmapForSprite_Doom;
		R_BrightmapForFlatNum = R_BrightmapForFlatNum_Doom;
		R_BrightmapForState = R_BrightmapForState_Doom;
	}
}
