/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
// cg_weapons.c -- events and effects dealing with weapons
#include "cg_local.h"

/*
==========================
CG_MachineGunEjectBrass
==========================
*/
static void CG_MachineGunEjectBrass( centity_t *cent ) {
	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t			velocity, xvelocity;
	vec3_t			offset, xoffset;
	float			waterScale = 1.0f;
	vec3_t			v[3];

	if ( cg_brassTime.integer <= 0 ) {
		return;
	}

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	velocity[0] = 0;
	velocity[1] = -50 + 40 * crandom();
	velocity[2] = 100 + 50 * crandom();

	le->leType = LE_FRAGMENT;
	le->startTime = cg.time;
	le->endTime = le->startTime + cg_brassTime.integer + ( cg_brassTime.integer / 4 ) * random();

	le->pos.trType = TR_GRAVITY;
	le->pos.trTime = cg.time - (rand()&15);

	AnglesToAxis( cent->lerpAngles, v );

	offset[0] = 8;
	offset[1] = -4;
	offset[2] = 24;

	xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
	xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
	xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];
	VectorAdd( cent->lerpOrigin, xoffset, re->origin );

	VectorCopy( re->origin, le->pos.trBase );

	if ( CG_PointContents( re->origin, -1 ) & CONTENTS_WATER ) {
		waterScale = 0.10f;
	}

	xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
	xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
	xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];
	VectorScale( xvelocity, waterScale, le->pos.trDelta );

	AxisCopy( axisDefault, re->axis );
	re->hModel = cgs.media.machinegunBrassModel;

	if (cg_leiBrassNoise.integer)
	le->bounceFactor = 0.6f * waterScale;
	else
	le->bounceFactor = 0.4 * waterScale;

	le->angles.trType = TR_LINEAR;
	le->angles.trTime = cg.time;
	le->angles.trBase[0] = rand()&31;
	le->angles.trBase[1] = rand()&31;
	le->angles.trBase[2] = rand()&31;
	le->angles.trDelta[0] = 2;
	le->angles.trDelta[1] = 1;
	le->angles.trDelta[2] = 0;

	le->leFlags = LEF_TUMBLE;
	le->leBounceSoundType = LEBS_BRASS;
	le->leMarkType = LEMT_NONE;
}



/*
==========================
CG_ShotgunEjectBrass
==========================
*/
static void CG_ShotgunEjectBrass( centity_t *cent ) {
	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t			velocity, xvelocity;
	vec3_t			offset, xoffset;
	vec3_t			v[3];
	int				i;

	if ( cg_brassTime.integer <= 0 ) {
		return;
	}

	for ( i = 0; i < 2; i++ ) {
		float	waterScale = 1.0f;

		le = CG_AllocLocalEntity();
		re = &le->refEntity;

		velocity[0] = 60 + 60 * crandom();
		if ( i == 0 ) {
			velocity[1] = 40 + 10 * crandom();
		} else {
			velocity[1] = -40 + 10 * crandom();
		}
		velocity[2] = 100 + 50 * crandom();

		le->leType = LE_FRAGMENT;
		le->startTime = cg.time;
		le->endTime = le->startTime + cg_brassTime.integer*3 + cg_brassTime.integer * random();

		le->pos.trType = TR_GRAVITY;
		le->pos.trTime = cg.time;

		AnglesToAxis( cent->lerpAngles, v );

		offset[0] = 8;
		offset[1] = 0;
		offset[2] = 24;

		xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
		xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
		xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];
		VectorAdd( cent->lerpOrigin, xoffset, re->origin );
		VectorCopy( re->origin, le->pos.trBase );
		if ( CG_PointContents( re->origin, -1 ) & CONTENTS_WATER ) {
			waterScale = 0.10f;
		}

		xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
		xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
		xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];
		VectorScale( xvelocity, waterScale, le->pos.trDelta );

		VectorScale( xvelocity, waterScale, le->avelocity.trDelta );

		AxisCopy( axisDefault, re->axis );
		re->hModel = cgs.media.shotgunBrassModel;
		if (cg_leiBrassNoise.integer)
		le->bounceFactor = 0.5f;
		else
		le->bounceFactor = 0.3f;

		le->angles.trType = TR_LINEAR;
		le->angles.trTime = cg.time;
		le->angles.trBase[0] = rand()&31;
		le->angles.trBase[1] = rand()&31;
		le->angles.trBase[2] = rand()&31;
		le->angles.trDelta[0] = 1;
		le->angles.trDelta[1] = 0.5;
		le->angles.trDelta[2] = 0;

		le->leFlags = LEF_TUMBLE;
		le->leBounceSoundType = LEBS_SHELL; // LEILEI shell noises
		le->leMarkType = LEMT_NONE;
	}
}


//#ifdef MISSIONPACK
/*
==========================
CG_NailgunEjectBrass
==========================
*/
static void CG_NailgunEjectBrass( centity_t *cent ) {
	localEntity_t	*smoke;
	vec3_t			origin;
	vec3_t			v[3];
	vec3_t			offset;
	vec3_t			xoffset;
	vec3_t			up;

	AnglesToAxis( cent->lerpAngles, v );

	offset[0] = 0;
	offset[1] = -12;
	offset[2] = 24;

	xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
	xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
	xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];
	VectorAdd( cent->lerpOrigin, xoffset, origin );

	VectorSet( up, 0, 0, 64 );

	smoke = CG_SmokePuff( origin, up, 32, 1, 1, 1, 0.33f, 700, cg.time, 0, 0, cgs.media.smokePuffShader );
	// use the optimized local entity add
	smoke->leType = LE_SCALE_FADE;
}
//#endif


/*
==========================
CG_RailTrail
==========================
*/
void CG_RailTrail (clientInfo_t *ci, vec3_t start, vec3_t end) {
	vec3_t axis[36], move, move2, next_move, vec, temp;
	float  len;
	int    i, j, skip;
 
	localEntity_t *le; 
	refEntity_t   *re;


 
#define RADIUS   4
#define ROTATION 1
#define SPACING  5
 
	start[2] -= 4;
 
	le = CG_AllocLocalEntity();
	re = &le->refEntity;
 
	le->leType = LE_FADE_RGB;
	le->startTime = cg.time;
	le->endTime = cg.time + cg_railTrailTime.value;
	le->lifeRate = 1.0 / (le->endTime - le->startTime);
 
	re->shaderTime = cg.time / 1000.0f;
	re->reType = RT_RAIL_CORE;
	re->customShader = cgs.media.railCoreShader;
 
	VectorCopy(start, re->origin);
	VectorCopy(end, re->oldorigin);
 
	re->shaderRGBA[0] = ci->color1[0] * 255;
	re->shaderRGBA[1] = ci->color1[1] * 255;
	re->shaderRGBA[2] = ci->color1[2] * 255;
	re->shaderRGBA[3] = 255;

	le->color[0] = ci->color1[0] * 0.75;
	le->color[1] = ci->color1[1] * 0.75;
	le->color[2] = ci->color1[2] * 0.75;
	le->color[3] = 1.0f;

	AxisClear( re->axis );
 
	if (cg_oldRail.integer)
	{
		// nudge down a bit so it isn't exactly in center
		re->origin[2] -= 8;
		re->oldorigin[2] -= 8;

		// leilei - reimplementing the rail discs that were removed in 1.30
		if (cg_oldRail.integer > 1){

		le = CG_AllocLocalEntity();
		re = &le->refEntity;

		VectorCopy(start, re->origin);
		VectorCopy(end, re->oldorigin);
		le->leType = LE_FADE_RGB;
		le->startTime = cg.time;
		le->endTime = cg.time + cg_railTrailTime.value;
		le->lifeRate = 1.0 / (le->endTime - le->startTime);
	 
		re->shaderTime = cg.time / 1000.0f;
		re->reType = RT_RAIL_RINGS;
		re->customShader = cgs.media.railRingsShader;
		re->shaderRGBA[0] = ci->color1[0] * 255;
		re->shaderRGBA[1] = ci->color1[1] * 255;
		re->shaderRGBA[2] = ci->color1[2] * 255;
		re->shaderRGBA[3] = 255;
	
		le->color[0] = ci->color1[0] * 0.75;
		le->color[1] = ci->color1[1] * 0.75;
		le->color[2] = ci->color1[2] * 0.75;
		le->color[3] = 1.0f;	

		re->origin[2] -= 8;
		re->oldorigin[2] -= 8;

		if (cg_oldRail.integer > 2){		// use the secondary color instead
			re->shaderRGBA[0] = ci->color2[0] * 255;
			re->shaderRGBA[1] = ci->color2[1] * 255;
			re->shaderRGBA[2] = ci->color2[2] * 255;
			re->shaderRGBA[3] = 255;
		
			le->color[0] = ci->color2[0] * 0.75;
			le->color[1] = ci->color2[1] * 0.75;
			le->color[2] = ci->color2[2] * 0.75;
			le->color[3] = 1.0f;	
			}

		}
		return;
	}

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);
	PerpendicularVector(temp, vec);
	for (i = 0 ; i < 36; i++)
	{
		RotatePointAroundVector(axis[i], vec, temp, i * 10);//banshee 2.4 was 10
	}

	VectorMA(move, 20, vec, move);
	VectorCopy(move, next_move);
	VectorScale (vec, SPACING, vec);

	skip = -1;
 
	j = 18;
	for (i = 0; i < len; i += SPACING)
	{
		if (i != skip)
		{
			skip = i + SPACING;
			le = CG_AllocLocalEntity();
			re = &le->refEntity;
			le->leFlags = LEF_PUFF_DONT_SCALE;
			le->leType = LE_MOVE_SCALE_FADE;
			le->startTime = cg.time;
			le->endTime = cg.time + (i>>1) + 600;
			le->lifeRate = 1.0 / (le->endTime - le->startTime);

			re->shaderTime = cg.time / 1000.0f;
			re->reType = RT_SPRITE;
			re->radius = 1.1f;
			re->customShader = cgs.media.railRingsShader;

			re->shaderRGBA[0] = ci->color2[0] * 255;
			re->shaderRGBA[1] = ci->color2[1] * 255;
			re->shaderRGBA[2] = ci->color2[2] * 255;
			re->shaderRGBA[3] = 255;

			le->color[0] = ci->color2[0] * 0.75;
			le->color[1] = ci->color2[1] * 0.75;
			le->color[2] = ci->color2[2] * 0.75;
			le->color[3] = 1.0f;

			le->pos.trType = TR_LINEAR;
			le->pos.trTime = cg.time;

			VectorCopy( move, move2);
			VectorMA(move2, RADIUS , axis[j], move2);
			VectorCopy(move2, le->pos.trBase);

			le->pos.trDelta[0] = axis[j][0]*6;
			le->pos.trDelta[1] = axis[j][1]*6;
			le->pos.trDelta[2] = axis[j][2]*6;
		}

		VectorAdd (move, vec, move);

		j = (j + ROTATION) % 36;
	}
}

/*
==========================
CG_OldRocketTrail	(for the crappy old rocket trail.)
==========================
*/
static void CG_OldRocketTrail( centity_t *ent, const weaponInfo_t *wi ) {
	int		step;
	vec3_t	origin, lastPos;
	int		t;
	int		startTime, contents;
	int		lastContents;
	entityState_t	*es;
	vec3_t	up;
	localEntity_t	*smoke;

	if ( cg_noProjectileTrail.integer ) {
		return;
	}

	up[0] = 0;
	up[1] = 0;
	up[2] = 0;

	step = 50;

	es = &ent->currentState;
	startTime = ent->trailTime;
	t = step * ( (startTime + step) / step );

	BG_EvaluateTrajectory( &es->pos, cg.time, origin );
	contents = CG_PointContents( origin, -1 );

	// if object (e.g. grenade) is stationary, don't toss up smoke
	if ( es->pos.trType == TR_STATIONARY ) {
		ent->trailTime = cg.time;
		return;
	}

	BG_EvaluateTrajectory( &es->pos, ent->trailTime, lastPos );
	lastContents = CG_PointContents( lastPos, -1 );

	ent->trailTime = cg.time;

	if ( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {
		if ( contents & lastContents & CONTENTS_WATER ) {
			CG_BubbleTrail( lastPos, origin, 8 );
		}
		return;
	}

	for ( ; t <= ent->trailTime ; t += step ) {
		BG_EvaluateTrajectory( &es->pos, t, lastPos );

		smoke = CG_SmokePuff( lastPos, up, 
					  wi->trailRadius, 
					  1, 1, 1, 0.33f,
					  wi->wiTrailTime, 
					  t,
					  0,
					  0, 
					  cgs.media.smokePuffShader );
		// use the optimized local entity add
		smoke->leType = LE_SCALE_FADE;
	}

}

/*
==========================
CG_LeiSmokeTrail 
==========================
*/

static void CG_LeiSmokeTrail( centity_t *ent, const weaponInfo_t *wi ) {
	int		step;
	vec3_t	origin, lastPos;
	int		t;
	int		startTime, contents;
	int		lastContents;
	entityState_t	*es;
	vec3_t	up;
	localEntity_t	*smoke;
	int		therando;
	int		theradio;

	if ( cg_noProjectileTrail.integer ) {
		return;
	}

	up[0] = 5 - 10 * crandom();
	up[1] = 5 - 10 * crandom();
	up[2] = 8 - 5 * crandom();

	step = 18;

	es = &ent->currentState;
	startTime = ent->trailTime;
	t = step * ( (startTime + step) / step );

	BG_EvaluateTrajectory( &es->pos, cg.time, origin );
	contents = CG_PointContents( origin, -1 );

	// if object (e.g. grenade) is stationary, don't toss up smoke
	if ( es->pos.trType == TR_STATIONARY ) {
		ent->trailTime = cg.time;
		return;
	}

	BG_EvaluateTrajectory( &es->pos, ent->trailTime, lastPos );
	lastContents = CG_PointContents( lastPos, -1 );

	ent->trailTime = cg.time;

	if ( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {
		if ( contents & lastContents & CONTENTS_WATER ) {
			CG_BubbleTrail( lastPos, origin, 8 );
		}
		return;
	}

	for ( ; t <= ent->trailTime ; t += step ) {
		BG_EvaluateTrajectory( &es->pos, t, lastPos );
		therando = crandom() * 4;
		
		theradio =  wi->trailRadius * (rand() * 0.7); // what is this doing here
	if (therando == 3)		smoke = CG_SmokePuff( lastPos, up, 27, 1, 1, 1, 0.9f, wi->wiTrailTime,  t, 0, 0,  cgs.media.lsmkShader1 );
	else if (therando == 1)		smoke = CG_SmokePuff( lastPos, up, 27, 1, 1, 1, 0.9f, wi->wiTrailTime,  t, 0, 0,  cgs.media.lsmkShader2 );
	else	if (therando == 2)	smoke = CG_SmokePuff( lastPos, up, 27, 1, 1, 1, 0.9f, wi->wiTrailTime,  t, 0, 0,  cgs.media.lsmkShader3 );
	else				smoke = CG_SmokePuff( lastPos, up, 27, 1, 1, 1, 0.9f, wi->wiTrailTime,  t, 0, 0,  cgs.media.lsmkShader4 );
		// use the optimized local entity add
		smoke->leType = LE_MOVE_SCALE_FADE;
		//smoke->trType = TR_GRAVITY;
	}

}


static void CG_LeiPlasmaTrail( centity_t *ent, const weaponInfo_t *wi ) {
	int		step;
	vec3_t	origin, lastPos;
	int		t;
	int		startTime, contents;
	int		lastContents;
	entityState_t	*es;
	vec3_t	up;
	localEntity_t	*smoke;

	if ( cg_noProjectileTrail.integer ) {
		return;
	}

	up[0] = 0;
	up[1] = 0;
	up[2] = 0;

	step = 16;

	es = &ent->currentState;
	startTime = ent->trailTime;
	t = step * ( (startTime + step) / step );

	BG_EvaluateTrajectory( &es->pos, cg.time, origin );
	contents = CG_PointContents( origin, -1 );

	// if object (e.g. grenade) is stationary, don't toss up smoke
	if ( es->pos.trType == TR_STATIONARY ) {
		ent->trailTime = cg.time;
		return;
	}

	BG_EvaluateTrajectory( &es->pos, ent->trailTime, lastPos );
	lastContents = CG_PointContents( lastPos, -1 );

	ent->trailTime = cg.time;

	if ( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {
		return;
	}

	for ( ; t <= ent->trailTime ; t += step ) {
		BG_EvaluateTrajectory( &es->pos, t, lastPos );

		smoke = CG_SmokePuff( lastPos, up, 27, 1, 1, 1, 0.9f, wi->wiTrailTime,  t, 0, 0,  cgs.media.lsmkShader1 );
		// use the optimized local entity add
		smoke->leType = LE_SCALE_FADE;
		//smoke->trType = TR_GRAVITY;
	}

}


//#ifdef MISSIONPACK
/*
==========================
CG_NailTrail
==========================
*/
static void CG_NailTrail( centity_t *ent, const weaponInfo_t *wi ) {
	int		step;
	vec3_t	origin, lastPos;
	int		t;
	int		startTime, contents;
	int		lastContents;
	entityState_t	*es;
	vec3_t	up;
	localEntity_t	*smoke;

	if ( cg_noProjectileTrail.integer ) {
		return;
	}

	up[0] = 0;
	up[1] = 0;
	up[2] = 0;

	step = 50;

	es = &ent->currentState;
	startTime = ent->trailTime;
	t = step * ( (startTime + step) / step );

	BG_EvaluateTrajectory( &es->pos, cg.time, origin );
	contents = CG_PointContents( origin, -1 );

	// if object (e.g. grenade) is stationary, don't toss up smoke
	if ( es->pos.trType == TR_STATIONARY ) {
		ent->trailTime = cg.time;
		return;
	}

	BG_EvaluateTrajectory( &es->pos, ent->trailTime, lastPos );
	lastContents = CG_PointContents( lastPos, -1 );

	ent->trailTime = cg.time;

	if ( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {
		if ( contents & lastContents & CONTENTS_WATER ) {
			CG_BubbleTrail( lastPos, origin, 8 );
		}
		return;
	}

	for ( ; t <= ent->trailTime ; t += step ) {
		BG_EvaluateTrajectory( &es->pos, t, lastPos );

		smoke = CG_SmokePuff( lastPos, up, 
					  wi->trailRadius, 
					  1, 1, 1, 0.33f,
					  wi->wiTrailTime, 
					  t,
					  0,
					  0, 
					  cgs.media.nailPuffShader );
		// use the optimized local entity add
		smoke->leType = LE_SCALE_FADE;
	}

}
//#endif

/*
==========================
CG_NailTrail
==========================
*/
static void CG_OldPlasmaTrail( centity_t *cent, const weaponInfo_t *wi ) {
	localEntity_t	*le;
	refEntity_t		*re;
	entityState_t	*es;
	vec3_t			velocity, xvelocity, origin;
	vec3_t			offset, xoffset;
	vec3_t			v[3];
	int				t, startTime, step;

	float	waterScale = 1.0f;

	if ( cg_noProjectileTrail.integer || cg_oldPlasma.integer ) {
		return;
	}

	step = 50;

	es = &cent->currentState;
	startTime = cent->trailTime;
	t = step * ( (startTime + step) / step );

	BG_EvaluateTrajectory( &es->pos, cg.time, origin );

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	velocity[0] = 60 - 120 * crandom();
	velocity[1] = 40 - 80 * crandom();
	velocity[2] = 100 - 200 * crandom();

	le->leType = LE_MOVE_SCALE_FADE;
	le->leFlags = LEF_TUMBLE;
	le->leBounceSoundType = LEBS_NONE;
	le->leMarkType = LEMT_NONE;

	le->startTime = cg.time;
	le->endTime = le->startTime + 600;

	le->pos.trType = TR_GRAVITY;
	le->pos.trTime = cg.time;

	AnglesToAxis( cent->lerpAngles, v );

	offset[0] = 2;
	offset[1] = 2;
	offset[2] = 2;

	xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
	xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
	xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];

	VectorAdd( origin, xoffset, re->origin );
	VectorCopy( re->origin, le->pos.trBase );

	if ( CG_PointContents( re->origin, -1 ) & CONTENTS_WATER ) {
		waterScale = 0.10f;
	}

	xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
	xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
	xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];
	VectorScale( xvelocity, waterScale, le->pos.trDelta );

	AxisCopy( axisDefault, re->axis );
    re->shaderTime = cg.time / 1000.0f;
    re->reType = RT_SPRITE;
    re->radius = 0.25f;
	re->customShader = cgs.media.railRingsShader;
	le->bounceFactor = 0.3f;


    re->shaderRGBA[0] = wi->flashDlightColor[0] * 63;
    re->shaderRGBA[1] = wi->flashDlightColor[1] * 63;
    re->shaderRGBA[2] = wi->flashDlightColor[2] * 63;
    re->shaderRGBA[3] = 63;

    le->color[0] = wi->flashDlightColor[0] * 0.2;
    le->color[1] = wi->flashDlightColor[1] * 0.2;
    le->color[2] = wi->flashDlightColor[2] * 0.2;
    le->color[3] = 0.25f;

	le->angles.trType = TR_LINEAR;
	le->angles.trTime = cg.time;
	le->angles.trBase[0] = rand()&31;
	le->angles.trBase[1] = rand()&31;
	le->angles.trBase[2] = rand()&31;
	le->angles.trDelta[0] = 1;
	le->angles.trDelta[1] = 0.5;
	le->angles.trDelta[2] = 0;

}
/*
==========================
CG_GrappleTrail
==========================
*/
void CG_GrappleTrail( centity_t *ent, const weaponInfo_t *wi ) {
	vec3_t	origin;
	entityState_t	*es;
	vec3_t			forward, up;
	refEntity_t		beam;

	es = &ent->currentState;

	BG_EvaluateTrajectory( &es->pos, cg.time, origin );
	ent->trailTime = cg.time;

	memset( &beam, 0, sizeof( beam ) );
	//FIXME adjust for muzzle position
	VectorCopy ( cg_entities[ ent->currentState.otherEntityNum ].lerpOrigin, beam.origin );
	beam.origin[2] += 26;
	AngleVectors( cg_entities[ ent->currentState.otherEntityNum ].lerpAngles, forward, NULL, up );
	VectorMA( beam.origin, -6, up, beam.origin );
	VectorCopy( origin, beam.oldorigin );

	if (Distance( beam.origin, beam.oldorigin ) < 64 )
		return; // Don't draw if close

	beam.reType = RT_RAIL_CORE;
	beam.customShader = cgs.media.grappleShader;

	AxisClear( beam.axis );
	beam.shaderRGBA[0] = 0xff;
	beam.shaderRGBA[1] = 0xff;
	beam.shaderRGBA[2] = 0xff;
	beam.shaderRGBA[3] = 0xff;
	trap_R_AddRefEntityToScene( &beam );
}

/*
==========================
CG_GrenadeTrail
==========================
*/
// LEILEI enhancment
static void CG_RocketTrail( centity_t *ent, const weaponInfo_t *wi ) {

		if (cg_leiEnhancement.integer) {
			CG_LeiSmokeTrail( ent, wi );
		}
		else
		{	
			CG_OldRocketTrail( ent, wi );
		}
}

static void CG_PlasmaTrail( centity_t *ent, const weaponInfo_t *wi ) {

		if (cg_leiEnhancement.integer) {
			CG_LeiPlasmaTrail( ent, wi );
		}
		else
		{	
			CG_OldPlasmaTrail( ent, wi );
		}
}


static void CG_GrenadeTrail( centity_t *ent, const weaponInfo_t *wi ) {
	CG_RocketTrail( ent, wi );
}

	




/*
=================
CG_RegisterWeapon

The server says this item is used on this level
=================
*/
void CG_RegisterWeapon( int weaponNum ) {
	weaponInfo_t	*weaponInfo;
	gitem_t			*item, *ammo;
	char			path[MAX_QPATH];
	vec3_t			mins, maxs;
	int				i;

	weaponInfo = &cg_weapons[weaponNum];

	if ( weaponNum == 0 ) {
		return;
	}

	if ( weaponInfo->registered ) {
		return;
	}

	memset( weaponInfo, 0, sizeof( *weaponInfo ) );
	weaponInfo->registered = qtrue;

	for ( item = bg_itemlist + 1 ; item->classname ; item++ ) {
		if ( item->giType == IT_WEAPON && item->giTag == weaponNum ) {
			weaponInfo->item = item;
			break;
		}
	}
	if ( !item->classname ) {
		CG_Error( "Couldn't find weapon %i", weaponNum );
	}
	CG_RegisterItemVisuals( item - bg_itemlist );

	// load cmodel before model so filecache works
	weaponInfo->weaponModel = trap_R_RegisterModel( item->world_model[0] );

	// calc midpoint for rotation
	trap_R_ModelBounds( weaponInfo->weaponModel, mins, maxs );
	for ( i = 0 ; i < 3 ; i++ ) {
		weaponInfo->weaponMidpoint[i] = mins[i] + 0.5 * ( maxs[i] - mins[i] );
	}

	weaponInfo->weaponIcon = trap_R_RegisterShader( item->icon );
	weaponInfo->ammoIcon = trap_R_RegisterShader( item->icon );

	for ( ammo = bg_itemlist + 1 ; ammo->classname ; ammo++ ) {
		if ( ammo->giType == IT_AMMO && ammo->giTag == weaponNum ) {
			break;
		}
	}
	if ( ammo->classname && ammo->world_model[0] ) {
		weaponInfo->ammoModel = trap_R_RegisterModel( ammo->world_model[0] );
	}

	Q_strncpyz( path, item->world_model[0], MAX_QPATH );
	COM_StripExtension(path, path, sizeof(path));
	strcat( path, "_flash.md3" );
	weaponInfo->flashModel = trap_R_RegisterModel( path );

	Q_strncpyz( path, item->world_model[0], MAX_QPATH );
	COM_StripExtension(path, path, sizeof(path));
	strcat( path, "_barrel.md3" );
	weaponInfo->barrelModel = trap_R_RegisterModel( path );

	Q_strncpyz( path, item->world_model[0], MAX_QPATH );
	COM_StripExtension(path, path, sizeof(path));
	strcat( path, "_hand.md3" );
	weaponInfo->handsModel = trap_R_RegisterModel( path );

	if ( !weaponInfo->handsModel ) {
		weaponInfo->handsModel = trap_R_RegisterModel( "models/weapons2/shotgun/shotgun_hand.md3" );
	}

	weaponInfo->loopFireSound = qfalse;

	switch ( weaponNum ) {
	case WP_GAUNTLET:
		MAKERGB( weaponInfo->flashDlightColor, 0.6f, 0.6f, 1.0f );
		weaponInfo->firingSound = trap_S_RegisterSound( "sound/weapons/melee/fstrun.wav", qfalse );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/melee/fstatck.wav", qfalse );
		break;

	case WP_LIGHTNING:
		MAKERGB( weaponInfo->flashDlightColor, 0.6f, 0.6f, 1.0f );
		weaponInfo->readySound = trap_S_RegisterSound( "sound/weapons/melee/fsthum.wav", qfalse );
		weaponInfo->firingSound = trap_S_RegisterSound( "sound/weapons/lightning/lg_hum.wav", qfalse );

		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/lightning/lg_fire.wav", qfalse );
		cgs.media.lightningShader = trap_R_RegisterShader( "lightningBoltNew");
		cgs.media.llightninghalo = trap_R_RegisterShader( "leilightninghalo");	// leilei
		cgs.media.llightningsubbeam = trap_R_RegisterShader( "leilightningsubbeam");	// leilei
		cgs.media.llightningbeam = trap_R_RegisterShader( "leilightningbeam");	// leilei
		cgs.media.lightningExplosionModel = trap_R_RegisterModel( "models/weaphits/crackle.md3" );
		cgs.media.sfx_lghit1 = trap_S_RegisterSound( "sound/weapons/lightning/lg_hit.wav", qfalse );
		cgs.media.sfx_lghit2 = trap_S_RegisterSound( "sound/weapons/lightning/lg_hit2.wav", qfalse );
		cgs.media.sfx_lghit3 = trap_S_RegisterSound( "sound/weapons/lightning/lg_hit3.wav", qfalse );

		break;

	case WP_GRAPPLING_HOOK:
		MAKERGB( weaponInfo->flashDlightColor, 0.6f, 0.6f, 1.0f );
		weaponInfo->missileModel = trap_R_RegisterModel( "models/ammo/hook/hook.md3" );
		weaponInfo->missileTrailFunc = CG_GrappleTrail;
		weaponInfo->missileDlight = 0;
		weaponInfo->wiTrailTime = 2000;
		weaponInfo->trailRadius = 64;
		MAKERGB( weaponInfo->missileDlightColor, 1, 0.75f, 0 );
		cgs.media.grappleShader = trap_R_RegisterShader( "grappleRope");
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/grapple/grapfire.wav", qfalse );
		weaponInfo->missileSound = trap_S_RegisterSound( "sound/weapons/grapple/grappull.wav", qfalse );
                //cgs.media.lightningShader = trap_R_RegisterShader( "lightningBoltNew");
		break;

//#ifdef MISSIONPACK
	case WP_CHAINGUN:
		weaponInfo->firingSound = trap_S_RegisterSound( "sound/weapons/vulcan/wvulfire.wav", qfalse );
		weaponInfo->loopFireSound = qtrue;
		MAKERGB( weaponInfo->flashDlightColor, 1, 1, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/vulcan/vulcanf1b.wav", qfalse );
		weaponInfo->flashSound[1] = trap_S_RegisterSound( "sound/weapons/vulcan/vulcanf2b.wav", qfalse );
		weaponInfo->flashSound[2] = trap_S_RegisterSound( "sound/weapons/vulcan/vulcanf3b.wav", qfalse );
		weaponInfo->flashSound[3] = trap_S_RegisterSound( "sound/weapons/vulcan/vulcanf4b.wav", qfalse );
		weaponInfo->ejectBrassFunc = CG_MachineGunEjectBrass;
		cgs.media.bulletExplosionShader = trap_R_RegisterShader( "bulletExplosion" );
		break;
//#endif

	case WP_MACHINEGUN:
		MAKERGB( weaponInfo->flashDlightColor, 1, 1, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/machinegun/machgf1b.wav", qfalse );
		weaponInfo->flashSound[1] = trap_S_RegisterSound( "sound/weapons/machinegun/machgf2b.wav", qfalse );
		weaponInfo->flashSound[2] = trap_S_RegisterSound( "sound/weapons/machinegun/machgf3b.wav", qfalse );
		weaponInfo->flashSound[3] = trap_S_RegisterSound( "sound/weapons/machinegun/machgf4b.wav", qfalse );
		weaponInfo->ejectBrassFunc = CG_MachineGunEjectBrass;
		cgs.media.bulletExplosionShader = trap_R_RegisterShader( "bulletExplosion" );
		break;

	case WP_SHOTGUN:
		MAKERGB( weaponInfo->flashDlightColor, 1, 1, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/shotgun/sshotf1b.wav", qfalse );
		weaponInfo->ejectBrassFunc = CG_ShotgunEjectBrass;
		break;

	case WP_ROCKET_LAUNCHER:
		weaponInfo->missileModel = trap_R_RegisterModel( "models/ammo/rocket/rocket.md3" );
		weaponInfo->missileSound = trap_S_RegisterSound( "sound/weapons/rocket/rockfly.wav", qfalse );
		weaponInfo->missileTrailFunc = CG_RocketTrail;
		weaponInfo->missileDlight = 200;
		weaponInfo->wiTrailTime = 2000;
		weaponInfo->trailRadius = 64;
		
		MAKERGB( weaponInfo->missileDlightColor, 1, 0.75f, 0 );
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.75f, 0 );

		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/rocket/rocklf1a.wav", qfalse );
		cgs.media.rocketExplosionShader = trap_R_RegisterShader( "rocketExplosion" );
		break;

//#ifdef MISSIONPACK
	case WP_PROX_LAUNCHER:
		weaponInfo->missileModel = trap_R_RegisterModel( "models/weaphits/proxmine.md3" );
		weaponInfo->missileTrailFunc = CG_GrenadeTrail;
		weaponInfo->wiTrailTime = 700;
		weaponInfo->trailRadius = 32;
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.70f, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/proxmine/wstbfire.wav", qfalse );
		cgs.media.grenadeExplosionShader = trap_R_RegisterShader( "grenadeExplosion" );
		break;
//#endif

	case WP_GRENADE_LAUNCHER:
		weaponInfo->missileModel = trap_R_RegisterModel( "models/ammo/grenade1.md3" );
		weaponInfo->missileTrailFunc = CG_GrenadeTrail;
		weaponInfo->wiTrailTime = 700;
		weaponInfo->trailRadius = 32;
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.70f, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/grenade/grenlf1a.wav", qfalse );
		cgs.media.grenadeExplosionShader = trap_R_RegisterShader( "grenadeExplosion" );
		break;

//#ifdef MISSIONPACK
	case WP_NAILGUN:
		weaponInfo->ejectBrassFunc = CG_NailgunEjectBrass;
		weaponInfo->missileTrailFunc = CG_NailTrail;
//		weaponInfo->missileSound = trap_S_RegisterSound( "sound/weapons/nailgun/wnalflit.wav", qfalse );
		weaponInfo->trailRadius = 16;
		weaponInfo->wiTrailTime = 250;
		weaponInfo->missileModel = trap_R_RegisterModel( "models/weaphits/nail.md3" );
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.75f, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/nailgun/wnalfire.wav", qfalse );
		break;
//#endif

	case WP_PLASMAGUN:
//		weaponInfo->missileModel = cgs.media.invulnerabilityPowerupModel;
		weaponInfo->missileTrailFunc = CG_PlasmaTrail;
		weaponInfo->missileSound = trap_S_RegisterSound( "sound/weapons/plasma/lasfly.wav", qfalse );
		MAKERGB( weaponInfo->flashDlightColor, 0.6f, 0.6f, 1.0f );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/plasma/hyprbf1a.wav", qfalse );
		cgs.media.plasmaExplosionShader = trap_R_RegisterShader( "plasmaExplosion" );
		cgs.media.railRingsShader = trap_R_RegisterShader( "railDisc" );
		break;

	case WP_RAILGUN:
		weaponInfo->readySound = trap_S_RegisterSound( "sound/weapons/railgun/rg_hum.wav", qfalse );
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.5f, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/railgun/railgf1a.wav", qfalse );
		cgs.media.railExplosionShader = trap_R_RegisterShader( "railExplosion" );
		cgs.media.railRingsShader = trap_R_RegisterShader( "railDisc" );
		cgs.media.railCoreShader = trap_R_RegisterShader( "railCore" );
		break;

	case WP_BFG:
		weaponInfo->readySound = trap_S_RegisterSound( "sound/weapons/bfg/bfg_hum.wav", qfalse );
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.7f, 1 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/bfg/bfg_fire.wav", qfalse );
		cgs.media.bfgExplosionShader = trap_R_RegisterShader( "bfgExplosion" );
		weaponInfo->missileModel = trap_R_RegisterModel( "models/weaphits/bfg.md3" );
		weaponInfo->missileSound = trap_S_RegisterSound( "sound/weapons/rocket/rockfly.wav", qfalse );
		break;

	 default:
		MAKERGB( weaponInfo->flashDlightColor, 1, 1, 1 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/rocket/rocklf1a.wav", qfalse );
		break;
	}
}

/*
=================
CG_RegisterItemVisuals

The server says this item is used on this level
=================
*/
void CG_RegisterItemVisuals( int itemNum ) {
	itemInfo_t		*itemInfo;
	gitem_t			*item;

	if ( itemNum < 0 || itemNum >= bg_numItems ) {
		CG_Error( "CG_RegisterItemVisuals: itemNum %d out of range [0-%d]", itemNum, bg_numItems-1 );
	}

	itemInfo = &cg_items[ itemNum ];
	if ( itemInfo->registered ) {
		return;
	}

	item = &bg_itemlist[ itemNum ];

	memset( itemInfo, 0, sizeof( &itemInfo ) );
	itemInfo->registered = qtrue;

	itemInfo->models[0] = trap_R_RegisterModel( item->world_model[0] );

	itemInfo->icon = trap_R_RegisterShader( item->icon );

	if ( item->giType == IT_WEAPON ) {
		CG_RegisterWeapon( item->giTag );
	}

	//
	// powerups have an accompanying ring or sphere
	//
	if ( item->giType == IT_POWERUP || item->giType == IT_HEALTH || 
		item->giType == IT_ARMOR || item->giType == IT_HOLDABLE ) {
		if ( item->world_model[1] ) {
			itemInfo->models[1] = trap_R_RegisterModel( item->world_model[1] );
		}
	}
}


/*
========================================================================================

VIEW WEAPON

========================================================================================
*/

/*
=================
CG_MapTorsoToWeaponFrame

=================
*/
static int CG_MapTorsoToWeaponFrame( clientInfo_t *ci, int frame ) {

	// change weapon
	if ( frame >= ci->animations[TORSO_DROP].firstFrame 
		&& frame < ci->animations[TORSO_DROP].firstFrame + 9 ) {
		return frame - ci->animations[TORSO_DROP].firstFrame + 6;
	}

	// stand attack
	if ( frame >= ci->animations[TORSO_ATTACK].firstFrame 
		&& frame < ci->animations[TORSO_ATTACK].firstFrame + 6 ) {
		return 1 + frame - ci->animations[TORSO_ATTACK].firstFrame;
	}

	// stand attack 2
	if ( frame >= ci->animations[TORSO_ATTACK2].firstFrame 
		&& frame < ci->animations[TORSO_ATTACK2].firstFrame + 6 ) {
		return 1 + frame - ci->animations[TORSO_ATTACK2].firstFrame;
	}
	
	return 0;
}

// Stuff from Darkplaces
#define bound(min,num,max) (num >= min ? (num < max ? num : max) : min)
static vec_t lowpass(vec_t value, vec_t frac, vec_t *store)
{
	frac = bound(0, frac, 1);
	return (*store = *store * (1 - frac) + value * frac);
}

static vec_t lowpass_limited(vec_t value, vec_t frac, vec_t limit, vec_t *store)
{
	lowpass(value, frac, store);
	return (*store = bound(value - limit, *store, value + limit));
}

static vec_t highpass(vec_t value, vec_t frac, vec_t *store)
{
	return value - lowpass(value, frac, store);
}

static vec_t highpass_limited(vec_t value, vec_t frac, vec_t limit, vec_t *store)
{
	return value - lowpass_limited(value, frac, limit, store);
}

static void lowpass3(vec3_t value, vec_t fracx, vec_t fracy, vec_t fracz, vec3_t store, vec3_t out)
{
	out[0] = lowpass(value[0], fracx, &store[0]);
	out[1] = lowpass(value[1], fracy, &store[1]);
	out[2] = lowpass(value[2], fracz, &store[2]);
}

static void highpass3(vec3_t value, vec_t fracx, vec_t fracy, vec_t fracz, vec3_t store, vec3_t out)
{
	out[0] = highpass(value[0], fracx, &store[0]);
	out[1] = highpass(value[1], fracy, &store[1]);
	out[2] = highpass(value[2], fracz, &store[2]);
}

static void highpass3_limited(vec3_t value, vec_t fracx, vec_t limitx, vec_t fracy, vec_t limity, vec_t fracz, vec_t limitz, vec3_t store, vec3_t out)
{
	out[0] = highpass_limited(value[0], fracx, limitx, &store[0]);
	out[1] = highpass_limited(value[1], fracy, limity, &store[1]);
	out[2] = highpass_limited(value[2], fracz, limitz, &store[2]);
}

//end stuff from Darkplaces
/*
==============
CG_CalculateWeaponPosition
==============
*/
vec3_t	gunorg_prev;

static void CG_CalculateWeaponPosition( vec3_t origin, vec3_t angles ) {
	float	scale;
	int		delta;
	float	fracsin;

	VectorCopy( cg.refdef.vieworg, origin );
	VectorCopy( cg.refdefViewAngles, angles );

	VectorCopy( cg.refdef.vieworg, gunorg_prev );

	// on odd legs, invert some angles
	if ( cg.bobcycle & 1 ) {
		scale = -cg.xyspeed;
	} else {
		scale = cg.xyspeed;
	}

	// gun angles from bobbing


// Weapon leaning and crap from Darkplaces
/*
	{
		vec3_t		gunorg_hp, gunorg_lp;
		float		side_hp1 = 0.4;
		float		side_hp2 = 0.4;
		float		side_hp3 = 0.4;
		float		side_limit = 1;

		float		up_hp1 = 0.4;
		float		up_hp2 = 0.4;
		float		up_hp3 = 0.4;
		float		up_limit = 1;
		VectorAdd(gunorg_hp, gunorg_prev, gunorg_hp);

		highpass3_limited(cg.refdef.vieworg, cg.time*(int)side_hp1, (int)side_limit, cg.time*(int)side_hp1, (int)side_limit, cg.time*(int)up_hp1, (int)up_limit, gunorg_hp, origin);
		VectorCopy(origin, gunorg_prev);
		VectorSubtract(gunorg_hp, gunorg_prev, gunorg_hp);
		VectorCopy(cg.refdef.vieworg, origin);

		VectorAdd(gunorg_hp, gunorg_prev, gunorg_hp);
		highpass3_limited(cg.refdef.vieworg, cg.time*(int)side_hp1, (int)side_limit, cg.time*(int)side_hp1, (int)side_limit, cg.time*(int)up_hp1, (int)up_limit, gunorg_hp, origin);
		VectorCopy(origin, gunorg_prev);
		VectorSubtract(gunorg_hp, gunorg_prev, gunorg_hp);
	}
*/
	
/*
	{
		vec3_t		gunorg_hp, gunorg_lp;
		float		side_hp1 = 10.4;
		float		side_hp2 = 10.4;
		float		side_hp3 = 10.4;
		float		side_limit = 1;

		float		up_hp1 = 0.4;
		float		up_hp2 = 0.4;
		float		up_hp3 = 0.4;
		float		up_limit = 1;

		VectorCopy(origin, gunorg_prev);

		highpass3_limited(cg.refdef.vieworg, cg.time*(int)side_hp1, (int)side_limit, cg.time*(int)side_hp1, (int)side_limit, cg.time*(int)up_hp1, (int)up_limit, gunorg_hp, origin);
		VectorCopy(origin, gunorg_prev);
		VectorSubtract(gunorg_hp, gunorg_prev, gunorg_hp);
	//	VectorCopy(gunorg_prev, origin);
	//	VectorCopy(cg.refdef.vieworg, origin);
	//	VectorAdd(gunorg_hp, origin);


	}
*/

	// Engoo bobbing port
	if (cg_bobmodel.integer)
	{
		vec3_t		forward, right, up;
		float		bob;
		float 		s =  cg.time * scale * 0.001;
		//float		sinsin;
	
		//sinsin = fabs( sin( ( ps->bobCycle & 127 ) / 127.0 * M_PI ) );
		AngleVectors (angles, forward, right, up);
	
	
		// Arc 1
		if (cg_bobmodel.integer == 1){
		bob = scale * 2 * 0.05 * cg.bobfracsin * 0.04;
		VectorMA (origin, bob, right, origin);
	
		bob = cos(scale * 0.07 * cg.bobfracsin * 0.05) - cos(scale * 0.07 * cg.bobfracsin * 0.1);
		VectorMA (origin, bob, up, origin);
		}
	
		// Thrust
		if (cg_bobmodel.integer == 2){
	
		bob = scale * 2 * 0.05 * cg.bobfracsin * 0.04;
		VectorMA (origin, bob, forward, origin);
	
	
		}


		if (cg_bobmodel.integer == 3){
		bob = scale * 2 * 0.05 * cg.bobfracsin * 0.04;
		VectorMA (origin, bob, right, origin);
	
		bob = cos(scale * 0.07 * cg.bobfraccos * 0.05) - sin(scale * 0.07 * cg.bobfracsin * 0.1);
		VectorMA (origin, bob, up, origin);
		}

	}
	else
	{
		angles[ROLL] += scale * cg.bobfracsin * 0.005;
		angles[YAW] += scale * cg.bobfracsin * 0.01;
		angles[PITCH] += cg.xyspeed * cg.bobfracsin * 0.005;
	}
	
	// leilei - fudgeweapon ported directly from quake :D

	if (cg_viewnudge.integer){

		if (cg_viewsize.integer== 110)
			origin[2] += 1;
		else if (cg_viewsize.integer == 100)
			origin[2] += 2;
		else if (cg_viewsize.integer == 90)
			origin[2] += 1;
		else if (cg_viewsize.integer == 80)
			origin[2] += 0.5;

		}


	// drop the weapon when landing
	delta = cg.time - cg.landTime;
	if ( delta < LAND_DEFLECT_TIME ) {
		origin[2] += cg.landChange*0.25 * delta / LAND_DEFLECT_TIME;
	} else if ( delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME ) {
		origin[2] += cg.landChange*0.25 * 
			(LAND_DEFLECT_TIME + LAND_RETURN_TIME - delta) / LAND_RETURN_TIME;
	}

#if 0
	// drop the weapon when stair climbing
	delta = cg.time - cg.stepTime;
	if ( delta < STEP_TIME/2 ) {
		origin[2] -= cg.stepChange*0.25 * delta / (STEP_TIME/2);
	} else if ( delta < STEP_TIME ) {
		origin[2] -= cg.stepChange*0.25 * (STEP_TIME - delta) / (STEP_TIME/2);
	}
#endif

	// idle drift
	if (!cg_bobmodel.integer){
	scale = cg.xyspeed + 40;
	fracsin = sin( cg.time * 0.001 );
	angles[ROLL] += scale * fracsin * 0.01;
	angles[YAW] += scale * fracsin * 0.01;
	angles[PITCH] += scale * fracsin * 0.01;
	}



	
	
}


/*
===============
CG_LightningBolt

Origin will be the exact tag point, which is slightly
different than the muzzle point used for determining hits.
The cent should be the non-predicted cent if it is from the player,
so the endpoint will reflect the simulated strike (lagging the predicted
angle)
===============
*/

/*
===============
CG_LightningHalo

a rough glow effect for the lightning beam to give it a more exaggerated feeling
===============
*/
void CG_LightningHalo( vec3_t source, vec3_t dest ) {
	vec3_t		forward, right;
	polyVert_t	verts[4];
	vec3_t		line;
	float		len, begin, end;
	vec3_t		start, finish;
	vec3_t		midpoint;
	float 		halowidth;
	vec3_t		colar;
	// tracer
	VectorSubtract( dest, source, forward );
	len = VectorNormalize( forward );


	colar[0] = 26;
	colar[1] = 50;
	colar[2] = 65;

	halowidth = 20 + crandom() * 6;
	// start at least a little ways from the muzzle
	if ( len < 0 ) {
		return;
	}
//	begin = 50 * (len - 60);
	begin = 0;

	end = len;
//	if ( end > len ) {
//		end = len;
//	}
	VectorMA( source, begin, forward, start );
	VectorMA( source, end, forward, finish );

	line[0] = DotProduct( forward, cg.refdef.viewaxis[1] );
	line[1] = DotProduct( forward, cg.refdef.viewaxis[2] );

	VectorScale( cg.refdef.viewaxis[1], line[1], right );
	VectorMA( right, -line[0], cg.refdef.viewaxis[2], right );
	VectorNormalize( right );

	VectorMA( finish, halowidth, right, verts[0].xyz );
	verts[0].st[0] = 0;
	verts[0].st[1] = 0;
	verts[0].modulate[0] = colar[0];
	verts[0].modulate[1] = colar[1];
	verts[0].modulate[2] = colar[2];
	verts[0].modulate[3] = 255;

	VectorMA( finish, -halowidth, right, verts[1].xyz );
	verts[1].st[0] = 0;
	verts[1].st[1] = 1;
	verts[1].modulate[0] = colar[0];
	verts[1].modulate[1] = colar[1];
	verts[1].modulate[2] = colar[2];
	verts[1].modulate[3] = 255;

	VectorMA( start, -halowidth, right, verts[2].xyz );
	verts[2].st[0] = 1;
	verts[2].st[1] = 1;
	verts[2].modulate[0] = colar[0];
	verts[2].modulate[1] = colar[1];
	verts[2].modulate[2] = colar[2];
	verts[2].modulate[3] = 255;

	VectorMA( start, halowidth, right, verts[3].xyz );
	verts[3].st[0] = 1;
	verts[3].st[1] = 0;
	verts[3].modulate[0] = colar[0];
	verts[3].modulate[1] = colar[1];
	verts[3].modulate[2] = colar[2];
	verts[3].modulate[3] = 255;

	trap_R_AddPolyToScene( cgs.media.llightninghalo, 4, verts );

}




/*
===============
CG_LightningBeam

===============
*/
void CG_LightningBeam( vec3_t source, vec3_t dest ) {
	vec3_t		forward, right;
	polyVert_t	verts[4];
	vec3_t		line;
	float		len, begin, end;
	vec3_t		start, finish;
	vec3_t		midpoint;
	float 		halowidth;
	vec3_t		colar;
	// tracer
	VectorSubtract( dest, source, forward );
	len = VectorNormalize( forward );


	colar[0] = 99;
	colar[1] = 163;
	colar[2] = 255;

	halowidth = 2;
	// start at least a little ways from the muzzle
	if ( len < 0 ) {
		return;
	}
	begin = 0;

	end = len;

	VectorMA( source, begin, forward, start );
	VectorMA( source, end, forward, finish );

	line[0] = DotProduct( forward, cg.refdef.viewaxis[1] );
	line[1] = DotProduct( forward, cg.refdef.viewaxis[2] );

	VectorScale( cg.refdef.viewaxis[1], line[1], right );
	VectorMA( right, -line[0], cg.refdef.viewaxis[2], right );
	VectorNormalize( right );

	VectorMA( finish, halowidth, right, verts[0].xyz );
	verts[0].st[0] = 0;
	verts[0].st[1] = 0;
	verts[0].modulate[0] = colar[0];
	verts[0].modulate[1] = colar[1];
	verts[0].modulate[2] = colar[2];
	verts[0].modulate[3] = 255;

	VectorMA( finish, -halowidth, right, verts[1].xyz );
	verts[1].st[0] = 0;
	verts[1].st[1] = 1;
	verts[1].modulate[0] = colar[0];
	verts[1].modulate[1] = colar[1];
	verts[1].modulate[2] = colar[2];
	verts[1].modulate[3] = 255;

	VectorMA( start, -halowidth, right, verts[2].xyz );
	verts[2].st[0] = 1;
	verts[2].st[1] = 1;
	verts[2].modulate[0] = colar[0];
	verts[2].modulate[1] = colar[1];
	verts[2].modulate[2] = colar[2];
	verts[2].modulate[3] = 255;

	VectorMA( start, halowidth, right, verts[3].xyz );
	verts[3].st[0] = 1;
	verts[3].st[1] = 0;
	verts[3].modulate[0] = colar[0];
	verts[3].modulate[1] = colar[1];
	verts[3].modulate[2] = colar[2];
	verts[3].modulate[3] = 255;

	trap_R_AddPolyToScene( cgs.media.llightningbeam, 4, verts );

}



/*
===============
CG_LightningStatic

smaller randomy zaps to go around a bolt to add
to the whole lightningy effect. Intended to be inspired by
many many many many many many big video games from the year
of 2000.  Popular because it's a way for convincing lightning
with a much smaller texture cache.
===============
*/


// Even though this is not good for performance, I want to make these some global so I can
// mutate them in another function for the sake of timing. I do not like how flickering lightning looks
// I just want the lightning bolts to slowly animate :D

float		oldrandom1[16];
float		oldrandom2[16];
float		oldrandom3[16];
vec3_t		newrandom;

float		therandom1[16];
float		therandom2[16];
float		therandom3[16];

float		literand_finished;
float		litelerp_finished;
float		literand_time;

float		newrandom1[16];
float		newrandom2[16];
float		newrandom3[16];

// Process some random lightning iterations
// TODO: Lerp this properly
void CG_LightningRandom() {

	int it;


	if (cg.time > literand_finished){
	newrandom[0] =  (crandom() * 7);
	newrandom[1] =  (crandom() * 7);
	newrandom[2] =  (crandom() * 7);

//	newrandom[0] =  (rand() & 7);
//	newrandom[1] =  (rand() & 7);
//	newrandom[2] =  (rand() & 7);


	literand_finished = cg.time + 50;
	}


//	oldrandom1[0] = 0;
//	oldrandom2[0] = 0;
//	oldrandom3[0] = 0;

	//oldrandom1[15] = 0;
//	oldrandom2[15] = 0;
//	oldrandom3[15] = 0;
	if (cg.time > litelerp_finished){
	for(it=0;it<16;it++){


	// lerp the old value	
//
	//	oldrandom1[it] = oldrandom1[it-1] + oldrandom1[it] + therandom1[it] * 0.33;
	//	oldrandom2[it] = oldrandom2[it-1] + oldrandom2[it] + therandom2[it] * 0.33;
	//	oldrandom3[it] = oldrandom3[it-1] + oldrandom3[it] + therandom3[it] * 0.33;

		oldrandom1[it] = therandom1[it] * 0.35;
		oldrandom2[it] = therandom2[it] * 0.35;
		oldrandom3[it] = therandom3[it] * 0.35;

	// make a new value

		therandom1[it] = oldrandom1[it] + newrandom[0] * 0.5;
		therandom2[it] = oldrandom2[it] + newrandom[1] * 0.5;
		therandom3[it] = oldrandom3[it] + newrandom[2] * 0.5;

	//	therandom1[it] = therandom1[it] + newrandom[0] * 0.5;
	//	therandom2[it] = therandom2[it] + newrandom[1] * 0.5;
	//	therandom3[it] = therandom3[it] + newrandom[2] * 0.5;
	



	//	therandom1[it] = crandom() * 3;
	//	therandom2[it] = crandom() * 3;
	//	therandom3[it] = crandom() * 3;

	// scroll

	//	therandom1[it+1] = therandom1[it];
	//	therandom2[it+1] = therandom2[it];
	//	therandom3[it+1] = therandom3[it];

		}
	litelerp_finished = cg.time;
	}

	// scrolling
	for(it=1;it<15;it++){

		oldrandom1[it] = oldrandom1[it+1];
		oldrandom2[it] = oldrandom2[it+1];
		oldrandom3[it] = oldrandom3[it+1];

	//	therandom1[it] = therandom1[it+1];
	//	therandom2[it] = therandom2[it+1];
	//	therandom3[it] = therandom3[it+1];

	}

}

// Fetch from our random lightning iterations
void CG_LightningRandFetch(int offs) {

	int it;

	for(it=1;it<15;it++){

		newrandom1[it] = therandom1[it + offs];
		newrandom2[it] = therandom2[it + offs];
		newrandom3[it] = therandom3[it + offs];


		}

}

void CG_LightningStatic( vec3_t source, vec3_t dest, int loose ) {
	vec3_t		forward, right, up;
	polyVert_t	verts[4];
	vec3_t		line;
	float		len, begin, end;
	float		belen, beakin, bend, beggin, drifty;
	vec3_t		start, finish;
	vec3_t		fart, inish, dinish;
	vec3_t		midpoint;
	float 		halowidth;
	vec3_t		colar;
	int		stahp, it;
	float		plz, ok;
	vec3_t		randy;
	vec3_t		oldvert, oldvert2, oldvert3, oldvert4;
	polyVert_t	oldverts[4];
	float		faed, faed2;
	float		randam;

	float		plzstop;
	// tracer
	VectorSubtract( dest, source, forward );
	len = VectorNormalize( forward );

	stahp =	16;
	colar[0] = 96;
	colar[1] = 170;
	colar[2] = 255;

	halowidth = 3;
	// start at least a little ways from the muzzle
	if ( len < 0 ) {
		return;
	}


	beakin = 0;

	bend = len;

	for(it=1;it<stahp;it++){

	if (it < 2){

	begin = 0;
	faed = 0;
	faed2 = 0.5;

	end = len / (stahp * 2);
	beggin = end;
	}
	else
	{
	begin = beggin;
	end = (len / stahp) * it;
	beggin = end;
	}
	//begin = begin;
	fart[0] = start[0];
	fart[1] = start[1];
	fart[2] = start[2];

	drifty = randy[2];



	faed = it - 4 * 0.3;
	faed2 = it - 3 * 0.3;

	if (faed > 1)
		faed = 1;
	if (faed2 > 1)
		faed2 = 1;

	if (faed < 0)
		faed = 0;
	if (faed2 < 0)
		faed2 = 0;


	inish[0] = finish[0];
	inish[1] = finish[1];
	inish[2] = finish[2];


	VectorMA( source, begin, forward, fart );
	VectorMA( source, end, forward, inish );

	if (it>1){
	fart[0] = dinish[0];
	fart[1] = dinish[1];
	fart[2] = dinish[2];

	}

	oldrandom1[0] = 0;
	oldrandom2[0] = 0;
	oldrandom3[0] = 0;

	randam = len * 0.02;



	if (it>1){
	int e;

	if (it == 15)
	plzstop = 0;
	else if (it == 14)
	plzstop = 0.2;
	else if (it == 13)
	plzstop = 0.7;
	else if (it == 12)
	plzstop = 0.8;
	else if (it == 0)
	plzstop = 0;
	else if (it == 1)
	plzstop = 0.3;
	else if (it == 2)
	plzstop = 0.7;
	else
	plzstop = 1;

	inish[0] += (oldrandom1[it-1] + newrandom1[it] * 0.5) * plzstop;
	inish[1] += (oldrandom2[it-1] + newrandom2[it] * 0.5) * plzstop;
	inish[2] += (oldrandom3[it-1] + newrandom3[it] * 0.5) * plzstop;

	oldrandom1[it] = oldrandom1[it-1] + newrandom1[it] * 0.5;
	oldrandom2[it] = oldrandom2[it-1] + newrandom2[it] * 0.5;
	oldrandom3[it] = oldrandom3[it-1] + newrandom3[it] * 0.5;
	}

	dinish[0] = inish[0];
	dinish[1] = inish[1];
	dinish[2] = inish[2];

	line[0] = DotProduct( forward, cg.refdef.viewaxis[1] );
	line[1] = DotProduct( forward, cg.refdef.viewaxis[2] );

	VectorScale( cg.refdef.viewaxis[1], line[1], right );
	VectorMA( right, -line[0], cg.refdef.viewaxis[2], right );

	VectorNormalize( right );

	VectorMA( inish, halowidth, right, verts[0].xyz );

	verts[0].st[0] = it;

	verts[0].st[1] = 0;
	verts[0].modulate[0] = colar[0] * faed;
	verts[0].modulate[1] = colar[1] * faed;
	verts[0].modulate[2] = colar[2] * faed;
	verts[0].modulate[3] = 255;

	VectorMA( inish, -halowidth, right, verts[1].xyz );
	verts[1].st[0] = it;
	verts[1].st[1] = 1;
	verts[1].modulate[0] = colar[0] * faed;
	verts[1].modulate[1] = colar[1] * faed;
	verts[1].modulate[2] = colar[2] * faed;
	verts[1].modulate[3] = 255;
	
	VectorMA( fart, -halowidth, right, verts[2].xyz );
	verts[2].st[0] = it - 1;
	verts[2].st[1] = 1;
	verts[2].modulate[0] = colar[0]  * faed2;
	verts[2].modulate[1] = colar[1]  * faed2;
	verts[2].modulate[2] = colar[2]  * faed2;
	verts[2].modulate[3] = 255;

	VectorMA( fart, halowidth, right, verts[3].xyz );

	verts[3].st[0] = it - 1;
	verts[3].st[1] = 0;
	verts[3].modulate[0] = colar[0] * faed2;
	verts[3].modulate[1] = colar[1] * faed2;
	verts[3].modulate[2] = colar[2] * faed2;
	verts[3].modulate[3] = 255;
	
	oldverts[0] = verts[0];
	oldverts[1] = verts[1];
	oldverts[2] = verts[2];
	oldverts[3] = verts[3];

	
	trap_R_AddPolyToScene( cgs.media.llightningsubbeam, 4, verts );


	//verts[0].st[0] = (stahp / it) * 0.15;
	//verts[3].st[0] = 1 - (stahp / it) * 0.15;

	}


}


static void CG_LightningBolt( centity_t *cent, vec3_t origin ) {
	trace_t  trace;
	refEntity_t  beam;
	refEntity_t  halo;				// leilei - cg_leiEnhancement 2
	vec3_t   forward;
	vec3_t   muzzlePoint, endPoint;

	if (cent->currentState.weapon != WP_LIGHTNING) {
		return;
	}

	memset( &beam, 0, sizeof( beam ) );

	if (cg_leiEnhancement.integer > 1)
	memset( &halo, 0, sizeof( halo ) );

//unlagged - attack prediction #1
	// if the entity is us, unlagged is on server-side, and we've got it on for the lightning gun
	if ( (cent->currentState.number == cg.predictedPlayerState.clientNum) && cgs.delagHitscan &&
			( cg_delag.integer & 1 || cg_delag.integer & 8 ) ) {
		// always shoot straight forward from our current position
		AngleVectors( cg.predictedPlayerState.viewangles, forward, NULL, NULL );
		VectorCopy( cg.predictedPlayerState.origin, muzzlePoint );
	}
	else
//unlagged - attack prediction #1

	// CPMA  "true" lightning
        if ((cent->currentState.number == cg.predictedPlayerState.clientNum) && (cg_trueLightning.value != 0)) {
		vec3_t angle;
		int i;

//unlagged - true lightning
		// might as well fix up true lightning while we're at it
		vec3_t viewangles;
		VectorCopy( cg.predictedPlayerState.viewangles, viewangles );
//unlagged - true lightning

		for (i = 0; i < 3; i++) {
			float a = cent->lerpAngles[i] - cg.refdefViewAngles[i];
			if (a > 180) {
				a -= 360;
			}
			if (a < -180) {
				a += 360;
			}

			angle[i] = cg.refdefViewAngles[i] + a * (1.0 - cg_trueLightning.value);
			if (angle[i] < 0) {
				angle[i] += 360;
			}
			if (angle[i] > 360) {
				angle[i] -= 360;
			}
		}

		AngleVectors(angle, forward, NULL, NULL );
//unlagged - true lightning
//		VectorCopy(cent->lerpOrigin, muzzlePoint );
//		VectorCopy(cg.refdef.vieworg, muzzlePoint );
		// *this* is the correct origin for true lightning
		VectorCopy(cg.predictedPlayerState.origin, muzzlePoint );
//unlagged - true lightning
	} else {
		// !CPMA
		AngleVectors( cent->lerpAngles, forward, NULL, NULL );
		VectorCopy(cent->lerpOrigin, muzzlePoint );
	}

	// FIXME: crouch
	if (cg_enableQ.integer)
	muzzlePoint[2] += QUACK_VIEWHEIGHT;
	else
	muzzlePoint[2] += DEFAULT_VIEWHEIGHT;

	VectorMA( muzzlePoint, 14, forward, muzzlePoint );

	// project forward by the lightning range
	VectorMA( muzzlePoint, LIGHTNING_RANGE, forward, endPoint );

	// see if it hit a wall
	CG_Trace( &trace, muzzlePoint, vec3_origin, vec3_origin, endPoint, 
		cent->currentState.number, MASK_SHOT );

	// this is the endpoint
	VectorCopy( trace.endpos, beam.oldorigin );

	// use the provided origin, even though it may be slightly
	// different than the muzzle origin
	VectorCopy( origin, beam.origin );



	beam.reType = RT_LIGHTNING;
	beam.customShader = cgs.media.lightningShader;
	if (cg_leiEnhancement.integer < 1) 
	trap_R_AddRefEntityToScene( &beam );

	// leilei - Nice new super beam effect!
	if (cg_leiEnhancement.integer > 1) 
	{
		int eth, ath, e;
		float	len;
		vec3_t forward;
		vec3_t	wee, woo, wah;

		// 1 main bolt and its halo
		CG_LightningHalo(beam.origin, beam.oldorigin);
		CG_LightningBeam(beam.origin, beam.oldorigin);

		// 3 random subbolts	
		CG_LightningRandom();
		CG_LightningRandFetch(0);
		CG_LightningStatic(beam.origin, beam.oldorigin, 0);
		CG_LightningRandFetch(8);
		CG_LightningStatic(beam.origin, beam.oldorigin, 0);
		CG_LightningRandFetch(12);
		CG_LightningStatic(beam.origin, beam.oldorigin, 0);



	}

	// add the impact flare if it hit something
	if ( trace.fraction < 1.0 ) {
		vec3_t	angles;
		vec3_t	dir;

		VectorSubtract( beam.oldorigin, beam.origin, dir );
		VectorNormalize( dir );

		memset( &beam, 0, sizeof( beam ) );
		beam.hModel = cgs.media.lightningExplosionModel;

		VectorMA( trace.endpos, -16, dir, beam.origin );

		// make a random orientation
		angles[0] = rand() % 360;
		angles[1] = rand() % 360;
		angles[2] = rand() % 360;
		AnglesToAxis( angles, beam.axis );
		trap_R_AddRefEntityToScene( &beam );
	}
}
/*




static void CG_LightningBolt( centity_t *cent, vec3_t origin ) {
	trace_t		trace;
	refEntity_t		beam;
	vec3_t			forward;
	vec3_t			muzzlePoint, endPoint;

	if ( cent->currentState.weapon != WP_LIGHTNING ) {
		return;
	}

	memset( &beam, 0, sizeof( beam ) );

	// find muzzle point for this frame
	VectorCopy( cent->lerpOrigin, muzzlePoint );
	AngleVectors( cent->lerpAngles, forward, NULL, NULL );

	// FIXME: crouch
	if (cg_enableQ.integer)
	muzzlePoint[2] += QUACK_VIEWHEIGHT;
	else
	muzzlePoint[2] += DEFAULT_VIEWHEIGHT;

	VectorMA( muzzlePoint, 14, forward, muzzlePoint );

	// project forward by the lightning range
	VectorMA( muzzlePoint, LIGHTNING_RANGE, forward, endPoint );

	// see if it hit a wall
	CG_Trace( &trace, muzzlePoint, vec3_origin, vec3_origin, endPoint, 
		cent->currentState.number, MASK_SHOT );

	// this is the endpoint
	VectorCopy( trace.endpos, beam.oldorigin );

	// use the provided origin, even though it may be slightly
	// different than the muzzle origin
	VectorCopy( origin, beam.origin );

	beam.reType = RT_LIGHTNING;
	beam.customShader = cgs.media.lightningShader;
	trap_R_AddRefEntityToScene( &beam );

	// add the impact flare if it hit something
	if ( trace.fraction < 1.0 ) {
		vec3_t	angles;
		vec3_t	dir;

		VectorSubtract( beam.oldorigin, beam.origin, dir );
		VectorNormalize( dir );

		memset( &beam, 0, sizeof( beam ) );
		beam.hModel = cgs.media.lightningExplosionModel;

		VectorMA( trace.endpos, -16, dir, beam.origin );

		// make a random orientation
		angles[0] = rand() % 360;
		angles[1] = rand() % 360;
		angles[2] = rand() % 360;
		AnglesToAxis( angles, beam.axis );
		trap_R_AddRefEntityToScene( &beam );
	}
}
*/

/*
===============
CG_SpawnRailTrail

Origin will be the exact tag point, which is slightly
different than the muzzle point used for determining hits.
===============
*/
static void CG_SpawnRailTrail( centity_t *cent, vec3_t origin ) {
	clientInfo_t	*ci;

	if ( cent->currentState.weapon != WP_RAILGUN ) {
		return;
	}
	if ( !cent->pe.railgunFlash ) {
		return;
	}
	cent->pe.railgunFlash = qtrue;
	ci = &cgs.clientinfo[ cent->currentState.clientNum ];
	CG_RailTrail( ci, origin, cent->pe.railgunImpact );
}


/*
======================
CG_MachinegunSpinAngle
======================
*/
#define		SPIN_SPEED	0.9
#define		COAST_TIME	1000
static float	CG_MachinegunSpinAngle( centity_t *cent ) {
	int		delta;
	float	angle;
	float	speed;

	delta = cg.time - cent->pe.barrelTime;
	if ( cent->pe.barrelSpinning ) {
		angle = cent->pe.barrelAngle + delta * SPIN_SPEED;
	} else {
		if ( delta > COAST_TIME ) {
			delta = COAST_TIME;
		}

		speed = 0.5 * ( SPIN_SPEED + (float)( COAST_TIME - delta ) / COAST_TIME );
		angle = cent->pe.barrelAngle + delta * speed;
	}

	if ( cent->pe.barrelSpinning == !(cent->currentState.eFlags & EF_FIRING) ) {
		cent->pe.barrelTime = cg.time;
		cent->pe.barrelAngle = AngleMod( angle );
		cent->pe.barrelSpinning = !!(cent->currentState.eFlags & EF_FIRING);
//#ifdef MISSIONPACK
		if ( cent->currentState.weapon == WP_CHAINGUN && !cent->pe.barrelSpinning ) {
			trap_S_StartSound( NULL, cent->currentState.number, CHAN_WEAPON, trap_S_RegisterSound( "sound/weapons/vulcan/wvulwind.wav", qfalse ) );
		}
//#endif
	}

	return angle;
}


/*
========================
CG_AddWeaponWithPowerups
========================
*/
static void CG_AddWeaponWithPowerups( refEntity_t *gun, int powerups ) {
	// add powerup effects
	if ( powerups & ( 1 << PW_INVIS ) ) {
            if( (cgs.dmflags & DF_INVIS) == 0) {
		gun->customShader = cgs.media.invisShader;
		trap_R_AddRefEntityToScene( gun );
            }
	} else {
		trap_R_AddRefEntityToScene( gun );

		if ( powerups & ( 1 << PW_BATTLESUIT ) ) {
			gun->customShader = cgs.media.battleWeaponShader;
			trap_R_AddRefEntityToScene( gun );
		}
		if ( powerups & ( 1 << PW_QUAD ) ) {
			gun->customShader = cgs.media.quadWeaponShader;
			trap_R_AddRefEntityToScene( gun );
		}
	}
}


/*
=============
CG_AddPlayerWeapon

Used for both the view weapon (ps is valid) and the world modelother character models (ps is NULL)
The main player will have this called for BOTH cases, so effects like light and
sound should only be done on the world model case.
=============
*/
void CG_AddPlayerWeapon( refEntity_t *parent, playerState_t *ps, centity_t *cent, int team ) {
	refEntity_t	gun;
	refEntity_t	barrel;
	refEntity_t	flash;
	vec3_t		angles;
	weapon_t	weaponNum;
	weaponInfo_t	*weapon;
	centity_t	*nonPredictedCent;
	orientation_t	lerped;

	weaponNum = cent->currentState.weapon;

	CG_RegisterWeapon( weaponNum );
	weapon = &cg_weapons[weaponNum];

	// add the weapon
	memset( &gun, 0, sizeof( gun ) );
	VectorCopy( parent->lightingOrigin, gun.lightingOrigin );
	gun.shadowPlane = parent->shadowPlane;
	gun.renderfx = parent->renderfx;

	// set custom shading for railgun refire rate
	if ( ps || cent->currentState.clientNum == cg.predictedPlayerState.clientNum ) {
		if ( cg.predictedPlayerState.weapon == WP_RAILGUN 
			&& cg.predictedPlayerState.weaponstate == WEAPON_FIRING ) {
			float	f;

			f = (float)cg.predictedPlayerState.weaponTime / 1500;
			gun.shaderRGBA[1] = 0;
			gun.shaderRGBA[0] = 
			gun.shaderRGBA[2] = 255 * ( 1.0 - f );
		} else {
			gun.shaderRGBA[0] = 255;
			gun.shaderRGBA[1] = 255;
			gun.shaderRGBA[2] = 255;
			gun.shaderRGBA[3] = 255;
		}
	}

	gun.hModel = weapon->weaponModel;
	if (!gun.hModel) {
		return;
	}

	if ( !ps ) {
		// add weapon ready sound
		cent->pe.lightningFiring = qfalse;
		if ( ( cent->currentState.eFlags & EF_FIRING ) && weapon->firingSound ) {
			// lightning gun and guantlet make a different sound when fire is held down
			trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, weapon->firingSound );
			cent->pe.lightningFiring = qtrue;
		} else if ( weapon->readySound ) {
			trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, weapon->readySound );
		}
	}

	trap_R_LerpTag(&lerped, parent->hModel, parent->oldframe, parent->frame,
		1.0 - parent->backlerp, "tag_weapon");
	VectorCopy(parent->origin, gun.origin);

	VectorMA(gun.origin, lerped.origin[0], parent->axis[0], gun.origin);

	// Make weapon appear left-handed for 2 and centered for 3
	if(ps && cg_drawGun.integer == 2)
		VectorMA(gun.origin, -lerped.origin[1], parent->axis[1], gun.origin);
	else if(!ps || cg_drawGun.integer != 3)
       	VectorMA(gun.origin, lerped.origin[1], parent->axis[1], gun.origin);

	VectorMA(gun.origin, lerped.origin[2], parent->axis[2], gun.origin);

	MatrixMultiply(lerped.axis, ((refEntity_t *)parent)->axis, gun.axis);
	gun.backlerp = parent->backlerp;

	CG_AddWeaponWithPowerups( &gun, cent->currentState.powerups );

	// add the spinning barrel
	if ( weapon->barrelModel ) {
		memset( &barrel, 0, sizeof( barrel ) );
		VectorCopy( parent->lightingOrigin, barrel.lightingOrigin );
		barrel.shadowPlane = parent->shadowPlane;
		barrel.renderfx = parent->renderfx;

		barrel.hModel = weapon->barrelModel;
		angles[YAW] = 0;
		angles[PITCH] = 0;
		angles[ROLL] = CG_MachinegunSpinAngle( cent );
		AnglesToAxis( angles, barrel.axis );

		CG_PositionRotatedEntityOnTag( &barrel, &gun, weapon->weaponModel, "tag_barrel" );

		CG_AddWeaponWithPowerups( &barrel, cent->currentState.powerups );
	}

	// make sure we aren't looking at cg.predictedPlayerEntity for LG
	nonPredictedCent = &cg_entities[cent->currentState.clientNum];

	// if the index of the nonPredictedCent is not the same as the clientNum
	// then this is a fake player (like on teh single player podiums), so
	// go ahead and use the cent
	if( ( nonPredictedCent - cg_entities ) != cent->currentState.clientNum ) {
		nonPredictedCent = cent;
	}

	// add the flash
	if ( ( weaponNum == WP_LIGHTNING || weaponNum == WP_GAUNTLET || weaponNum == WP_GRAPPLING_HOOK )
		&& ( nonPredictedCent->currentState.eFlags & EF_FIRING ) ) 
	{
		// continuous flash
	} else {
		// impulse flash
		if ( cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_TIME && !cent->pe.railgunFlash ) {
			return;
		}
	}

	memset( &flash, 0, sizeof( flash ) );
	VectorCopy( parent->lightingOrigin, flash.lightingOrigin );
	flash.shadowPlane = parent->shadowPlane;
	flash.renderfx = parent->renderfx;

	flash.hModel = weapon->flashModel;
	if (!flash.hModel) {
		return;
	}
	angles[YAW] = 0;
	angles[PITCH] = 0;
	angles[ROLL] = crandom() * 10;
	AnglesToAxis( angles, flash.axis );

	// colorize the railgun blast
	if ( weaponNum == WP_RAILGUN ) {
		clientInfo_t	*ci;

		ci = &cgs.clientinfo[ cent->currentState.clientNum ];
		flash.shaderRGBA[0] = 255 * ci->color1[0];
		flash.shaderRGBA[1] = 255 * ci->color1[1];
		flash.shaderRGBA[2] = 255 * ci->color1[2];
	}

	CG_PositionRotatedEntityOnTag( &flash, &gun, weapon->weaponModel, "tag_flash");
	trap_R_AddRefEntityToScene( &flash );

	if ( ps || cg.renderingThirdPerson ||
		cent->currentState.number != cg.predictedPlayerState.clientNum ) {
		// add lightning bolt
		CG_LightningBolt( nonPredictedCent, flash.origin );

		// add rail trail
		CG_SpawnRailTrail( cent, flash.origin );

		if ( weapon->flashDlightColor[0] || weapon->flashDlightColor[1] || weapon->flashDlightColor[2] ) {
			trap_R_AddLightToScene( flash.origin, 300 + (rand()&31), weapon->flashDlightColor[0],
				weapon->flashDlightColor[1], weapon->flashDlightColor[2] );
		}
	}
}

/*
==============
CG_AddViewWeapon

Add the weapon, and flash for the player's view
==============
*/
void CG_AddViewWeapon( playerState_t *ps ) {
	refEntity_t	hand;
	centity_t	*cent;
	clientInfo_t	*ci;
	float		fovOffset;
	vec3_t		angles;
	weaponInfo_t	*weapon;

	// leilei test

	//refEntity_t	torso;
	//centity_t	*centorso;

	//ci->headModel
	if ( ps->persistant[PERS_TEAM] == TEAM_SPECTATOR ) {
		return;
	}

	if ( ps->pm_type == PM_INTERMISSION ) {
		return;
	}

	// no gun if in third person view or a camera is active
	//if ( cg.renderingThirdPerson || cg.cameraMode) {
	if ( cg.renderingThirdPerson ) {
		return;
	}


	// allow the gun to be completely removed
	if ( !cg_drawGun.integer ) {
		vec3_t		origin;

		if ( cg.predictedPlayerState.eFlags & EF_FIRING ) {
			// special hack for lightning gun...
			VectorCopy( cg.refdef.vieworg, origin );
			VectorMA( origin, -8, cg.refdef.viewaxis[2], origin );
			CG_LightningBolt( &cg_entities[ps->clientNum], origin );
		}
		return;
	}

	// don't draw if testing a gun model
	if ( cg.testGun ) {
		return;
	}

	// drop gun lower at higher fov
	if ( cg_fov.integer > 90 ) {
		fovOffset = -0.2 * ( cg_fov.integer - 90 );
	} else {
		fovOffset = 0;
	}

	cent = &cg.predictedPlayerEntity;	// &cg_entities[cg.snap->ps.clientNum];
	CG_RegisterWeapon( ps->weapon );
	weapon = &cg_weapons[ ps->weapon ];

	memset (&hand, 0, sizeof(hand));

	// set up gun position
	CG_CalculateWeaponPosition( hand.origin, angles );

	VectorMA( hand.origin, cg_gun_x.value, cg.refdef.viewaxis[0], hand.origin );
	VectorMA( hand.origin, cg_gun_y.value, cg.refdef.viewaxis[1], hand.origin );
	VectorMA( hand.origin, (cg_gun_z.value+fovOffset), cg.refdef.viewaxis[2], hand.origin );

	AnglesToAxis( angles, hand.axis );

	// map torso animations to weapon animations
	if ( cg_gun_frame.integer ) {
		// development tool
		hand.frame = hand.oldframe = cg_gun_frame.integer;
		hand.backlerp = 0;
	} else {
		// get clientinfo for animation map
		ci = &cgs.clientinfo[ cent->currentState.clientNum ];
		hand.frame = CG_MapTorsoToWeaponFrame( ci, cent->pe.torso.frame );
		hand.oldframe = CG_MapTorsoToWeaponFrame( ci, cent->pe.torso.oldFrame );
		hand.backlerp = cent->pe.torso.backlerp;
	}

	hand.hModel = weapon->handsModel;
	hand.renderfx = RF_DEPTHHACK | RF_FIRST_PERSON | RF_MINLIGHT;

	// add everything onto the hand
	CG_AddPlayerWeapon( &hand, ps, &cg.predictedPlayerEntity, ps->persistant[PERS_TEAM] );
}

/*
==============================================================================

WEAPON SELECTION

==============================================================================
*/

/*
===================
CG_DrawWeaponSelect
===================
*/
void CG_DrawWeaponSelect( void ) {
	int		i;
	int		bits;
	int		count;
	float		*color;
	vec4_t		realColor; 
	
	

	// don't display if dead
	if ( cg.predictedPlayerState.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	color = CG_FadeColor( cg.weaponSelectTime, WEAPON_SELECT_TIME );

	//Elimination: Always show weapon bar
	if(cg_alwaysWeaponBar.integer) {
		realColor[0] = 1.0;
		realColor[1] = 1.0;
		realColor[2] = 1.0;
		realColor[3] = 1.0;
		color = realColor;
	}

	if ( !color ) {
		return;
	}
	trap_R_SetColor( color );

	// showing weapon select clears pickup item display, but not the blend blob
	cg.itemPickupTime = 0;

	// count the number of weapons owned
	bits = cg.snap->ps.stats[ STAT_WEAPONS ];
	count = 0;
	for ( i = 1 ; i < MAX_WEAPONS ; i++ ) {
		if ( bits & ( 1 << i ) ) {
			count++;
		}
	}
	
	switch(cg_weaponBarStyle.integer){
		case 0:
			CG_DrawWeaponBar0(count,bits);
			break;
		case 1:
			CG_DrawWeaponBar1(count,bits);
			break;
		case 2:
			CG_DrawWeaponBar2(count,bits, color);
			break;
		case 3:
			CG_DrawWeaponBar3(count,bits, color);
			break;
		case 4:
			CG_DrawWeaponBar4(count,bits, color);
			break;
		case 5:
			CG_DrawWeaponBar5(count,bits, color);
			break;
		case 6:
			CG_DrawWeaponBar6(count,bits, color);
			break;
		case 7:
			CG_DrawWeaponBar7(count,bits, color);
			break;
	}
	trap_R_SetColor(NULL);
	return;
}

/*
===============
CG_DrawWeaponBar0
===============
*/

void CG_DrawWeaponBar0(int count, int bits){

	int y = 380;
	int x = 320 - count * 20;
	int i;
	
	for ( i = 0 ; i < MAX_WEAPONS ; i++ ) {
                //Sago: Do mad change of grapple placement:
                if(i==10)
                    continue;
                if(i==0)
                    i=10;
		if ( !( bits & ( 1 << i ) ) ) {
                    if(i==10)
                        i=0;
			continue;
		}

		CG_RegisterWeapon( i );
		// draw weapon icon
		CG_DrawPic( x, y, 32, 32, cg_weapons[i].weaponIcon );

		// draw selection marker
		if ( i == cg.weaponSelect ) {
			CG_DrawPic( x-4, y-4, 40, 40, cgs.media.selectShader );
		}

		// no ammo cross on top
		if ( !cg.snap->ps.ammo[ i ] ) {
			  CG_DrawPic( x, y, 32, 32, cgs.media.noammoShader );
		}

		x += 40;
                //Sago: Undo mad change of weapons
                if(i==10)
                        i=0;
	}
}

/*
===============
CG_DrawWeaponBar1
===============
*/

void CG_DrawWeaponBar1(int count, int bits){

	int y = 380;
	int x = 320 - count * 20;
	int i;
	int ammo;
	int br;
	int max;
	float red[4];
	float yellow[4];
	float green[4];
	
	red[0] = 1.0f;
	red[1] = 0;
	red[2] = 0;
	red[3] = 1.0f;
	
	yellow[0] = 1.0f;
	yellow[1] = 0.6f;
	yellow[2] = 0;
	yellow[3] = 1.0f;
	
	green[0] = 0;
	green[1] = 1.0f;
	green[2] = 0;
	green[3] = 1.0f;
	
	for ( i = 0 ; i < MAX_WEAPONS ; i++ ) {
                //Sago: Do mad change of grapple placement:
                if(i==10)
                    continue;
                if(i==0)
                    i=10;
		if ( !( bits & ( 1 << i ) ) ) {
                    if(i==10)
                        i=0;
			continue;
		}
		
		ammo=cg.snap->ps.ammo[i];
			
		switch(i) {
			case WP_MACHINEGUN: max = 100; break;
			case WP_SHOTGUN: max = 10; break;
			case WP_GRENADE_LAUNCHER: max = 10; break;
			case WP_ROCKET_LAUNCHER: max = 10; break;
			case WP_LIGHTNING: max = 100; break;
			case WP_RAILGUN: max = 10; break;
			case WP_PLASMAGUN: max = 50; break;
			case WP_BFG: max = 10; break;
			case WP_NAILGUN: max = 10; break;
			case WP_PROX_LAUNCHER: max = 5; break;
			case WP_CHAINGUN: max = 100; break;
			default: max = 1; break;
		}
			
		ammo = (ammo*100)/max;
		if(ammo >=100)
			ammo=100;
			
		br=ammo*32/100;
			
		if(i!=WP_GAUNTLET && i!=WP_GRAPPLING_HOOK){
			if(ammo <= 20)
				CG_FillRect( x, y+38, br,4, red);
			if(ammo > 20 && ammo <= 50)
				CG_FillRect( x, y+38, br, 4, yellow);
			if(ammo > 50)
				CG_FillRect( x, y+38, br, 4, green);
		}

		CG_RegisterWeapon( i );
		// draw weapon icon
		CG_DrawPic( x, y, 32, 32, cg_weapons[i].weaponIcon );

		// draw selection marker
		if ( i == cg.weaponSelect ) {
			CG_DrawPic( x-4, y-4, 40, 40, cgs.media.selectShader );
		}

		// no ammo cross on top
		if ( !cg.snap->ps.ammo[ i ] ) {
			  CG_DrawPic( x, y, 32, 32, cgs.media.noammoShader );
		}

		x += 40;
                //Sago: Undo mad change of weapons
                if(i==10)
                        i=0;
	}
}

/*
===============
CG_DrawWeaponBar2
===============
*/

void CG_DrawWeaponBar2(int count, int bits, float *color){

	int y = 200 + count * 12;
	int x = 0;
	int i;
	int w;
	char *s;
	float red[4];
	float yellow[4];
	float blue[4];
	
	red[0] = 1.0f;
	red[1] = 0;
	red[2] = 0;
	red[3] = 0.4f;
	
	yellow[0] = 1.0f;
	yellow[1] = 1.0f;
	yellow[2] = 0;
	yellow[3] = 1.0f;
	
	blue[0] = 0;
	blue[1] = 0;
	blue[2] = 1.0f;
	blue[3] = 0.4f;
	
	for ( i = 0 ; i < MAX_WEAPONS ; i++ ) {
                //Sago: Do mad change of grapple placement:
                if(i==10)
                    continue;
                if(i==0)
                    i=10;
		if ( !( bits & ( 1 << i ) ) ) {
                    if(i==10)
                        i=0;
		    continue;
		}
			
		if(cg.snap->ps.ammo[i]){
			if ( i == cg.weaponSelect) {
				CG_FillRect( x, y, 50, 24, blue );
				CG_DrawRect( x, y, 50, 24, 2, yellow); 
			}
			else{   
				CG_FillRect( x, y,50, 24, blue );
			}
		}
		else{ 
			if ( i == cg.weaponSelect) { 
				CG_FillRect( x, y, 50, 24, red );
				CG_DrawRect( x, y, 50, 24, 2, yellow);
			}
			else{
				CG_FillRect( x, y,50, 24, red );
			}
		}
		     
                CG_RegisterWeapon( i );
			// draw weapon icon
			CG_DrawPic( x+2, y+4, 16, 16, cg_weapons[i].weaponIcon );
		
			/** Draw Weapon Ammo **/
			if(cg.snap->ps.ammo[ i ]!=-1){
				s = va("%i", cg.snap->ps.ammo[ i ] );
				w = CG_DrawStrlen( s ) * SMALLCHAR_WIDTH;
				CG_DrawSmallStringColor(x - w/2 + 32, y+4, s, color);
			}

			y -= 24;
                //Sago: Undo mad change of weapons
                if(i==10)
                        i=0;
	}	
}

/*
===============
CG_DrawWeaponBar3
===============
*/

void CG_DrawWeaponBar3(int count, int bits, float *color){

	int y = 200 + count * 12;
	int x = 0;
	int i;
	int ammo;
	int max;
	int br;
	int w;
	char *s;
	float red[4];
	float yellow[4];
	float green[4];
	float blue[4];
	
	red[0] = 1.0f;
	red[1] = 0;
	red[2] = 0;
	red[3] = 0.4f;
	
	yellow[0] = 1.0f;
	yellow[1] = 1.0f;
	yellow[2] = 0;
	yellow[3] = 1.0f;
	
	green[0] = 0;
	green[1] = 1.0f;
	green[2] = 0;
	green[3] = 1.0f;
	
	blue[0] = 0;
	blue[1] = 0;
	blue[2] = 1.0f;
	blue[3] = 0.4f;
	
	for ( i = 0 ; i < MAX_WEAPONS ; i++ ) {
                //Sago: Do mad change of grapple placement:
                if(i==10)
                    continue;
                if(i==0)
                    i=10;
		if ( !( bits & ( 1 << i ) ) ) {
                    if(i==10)
                        i=0;
			continue;
		}
		
		ammo=cg.snap->ps.ammo[i];

		switch(i) {
			case WP_MACHINEGUN: max = 100; break;
			case WP_SHOTGUN: max = 10; break;
			case WP_GRENADE_LAUNCHER: max = 10; break;
			case WP_ROCKET_LAUNCHER: max = 10; break;
			case WP_LIGHTNING: max = 100; break;
			case WP_RAILGUN: max = 10; break;
			case WP_PLASMAGUN: max = 50; break;
			case WP_BFG: max = 10; break;
			case WP_NAILGUN: max = 10; break;
			case WP_PROX_LAUNCHER: max = 5; break;
			case WP_CHAINGUN: max = 100; break;
			default: max = 1; break;
		}
			
		ammo = (ammo*100)/max;
		if(ammo >=100)
			ammo=100;
			
		br=ammo*20/100;
				
		if(i!=WP_GAUNTLET && i!=WP_GRAPPLING_HOOK){
			if(ammo <= 20)
				CG_FillRect( 51, y+2+20-br, 4,br, red);
			if(ammo > 20 && ammo <= 50)
				CG_FillRect( 51, y+2+20-br, 4,br, yellow);
			if(ammo > 50)
				CG_FillRect( 51, y+2+20-br, 4,br, green);
		}
			
		if(cg.snap->ps.ammo[i]){
			if ( i == cg.weaponSelect) {
				CG_FillRect( x, y, 50, 24, blue );
				CG_DrawRect( x, y, 50, 24, 2, yellow); 
			}
			else{   
				CG_FillRect( x, y,50, 24, blue );
			}
		}
		else{ 
			if ( i == cg.weaponSelect) { 
				CG_FillRect( x, y, 50, 24, red );
				CG_DrawRect( x, y, 50, 24, 2, yellow);
			}
			else{
					CG_FillRect( x, y,50, 24, red );
			}
		}
		     CG_RegisterWeapon( i );
			// draw weapon icon
			CG_DrawPic( x+2, y+4, 16, 16, cg_weapons[i].weaponIcon );
		
			/** Draw Weapon Ammo **/
			if(cg.snap->ps.ammo[ i ]!=-1){
				s = va("%i", cg.snap->ps.ammo[ i ] );
				w = CG_DrawStrlen( s ) * SMALLCHAR_WIDTH;
				CG_DrawSmallStringColor(x - w/2 + 32, y+4, s, color);
			}

			y -= 24;
                        //Sago: Undo mad change of weapons
                if(i==10)
                        i=0;
	}
}

/*
===============
CG_DrawWeaponBar4
===============
*/

void CG_DrawWeaponBar4(int count, int bits, float *color){

	int y = 200 + count * 12;
	int x = 0;
	int i;
	float ammo;
	int max;
	int w;
	char *s;
	float boxColor[4];
	float yellow[4];
	
	boxColor[1]=0;
	boxColor[3]=0.4f;
	
	yellow[0] = 1.0f;
	yellow[1] = 1.0f;
	yellow[2] = 0;
	yellow[3] = 1.0f;
	
	for ( i = 0 ; i < MAX_WEAPONS ; i++ ) {
                //Sago: Do mad change of grapple placement:
                if(i==10)
                    continue;
                if(i==0)
                    i=10;
		if ( !( bits & ( 1 << i ) ) ) {
                    if(i==10)
                        i=0;
			continue;
		}
		
		ammo=cg.snap->ps.ammo[i];

		switch(i) {
			case WP_MACHINEGUN: max = 100; break;
			case WP_SHOTGUN: max = 10; break;
			case WP_GRENADE_LAUNCHER: max = 10; break;
			case WP_ROCKET_LAUNCHER: max = 10; break;
			case WP_LIGHTNING: max = 100; break;
			case WP_RAILGUN: max = 10; break;
			case WP_PLASMAGUN: max = 50; break;
			case WP_BFG: max = 10; break;
			case WP_NAILGUN: max = 10; break;
			case WP_PROX_LAUNCHER: max = 5; break;
			case WP_CHAINGUN: max = 100; break;
			default: max = 1; break;
		}
			
		ammo = (ammo*100)/max;
			
		if((ammo >=100) || (ammo < 0))
			ammo=100;
			
		boxColor[2]=(ammo/100.0f)*1.0f;
		boxColor[0]=1.0f-(ammo/100.0f)*1.0f;	
		
		if ( i == cg.weaponSelect) {
			CG_FillRect( x, y, 50, 24, boxColor );
			CG_DrawRect( x, y, 50, 24, 2, yellow); 
		}
		else{   
			CG_FillRect( x, y,50, 24, boxColor );
		}
			CG_RegisterWeapon( i );
			// draw weapon icon
			CG_DrawPic( x+2, y+4, 16, 16, cg_weapons[i].weaponIcon );
		
			/** Draw Weapon Ammo **/
			if(cg.snap->ps.ammo[ i ]!=-1){
				s = va("%i", cg.snap->ps.ammo[ i ] );
				w = CG_DrawStrlen( s ) * SMALLCHAR_WIDTH;
				CG_DrawSmallStringColor(x - w/2 + 32, y+4, s, color);
			}

			y -= 24;
                        //Sago: Undo mad change of weapons
                if(i==10)
                        i=0;
	}
}

/*
===============
CG_DrawWeaponBar5
===============
*/

void CG_DrawWeaponBar5(int count, int bits, float *color){

	int y = 380;
	int x = 320 - count * 15;
	int i;
	int w;
	char *s;
	float red[4];
	float yellow[4];
	float blue[4];
	
	red[0] = 1.0f;
	red[1] = 0;
	red[2] = 0;
	red[3] = 0.4f;
	
	yellow[0] = 1.0f;
	yellow[1] = 1.0f;
	yellow[2] = 0;
	yellow[3] = 1.0f;
	
	blue[0] = 0;
	blue[1] = 0;
	blue[2] = 1.0f;
	blue[3] = 0.4f;
	
	for ( i = 0 ; i < MAX_WEAPONS ; i++ ) {
                //Sago: Do mad change of grapple placement:
                if(i==10)
                    continue;
                if(i==0)
                    i=10;
		if ( !( bits & ( 1 << i ) ) ) {
                    if(i==10)
                        i=0;
			continue;
		}
			
		if(cg.snap->ps.ammo[i]){
			if ( i == cg.weaponSelect) {
				CG_FillRect( x, y , 30 , 38, blue );
				CG_DrawRect( x, y, 30 ,38 ,2, yellow); 
			}
			else{   
				CG_FillRect( x, y,30, 38, blue );
			}
		}
		else{ 
			if ( i == cg.weaponSelect) {
				CG_FillRect( x, y , 30 , 38, red );
				CG_DrawRect( x , y, 30,38,2, yellow); 
			}
			else{
				CG_FillRect( x, y,30, 38, red );
			}
		}
		CG_RegisterWeapon( i );		
		CG_DrawPic( x+7, y+2, 16, 16, cg_weapons[i].weaponIcon );
			
		if(cg.snap->ps.ammo[ i ]!=-1){
			s = va("%i", cg.snap->ps.ammo[ i ] );
			w = CG_DrawStrlen( s ) * SMALLCHAR_WIDTH;
			CG_DrawSmallStringColor(x - w/2 + 15, y+20, s, color);
		}
			
		x += 30;
                //Sago: Undo mad change of weapons
                if(i==10)
                        i=0;
	}
}

/*
===============
CG_DrawWeaponBar6
===============
*/

void CG_DrawWeaponBar6(int count, int bits, float *color){

	int y = 380;
	int x = 320 - count * 15;
	int i;
	int ammo;
	int max;
	int br;
	int w;
	char *s;
	float red[4];
	float yellow[4];
	float green[4];
	float blue[4];
	
	red[0] = 1.0f;
	red[1] = 0;
	red[2] = 0;
	red[3] = 0.4f;
	
	yellow[0] = 1.0f;
	yellow[1] = 1.0f;
	yellow[2] = 0;
	yellow[3] = 1.0f;
	
	green[0] = 0;
	green[1] = 1.0f;
	green[2] = 0;
	green[3] = 1.0f;
	
	blue[0] = 0;
	blue[1] = 0;
	blue[2] = 1.0f;
	blue[3] = 0.4f;
	
	for ( i = 0 ; i < MAX_WEAPONS ; i++ ) {
                //Sago: Do mad change of grapple placement:
                if(i==10)
                    continue;
                if(i==0)
                    i=10;
		if ( !( bits & ( 1 << i ) ) ) {
                    if(i==10)
                        i=0;
			continue;
		}
		
		ammo=cg.snap->ps.ammo[i];
		
		switch(i) {
			case WP_MACHINEGUN: max = 100; break;
			case WP_SHOTGUN: max = 10; break;
			case WP_GRENADE_LAUNCHER: max = 10; break;
			case WP_ROCKET_LAUNCHER: max = 10; break;
			case WP_LIGHTNING: max = 100; break;
			case WP_RAILGUN: max = 10; break;
			case WP_PLASMAGUN: max = 50; break;
			case WP_BFG: max = 10; break;
			case WP_NAILGUN: max = 10; break;
			case WP_PROX_LAUNCHER: max = 5; break;
			case WP_CHAINGUN: max = 100; break;
			default: max = 1; break;
		}
			
		ammo = (ammo*100)/max;
			
		if(ammo >=100)
			ammo=100;
			
		br=ammo*26/100;
				
		if(i!=WP_GAUNTLET && i!=WP_GRAPPLING_HOOK){
			if(ammo <= 20)
				CG_FillRect( x+2, y +40, br, 4, red);
			if(ammo > 20 && ammo <= 50)
				CG_FillRect( x+2, y+40, br, 4, yellow);
			if(ammo > 50)
				CG_FillRect( x+2, y+40, br, 4, green);
		}
			
		if(cg.snap->ps.ammo[i]){
			if ( i == cg.weaponSelect) {
				CG_FillRect( x, y , 30 , 38, blue );
				CG_DrawRect( x, y, 30 ,38 ,2, yellow); 
			}
			else{   
					CG_FillRect( x, y,30, 38, blue );
			}
		}
		else{ 
			if ( i == cg.weaponSelect) {
				CG_FillRect( x, y , 30 , 38, red );
				CG_DrawRect( x , y, 30,38,2, yellow); 
			}
			else{
					CG_FillRect( x, y,30, 38, red );
			}
		}	
		CG_RegisterWeapon( i );	
		CG_DrawPic( x+7, y+2, 16, 16, cg_weapons[i].weaponIcon );
			
		if(cg.snap->ps.ammo[ i ]!=-1){
			s = va("%i", cg.snap->ps.ammo[ i ] );
			w = CG_DrawStrlen( s ) * SMALLCHAR_WIDTH;
			CG_DrawSmallStringColor(x - w/2 + 15, y+20, s, color);
		}
			
		x += 30;
                //Sago: Undo mad change of weapons
                if(i==10)
                        i=0;
	}	
}

/*
===============
CG_DrawWeaponBar7
===============
*/

void CG_DrawWeaponBar7(int count, int bits, float *color){

	int y = 380;
	int x = 320 - count * 15;
	int i;
	float ammo;
	float max;
	int w;
	char *s;
	float yellow[4];
	float boxColor[4];
	
	boxColor[1]=0;
	boxColor[3]=0.4f;
	
	yellow[0] = 1.0f;
	yellow[1] = 1.0f;
	yellow[2] = 0;
	yellow[3] = 1.0f;
	
	for ( i = 0 ; i < MAX_WEAPONS ; i++ ) {
                //Sago: Do mad change of grapple placement:
                if(i==10)
                    continue;
                if(i==0)
                    i=10;
		if ( !( bits & ( 1 << i ) ) ) {
                    if(i==10)
                        i=0;
			continue;
		}
		
		ammo=cg.snap->ps.ammo[i];
		
		switch(i) {
			case WP_MACHINEGUN: max = 100; break;
			case WP_SHOTGUN: max = 10; break;
			case WP_GRENADE_LAUNCHER: max = 10; break;
			case WP_ROCKET_LAUNCHER: max = 10; break;
			case WP_LIGHTNING: max = 100; break;
			case WP_RAILGUN: max = 10; break;
			case WP_PLASMAGUN: max = 50; break;
			case WP_BFG: max = 10; break;
			case WP_NAILGUN: max = 10; break;
			case WP_PROX_LAUNCHER: max = 5; break;
			case WP_CHAINGUN: max = 100; break;
			default: max = 1; break;
		}
			
		ammo = (ammo*100)/max;
			
		if((ammo >=100) || (ammo < 0))
			ammo=100;
			
		boxColor[2]=(ammo/100.0f)*1.0f;
		boxColor[0]=1.0f-(ammo/100.0f)*1.0f;
				
		if ( i == cg.weaponSelect) {
			CG_FillRect( x, y , 30 , 38, boxColor );
			CG_DrawRect( x, y, 30 ,38 ,2, yellow); 
		}
		else{   
				CG_FillRect( x, y,30, 38, boxColor );
		}
		CG_RegisterWeapon( i );	
		CG_DrawPic( x+7, y+2, 16, 16, cg_weapons[i].weaponIcon );
			
		if(cg.snap->ps.ammo[ i ]!=-1){
			s = va("%i", cg.snap->ps.ammo[ i ] );
			w = CG_DrawStrlen( s ) * SMALLCHAR_WIDTH;
			CG_DrawSmallStringColor(x - w/2 + 15, y+20, s, color);
		}
		
		x += 30;
                //Sago: Undo mad change of weapons
                if(i==10)
                        i=0;
	}
}


/*
===============
CG_WeaponSelectable
===============
*/
static qboolean CG_WeaponSelectable( int i ) {
	if ( !cg.snap->ps.ammo[i] ) {
		return qfalse;
	}
	if ( ! (cg.snap->ps.stats[ STAT_WEAPONS ] & ( 1 << i ) ) ) {
		return qfalse;
	}

	return qtrue;
}

/*
===============
CG_NextWeapon_f
===============
*/
void CG_NextWeapon_f( void ) {
	int		i;
	int		original;

	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}

	cg.weaponSelectTime = cg.time;
	original = cg.weaponSelect;
        //Part of mad hook select code:
        if(cg.weaponSelect == WP_GRAPPLING_HOOK)
            cg.weaponSelect = 0;

	for ( i = 0 ; i < MAX_WEAPONS ; i++ ) {
		cg.weaponSelect++;
		if ( cg.weaponSelect == MAX_WEAPONS ) {
			cg.weaponSelect = 0;
		}
		if ( cg.weaponSelect == WP_GAUNTLET ) {
			continue;		// never cycle to gauntlet
		}
                //Sago: Mad change of grapple order
                if( cg.weaponSelect == WP_GRAPPLING_HOOK)  {
                    continue;
                }
                if( cg.weaponSelect == 0)
                    cg.weaponSelect = WP_GRAPPLING_HOOK;
                if ( cg.weaponSelect == WP_GRAPPLING_HOOK && !cg_cyclegrapple.integer ) {
                    cg.weaponSelect = 0;
                    continue;		// never cycle to grapple unless the client wants it
		}
		if ( CG_WeaponSelectable( cg.weaponSelect ) ) {
			break;
		}
                if( cg.weaponSelect == WP_GRAPPLING_HOOK)
                    cg.weaponSelect = 0;
	}
	if ( i == MAX_WEAPONS ) {
		cg.weaponSelect = original;
	}
}

/*
===============
CG_PrevWeapon_f
===============
*/
void CG_PrevWeapon_f( void ) {
	int		i;
	int		original;

	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}

	cg.weaponSelectTime = cg.time;
	original = cg.weaponSelect;
        //Part of mad hook select code:
        if(cg.weaponSelect == WP_GRAPPLING_HOOK)
            cg.weaponSelect = 0;

	for ( i = 0 ; i < MAX_WEAPONS ; i++ ) {
		cg.weaponSelect--;
		if ( cg.weaponSelect == -1 ) {
			cg.weaponSelect = MAX_WEAPONS - 1;
		}
		if ( cg.weaponSelect == WP_GAUNTLET ) {
			continue;		// never cycle to gauntlet
		}
                //Sago: Mad change of grapple order
                if( cg.weaponSelect == WP_GRAPPLING_HOOK)  {
                    continue;
                }
                if( cg.weaponSelect == 0)
                    cg.weaponSelect = WP_GRAPPLING_HOOK;
                if ( cg.weaponSelect == WP_GRAPPLING_HOOK && !cg_cyclegrapple.integer ) {
                    cg.weaponSelect = 0;
                    continue;		// never cycle to grapple unless the client wants it
		}
		if ( CG_WeaponSelectable( cg.weaponSelect ) ) {
			break;
		}
                if( cg.weaponSelect == WP_GRAPPLING_HOOK)
                    cg.weaponSelect = 0;
	}
	if ( i == MAX_WEAPONS ) {
		cg.weaponSelect = original;
	}
}

/*
===============
CG_Weapon_f
===============
*/
void CG_Weapon_f( void ) {
	int		num;

	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}

	num = atoi( CG_Argv( 1 ) );

	if ( num < 1 || num > MAX_WEAPONS-1 ) {
		return;
	}

	cg.weaponSelectTime = cg.time;

	if ( ! ( cg.snap->ps.stats[STAT_WEAPONS] & ( 1 << num ) ) ) {
		return;		// don't have the weapon
	}

	cg.weaponSelect = num;
}

/*
===================
CG_OutOfAmmoChange

The current weapon has just run out of ammo
===================
*/
void CG_OutOfAmmoChange( void ) {
	int		i;

	cg.weaponSelectTime = cg.time;

	for ( i = MAX_WEAPONS-1 ; i > 0 ; i-- ) {
		if ( CG_WeaponSelectable( i ) && i != WP_GRAPPLING_HOOK ) {
			cg.weaponSelect = i;
			break;
		}
	}
}



/*
===================================================================================================

WEAPON EVENTS

===================================================================================================
*/

/*
================
CG_FireWeapon

Caused by an EV_FIRE_WEAPON event
================
*/
void CG_FireWeapon( centity_t *cent ) {
	entityState_t *ent;
	int				c;
	weaponInfo_t	*weap;

	if((cgs.gametype == GT_ELIMINATION || cgs.gametype == GT_CTF_ELIMINATION) && cgs.roundStartTime>=cg.time)
		return; //if we havn't started in ELIMINATION then do not fire

	ent = &cent->currentState;
	if ( ent->weapon == WP_NONE ) {
		return;
	}
	if ( ent->weapon >= WP_NUM_WEAPONS ) {
		CG_Error( "CG_FireWeapon: ent->weapon >= WP_NUM_WEAPONS" );
		return;
	}
	weap = &cg_weapons[ ent->weapon ];

	// mark the entity as muzzle flashing, so when it is added it will
	// append the flash to the weapon model
	cent->muzzleFlashTime = cg.time;

	// lightning gun only does this this on initial press
	if ( ent->weapon == WP_LIGHTNING ) {
		if ( cent->pe.lightningFiring ) {
			return;
		}
	}

	// play quad sound if needed
	if ( cent->currentState.powerups & ( 1 << PW_QUAD ) ) {
		trap_S_StartSound (NULL, cent->currentState.number, CHAN_ITEM, cgs.media.quadSound );
	}

	// play a sound
	for ( c = 0 ; c < 4 ; c++ ) {
		if ( !weap->flashSound[c] ) {
			break;
		}
	}
	if ( c > 0 ) {
		c = rand() % c;
		if ( weap->flashSound[c] )
		{
			trap_S_StartSound( NULL, ent->number, CHAN_WEAPON, weap->flashSound[c] );
		}
	}

	// do brass ejection
	if ( weap->ejectBrassFunc && cg_brassTime.integer > 0 ) {
		weap->ejectBrassFunc( cent );
	}

//unlagged - attack prediction #1
	CG_PredictWeaponEffects( cent );
//unlagged - attack prediction #1
}


/*
==========================
CG_Explosionia LEILEI
==========================

static void CG_Explosionia ( centity_t *cent ) {
	localEntity_t	*le;
	ec3_t			velocity, xvelocity;
	vec3_t			offset, xoffset;
	float			waterScale = 1.0f;
	vec3_t			v[3];

	if ( cg_brassTime.integer <= 0 ) {
		return;
	}

	le = CG_AllocLocalEntity();
	

	velocity[0] = -50 + 100 * crandom();
	velocity[1] = -50 + 100 * crandom();
	velocity[2] = -50 + 100 * crandom();

	le->leType = LE_FALL_SCALE_FADE;
	le->startTime = cg.time;
	le->endTime = le->startTime + cg_brassTime.integer + ( cg_brassTime.integer / 4 ) * random();

	//le->pos.trType = TR_GRAVITY;
	le->pos.trTime = cg.time - (rand()&15);

	AnglesToAxis( cent->lerpAngles, v );

	offset[0] = 8;
	offset[1] = -4;
	offset[2] = 24;

	xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
	xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
	xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];
	VectorAdd( cent->lerpOrigin, xoffset, re->origin );

	VectorCopy( re->origin, le->pos.trBase );

	if ( CG_PointContents( re->origin, -1 ) & CONTENTS_WATER ) {
		waterScale = 0.10f;
	}

	xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
	xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
	xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];
	VectorScale( xvelocity, waterScale, le->pos.trDelta );

	le->bounceFactor = 0.4 * waterScale;

	le->angles.trType = TR_LINEAR;
	le->angles.trTime = cg.time;
	le->angles.trBase[0] = rand()&31;
	le->angles.trBase[1] = rand()&31;
	le->angles.trBase[2] = rand()&31;
	le->angles.trDelta[0] = 2;
	le->angles.trDelta[1] = 1;
	le->angles.trDelta[2] = 0;
	le = CG_SmokePuff( le->origin, le->velocity, 
					  30,		// radius
					  1, 1, 1, 1,	// color
					  2000,		// trailTime
					  cg.time,		// startTime
					  0,		// fadeInTime
					  0,		// flags
					  cgs.media.lbumShader1 );
	le->leFlags = LEF_TUMBLE;
	le->leBounceSoundType = LEBS_NONE;
	le->leMarkType = LEMT_NONE;
}

*/





void CG_LFX_RocketBoom (vec3_t origin, vec3_t dir) {
	qhandle_t		mod;
	qhandle_t		mark;
	qhandle_t		shader;
	sfxHandle_t		sfx;
	float			radius;
	float			light;
	vec3_t			lightColor;
	localEntity_t	*le;
	int				r;
	qboolean		alphaFade;
	qboolean		isSprite;
	int				duration;
	vec3_t			sprOrg;
	vec3_t			sprVel;
	vec4_t colory, colory2, colory3, colory4;

/*
	mark = 0;
	radius = 32;
	sfx = 0;
	mod = 0;
	shader = 0;
	light = 0;
	lightColor[0] = 1;
	lightColor[1] = 1;
	lightColor[2] = 0;

	// set defaults
	isSprite = qfalse;
	duration = 600;

		mod = cgs.media.dishFlashModel;
		shader = cgs.media.rocketExplosionShader;
		sfx = cgs.media.sfx_rockexp;
		mark = cgs.media.burnMarkShader;
		radius = 64;
		light = 300;
		isSprite = qtrue;
		duration = 1000;

		shader = cgs.media.addshock;
		mod = cgs.media.bulletFlashModel;
		radius = 64;
		light = 700;
		isSprite = qfalse;
		duration = 5300;
		lightColor[0] = 1;
		lightColor[1] = 0.35;
		lightColor[2] = 0.0;

*/

		// shockwave first.

		colory[0] = 1.0; colory[1] = 1.0; colory[2] = 1.0; colory[3] = 1.0;
		colory2[0] = 1.0; colory2[1] = 1.0; colory2[2] = 0.5; colory2[3] = 0.9;
		colory3[0] = 0.7; colory3[1] = 0.3; colory3[2] = 0.1; colory3[3] = 0.7;
		colory4[0] = 0.0; colory4[1] = 0.0; colory4[2] = 0.0; colory4[3] = 0.0;
		VectorMA( origin, 4, dir, sprOrg );
		VectorScale( dir, 64, sprVel );

		CG_LFX_Shock (sprOrg, dir, 0, 0, colory, colory2, colory3, colory4, colory4, 1, 500, 270, 1);

		colory[0] = 0.5; colory[1] = 0.5; colory[2] = 0.1; colory[3] = 1.0;
		colory2[0] = 0.3; colory2[1] = 0.2; colory2[2] = 0.1; colory2[3] = 0.9;
		colory3[0] = 0.2; colory3[1] = 0.1; colory3[2] = 0.1; colory3[3] = 0.7;
		colory4[0] = 0.0; colory4[1] = 0.0; colory4[2] = 0.0; colory4[3] = 0.0;

		CG_LFX_Shock (sprOrg, dir, 0, -683, colory, colory2, colory3, colory4, colory4, 13, 160, 120, 1);


		// fireball

		colory[0] = 1.0; colory[1] = 0.2; colory[2] = 0.1; colory[3] = 1.0;
		colory2[0] = 0.5; colory2[1] = 0.0; colory2[2] = 0.0; colory2[3] = 0.9;
		colory3[0] = 0.1; colory3[1] = 0.0; colory3[2] = 0.0; colory3[3] = 0.7;
		colory4[0] = 0.0; colory4[1] = 0.0; colory4[2] = 0.0; colory4[3] = 0.0;
		VectorMA( origin, 12, dir, sprOrg );
		VectorScale( dir, 64, sprVel );

		CG_LFX_Smoke (sprOrg, sprVel, 32, 0.54, colory, colory2, colory3, colory4, colory4, 42, 1200, 54, 1);
	
		colory[0] = 1.0; colory[1] = 1.0; colory[2] = 0.9; colory[3] = 1.0;
		colory2[0] = 1.0; colory2[1] = 1.0; colory2[2] = 0.7; colory2[3] = 0.9;
		colory3[0] = 0.4; colory3[1] = 0.3; colory3[2] = 0.2; colory3[3] = 0.7;
		colory4[0] = 0.1; colory4[1] = 0.0; colory4[2] = 0.0; colory4[3] = 0.0;
	
		CG_LFX_Smoke (sprOrg, sprVel, 62, 2, colory, colory2, colory3, colory4, colory4, 72, 200,84, 1);
		CG_LFX_Smoke (sprOrg, sprVel, 32, 0.54, colory, colory2, colory3, colory4, colory4, 42, 600, 74, 1);

		CG_LFX_Smoke (sprOrg, sprVel, 44, 1.3, colory, colory2, colory3, colory4, colory4, 3, 800,3, 1);


		CG_LFX_Smoke (sprOrg, sprVel, 32, 0.54, colory, colory2, colory3, colory4, colory4, 42, 600, 74, 1);

		// Shroom Cloud
		VectorMA( origin, 16, dir, sprOrg );
		VectorScale( dir, 64, sprVel );

		colory[0] = 0.5; colory[1] = 0.0; colory[2] = 0.0; colory[3] = 0.0;
		colory2[0] = 1.0; colory2[1] = 1.0; colory2[2] = 0.2; colory2[3] = 0.2;
		colory3[0] = 0.5; colory3[1] = 0.1; colory3[2] = 0.0; colory3[3] = 0.5;
		colory4[0] = 0.0; colory4[1] = 0.0; colory4[2] = 0.0; colory4[3] = 0.0;
	

		CG_LFX_Smoke (sprOrg, sprVel, 3, 0.7, colory, colory2, colory3, colory4, colory4, 22, 300,135, 1);

		CG_LFX_Smoke (sprOrg, sprVel, 355, 2.7, colory, colory2, colory3, colory4, colory4, 13, 220,15, 1);


		CG_LFX_Smoke (sprOrg, sprVel, 44, 0.7, colory, colory2, colory3, colory4, colory4, 3, 1200,3, 1);



		// Aftersmoke
		colory[0] = 0.0; colory[1] = 0.0; colory[2] = 0.0; colory[3] = 0.0;
		colory2[0] = 0.2; colory2[1] = 0.2; colory2[2] = 0.2; colory2[3] = 0.2;
		colory3[0] = 0.9; colory3[1] = 0.9; colory3[2] = 0.9; colory3[3] = 0.5;
		colory4[0] = 0.0; colory4[1] = 0.0; colory4[2] = 0.0; colory4[3] = 0.0;
		VectorMA( origin, 32, dir, sprOrg );
		CG_LFX_Smoke (sprOrg, sprVel, 4, 0.2, colory, colory2, colory3, colory4, colory4, 144, 5500,24, 3);
		CG_LFX_Smoke (sprOrg, sprVel, 9, 0.2, colory, colory2, colory3, colory4, colory4, 44, 3500,24, 3);
		CG_LFX_Smoke (sprOrg, sprVel, 12, 0.2, colory, colory2, colory3, colory4, colory4, 24, 1500,24, 3);

}


/*
=================
CG_MissileHitWall

Caused by an EV_MISSILE_MISS event, or directly by local bullet tracing
=================
*/
void CG_MissileHitWall( int weapon, int clientNum, vec3_t origin, vec3_t dir, impactSound_t soundType ) {
	qhandle_t		mod;
	qhandle_t		mark;
	qhandle_t		shader;
	sfxHandle_t		sfx;
	float			radius;
	float			light;
	vec3_t			lightColor;
	localEntity_t	*le;
	int				r;
	qboolean		alphaFade;
	qboolean		isSprite;
	int				duration;
	vec3_t			sprOrg;
	vec3_t			sprVel;

	mark = 0;
	radius = 32;
	sfx = 0;
	mod = 0;
	shader = 0;
	light = 0;
	lightColor[0] = 1;
	lightColor[1] = 1;
	lightColor[2] = 0;

	// set defaults
	isSprite = qfalse;
	duration = 600;

	switch ( weapon ) {
	default:
//#ifdef MISSIONPACK
	case WP_NAILGUN:
				if (cg_leiEnhancement.integer == 2000) {
		
				vec4_t colory, colory2, colory3, colory4;
					colory[0] = 1.0; colory[1] = 1.0; colory[2] = 1.0; colory[3] = 1.0;
					colory2[0] = 0.7; colory2[1] = 1.0; colory2[2] = 1.0; colory2[3] = 0.9;
					colory3[0] = 0.3; colory3[1] = 0.6; colory3[2] = 0.6; colory3[3] = 0.7;
					colory4[0] = 0.0; colory4[1] = 0.0; colory4[2] = 0.0; colory4[3] = 0.0;
			
		
					VectorScale( dir, 2, sprVel );
						VectorMA( origin, 2, dir, sprOrg );		
				//CG_LFX_Smoke (sprOrg, sprVel, 32, 2, colory, colory2, colory3, colory4, colory4, 42, 500,2664, 1);
		
			//	CG_LFX_Smoke (sprOrg, sprVel, 16, 0.7, colory, colory2, colory3, colory4, colory4, 33, 1200,33, 1);
		
				// shockwave first.
		
				colory[0] = 1.0; colory[1] = 1.0; colory[2] = 1.0; colory[3] = 1.0;
				colory2[0] = 0.8; colory2[1] = 1.0; colory2[2] = 1.0; colory2[3] = 0.9;
				colory3[0] = 0.0; colory3[1] = 0.5; colory3[2] = 0.8; colory3[3] = 0.7;
				colory4[0] = 0.0; colory4[1] = 0.0; colory4[2] = 0.0; colory4[3] = 0.0;
				VectorMA( origin, 4, dir, sprOrg );
				VectorScale( dir, 64, sprVel );
		
				CG_LFX_Shock (sprOrg, dir, 0, 0, colory, colory2, colory3, colory4, colory4, 1, 190, 90, 1);
		
		
							break;
							}
		if( soundType == IMPACTSOUND_FLESH ) {
			sfx = cgs.media.sfx_nghitflesh;
		} else if( soundType == IMPACTSOUND_METAL ) {
			sfx = cgs.media.sfx_nghitmetal;
		} else {
			sfx = cgs.media.sfx_nghit;
		}
		mark = cgs.media.holeMarkShader;
		radius = 12;
		break;
//#endif
	case WP_LIGHTNING:
		// no explosion at LG impact, it is added with the beam
		r = rand() & 3;
		
		if ( r < 2 ) {
			sfx = cgs.media.sfx_lghit2;
		} else if ( r == 2 ) {
			sfx = cgs.media.sfx_lghit1;
		} else {
			sfx = cgs.media.sfx_lghit3;
		}
		mark = cgs.media.holeMarkShader;
		radius = 12;
		break;
//#ifdef MISSIONPACK
	case WP_PROX_LAUNCHER:
		mod = cgs.media.dishFlashModel;
		shader = cgs.media.grenadeExplosionShader;
		sfx = cgs.media.sfx_proxexp;
		mark = cgs.media.burnMarkShader;
		radius = 64;
		light = 300;
		isSprite = qtrue;
		// LEILEI START enhancement
			if (cg_leiEnhancement.integer == 2000) {
				lightColor[0] = 0.7;	// subtler explosion colors
				lightColor[1] = 0.6;
				lightColor[2] = 0.4;
				CG_LFX_RocketBoom(origin, dir);
				break;
				}
			if (cg_leiEnhancement.integer == 1996) {
				radius = 64;
				light = 400;
				isSprite = qtrue;
				duration = 2000;
				lightColor[0] = 1;
				lightColor[1] = 1;
				lightColor[2] = 1;
				VectorMA( origin, 24, dir, sprOrg );
				CG_RunParticleEffect(sprOrg, sprVel, 7, 20);
				CG_QarticleExplosion(sprOrg);
				break;
				}
		if (cg_leiEnhancement.integer) {
		// some more fireball, fireball, fireball, fire fire!
		VectorMA( origin, 24, dir, sprOrg );
		VectorScale( dir, 64, sprVel );
		lightColor[0] = 0.7;	// subtler explosion colors
		lightColor[1] = 0.6;
		lightColor[2] = 0.4;
		VectorMA( origin, 4, dir, sprOrg );
		VectorScale( dir, 2, sprVel );
		VectorMA( origin, 4, dir, sprOrg );
		VectorScale( dir, 42, sprVel );
		CG_ParticleExplosion( "explode1", sprOrg, sprVel, 700, 10, 108 );
		VectorMA( origin, 4, dir, sprOrg );
		VectorScale( dir, 82, sprVel );
		CG_ParticleExplosion( "explode1", sprOrg, sprVel, 400, 11, 158 );
		VectorMA( origin, -5, dir, sprOrg );
		VectorScale( dir, 182, sprVel );
		CG_ParticleExplosion( "explode1", sprOrg, sprVel, 600, 47, 88 );
		VectorMA( origin, -5, dir, sprOrg );
		VectorScale( dir, 64, sprVel );
		CG_ParticleExplosion( "explode1", sprOrg, sprVel, 110, 72, 28 );
		mod = 1; // turns off the sprite (unfortunately, disables dlight)
		}
		// LEILEI END enhancement

		break;
//#endif
	case WP_GRENADE_LAUNCHER:

		mod = cgs.media.dishFlashModel;
		shader = cgs.media.grenadeExplosionShader;
		sfx = cgs.media.sfx_rockexp;
		mark = cgs.media.burnMarkShader;
		radius = 64;
		light = 300;
		isSprite = qtrue;
		// LEILEI START enhancement
			if (cg_leiEnhancement.integer == 2000) {
				lightColor[0] = 0.7;	// subtler explosion colors
				lightColor[1] = 0.6;
				lightColor[2] = 0.4;
				CG_LFX_RocketBoom(origin, dir);
				break;
				}
			if (cg_leiEnhancement.integer == 1996) {
				radius = 64;
				light = 400;
				isSprite = qtrue;
				duration = 2000;
				lightColor[0] = 1;
				lightColor[1] = 1;
				lightColor[2] = 1;
				VectorMA( origin, 24, dir, sprOrg );
				CG_RunParticleEffect(sprOrg, sprVel, 7, 20);
				CG_QarticleExplosion(sprOrg);
				break;
				}
		if (cg_leiEnhancement.integer) {
		// some more fireball, fireball, fireball, fire fire!
		VectorMA( origin, 24, dir, sprOrg );
		VectorScale( dir, 64, sprVel );
		lightColor[0] = 0.7;	// subtler explosion colors
		lightColor[1] = 0.6;
		lightColor[2] = 0.4;
		VectorMA( origin, 4, dir, sprOrg );
		VectorScale( dir, 2, sprVel );
		VectorMA( origin, 4, dir, sprOrg );
		VectorScale( dir, 42, sprVel );
		CG_ParticleExplosion( "explode1", sprOrg, sprVel, 700, 10, 108 );
		VectorMA( origin, 4, dir, sprOrg );
		VectorScale( dir, 82, sprVel );
		CG_ParticleExplosion( "explode1", sprOrg, sprVel, 400, 11, 158 );
		VectorMA( origin, -5, dir, sprOrg );
		VectorScale( dir, 182, sprVel );
		CG_ParticleExplosion( "explode1", sprOrg, sprVel, 600, 47, 88 );
		VectorMA( origin, -5, dir, sprOrg );
		VectorScale( dir, 64, sprVel );
		CG_ParticleExplosion( "explode1", sprOrg, sprVel, 110, 72, 28 );
		mod = 1; // turns off the sprite (unfortunately, disables dlight)
		}
		// LEILEI END enhancement
		break;
	case WP_ROCKET_LAUNCHER:

		mod = cgs.media.dishFlashModel;
		shader = cgs.media.rocketExplosionShader;
		sfx = cgs.media.sfx_rockexp;
		mark = cgs.media.burnMarkShader;
		radius = 64;
		light = 300;
		isSprite = qtrue;
		duration = 1000;
		lightColor[0] = 1;
		lightColor[1] = 0.75;
		lightColor[2] = 0.0;

	// Common ROCKET effect
		
		// LEILEI START enhancement
			if (cg_leiEnhancement.integer == 2000) {
				lightColor[0] = 0.7;	// subtler explosion colors
				lightColor[1] = 0.6;
				lightColor[2] = 0.4;
				CG_LFX_RocketBoom(origin, dir);
				break;
				}
			if (cg_leiEnhancement.integer == 1996) {
				radius = 64;
				light = 400;
				isSprite = qtrue;
				duration = 2000;
				lightColor[0] = 1;
				lightColor[1] = 1;
				lightColor[2] = 1;
				VectorMA( origin, 24, dir, sprOrg );
				CG_RunParticleEffect(sprOrg, sprVel, 7, 20);
				CG_QarticleExplosion(sprOrg);
				break;
				}
		if (!cg_oldRocket.integer) {
			// explosion sprite animation
			VectorMA( origin, 24, dir, sprOrg );
			VectorScale( dir, 64, sprVel );

			CG_ParticleExplosion( "explode1", sprOrg, sprVel, 1400, 20, 30 );
		}


		if (cg_leiEnhancement.integer) {
		// some more fireball, fireball, fireball, fire fire!
		VectorMA( origin, 24, dir, sprOrg );
		VectorScale( dir, 64, sprVel );
		lightColor[0] = 0.7;	// subtler explosion colors
		lightColor[1] = 0.6;
		lightColor[2] = 0.4;
		VectorMA( origin, 4, dir, sprOrg );
		VectorScale( dir, 2, sprVel );
		VectorMA( origin, 4, dir, sprOrg );
		VectorScale( dir, 42, sprVel );
		CG_ParticleExplosion( "explode1", sprOrg, sprVel, 700, 10, 108 );
		VectorMA( origin, 4, dir, sprOrg );
		VectorScale( dir, 82, sprVel );
		CG_ParticleExplosion( "explode1", sprOrg, sprVel, 400, 11, 158 );
		VectorMA( origin, -5, dir, sprOrg );
		VectorScale( dir, 182, sprVel );
		CG_ParticleExplosion( "explode1", sprOrg, sprVel, 600, 47, 88 );
		VectorMA( origin, -5, dir, sprOrg );
		VectorScale( dir, 64, sprVel );
		CG_ParticleExplosion( "explode1", sprOrg, sprVel, 110, 72, 28 );
		//mod = 1; // turns off the sprite (unfortunately, disables dlight)
		CG_LeiSparks(sprOrg, dir, 2400, 0, 0, 2);
		CG_LeiSparks(sprOrg, dir, 2400, 0, 0, 2);
		CG_LeiSparks(sprOrg, dir, 2400, 0, 0, 1);
		}
		// LEILEI END enhancement
		break;
	case WP_RAILGUN:
		mod = cgs.media.ringFlashModel;
		shader = cgs.media.railExplosionShader;
		sfx = cgs.media.sfx_plasmaexp;
		mark = cgs.media.energyMarkShader;
		radius = 24;
			if (cg_leiEnhancement.integer == 2000) {
		
				vec4_t colory, colory2, colory3, colory4;

	
			// shockwave first.
		
				colory[0] = 0.8; colory[1] = 0.6; colory[2] = 0.5; colory[3] = 1.0;
				colory2[0] = 0.7; colory2[1] = 0.55; colory2[2] = 0.5; colory2[3] = 0.9;
				colory3[0] = 0.6; colory3[1] = 0.5; colory3[2] = 0.5; colory3[3] = 0.7;
				colory4[0] = 0.0; colory4[1] = 0.0; colory4[2] = 0.0; colory4[3] = 0.0;
				VectorMA( origin, 4, dir, sprOrg );
				VectorScale( dir, 64, sprVel );
		
				CG_LFX_Shock (origin, dir, 0, 0, colory, colory2, colory3, colory4, colory4, 1, 1200, 150,1);
				CG_LFX_Shock (origin, dir, 0, 0, colory, colory2, colory3, colory4, colory4, 1, 1000, 110,1);
				CG_LFX_Shock (origin, dir, 0, 0, colory, colory2, colory3, colory4, colory4, 1, 900, 100,1);
				CG_LFX_Shock (origin, dir, 0, 0, colory, colory2, colory3, colory4, colory4, 1, 600, 90,1);
				CG_LFX_Shock (origin, dir, 0, 0, colory, colory2, colory3, colory4, colory4, 1, 300, 50,1);
					break;
			}

		if (cg_leiEnhancement.integer > 1) {	// leilei - extra dlight
		lightColor[0] = 0.8;
		lightColor[1] = 0.9;
		lightColor[2] = 1.0;
		radius = 1;
		light = 60;
	
		}
		break;
	case WP_PLASMAGUN:
		mod = cgs.media.ringFlashModel;
		shader = cgs.media.plasmaExplosionShader;
		sfx = cgs.media.sfx_plasmaexp;
		mark = cgs.media.energyMarkShader;
		radius = 16;
		VectorMA( origin, 4, dir, sprOrg );
		VectorScale( dir, 64, sprVel );
		if (cg_leiEnhancement.integer == 2000) {
		vec4_t colory, colory2, colory3, colory4;	colory[0] = 0.6; colory[1] = 0.7; colory[2] = 1.0; colory[3] = 1.0;
			colory2[0] = 0.2; colory2[1] = 0.4; colory2[2] = 1.0; colory2[3] = 0.9;
			colory3[0] = 0.0; colory3[1] = 0.2; colory3[2] = 1.0; colory3[3] = 0.7;
			colory4[0] = 0.0; colory4[1] = 0.0; colory4[2] = 0.0; colory4[3] = 0.0;
	
		
		//CG_LFX_Smoke (sprOrg, sprVel, 32, 2, colory, colory2, colory3, colory4, colory4, 42, 500,2664, 1);
		//CG_LFX_Smoke (sprOrg, sprVel, 66, 0.7, colory, colory2, colory3, colory4, colory4, 3, 500,24, 1);
	//	CG_LFX_Smoke (sprOrg, sprVel, 12, 0.4, colory, colory2, colory3, colory4, colory4, 11, 200, 1);
	//	CG_LFX_Smoke (sprOrg, sprVel, 52, 0.1, colory, colory2, colory3, colory4, colory4, 21, 280, 1);
	//	CG_ParticleImpactSmokePuff (cgs.media.alfsmoke, sprOrg);


			// shockwave 
		
				colory[0] = 0.9; colory[1] = 0.8; colory[2] = 1.0; colory[3] = 1.0;
				colory2[0] = 0.7; colory2[1] = 0.5; colory2[2] = 0.7; colory2[3] = 0.9;
				colory3[0] = 0.1; colory3[1] = 0.1; colory3[2] = 0.3; colory3[3] = 0.7;
				colory4[0] = 0.0; colory4[1] = 0.0; colory4[2] = 0.0; colory4[3] = 0.0;
				VectorMA( origin, 4, dir, sprOrg );
				VectorScale( dir, 64, sprVel );
		
				CG_LFX_Shock (origin, dir, 0, 0, colory, colory2, colory3, colory4, colory4, 1, 400, 120,1);
				CG_LFX_Smoke (sprOrg, sprVel, 36, 1, colory, colory2, colory3, colory4, colory4, 3, 600,24, 1);
					break;
		}
		if (cg_leiEnhancement.integer > 1) {	// leilei - extra dlight
		lightColor[0] = 0.3;
		lightColor[1] = 0.4;
		lightColor[2] = 1.0;
		light = 80;
	
		}

		break;
	case WP_BFG:

		mod = cgs.media.dishFlashModel;
		shader = cgs.media.bfgExplosionShader;
		sfx = cgs.media.sfx_rockexp;
		mark = cgs.media.burnMarkShader;
		radius = 32;
		isSprite = qtrue;

	if (cg_leiEnhancement.integer == 2000) 
		{

	vec4_t colory, colory2, colory3, colory4;	
		colory[0] = 0.9; colory[1] = 1.0; colory[2] = 1.0; colory[3] = 1.0;
		colory2[0] = 0.3; colory2[1] = 1.0; colory2[2] = 0.7; colory2[3] = 0.9;
		colory3[0] = 0.1; colory3[1] = 1.0; colory3[2] = 0.2; colory3[3] = 0.7;
		colory4[0] = 0.0; colory4[1] = 0.0; colory4[2] = 0.0; colory4[3] = 0.0;
	
		VectorMA( origin, 4, dir, sprOrg );
		VectorScale( dir, 64, sprVel );
		
		CG_LFX_Smoke (sprOrg, sprVel, 62, 2, colory, colory2, colory3, colory4, colory4, 72, 200,84, 1);
		CG_LFX_Smoke (sprOrg, sprVel, 0, 2, colory, colory2, colory3, colory4, colory4, 5, 400,24, 1);
		CG_LFX_Smoke (sprOrg, sprVel, 32, 0.54, colory, colory2, colory3, colory4, colory4, 42, 700, 24, 0);

		// Shroom Cloud

		colory[0] = 0.0; colory[1] = 0.0; colory[2] = 0.0; colory[3] = 0.0;
		colory2[0] = 0.1; colory2[1] = 0.4; colory2[2] = 0.2; colory2[3] = 0.2;
		colory3[0] = 0.1; colory3[1] = 1.0; colory3[2] = 0.2; colory3[3] = 0.5;
		colory4[0] = 0.0; colory4[1] = 0.0; colory4[2] = 0.0; colory4[3] = 0.0;
	

		CG_LFX_Smoke (sprOrg, sprVel, 3, 2, colory, colory2, colory3, colory4, colory4, 52, 1100,155, 0);
	//	CG_LFX_Smoke (sprOrg, sprVel, 12, 0.4, colory, colory2, colory3, colory4, colory4, 11, 200,24, 1);
	//	CG_LFX_Smoke (sprOrg, sprVel, 52, 0.1, colory, colory2, colory3, colory4, colory4, 21, 280,24, 1);

		}
			if (cg_leiEnhancement.integer == 1996) {
				radius = 64;
				light = 400;
				isSprite = qtrue;
				duration = 2000;
				lightColor[0] = 1;
				lightColor[1] = 1;
				lightColor[2] = 1;
				VectorMA( origin, 24, dir, sprOrg );
				CG_RunParticleEffect(sprOrg, sprVel, 7, 20);
				CG_QarticleExplosion(sprOrg);
				break;
				}
		if (cg_leiEnhancement.integer > 1) {	// leilei - extra dlight
		lightColor[0] = 0.4;
		lightColor[1] = 1.0;
		lightColor[2] = 0.5;
		light = 470;
	
		}
		break;
	case WP_SHOTGUN:
	if (cg_leiEnhancement.integer == 2000) 
		{

		vec4_t colory, colory2, colory3, colory4;	
		mod = cgs.media.bulletFlashModel;
		shader = cgs.media.bulletExplosionShader;
		mark = cgs.media.bulletMarkShader;
		light = 0;
		radius = 1;
		duration = 10;	// bit more instant

		colory[0] = 0.7; colory[1] = 0.7; colory[2] = 0.7; colory[3] = 0.0;
		colory2[0] = 0.7; colory2[1] = 0.7; colory2[2] = 0.7; colory2[3] = 0.5;
		colory3[0] = 0.6; colory3[1] = 0.6; colory3[2] = 0.6; colory3[3] = 0.8;
		colory4[0] = 0.6; colory4[1] = 0.6; colory4[2] = 0.6; colory4[3] = 0.0;
	
		VectorMA( origin, 4, dir, sprOrg );
		VectorScale( dir, 1, sprVel );
		
		//CG_LFX_Smoke (sprOrg, sprVel, 62, 5, colory, colory2, colory3, colory4, colory4, 4, 1200,4, 1);
		CG_LFX_Smoke (sprOrg, sprVel, 3, 35, colory, colory2, colory3, colory4, colory4, 50, 1700,26, 0);
		//CG_LFX_Smoke (sprOrg, sprVel, 32, 0.54, colory, colory2, colory3, colory4, colory4, 42, 700, 24, 0);

			// shockwave 
		
				colory[0] = 0.9; colory[1] = 0.8; colory[2] = 1.0; colory[3] = 1.0;
				colory2[0] = 0.7; colory2[1] = 0.5; colory2[2] = 0.2; colory2[3] = 0.9;
				colory3[0] = 0.1; colory3[1] = 0.1; colory3[2] = 0.1; colory3[3] = 0.7;
				colory4[0] = 0.0; colory4[1] = 0.0; colory4[2] = 0.0; colory4[3] = 0.0;
				VectorMA( origin, 4, dir, sprOrg );
				VectorScale( dir, 64, sprVel );
	
				CG_LFX_Shock (origin, dir, 0, 32, colory, colory2, colory3, colory4, colory4, 1, 200, 50,1);
					break;
		}

		if (cg_leiEnhancement.integer == 1996) {	// leilei - q effects

	

		mod = cgs.media.bulletFlashModel;
		shader = cgs.media.bulletExplosionShader;
		mark = cgs.media.bulletMarkShader;
		light = 0;
		radius = 1;
		duration = 10;	// bit more instant

		VectorMA( origin, -5, dir, sprOrg );
		VectorScale( dir, 64, sprVel );
		//CG_RunParticleEffect(sprOrg, sprOrg, 7, 20);

		break;
		}

		mod = cgs.media.bulletFlashModel;
		shader = cgs.media.bulletExplosionShader;
		mark = cgs.media.bulletMarkShader;


#if 0
//Some problems here
		if (cg_leiEnhancement.integer) {
			if( soundType == IMPACTSOUND_FLESH ) {
				mark = cgs.media.lbldShader2;
				} else if( soundType == IMPACTSOUND_METAL ) {
				r = rand() & 4;		if ( r < 3 ) {	mark = cgs.media.lmarkmetal1;
				} else if ( r == 2 ) { mark = cgs.media.lmarkmetal2;
				} else if ( r == 1 ) { mark = cgs.media.lmarkmetal3;
				} else { mark = cgs.media.lmarkmetal4;	}	
				} else {
				r = rand() & 4;		if ( r < 3 ) {	mark = cgs.media.lmarkbullet1;
				} else if ( r == 2 ) { mark = cgs.media.lmarkbullet2;
				} else if ( r == 1 ) { mark = cgs.media.lmarkbullet3;
				} else { mark = cgs.media.lmarkbullet4;
				}       }
			}
#endif


		if (cg_leiEnhancement.integer > 1) {	// leilei - extra dlight
		lightColor[0] = 0.3;
		lightColor[1] = 0.2;
		lightColor[2] = 0.1;
		light = 66;
		duration = 120;	// bit more instant
		}
		sfx = 0;
		radius = 4;
		break;

//#ifdef MISSIONPACK
	case WP_CHAINGUN:
		mod = cgs.media.bulletFlashModel;
		if (cg_leiEnhancement.integer == 1996) {	// leilei - q effects
		light = 0;
		radius = 1;
		duration = 1;	// bit more instant
		break;
		}

		if( soundType == IMPACTSOUND_FLESH ) {
			sfx = cgs.media.sfx_chghitflesh;
		} else if( soundType == IMPACTSOUND_METAL ) {
			sfx = cgs.media.sfx_chghitmetal;
		} else {
			sfx = cgs.media.sfx_chghit;
		}

		mark = cgs.media.bulletMarkShader;
#if 0
//Some problems here
		if (cg_leiEnhancement.integer) {

			if( soundType == IMPACTSOUND_FLESH ) {
				mark = cgs.media.lbldShader2;
			} else if( soundType == IMPACTSOUND_METAL ) {
				r = rand() & 4;		if ( r < 3 ) {	mark = cgs.media.lmarkmetal1;
				} else if ( r == 2 ) { mark = cgs.media.lmarkmetal2;
				} else if ( r == 1 ) { mark = cgs.media.lmarkmetal3;
				} else { mark = cgs.media.lmarkmetal4;	}	
				} else {
				r = rand() & 4;		if ( r < 3 ) {	mark = cgs.media.lmarkbullet1;
				} else if ( r == 2 ) { mark = cgs.media.lmarkbullet2;
				} else if ( r == 1 ) { mark = cgs.media.lmarkbullet3;
				} else { mark = cgs.media.lmarkbullet4;	}
				}
			}
#endif
		r = rand() & 3;
		if ( r < 2 ) {
			sfx = cgs.media.sfx_ric1;
		} else if ( r == 2 ) {
			sfx = cgs.media.sfx_ric2;
		} else {
			sfx = cgs.media.sfx_ric3;
		}

		radius = 8;

		break;
//#endif

	case WP_MACHINEGUN:
		mod = cgs.media.bulletFlashModel;
		shader = cgs.media.bulletExplosionShader;
		mark = cgs.media.bulletMarkShader;
	if (cg_leiEnhancement.integer == 2000) 
		{

		vec4_t colory, colory2, colory3, colory4;	
		colory[0] = 0.7; colory[1] = 0.7; colory[2] = 0.7; colory[3] = 0.0;
		colory2[0] = 0.7; colory2[1] = 0.7; colory2[2] = 0.7; colory2[3] = 0.5;
		colory3[0] = 0.6; colory3[1] = 0.6; colory3[2] = 0.6; colory3[3] = 0.8;
		colory4[0] = 0.6; colory4[1] = 0.6; colory4[2] = 0.6; colory4[3] = 0.0;
	
		VectorMA( origin, 4, dir, sprOrg );
		VectorScale( dir, 1, sprVel );
			duration = 10;	// bit more instant
	
		CG_LFX_Smoke (sprOrg, sprVel, 3, 85, colory, colory2, colory3, colory4, colory4, 50, 1200,12, 0);

			// shockwave 
		
				colory[0] = 0.9; colory[1] = 0.8; colory[2] = 1.0; colory[3] = 1.0;
				colory2[0] = 0.7; colory2[1] = 0.5; colory2[2] = 0.2; colory2[3] = 0.9;
				colory3[0] = 0.1; colory3[1] = 0.1; colory3[2] = 0.1; colory3[3] = 0.7;
				colory4[0] = 0.0; colory4[1] = 0.0; colory4[2] = 0.0; colory4[3] = 0.0;
				VectorMA( origin, 4, dir, sprOrg );
				VectorScale( dir, 64, sprVel );
	
				CG_LFX_Shock (origin, dir, 0, 32, colory, colory2, colory3, colory4, colory4, 1, 200, 40,1);
					break;
		}
		if (cg_leiEnhancement.integer == 1996) {	// leilei - q effects
		light = 0;
		radius = 1;
		duration = 1;	// bit more instant
		break;
		}
#if 0
//Some problems here
		if (cg_leiEnhancement.integer) {
			if( soundType == IMPACTSOUND_FLESH ) {
				mark = cgs.media.lbldShader2;
				} else if( soundType == IMPACTSOUND_METAL ) {
				r = rand() & 4;		if ( r < 3 ) {	mark = cgs.media.lmarkmetal1;
				} else if ( r == 2 ) { mark = cgs.media.lmarkmetal2;
				} else if ( r == 1 ) { mark = cgs.media.lmarkmetal3;
				} else { mark = cgs.media.lmarkmetal4;	}	
				} else {
				r = rand() & 4;		if ( r < 3 ) {	mark = cgs.media.lmarkbullet1;
				} else if ( r == 2 ) { mark = cgs.media.lmarkbullet2;
				} else if ( r == 1 ) { mark = cgs.media.lmarkbullet3;
				} else { mark = cgs.media.lmarkbullet4;
				}   	 
			VectorMA( origin, 4, dir, sprOrg );
			VectorScale( dir, 82, sprVel );

 }
			}
#endif
		r = rand() & 3;
		if ( r == 0 ) {
			sfx = cgs.media.sfx_ric1;
		} else if ( r == 1 ) {
			sfx = cgs.media.sfx_ric2;
		} else {
			sfx = cgs.media.sfx_ric3;
		}

		radius = 8;
		if (cg_leiEnhancement.integer > 1) {	// leilei - extra dlight
		lightColor[0] = 0.3;
		lightColor[1] = 0.2;
		lightColor[2] = 0.1;
		light = 76;
		duration = 120;	// bit more instant
		}
		break;
	}

	if ( sfx ) {
		trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, sfx );
	}

	//
	// create the explosion
	//
	if ( mod ) {
		le = CG_MakeExplosion( origin, dir, 
							   mod,	shader,
							   duration, isSprite );
		le->light = light;
		VectorCopy( lightColor, le->lightColor );
		if ( weapon == WP_RAILGUN ) {
			// colorize with client color
			VectorCopy( cgs.clientinfo[clientNum].color1, le->color );
			le->refEntity.shaderRGBA[0] = le->color[0] * 0xff;
			le->refEntity.shaderRGBA[1] = le->color[1] * 0xff;
			le->refEntity.shaderRGBA[2] = le->color[2] * 0xff;
			le->refEntity.shaderRGBA[3] = 0xff;
		}
	}

	//
	// impact mark
	//
	alphaFade = (mark == cgs.media.energyMarkShader);	// plasma fades alpha, all others fade color
	if ( weapon == WP_RAILGUN ) {
		float	*color;

		// colorize with client color
		color = cgs.clientinfo[clientNum].color1;
		CG_ImpactMark( mark, origin, dir, random()*360, color[0],color[1], color[2],1, alphaFade, radius, qfalse );
	} else {
		CG_ImpactMark( mark, origin, dir, random()*360, 1,1,1,1, alphaFade, radius, qfalse );
	}
}


/*
=================
CG_MissileHitPlayer
=================
*/
void CG_MissileHitPlayer( int weapon, vec3_t origin, vec3_t dir, int entityNum ) {
// LEILEI ENHANCEMENT
	if (cg_leiEnhancement.integer) {
		CG_SmokePuff( origin, dir, 22, 1, 1, 1, 1.0f, 900, cg.time, 0, 0,  cgs.media.lbldShader1 );
		CG_SpurtBlood( origin, dir, 1);
//		CG_SpurtBlood( origin, dir, 4);
//		CG_SpurtBlood( origin, dir, -12);
		}

	else
	CG_Bleed( origin, entityNum );

	// some weapons will make an explosion with the blood, while
	// others will just make the blood
	switch ( weapon ) {
	case WP_GRENADE_LAUNCHER:
	case WP_ROCKET_LAUNCHER:
//#ifdef MISSIONPACK
	case WP_NAILGUN:
	case WP_CHAINGUN:
	case WP_PROX_LAUNCHER:
//#endif
		CG_MissileHitWall( weapon, 0, origin, dir, IMPACTSOUND_FLESH );
		break;
	default:
		break;
	}
}



/*
============================================================================

SHOTGUN TRACING

============================================================================
*/

/*
================
CG_ShotgunPellet
================
*/
static void CG_ShotgunPellet( vec3_t start, vec3_t end, int skipNum ) {
	trace_t		tr;
	int sourceContentType, destContentType;

// LEILEI ENHACNEMENT
	localEntity_t	*smoke;
	vec3_t  kapow;



	CG_Trace( &tr, start, NULL, NULL, end, skipNum, MASK_SHOT );

	sourceContentType = CG_PointContents( start, 0 );
	destContentType = CG_PointContents( tr.endpos, 0 );

	// FIXME: should probably move this cruft into CG_BubbleTrail
	if ( sourceContentType == destContentType ) {
		if ( sourceContentType & CONTENTS_WATER ) {
			CG_BubbleTrail( start, tr.endpos, 32 );
		}
	} else if ( sourceContentType & CONTENTS_WATER ) {
		trace_t trace;

		trap_CM_BoxTrace( &trace, end, start, NULL, NULL, 0, CONTENTS_WATER );
		CG_BubbleTrail( start, trace.endpos, 32 );

// LEILEI ENHANCEMENT
				if (cg_leiEnhancement.integer) {
				// Water Splash
					VectorCopy( trace.plane.normal, kapow );
					
					kapow[0] = kapow[0] * (crandom() * 22);
					kapow[1] = kapow[1] * (crandom() * 22);
					kapow[2] = kapow[2] * (crandom() * 65 + 37);
					smoke = CG_SmokePuff( trace.endpos, kapow, 14, 1, 1, 1, 1.0f, 400, cg.time, 0, 0,  cgs.media.lsplShader );
					smoke = CG_SmokePuff( trace.endpos, kapow, 6, 1, 1, 1, 1.0f, 200, cg.time, 0, 0,  cgs.media.lsplShader );
					smoke = CG_SmokePuff( trace.endpos, kapow, 10, 1, 1, 1, 1.0f, 300, cg.time, 0, 0,  cgs.media.lsplShader );
						
				}
// END LEIHANCMENET
	} else if ( destContentType & CONTENTS_WATER ) {
		trace_t trace;

		trap_CM_BoxTrace( &trace, start, end, NULL, NULL, 0, CONTENTS_WATER );
		CG_BubbleTrail( tr.endpos, trace.endpos, 32 );

// LEILEI ENHANCEMENT
				if (cg_leiEnhancement.integer) {
				// Water Splash
					VectorCopy( trace.plane.normal, kapow );
					
					kapow[0] = kapow[0] * (crandom() * 22);
					kapow[1] = kapow[1] * (crandom() * 22);
					kapow[2] = kapow[2] * (crandom() * 65 + 37);
					smoke = CG_SmokePuff( trace.endpos, kapow, 14, 1, 1, 1, 1.0f, 400, cg.time, 0, 0,  cgs.media.lsplShader );
					smoke = CG_SmokePuff( trace.endpos, kapow, 6, 1, 1, 1, 1.0f, 200, cg.time, 0, 0,  cgs.media.lsplShader );
					smoke = CG_SmokePuff( trace.endpos, kapow, 10, 1, 1, 1, 1.0f, 300, cg.time, 0, 0,  cgs.media.lsplShader );
				}
// END LEIHANCMENET
	}

	if (  tr.surfaceFlags & SURF_NOIMPACT ) {
		return;
	}

	if ( cg_entities[tr.entityNum].currentState.eType == ET_PLAYER ) {
		CG_MissileHitPlayer( WP_SHOTGUN, tr.endpos, tr.plane.normal, tr.entityNum );
	} else {
		if ( tr.surfaceFlags & SURF_NOIMPACT ) {
			// SURF_NOIMPACT will not make a flame puff or a mark
			return;
		}
		if ( tr.surfaceFlags & SURF_METALSTEPS ) {
			CG_MissileHitWall( WP_SHOTGUN, 0, tr.endpos, tr.plane.normal, IMPACTSOUND_METAL );
// LEILEI ENHANCEMENT
			if (cg_leiEnhancement.integer == 2) {
					VectorCopy( tr.plane.normal, kapow );

					kapow[0] = kapow[0] * (crandom() * 65 + 37);
					kapow[1] = kapow[1] * (crandom() * 65 + 37);
					kapow[2] = kapow[2] * (crandom() * 65 + 37);
					CG_LeiSparks(tr.endpos, tr.plane.normal, 400, 0, 0, 7);
					CG_LeiSparks(tr.endpos, tr.plane.normal, 400, 0, 0, 3);
					CG_LeiSparks(tr.endpos, tr.plane.normal, 400, 0, 0, 1);
				
				}
// END LEIHANCMENET
		} else {
			CG_MissileHitWall( WP_SHOTGUN, 0, tr.endpos, tr.plane.normal, IMPACTSOUND_DEFAULT );
	
// LEILEI ENHANCEMENT

				if (cg_leiEnhancement.integer == 1996) {	// leilei - q effects
						vec3_t null;
					null[0] = null[1] = null[2] = 0;
//					CG_RunParticleEffect(tr.endpos,  tr.plane.normal, 7, 20);
					CG_RunParticleEffect(tr.endpos,  null, 7, 20);


						}

				if (cg_leiEnhancement.integer == 2) {
					VectorCopy( tr.plane.normal, kapow );

					kapow[0] = kapow[0] * (crandom() * 65 + 37);
					kapow[1] = kapow[1] * (crandom() * 65 + 37);
					kapow[2] = kapow[2] * (crandom() * 65 + 37);
					CG_LeiSparks(tr.endpos, tr.plane.normal, 400, 0, 0, 7);
					CG_LeiSparks(tr.endpos, tr.plane.normal, 400, 0, 0, 2);
					
					smoke = CG_SmokePuff( tr.endpos, kapow, 21, 1, 1, 1, 0.9f, 1200, cg.time, 0, 0,  cgs.media.lsmkShader2 );
					//smoke = CG_SmokePuff( tr.endpos, kapow, 21, 1, 1, 1, 0.9f, 1200, cg.time, 0, 0,  cgs.media.lbumShader1 );
#if 0
					CG_LeiPuff(tr.endpos, kapow, 500, 0, 0, 177, 6);
					CG_LeiPuff(tr.endpos, tr.plane.normal, 500, 0, 0, 127, 12);
					CG_LeiPuff(tr.endpos, tr.plane.normal, 500, 0, 0, 77, 16);
					CG_LeiPuff(tr.endpos, tr.plane.normal, 500, 0, 0, 127, 12);
					CG_LeiPuff(tr.endpos, tr.plane.normal, 500, 0, 0, 77, 16);
					CG_LeiPuff(tr.endpos, tr.plane.normal, 500, 0, 0, 127, 12);
					CG_LeiPuff(tr.endpos, tr.plane.normal, 500, 0, 0, 77, 16);
#endif
				}
// END LEIHANCMENET
		}
	}
}

/*
================
CG_ShotgunPattern

Perform the same traces the server did to locate the
hit splashes
================
*/
//unlagged - attack prediction
// made this non-static for access from cg_unlagged.c
void CG_ShotgunPattern( vec3_t origin, vec3_t origin2, int seed, int otherEntNum ) {
	int			i;
	float		r, u;
	vec3_t		end;
	vec3_t		forward, right, up;

	// derive the right and up vectors from the forward vector, because
	// the client won't have any other information
	VectorNormalize2( origin2, forward );
	PerpendicularVector( right, forward );
	CrossProduct( forward, right, up );

	// generate the "random" spread pattern
	for ( i = 0 ; i < DEFAULT_SHOTGUN_COUNT ; i++ ) {
		r = Q_crandom( &seed ) * DEFAULT_SHOTGUN_SPREAD * 16;
		u = Q_crandom( &seed ) * DEFAULT_SHOTGUN_SPREAD * 16;
		VectorMA( origin, 8192 * 16, forward, end);
		VectorMA (end, r, right, end);
		VectorMA (end, u, up, end);

		CG_ShotgunPellet( origin, end, otherEntNum );
	}
}

/*
==============
CG_ShotgunFire
==============
*/
void CG_ShotgunFire( entityState_t *es ) {
	vec3_t	v;
	int		contents;

	VectorSubtract( es->origin2, es->pos.trBase, v );
	VectorNormalize( v );
	VectorScale( v, 32, v );
	VectorAdd( es->pos.trBase, v, v );
	if ( cgs.glconfig.hardwareType != GLHW_RAGEPRO ) {
		// ragepro can't alpha fade, so don't even bother with smoke
		vec3_t			up;
		vec3_t			forward;

		contents = CG_PointContents( es->pos.trBase, 0 );
		if ( !( contents & CONTENTS_WATER ) ) {
			VectorSet( up, 0, 0, 8 );
// LEILEI ENHANCEMENT
				if (cg_leiEnhancement.integer) {
				// Shotgun puffy
			//		CG_LeiSparks(v, forward, 200, 0, 0, 7);
			//		CG_LeiSparks(v, forward, 200, 0, 0, 7);
			//		CG_LeiSparks(v, forward, 200, 0, 0, 7);
			//		CG_LeiSparks(v, forward, 20, 0, 0, 7);
			//		CG_LeiSparks(v, forward, 20, 0, 0, 7);
			//		CG_LeiSparks(v, forward, 20, 0, 0, 7);
				/*	VectorSet( up, 4, 4, 4 );
					up[0] = up[0] * (crandom() * 22 + 44);	up[1] = up[1] * (crandom() * 22 + 44);	up[2] = up[2] * (crandom() * 22 + 44);
					CG_SmokePuff( v, up, 14, 1, 1, 1, 0.4f, 900, cg.time, 0, 0,  cgs.media.lsmkShader1 );
					up[0] = up[0] * (crandom() * 22 + 44);	up[1] = up[1] * (crandom() * 22 + 44);	up[2] = up[2] * (crandom() * 22 + 44);
					CG_SmokePuff( v, up, 14, 1, 1, 1, 0.4f, 900, cg.time, 0, 0,  cgs.media.lsmkShader2 );
					up[0] = up[0] * (crandom() * 22 + 44);	up[1] = up[1] * (crandom() * 22 + 44);	up[2] = up[2] * (crandom() * 22 + 44);
					CG_SmokePuff( v, up, 14, 1, 1, 1, 0.4f, 900, cg.time, 0, 0,  cgs.media.lsmkShader3 );
					up[0] = up[0] * (crandom() * 22 + 44);	up[1] = up[1] * (crandom() * 22 + 44);	up[2] = up[2] * (crandom() * 22 + 44);
					CG_SmokePuff( v, up, 14, 1, 1, 1, 0.4f, 900, cg.time, 0, 0,  cgs.media.lsmkShader4 );
				*/
				}
		else
			
// END LEIHANCMENET
			
CG_SmokePuff( v, up, 32, 1, 1, 1, 0.33f, 900, cg.time, 0, LEF_PUFF_DONT_SCALE, cgs.media.shotgunSmokePuffShader );

		}
	}
	CG_ShotgunPattern( es->pos.trBase, es->origin2, es->eventParm, es->otherEntityNum );
}

/*
============================================================================

BULLETS

============================================================================
*/


/*
===============
CG_Tracer
===============
*/
void CG_Tracer( vec3_t source, vec3_t dest ) {
	vec3_t		forward, right;
	polyVert_t	verts[4];
	vec3_t		line;
	float		len, begin, end;
	vec3_t		start, finish;
	vec3_t		midpoint;

	// tracer
	VectorSubtract( dest, source, forward );
	len = VectorNormalize( forward );

	// start at least a little ways from the muzzle
	if ( len < 100 ) {
		return;
	}
	begin = 50 + random() * (len - 60);
	end = begin + cg_tracerLength.value;
	if ( end > len ) {
		end = len;
	}
	VectorMA( source, begin, forward, start );
	VectorMA( source, end, forward, finish );

	line[0] = DotProduct( forward, cg.refdef.viewaxis[1] );
	line[1] = DotProduct( forward, cg.refdef.viewaxis[2] );

	VectorScale( cg.refdef.viewaxis[1], line[1], right );
	VectorMA( right, -line[0], cg.refdef.viewaxis[2], right );
	VectorNormalize( right );

	VectorMA( finish, cg_tracerWidth.value, right, verts[0].xyz );
	verts[0].st[0] = 0;
	verts[0].st[1] = 1;
	verts[0].modulate[0] = 255;
	verts[0].modulate[1] = 255;
	verts[0].modulate[2] = 255;
	verts[0].modulate[3] = 255;

	VectorMA( finish, -cg_tracerWidth.value, right, verts[1].xyz );
	verts[1].st[0] = 1;
	verts[1].st[1] = 0;
	verts[1].modulate[0] = 255;
	verts[1].modulate[1] = 255;
	verts[1].modulate[2] = 255;
	verts[1].modulate[3] = 255;

	VectorMA( start, -cg_tracerWidth.value, right, verts[2].xyz );
	verts[2].st[0] = 1;
	verts[2].st[1] = 1;
	verts[2].modulate[0] = 255;
	verts[2].modulate[1] = 255;
	verts[2].modulate[2] = 255;
	verts[2].modulate[3] = 255;

	VectorMA( start, cg_tracerWidth.value, right, verts[3].xyz );
	verts[3].st[0] = 0;
	verts[3].st[1] = 0;
	verts[3].modulate[0] = 255;
	verts[3].modulate[1] = 255;
	verts[3].modulate[2] = 255;
	verts[3].modulate[3] = 255;

	trap_R_AddPolyToScene( cgs.media.tracerShader, 4, verts );

	midpoint[0] = ( start[0] + finish[0] ) * 0.5;
	midpoint[1] = ( start[1] + finish[1] ) * 0.5;
	midpoint[2] = ( start[2] + finish[2] ) * 0.5;

	// add the tracer sound
	trap_S_StartSound( midpoint, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.tracerSound );

}


/*
======================
CG_CalcMuzzlePoint
======================
*/
static qboolean	CG_CalcMuzzlePoint( int entityNum, vec3_t muzzle ) {
	vec3_t		forward;
	centity_t	*cent;
	int			anim;

	if ( entityNum == cg.snap->ps.clientNum ) {
		VectorCopy( cg.snap->ps.origin, muzzle );
		muzzle[2] += cg.snap->ps.viewheight;
		AngleVectors( cg.snap->ps.viewangles, forward, NULL, NULL );
		VectorMA( muzzle, 14, forward, muzzle );
		return qtrue;
	}

	cent = &cg_entities[entityNum];
	if ( !cent->currentValid ) {
		return qfalse;
	}

	VectorCopy( cent->currentState.pos.trBase, muzzle );

	AngleVectors( cent->currentState.apos.trBase, forward, NULL, NULL );
	anim = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;
	if ( anim == LEGS_WALKCR || anim == LEGS_IDLECR ) {
		muzzle[2] += CROUCH_VIEWHEIGHT;
	} else {
	if (cg_enableQ.integer)
		muzzle[2] += QUACK_VIEWHEIGHT;
		else
		muzzle[2] += DEFAULT_VIEWHEIGHT;
	}

	VectorMA( muzzle, 14, forward, muzzle );

	return qtrue;

}

/*
======================
CG_Bullet

Renders bullet effects.
======================
*/
void CG_Bullet( vec3_t end, int sourceEntityNum, vec3_t normal, qboolean flesh, int fleshEntityNum ) {
	trace_t trace;
	int sourceContentType, destContentType;
	vec3_t		start;
// LEILEI ENHACNEMENT
	localEntity_t	*smoke;
	vec3_t	kapew;	
	vec3_t  kapow;






	// if the shooter is currently valid, calc a source point and possibly
	// do trail effects
	if ( sourceEntityNum >= 0 && cg_tracerChance.value > 0 ) {
		if ( CG_CalcMuzzlePoint( sourceEntityNum, start ) ) {
			sourceContentType = CG_PointContents( start, 0 );
			destContentType = CG_PointContents( end, 0 );

			// do a complete bubble trail if necessary
			if ( ( sourceContentType == destContentType ) && ( sourceContentType & CONTENTS_WATER ) ) {
				CG_BubbleTrail( start, end, 32 );
			}
			// bubble trail from water into air
			else if ( ( sourceContentType & CONTENTS_WATER ) ) {
				trap_CM_BoxTrace( &trace, end, start, NULL, NULL, 0, CONTENTS_WATER );
				CG_BubbleTrail( start, trace.endpos, 32 );


// LEILEI ENHANCEMENT
				if (cg_leiEnhancement.integer) {
				// Water Splash
					VectorCopy( trace.plane.normal, kapow );
					kapow[0] = kapow[0] * (crandom() * 22);
					kapow[1] = kapow[1] * (crandom() * 22);
					kapow[2] = kapow[2] * (crandom() * 65 + 37);
					smoke = CG_SmokePuff( trace.endpos, kapow, 14, 1, 1, 1, 1.0f, 400, cg.time, 0, 0,  cgs.media.lsplShader );
					smoke = CG_SmokePuff( trace.endpos, kapow, 6, 1, 1, 1, 1.0f, 200, cg.time, 0, 0,  cgs.media.lsplShader );
					smoke = CG_SmokePuff( trace.endpos, kapow, 10, 1, 1, 1, 1.0f, 300, cg.time, 0, 0,  cgs.media.lsplShader );
				//	CG_LeiSplash2(trace.endpos, kapow, 900, 0, 0, 444);
						
				}
// END LEIHANCMENET


			}
			// bubble trail from air into water
			else if ( ( destContentType & CONTENTS_WATER ) ) {
				trap_CM_BoxTrace( &trace, start, end, NULL, NULL, 0, CONTENTS_WATER );
				CG_BubbleTrail( trace.endpos, end, 32 );

// LEILEI ENHANCEMENT
				if (cg_leiEnhancement.integer) {
				// Water Splash
					VectorCopy( trace.plane.normal, kapow );
					
					kapow[0] = kapow[0] * (crandom() * 22);
					kapow[1] = kapow[1] * (crandom() * 22);
					kapow[2] = kapow[2] * (crandom() * 65 + 37);
					smoke = CG_SmokePuff( trace.endpos, kapow, 14, 1, 1, 1, 1.0f, 400, cg.time, 0, 0,  cgs.media.lsplShader );
					smoke = CG_SmokePuff( trace.endpos, kapow, 6, 1, 1, 1, 1.0f, 200, cg.time, 0, 0,  cgs.media.lsplShader );
					smoke = CG_SmokePuff( trace.endpos, kapow, 10, 1, 1, 1, 1.0f, 300, cg.time, 0, 0,  cgs.media.lsplShader );
			//CG_LeiSplash2(trace.endpos, kapow, 500, 0, 0, 1);
				}
// END LEIHANCMENET
			}

			// draw a tracer
			if ( random() < cg_tracerChance.value ) {
				CG_Tracer( start, end );
			}
		}
	}

	// impact splash and mark
	if ( flesh ) {
// LEILEI ENHANCEMENT
	if (cg_leiEnhancement.integer) {

		
						// Blood Hack
				VectorCopy( normal, kapow );
					
				kapow[0] = kapow[0] * (crandom() * 65 + 37);
				kapow[1] = kapow[1] * (crandom() * 65 + 37);
				kapow[2] = kapow[2] * (crandom() * 65 + 37);
				VectorCopy( kapow, kapew );

				kapew[0] = kapew[0] * (crandom() * 2 + 37);
				kapew[1] = kapew[1] * (crandom() * 2 + 37);
				kapew[2] = kapew[2] * (crandom() * 2 + 37);
	//	CG_LeiBlood2(end, kapow, 500, 0, 0, 94, 0.6);
	//	CG_LeiBlood2(end, kapew, 800, 0, 0, 54, 0.5);
	//	CG_LeiBlood2(end, kapow, 1200, 0, 0, 24, 1);
	//	CG_LeiBlood2(end, kapow, 1500, 0, 0, 24, 1.2);
	//	CG_LeiBlood2(end, kapow, 1200, 0, 0, 14, 1.7);



		CG_SmokePuff( end, kapow, 6, 1, 1, 1, 1.0f, 600, cg.time, 0, 0,  cgs.media.lbldShader1 );
		CG_SpurtBlood( end, kapow, 2);
		CG_SpurtBlood( end, kapew, 1);

	//	CG_SpurtBlood2( end, kapew, cg_leiDebug.integer, 1, 400);
	//	CG_SpurtBlood2( end, kapew, 41, 1, 700);
		//CG_Particle_Bleed(cgs.media.lbldShader1,kapew,'0 0 0', 0, 100);
//		CG_Particle_Bleed(cgs.media.lbldShader1,kapew,kapow, 0, 100);
//		CG_Particle_BloodCloud(self,end,'0 0 0');

if (cg_leiSuperGoreyAwesome.integer) {
		//CG_SpurtBlood( end, kapow, -2);
			}
		}

	else
		CG_Bleed( end, fleshEntityNum );
	} else {
		CG_MissileHitWall( WP_MACHINEGUN, 0, end, normal, IMPACTSOUND_DEFAULT );

				if (cg_leiEnhancement.integer == 1996) {	// leilei - q effects
						vec3_t null;
					null[0] = null[1] = null[2] = 0;
					CG_RunParticleEffect(end,  null, 7, 20);


						}
// LEILEI ENHANCEMENT
				else if (cg_leiEnhancement.integer) {

				// Smoke puff
					VectorCopy( normal, kapow );
					
					kapow[0] = kapow[0] * (crandom() * 65 + 37);
					kapow[1] = kapow[1] * (crandom() * 65 + 37);
					kapow[2] = kapow[2] * (crandom() * 65 + 37);
					VectorCopy( kapow, kapew );

					kapew[0] = kapew[0] * (crandom() * 65 + 37);
					kapew[1] = kapew[1] * (crandom() * 65 + 37);
					kapew[2] = kapew[2] * (crandom() * 65 + 37);


					smoke = CG_SmokePuff( end, kapow, 14, 1, 1, 1, 1.0f, 600, cg.time, 0, 0,  cgs.media.lsmkShader1 );
					CG_LeiSparks(end, normal, 200, 0, 0, 177);
					CG_LeiSparks(end, normal, 200, 0, 0, 155);
					CG_LeiSparks(end, normal, 200, 0, 0, 444);
					CG_LeiSparks(trace.endpos, trace.plane.normal, 300, 0, 0, 7);
					CG_LeiSparks(trace.endpos, trace.plane.normal, 300, 0, 0, 3);
					CG_LeiSparks(trace.endpos, trace.plane.normal, 300, 0, 0, 1);

				}
// END LEIHANCMENET
	}

}
