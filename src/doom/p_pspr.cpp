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
//	Weapon sprite animation, weapon objects.
//	Action functions for weapons.
//


#include "doomdef.hpp"
#include "d_event.hpp"

#include "deh_misc.hpp"

#include "m_random.hpp"
#include "p_local.hpp"
#include "s_sound.hpp"

// State.
#include "doomstat.hpp"

// Data.
#include "sounds.hpp"

#include "p_pspr.hpp"

const fixed_t LOWERSPEED = FRACUNIT*6;
const fixed_t RAISESPEED = FRACUNIT*6;

const fixed_t WEAPONBOTTOM = 128*FRACUNIT;
const fixed_t WEAPONTOP = 32*FRACUNIT;

// [crispy] weapon recoil {thrust, pitch} values
// thrust values from prboom-plus/src/p_pspr.c:73-83
static const int recoil_values[][2] = {
  {10,   0}, // wp_fist
  {10,   4}, // wp_pistol
  {30,   8}, // wp_shotgun
  {10,   4}, // wp_chaingun
  {100, 16}, // wp_missile
  {20,   4}, // wp_plasma
  {100, 20}, // wp_bfg
  {0,   -2}, // wp_chainsaw
  {80,  16}, // wp_supershotgun
};

// [crispy] add weapon recoil
// adapted from prboom-plus/src/p_pspr.c:484-495 (A_FireSomething ())
extern void P_Thrust (player_t* player, angle_t angle, fixed_t move);
void A_Recoil (player_t* player)
{
	if (player)
	{

		if (critical->recoil && !(player->mo->flags & MF_NOCLIP))
		{
			P_Thrust(player, ANG180 + player->mo->angle, 2048 * recoil_values[player->readyweapon][0]);
		}

		if (crispy->pitch)
		{
			player->recoilpitch = recoil_values[player->readyweapon][1];
		}
	}
}


//
// P_SetPsprite
//
void
P_SetPsprite
( player_t*	player,
  int		position,
  statenum_t	stnum ) 
{
    pspdef_t*	psp;
    state_t*	state;
	
    psp = &player->psprites[position];
	
    do
    {
	if (!stnum)
	{
	    // object removed itself
	    psp->state = NULL;
	    break;	
	}
	
	state = &states[stnum];
	psp->state = state;
	psp->tics = state->tics;	// could be 0

	if (state->misc1)
	{
	    // coordinate set
	    psp->sx = state->misc1 << FRACBITS;
	    psp->sy = state->misc2 << FRACBITS;
	    // [crispy] variable weapon sprite bob
	    psp->sx2 = psp->sx;
	    psp->sy2 = psp->sy;
	}
	
	// Call action routine.
	// Modified handling.
        if (state->action.call_if(player->mo, player, psp)) {
          if (!psp->state)
            break;
        }

	stnum = psp->state->nextstate;
	
    } while (!psp->tics);
    // an initial state of 0 could cycle through
}



//
// P_CalcSwing
//	
fixed_t		swingx;
fixed_t		swingy;

void P_CalcSwing (player_t*	player)
{
    fixed_t	swing;
    int		angle;
	
    // OPTIMIZE: tablify this.
    // A LUT would allow for different modes,
    //  and add flexibility.

    swing = player->bob;

    angle = (FINEANGLES/70*leveltime)&FINEMASK;
    swingx = FixedMul ( swing, finesine[angle]);

    angle = (FINEANGLES/70*leveltime+FINEANGLES/2)&FINEMASK;
    swingy = -FixedMul ( swingx, finesine[angle]);
}



//
// P_BringUpWeapon
// Starts bringing the pending weapon up
// from the bottom of the screen.
// Uses player
//
void P_BringUpWeapon (player_t* player)
{
    statenum_t	newstate;
	
    if (player->pendingweapon == wp_nochange)
	player->pendingweapon = player->readyweapon;
		
    if (player->pendingweapon == wp_chainsaw)
	S_StartSound (player->mo, sfx_sawup); // [crispy] intentionally not weapon sound source
		
#if 0
    // [crispy] play "power up" sound when selecting berserk fist...
    if (player->pendingweapon == wp_fist && player->powers[pw_strength])
    {
	// [crispy] ...only if not playing already
	if (player == &players[consoleplayer])
	{
	    S_StartSoundOnce (NULL, sfx_getpow);
	}
    }
#endif

    newstate =
        static_cast<statenum_t>(weaponinfo[player->pendingweapon].upstate);

    player->pendingweapon = wp_nochange;
    player->psprites[ps_weapon].sy = WEAPONBOTTOM;
    // [crispy] squat down weapon sprite
    player->psprites[ps_weapon].dy = 0;

    P_SetPsprite (player, ps_weapon, newstate);
}

//
// P_CheckAmmo
// Returns true if there is enough ammo to shoot.
// If not, selects the next weapon to use.
//
boolean P_CheckAmmo (player_t* player)
{
    ammotype_t		ammo;
    int			count;

    ammo = weaponinfo[player->readyweapon].ammo;

    // Minimal amount for one shot varies.
    if (player->readyweapon == wp_bfg)
	count = deh_bfg_cells_per_shot;
    else if (player->readyweapon == wp_supershotgun)
	count = 2;	// Double barrel.
    else
	count = 1;	// Regular.

    // [crispy] force weapon switch if weapon not owned
    // only relevant when removing current weapon with TNTWEAPx cheat
    if (!player->weaponowned[player->readyweapon])
    {
	ammo = am_clip; // [crispy] at least not am_noammo, see below
	count = INT_MAX;
    }

    // Some do not need ammunition anyway.
    // Return if current ammunition sufficient.
    if (ammo == am_noammo || player->ammo[ammo] >= count)
	return true;
		
    // Out of ammo, pick a weapon to change to.
    // Preferences are set here.
    do
    {
	if (player->weaponowned[wp_plasma]
	    && player->ammo[am_cell]
	    && (gamemode != shareware) )
	{
	    player->pendingweapon = wp_plasma;
	}
	else if (player->weaponowned[wp_supershotgun] 
		 && player->ammo[am_shell]>2
		 && (crispy->havessg) )
	{
	    player->pendingweapon = wp_supershotgun;
	}
	else if (player->weaponowned[wp_chaingun]
		 && player->ammo[am_clip])
	{
	    player->pendingweapon = wp_chaingun;
	}
	else if (player->weaponowned[wp_shotgun]
		 && player->ammo[am_shell])
	{
	    player->pendingweapon = wp_shotgun;
	}
	// [crispy] allow to remove the pistol via TNTWEAP2
	else if (player->ammo[am_clip] && player->weaponowned[wp_pistol])
	{
	    player->pendingweapon = wp_pistol;
	}
	else if (player->weaponowned[wp_chainsaw])
	{
	    player->pendingweapon = wp_chainsaw;
	}
	else if (player->weaponowned[wp_missile]
		 && player->ammo[am_misl])
	{
	    player->pendingweapon = wp_missile;
	}
	else if (player->weaponowned[wp_bfg]
		 && player->ammo[am_cell]>40
		 && (gamemode != shareware) )
	{
	    player->pendingweapon = wp_bfg;
	}
	else
	{
	    // If everything fails.
	    player->pendingweapon = wp_fist;
	}
	
    } while (player->pendingweapon == wp_nochange);

    // Now set appropriate weapon overlay.
    P_SetPsprite (player,
		  ps_weapon,
        static_cast<statenum_t>(weaponinfo[player->readyweapon].downstate));

    return false;	
}


//
// P_FireWeapon.
//
void P_FireWeapon (player_t* player)
{
    statenum_t	newstate;
	
    if (!P_CheckAmmo (player))
	return;
	
    P_SetMobjState (player->mo, S_PLAY_ATK1);
    newstate =
        static_cast<statenum_t>(weaponinfo[player->readyweapon].atkstate);
    P_SetPsprite (player, ps_weapon, newstate);
    P_NoiseAlert (player->mo, player->mo);
}



//
// P_DropWeapon
// Player died, so put the weapon away.
//
void P_DropWeapon (player_t* player)
{
    P_SetPsprite (player,
		  ps_weapon,
      static_cast<statenum_t>(weaponinfo[player->readyweapon].downstate));
}



//
// A_WeaponReady
// The player can fire the weapon
// or change to another weapon at this time.
// Follows after getting weapon up,
// or after previous attack/fire sequence.
//
void
A_WeaponReady
( mobj_t*	mobj,
  player_t*	player,
  pspdef_t*	psp )
{	
    statenum_t	newstate;
    int		angle;
    
    if (!player) return; // [crispy] let pspr action pointers get called from mobj states
    // get out of attack state
    if (player->mo->state == &states[S_PLAY_ATK1]
	|| player->mo->state == &states[S_PLAY_ATK2] )
    {
	P_SetMobjState (player->mo, S_PLAY);
    }
    
    if (player->readyweapon == wp_chainsaw
	&& psp->state == &states[S_SAW])
    {
	S_StartSound (player->so, sfx_sawidl); // [crispy] weapon sound source
    }
    
    // check for change
    //  if player is dead, put the weapon away
    if (player->pendingweapon != wp_nochange || !player->health)
    {
	// change weapon
	//  (pending weapon should allready be validated)
	newstate =
            static_cast<statenum_t>(weaponinfo[player->readyweapon].downstate);
	P_SetPsprite (player, ps_weapon, newstate);
	return;	
    }
    
    // check for fire
    //  the missile launcher and bfg do not auto fire
    if (player->cmd.buttons & BT_ATTACK)
    {
	if ( !player->attackdown
	     || (player->readyweapon != wp_missile
		 && player->readyweapon != wp_bfg) )
	{
	    player->attackdown = true;
	    P_FireWeapon (player);		
	    return;
	}
    }
    else
	player->attackdown = false;
    
    // bob the weapon based on movement speed
    angle = (128*leveltime)&FINEMASK;
    psp->sx = FRACUNIT + FixedMul (player->bob, finecosine[angle]);
    angle &= FINEANGLES/2-1;
    psp->sy = WEAPONTOP + FixedMul (player->bob, finesine[angle]);
}



//
// A_ReFire
// The player can re-fire the weapon
// without lowering it entirely.
//
void A_ReFire
( mobj_t*	mobj,
  player_t*	player,
  pspdef_t*	psp )
{
    
    if (!player) return; // [crispy] let pspr action pointers get called from mobj states
    // check for fire
    //  (if a weaponchange is pending, let it go through instead)
    if ( (player->cmd.buttons & BT_ATTACK) 
	 && player->pendingweapon == wp_nochange
	 && player->health)
    {
	player->refire++;
	P_FireWeapon (player);
    }
    else
    {
	player->refire = 0;
	P_CheckAmmo (player);
    }
}


void
A_CheckReload
( mobj_t*	mobj,
  player_t*	player,
  pspdef_t*	psp )
{
    if (!player) return; // [crispy] let pspr action pointers get called from mobj states
    P_CheckAmmo (player);
#if 0
    if (player->ammo[am_shell]<2)
	P_SetPsprite (player, ps_weapon, S_DSNR1);
#endif
}



//
// A_Lower
// Lowers current weapon,
//  and changes weapon at bottom.
//
void
A_Lower
( mobj_t*	mobj,
  player_t*	player,
  pspdef_t*	psp )
{	
    if (!player) return; // [crispy] let pspr action pointers get called from mobj states
    psp->sy += LOWERSPEED;

    // Is already down.
    if (psp->sy < WEAPONBOTTOM )
	return;

    // Player is dead.
    if (player->playerstate == PST_DEAD)
    {
	psp->sy = WEAPONBOTTOM;

	// don't bring weapon back up
	return;		
    }
    
    // The old weapon has been lowered off the screen,
    // so change the weapon and start raising it
    if (!player->health)
    {
	// Player is dead, so keep the weapon off screen.
	P_SetPsprite (player,  ps_weapon, S_NULL);
	return;	
    }
	
    player->readyweapon = player->pendingweapon; 

    P_BringUpWeapon (player);
}


//
// A_Raise
//
void
A_Raise
( mobj_t*	mobj,
  player_t*	player,
  pspdef_t*	psp )
{
    statenum_t	newstate;
	
    if (!player) return; // [crispy] let pspr action pointers get called from mobj states
    psp->sy -= RAISESPEED;

    if (psp->sy > WEAPONTOP )
	return;
    
    psp->sy = WEAPONTOP;
    
    // The weapon has been raised all the way,
    //  so change to the ready state.
    newstate =
        static_cast<statenum_t>(weaponinfo[player->readyweapon].readystate);

    P_SetPsprite (player, ps_weapon, newstate);
}



//
// A_GunFlash
//
void
A_GunFlash
( mobj_t*	mobj,
  player_t*	player,
  pspdef_t*	psp ) 
{
    if (!player) return; // [crispy] let pspr action pointers get called from mobj states
    P_SetMobjState (player->mo, S_PLAY_ATK2);
    P_SetPsprite (player,ps_flash,
        static_cast<statenum_t>(weaponinfo[player->readyweapon].flashstate));
}



//
// WEAPON ATTACKS
//


//
// A_Punch
//
void
A_Punch
( mobj_t*	mobj,
  player_t*	player,
  pspdef_t*	psp ) 
{
    angle_t	angle;
    int		damage;
    int		slope;
	
    if (!player) return; // [crispy] let pspr action pointers get called from mobj states
    damage = (P_Random ()%10+1)<<1;

    if (player->powers[pw_strength])	
	damage *= 10;

    angle = player->mo->angle;
    angle += angle_t(P_SubRandom() << 18);
    slope = P_AimLineAttack (player->mo, angle, MELEERANGE);
    P_LineAttack (player->mo, angle, MELEERANGE, slope, damage);

    // turn to face target
    if (linetarget)
    {
	S_StartSound (player->so, sfx_punch); // [crispy] weapon sound source
	player->mo->angle = R_PointToAngle2 (player->mo->x,
					     player->mo->y,
					     linetarget->x,
					     linetarget->y);
    }
}


//
// A_Saw
//
void
A_Saw
( mobj_t*	mobj,
  player_t*	player,
  pspdef_t*	psp ) 
{
    angle_t	angle;
    int		damage;
    int		slope;

    if (!player) return; // [crispy] let pspr action pointers get called from mobj states
    damage = 2*(P_Random ()%10+1);
    angle = player->mo->angle;
    angle += angle_t(P_SubRandom() << 18);
    
    // use meleerange + 1 se the puff doesn't skip the flash
    slope = P_AimLineAttack (player->mo, angle, MELEERANGE+1);
    P_LineAttack (player->mo, angle, MELEERANGE+1, slope, damage);

    A_Recoil (player);

    if (!linetarget)
    {
	S_StartSound (player->so, sfx_sawful); // [crispy] weapon sound source
	return;
    }
    S_StartSound (player->so, sfx_sawhit); // [crispy] weapon sound source
	
    // turn to face target
    angle = R_PointToAngle2 (player->mo->x, player->mo->y,
			     linetarget->x, linetarget->y);
    if (angle - player->mo->angle > ANG180)
    {
	if ((signed int) (angle - player->mo->angle) < -ANG90/20)
	    player->mo->angle = angle + ANG90/21;
	else
	    player->mo->angle -= ANG90/20;
    }
    else
    {
	if (angle - player->mo->angle > ANG90/20)
	    player->mo->angle = angle - ANG90/21;
	else
	    player->mo->angle += ANG90/20;
    }
    player->mo->flags |= MF_JUSTATTACKED;
}

// Doom does not check the bounds of the ammo array.  As a result,
// it is possible to use an ammo type > 4 that overflows into the
// maxammo array and affects that instead.  Through dehacked, for
// example, it is possible to make a weapon that decreases the max
// number of ammo for another weapon.  Emulate this.

static void DecreaseAmmo(player_t *player, int ammonum, int amount)
{
    if (ammonum < NUMAMMO)
    {
        player->ammo[ammonum] -= amount;
        // [crispy] never allow less than zero ammo
        if (player->ammo[ammonum] < 0)
        {
            player->ammo[ammonum] = 0;
        }
    }
    else
    {
        player->maxammo[ammonum - NUMAMMO] -= amount;
    }
}


//
// A_FireMissile
//
void
A_FireMissile
( mobj_t*	mobj,
  player_t*	player,
  pspdef_t*	psp ) 
{
    if (!player) return; // [crispy] let pspr action pointers get called from mobj states
    DecreaseAmmo(player, weaponinfo[player->readyweapon].ammo, 1);
    P_SpawnPlayerMissile (player->mo, MT_ROCKET);
}


//
// A_FireBFG
//
void
A_FireBFG
( mobj_t*	mobj,
  player_t*	player,
  pspdef_t*	psp ) 
{
    if (!player) return; // [crispy] let pspr action pointers get called from mobj states
    DecreaseAmmo(player, weaponinfo[player->readyweapon].ammo, 
                 deh_bfg_cells_per_shot);
    P_SpawnPlayerMissile (player->mo, MT_BFG);
}



//
// A_FirePlasma
//
void
A_FirePlasma
( mobj_t*	mobj,
  player_t*	player,
  pspdef_t*	psp ) 
{
    if (!player) return; // [crispy] let pspr action pointers get called from mobj states
    DecreaseAmmo(player, weaponinfo[player->readyweapon].ammo, 1);

    P_SetPsprite (player,
		  ps_flash,
        static_cast<statenum_t>(weaponinfo[player->readyweapon].flashstate +
                                (P_Random() & 1)));

    P_SpawnPlayerMissile (player->mo, MT_PLASMA);
}



//
// P_BulletSlope
// Sets a slope so a near miss is at aproximately
// the height of the intended target
//
fixed_t		bulletslope;


void P_BulletSlope (mobj_t*	mo)
{
    angle_t	an;
    
    if (critical->freeaim == FREEAIM_DIRECT)
    {
	bulletslope = PLAYER_SLOPE(mo->player);
    }
    else
    {
    // see which target is to be aimed at
    an = mo->angle;
    bulletslope = P_AimLineAttack (mo, an, 16*64*FRACUNIT);

    if (!linetarget)
    {
        an += angle_t(1<<26);
	bulletslope = P_AimLineAttack (mo, an, 16*64*FRACUNIT);
	if (!linetarget)
	{
            an -= angle_t(2<<26);
	    bulletslope = P_AimLineAttack (mo, an, 16*64*FRACUNIT);
	    if (!linetarget && critical->freeaim == FREEAIM_BOTH)
	    {
		bulletslope = PLAYER_SLOPE(mo->player);
	    }
	}
    }
    }
}


//
// P_GunShot
//
void
P_GunShot
( mobj_t*	mo,
  boolean	accurate )
{
    angle_t	angle;
    int		damage;
	
    damage = 5*(P_Random ()%3+1);
    angle = mo->angle;

    if (!accurate)
        angle += angle_t(P_SubRandom() << 18);

    P_LineAttack (mo, angle, MISSILERANGE, bulletslope, damage);
}


//
// A_FirePistol
//
void
A_FirePistol
( mobj_t*	mobj,
  player_t*	player,
  pspdef_t*	psp ) 
{
    if (!player) return; // [crispy] let pspr action pointers get called from mobj states
    S_StartSound (player->so, sfx_pistol); // [crispy] weapon sound source

    P_SetMobjState (player->mo, S_PLAY_ATK2);
    DecreaseAmmo(player, weaponinfo[player->readyweapon].ammo, 1);

    P_SetPsprite (player,
		  ps_flash,
        static_cast<statenum_t>(weaponinfo[player->readyweapon].flashstate));

    P_BulletSlope (player->mo);
    P_GunShot (player->mo, !player->refire);

    A_Recoil (player);
}


//
// A_FireShotgun
//
void
A_FireShotgun
( mobj_t*	mobj,
  player_t*	player,
  pspdef_t*	psp ) 
{
    int		i;
	
    if (!player) return; // [crispy] let pspr action pointers get called from mobj states
    S_StartSound (player->so, sfx_shotgn); // [crispy] weapon sound source
    P_SetMobjState (player->mo, S_PLAY_ATK2);

    DecreaseAmmo(player, weaponinfo[player->readyweapon].ammo, 1);

    P_SetPsprite (player,
		  ps_flash,
        static_cast<statenum_t>(weaponinfo[player->readyweapon].flashstate));

    P_BulletSlope (player->mo);
	
    for (i=0 ; i<7 ; i++)
	P_GunShot (player->mo, false);

    A_Recoil (player);
}



//
// A_FireShotgun2
//
void
A_FireShotgun2
( mobj_t*	mobj,
  player_t*	player,
  pspdef_t*	psp ) 
{
    int		i;
    angle_t	angle;
    int		damage;
		
	
    if (!player) return; // [crispy] let pspr action pointers get called from mobj states
    S_StartSound (player->so, sfx_dshtgn); // [crispy] weapon sound source
    P_SetMobjState (player->mo, S_PLAY_ATK2);

    DecreaseAmmo(player, weaponinfo[player->readyweapon].ammo, 2);

    P_SetPsprite (player,
		  ps_flash,
        static_cast<statenum_t>(weaponinfo[player->readyweapon].flashstate));

    P_BulletSlope (player->mo);
	
    for (i=0 ; i<20 ; i++)
    {
	damage = 5*(P_Random ()%3+1);
	angle = player->mo->angle;
	angle += angle_t(P_SubRandom() << ANGLETOFINESHIFT);
	P_LineAttack (player->mo,
		      angle,
		      MISSILERANGE,
		      bulletslope + (P_SubRandom() << 5), damage);
    }

    A_Recoil (player);
}


//
// A_FireCGun
//
void
A_FireCGun
( mobj_t*	mobj,
  player_t*	player,
  pspdef_t*	psp ) 
{
    if (!player) return; // [crispy] let pspr action pointers get called from mobj states
    S_StartSound (player->so, sfx_pistol); // [crispy] weapon sound source

    if (!player->ammo[weaponinfo[player->readyweapon].ammo])
	return;
		
    P_SetMobjState (player->mo, S_PLAY_ATK2);
    DecreaseAmmo(player, weaponinfo[player->readyweapon].ammo, 1);

    P_SetPsprite (player,
		  ps_flash,
        static_cast<statenum_t>(weaponinfo[player->readyweapon].flashstate +
                                psp->state - &states[S_CHAIN1]));

    P_BulletSlope (player->mo);
	
    P_GunShot (player->mo, !player->refire);

    A_Recoil (player);
}



//
// ?
//
void A_Light0 (mobj_t *mobj, player_t *player, pspdef_t *psp)
{
    if (!player) return; // [crispy] let pspr action pointers get called from mobj states
    player->extralight = 0;
}

void A_Light1 (mobj_t *mobj, player_t *player, pspdef_t *psp)
{
    if (!player) return; // [crispy] let pspr action pointers get called from mobj states
    player->extralight = 1;
}

void A_Light2 (mobj_t *mobj, player_t *player, pspdef_t *psp)
{
    if (!player) return; // [crispy] let pspr action pointers get called from mobj states
    player->extralight = 2;
}


//
// A_BFGSpray
// Spawn a BFG explosion on every monster in view
//
void A_BFGSpray (mobj_t* mo) 
{
    int			i;
    int			j;
    int			damage;
    angle_t		an;
	
    // offset angles from its attack angle
    for (i=0 ; i<40 ; i++)
    {
	an = mo->angle - ANG90/2 + ANG90/40*i;

	// mo->target is the originator (player)
	//  of the missile
	P_AimLineAttack (mo->target, an, 16*64*FRACUNIT);

	if (!linetarget)
	    continue;

	P_SpawnMobj (linetarget->x,
		     linetarget->y,
		     linetarget->z + (linetarget->height>>2),
		     MT_EXTRABFG);
	
	damage = 0;
	for (j=0;j<15;j++)
	    damage += (P_Random()&7) + 1;

	P_DamageMobj (linetarget, mo->target,mo->target, damage);
    }
}


//
// A_BFGsound
//
void
A_BFGsound
( mobj_t*	mobj,
  player_t*	player,
  pspdef_t*	psp )
{
    if (!player) return; // [crispy] let pspr action pointers get called from mobj states
    S_StartSound (player->mo, sfx_bfg); // [crispy] intentionally not weapon sound source
}



//
// P_SetupPsprites
// Called at start of level for each player.
//
void P_SetupPsprites (player_t* player) 
{
    int	i;
	
    // remove all psprites
    for (i=0 ; i<NUMPSPRITES ; i++)
	player->psprites[i].state = NULL;
		
    // spawn the gun
    player->pendingweapon = player->readyweapon;
    P_BringUpWeapon (player);
}




//
// P_MovePsprites
// Called every tic by player thinking routine.
//
void P_MovePsprites (player_t* player) 
{
    int		i;
    pspdef_t*	psp;

    psp = &player->psprites[0];
    for (i=0 ; i<NUMPSPRITES ; i++, psp++)
    {
	// a null state means not active
	if ( psp->state)
	{
	    // drop tic count and possibly change state

	    // a -1 tic count never changes
	    if (psp->tics != -1)	
	    {
		psp->tics--;
		if (!psp->tics)
		    P_SetPsprite (player, i, psp->state->nextstate);
	    }				
	}
    }
    
    player->psprites[ps_flash].sx = player->psprites[ps_weapon].sx;
    player->psprites[ps_flash].sy = player->psprites[ps_weapon].sy;

    // [crispy] apply bobbing (or centering) to the player's weapon sprite
    psp = &player->psprites[0];
    if (psp->state)
    {
	// [crispy] don't center vertically during lowering and raising states
	if (psp->state->misc1 ||
	    psp->state->action == A_Lower ||
	    psp->state->action == A_Raise)
	{
		psp->sx2 = psp->sx;
		psp->sy2 = psp->sy;
	}
	else
	if (psp->state->action == A_WeaponReady ||
	    crispy->centerweapon == CENTERWEAPON_BOB)
	{
		angle_t angle = (128 * leveltime) & FINEMASK;
		psp->sx2 = FRACUNIT + FixedMul(player->bob2, finecosine[angle]);
		angle &= FINEANGLES / 2 - 1;
		psp->sy2 = WEAPONTOP + FixedMul(player->bob2, finesine[angle]);
	}
	else
	// [crispy] center the weapon sprite horizontally and push up vertically
	if (crispy->centerweapon == CENTERWEAPON_CENTER)
	{
		psp->sx2 = FRACUNIT;
		psp->sy2 = WEAPONTOP;
	}
    }
    else
    {
	psp->sx2 = psp->sx;
	psp->sy2 = psp->sy;
    }

	// [crispy] squat down weapon sprite a bit after hitting the ground
	if (player->psp_dy_max)
	{
		psp->dy -= FRACUNIT;

		if (psp->dy < player->psp_dy_max)
		{
			psp->dy = -psp->dy;
		}

		if (psp->dy == 0)
		{
			player->psp_dy_max = 0;
		}
	}

	player->psprites[ps_flash].dy = psp->dy;
	player->psprites[ps_flash].sx2 = psp->sx2;
	player->psprites[ps_flash].sy2 = psp->sy2;
}


