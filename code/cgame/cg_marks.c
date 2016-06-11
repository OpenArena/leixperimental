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
// cg_marks.c -- wall marks

#include "cg_local.h"

/*
===================================================================

MARK POLYS

===================================================================
*/


markPoly_t	cg_activeMarkPolys;			// double linked list
markPoly_t	*cg_freeMarkPolys;			// single linked list
markPoly_t	cg_markPolys[MAX_MARK_POLYS];
static		int	markTotal;

/*
===================
CG_InitMarkPolys

This is called at startup and for tournement restarts
===================
*/
void	CG_InitMarkPolys( void ) {
	int		i;

	memset( cg_markPolys, 0, sizeof(cg_markPolys) );

	cg_activeMarkPolys.nextMark = &cg_activeMarkPolys;
	cg_activeMarkPolys.prevMark = &cg_activeMarkPolys;
	cg_freeMarkPolys = cg_markPolys;
	for ( i = 0 ; i < MAX_MARK_POLYS - 1 ; i++ ) {
		cg_markPolys[i].nextMark = &cg_markPolys[i+1];
	}
}


/*
==================
CG_FreeMarkPoly
==================
*/
void CG_FreeMarkPoly( markPoly_t *le ) {
	if ( !le->prevMark ) {
		CG_Error( "CG_FreeLocalEntity: not active" );
	}

	// remove from the doubly linked active list
	le->prevMark->nextMark = le->nextMark;
	le->nextMark->prevMark = le->prevMark;

	// the free list is only singly linked
	le->nextMark = cg_freeMarkPolys;
	cg_freeMarkPolys = le;
}

/*
===================
CG_AllocMark

Will allways succeed, even if it requires freeing an old active mark
===================
*/
markPoly_t	*CG_AllocMark( void ) {
	markPoly_t	*le;
	int time;

	if ( !cg_freeMarkPolys ) {
		// no free entities, so free the one at the end of the chain
		// remove the oldest active entity
		time = cg_activeMarkPolys.prevMark->time;
		while (cg_activeMarkPolys.prevMark && time == cg_activeMarkPolys.prevMark->time) {
			CG_FreeMarkPoly( cg_activeMarkPolys.prevMark );
		}
	}

	le = cg_freeMarkPolys;
	cg_freeMarkPolys = cg_freeMarkPolys->nextMark;

	memset( le, 0, sizeof( *le ) );

	// link into the active list
	le->nextMark = cg_activeMarkPolys.nextMark;
	le->prevMark = &cg_activeMarkPolys;
	cg_activeMarkPolys.nextMark->prevMark = le;
	cg_activeMarkPolys.nextMark = le;
	return le;
}



/*
=================
CG_ImpactMark

origin should be a point within a unit of the plane
dir should be the plane normal

temporary marks will not be stored or randomly oriented, but immediately
passed to the renderer.
=================
*/
#define	MAX_MARK_FRAGMENTS	128
#define	MAX_MARK_POINTS		384

void CG_ImpactMark( qhandle_t markShader, const vec3_t origin, const vec3_t dir, 
				   float orientation, float red, float green, float blue, float alpha,
				   qboolean alphaFade, float radius, qboolean temporary ) {
	vec3_t			axis[3];
	float			texCoordScale;
	vec3_t			originalPoints[4];
	byte			colors[4];
	int				i, j;
	int				numFragments;
	markFragment_t	markFragments[MAX_MARK_FRAGMENTS], *mf;
	vec3_t			markPoints[MAX_MARK_POINTS];
	vec3_t			projection;

	if ( !cg_addMarks.integer ) {
		return;
	}

	if ( radius <= 0 ) {
		CG_Error( "CG_ImpactMark called with <= 0 radius" );
	}

	//if ( markTotal >= MAX_MARK_POLYS ) {
	//	return;
	//}

	// create the texture axis
	VectorNormalize2( dir, axis[0] );
	PerpendicularVector( axis[1], axis[0] );
	//if (cg_leiEnhancement.integer) // LEILEI HACK HACK HACK - don't spin atlas variated particles (for consistent lighting on the texture)
	//orientation = 90; 
	
	RotatePointAroundVector( axis[2], axis[0], axis[1], orientation );
	CrossProduct( axis[0], axis[2], axis[1] );

	texCoordScale = 0.5 * 1.0 / radius;

	// create the full polygon
	for ( i = 0 ; i < 3 ; i++ ) {
		originalPoints[0][i] = origin[i] - radius * axis[1][i] - radius * axis[2][i];
		originalPoints[1][i] = origin[i] + radius * axis[1][i] - radius * axis[2][i];
		originalPoints[2][i] = origin[i] + radius * axis[1][i] + radius * axis[2][i];
		originalPoints[3][i] = origin[i] - radius * axis[1][i] + radius * axis[2][i];
	}

	// get the fragments
	VectorScale( dir, -20, projection );
	numFragments = trap_CM_MarkFragments( 4, (void *)originalPoints,
					projection, MAX_MARK_POINTS, markPoints[0],
					MAX_MARK_FRAGMENTS, markFragments );

	colors[0] = red * 255;
	colors[1] = green * 255;
	colors[2] = blue * 255;
	colors[3] = alpha * 255;

	for ( i = 0, mf = markFragments ; i < numFragments ; i++, mf++ ) {
		polyVert_t	*v;
		polyVert_t	verts[MAX_VERTS_ON_POLY];
		markPoly_t	*mark;

		// we have an upper limit on the complexity of polygons
		// that we store persistantly
		if ( mf->numPoints > MAX_VERTS_ON_POLY ) {
			mf->numPoints = MAX_VERTS_ON_POLY;
		}
		for ( j = 0, v = verts ; j < mf->numPoints ; j++, v++ ) {
			vec3_t		delta;

			VectorCopy( markPoints[mf->firstPoint + j], v->xyz );

			VectorSubtract( v->xyz, origin, delta );
			v->st[0] = 0.5 + DotProduct( delta, axis[1] ) * texCoordScale;
			v->st[1] = 0.5 + DotProduct( delta, axis[2] ) * texCoordScale;
			*(int *)v->modulate = *(int *)colors;
		}

		// if it is a temporary (shadow) mark, add it immediately and forget about it
		if ( temporary ) {
			trap_R_AddPolyToScene( markShader, mf->numPoints, verts );
			continue;
		}

		// otherwise save it persistantly
		mark = CG_AllocMark();
		mark->time = cg.time;
		mark->alphaFade = alphaFade;
		mark->markShader = markShader;
		mark->poly.numVerts = mf->numPoints;
		mark->color[0] = red;
		mark->color[1] = green;
		mark->color[2] = blue;
		mark->color[3] = alpha;
		memcpy( mark->verts, verts, mf->numPoints * sizeof( verts[0] ) );
		markTotal++;
	}
}


/*
===============
CG_AddMarks
===============
*/
#define	MARK_TOTAL_TIME		10000
#define	MARK_FADE_TIME		1000

void CG_AddMarks( void ) {
	int			j;
	markPoly_t	*mp, *next;
	int			t;
	int			fade;

	if ( !cg_addMarks.integer ) {
		return;
	}

	mp = cg_activeMarkPolys.nextMark;
	for ( ; mp != &cg_activeMarkPolys ; mp = next ) {
		// grab next now, so if the local entity is freed we
		// still have it
		next = mp->nextMark;

		// see if it is time to completely remove it
		if ( cg.time > mp->time + MARK_TOTAL_TIME ) {
			CG_FreeMarkPoly( mp );
			continue;
		}

		// fade out the energy bursts
		if ( mp->markShader == cgs.media.energyMarkShader ) {

			fade = 450 - 450 * ( (cg.time - mp->time ) / 3000.0 );
			if ( fade < 255 ) {
				if ( fade < 0 ) {
					fade = 0;
				}
				if ( mp->verts[0].modulate[0] != 0 ) {
					for ( j = 0 ; j < mp->poly.numVerts ; j++ ) {
						mp->verts[j].modulate[0] = mp->color[0] * fade;
						mp->verts[j].modulate[1] = mp->color[1] * fade;
						mp->verts[j].modulate[2] = mp->color[2] * fade;
					}
				}
			}
		}

		// fade all marks out with time
		t = mp->time + MARK_TOTAL_TIME - cg.time;
		if ( t < MARK_FADE_TIME ) {
			fade = 255 * t / MARK_FADE_TIME;
			if ( mp->alphaFade ) {
				for ( j = 0 ; j < mp->poly.numVerts ; j++ ) {
					mp->verts[j].modulate[3] = fade;
				}
			} else {
				for ( j = 0 ; j < mp->poly.numVerts ; j++ ) {
					mp->verts[j].modulate[0] = mp->color[0] * fade;
					mp->verts[j].modulate[1] = mp->color[1] * fade;
					mp->verts[j].modulate[2] = mp->color[2] * fade;
				}
			}
		}


		trap_R_AddPolyToScene( mp->markShader, mp->poly.numVerts, mp->verts );
	}
}

// cg_particles.c  


#define BLOODRED	2
#define EMISIVEFADE	3
#define GREY75		4

typedef struct particle_s
{
	struct particle_s	*next;

	float		time;
	float		endtime;

	vec3_t		org;
	vec3_t		vel;
	vec3_t		accel;
	int			color;
	float		colorvel;
	float		alpha;
	float		alphavel;
	int			type;
	qhandle_t	pshader;
	
	float		height;
	float		width;
				
	float		endheight;
	float		endwidth;
	
	float		start;
	float		end;

	float		startfade;
	qboolean	rotate;
	int			snum;
	
	qboolean	link;

	// Ridah
	int			shaderAnim;
	int			roll;

	int			accumroll;

	// leilei
	float		stretch;	
	vec3_t		angle;
	vec3_t		avelocity;
	vec3_t		src;
	vec3_t		dest;
	float		airfriction;
	int		qolor;		// quake palette color translation
	int		ramp;		// quake color ramping (rocket trails, explosion)
	qboolean	qarticle;	// is a quake style particle

	vec4_t		cols[5];	// fading color cycle
	vec3_t		stretchorg;	// old origin to stretch from
	vec3_t		stretchorg2;	// even older origin to stretch from
	vec3_t		dir;		// angle, direction
	float		bounce;		// how much elasticity does it have
	float		len;		// length of spark
	float		timetrail;	// time of trail......
} cparticle_t;

// more quake stuff


int ramp1[8] = { 0x6f, 0x6d, 0x6b, 0x69, 0x67, 0x65, 0x63, 0x61 };
int ramp2[8] = { 0x6f, 0x6e, 0x6d, 0x6c, 0x6b, 0x6a, 0x68, 0x66 };
int ramp3[8] = { 0x6d, 0x6b, 6, 5, 4, 3 };




float qpalette[256][3] = {
     0.000000, 0.000000, 0.000000 ,
     0.058824, 0.058824, 0.058824 ,
     0.121569, 0.121569, 0.121569 ,
     0.184314, 0.184314, 0.184314 ,
     0.247059, 0.247059, 0.247059 ,
     0.294118, 0.294118, 0.294118 ,
     0.356863, 0.356863, 0.356863 ,
     0.419608, 0.419608, 0.419608 ,
     0.482353, 0.482353, 0.482353 ,
     0.545098, 0.545098, 0.545098 ,
     0.607843, 0.607843, 0.607843 ,
     0.670588, 0.670588, 0.670588 ,
     0.733333, 0.733333, 0.733333 ,
     0.796078, 0.796078, 0.796078 ,
     0.858824, 0.858824, 0.858824 ,
     0.921569, 0.921569, 0.921569 ,
     0.058824, 0.043137, 0.027451 ,
     0.090196, 0.058824, 0.043137 ,
     0.121569, 0.090196, 0.043137 ,
     0.152941, 0.105882, 0.058824 ,
     0.184314, 0.137255, 0.074510 ,
     0.215686, 0.168627, 0.090196 ,
     0.247059, 0.184314, 0.090196 ,
     0.294118, 0.215686, 0.105882 ,
     0.325490, 0.231373, 0.105882 ,
     0.356863, 0.262745, 0.121569 ,
     0.388235, 0.294118, 0.121569 ,
     0.419608, 0.325490, 0.121569 ,
     0.450980, 0.341176, 0.121569 ,
     0.482353, 0.372549, 0.137255 ,
     0.513725, 0.403922, 0.137255 ,
     0.560784, 0.435294, 0.137255 ,
     0.043137, 0.043137, 0.058824 ,
     0.074510, 0.074510, 0.105882 ,
     0.105882, 0.105882, 0.152941 ,
     0.152941, 0.152941, 0.200000 ,
     0.184314, 0.184314, 0.247059 ,
     0.215686, 0.215686, 0.294118 ,
     0.247059, 0.247059, 0.341176 ,
     0.278431, 0.278431, 0.403922 ,
     0.309804, 0.309804, 0.450980 ,
     0.356863, 0.356863, 0.498039 ,
     0.388235, 0.388235, 0.545098 ,
     0.419608, 0.419608, 0.592157 ,
     0.450980, 0.450980, 0.639216 ,
     0.482353, 0.482353, 0.686275 ,
     0.513725, 0.513725, 0.733333 ,
     0.545098, 0.545098, 0.796078 ,
     0.000000, 0.000000, 0.000000 ,
     0.027451, 0.027451, 0.000000 ,
     0.043137, 0.043137, 0.000000 ,
     0.074510, 0.074510, 0.000000 ,
     0.105882, 0.105882, 0.000000 ,
     0.137255, 0.137255, 0.000000 ,
     0.168627, 0.168627, 0.027451 ,
     0.184314, 0.184314, 0.027451 ,
     0.215686, 0.215686, 0.027451 ,
     0.247059, 0.247059, 0.027451 ,
     0.278431, 0.278431, 0.027451 ,
     0.294118, 0.294118, 0.043137 ,
     0.325490, 0.325490, 0.043137 ,
     0.356863, 0.356863, 0.043137 ,
     0.388235, 0.388235, 0.043137 ,
     0.419608, 0.419608, 0.058824 ,
     0.027451, 0.000000, 0.000000 ,
     0.058824, 0.000000, 0.000000 ,
     0.090196, 0.000000, 0.000000 ,
     0.121569, 0.000000, 0.000000 ,
     0.152941, 0.000000, 0.000000 ,
     0.184314, 0.000000, 0.000000 ,
     0.215686, 0.000000, 0.000000 ,
     0.247059, 0.000000, 0.000000 ,
     0.278431, 0.000000, 0.000000 ,
     0.309804, 0.000000, 0.000000 ,
     0.341176, 0.000000, 0.000000 ,
     0.372549, 0.000000, 0.000000 ,
     0.403922, 0.000000, 0.000000 ,
     0.435294, 0.000000, 0.000000 ,
     0.466667, 0.000000, 0.000000 ,
     0.498039, 0.000000, 0.000000 ,
     0.074510, 0.074510, 0.000000 ,
     0.105882, 0.105882, 0.000000 ,
     0.137255, 0.137255, 0.000000 ,
     0.184314, 0.168627, 0.000000 ,
     0.215686, 0.184314, 0.000000 ,
     0.262745, 0.215686, 0.000000 ,
     0.294118, 0.231373, 0.027451 ,
     0.341176, 0.262745, 0.027451 ,
     0.372549, 0.278431, 0.027451 ,
     0.419608, 0.294118, 0.043137 ,
     0.466667, 0.325490, 0.058824 ,
     0.513725, 0.341176, 0.074510 ,
     0.545098, 0.356863, 0.074510 ,
     0.592157, 0.372549, 0.105882 ,
     0.639216, 0.388235, 0.121569 ,
     0.686275, 0.403922, 0.137255 ,
     0.137255, 0.074510, 0.027451 ,
     0.184314, 0.090196, 0.043137 ,
     0.231373, 0.121569, 0.058824 ,
     0.294118, 0.137255, 0.074510 ,
     0.341176, 0.168627, 0.090196 ,
     0.388235, 0.184314, 0.121569 ,
     0.450980, 0.215686, 0.137255 ,
     0.498039, 0.231373, 0.168627 ,
     0.560784, 0.262745, 0.200000 ,
     0.623529, 0.309804, 0.200000 ,
     0.686275, 0.388235, 0.184314 ,
     0.749020, 0.466667, 0.184314 ,
     0.811765, 0.560784, 0.168627 ,
     0.874510, 0.670588, 0.152941 ,
     0.937255, 0.796078, 0.121569 ,
     1.000000, 0.952941, 0.105882 ,
     0.043137, 0.027451, 0.000000 ,
     0.105882, 0.074510, 0.000000 ,
     0.168627, 0.137255, 0.058824 ,
     0.215686, 0.168627, 0.074510 ,
     0.278431, 0.200000, 0.105882 ,
     0.325490, 0.215686, 0.137255 ,
     0.388235, 0.247059, 0.168627 ,
     0.435294, 0.278431, 0.200000 ,
     0.498039, 0.325490, 0.247059 ,
     0.545098, 0.372549, 0.278431 ,
     0.607843, 0.419608, 0.325490 ,
     0.654902, 0.482353, 0.372549 ,
     0.717647, 0.529412, 0.419608 ,
     0.764706, 0.576471, 0.482353 ,
     0.827451, 0.639216, 0.545098 ,
     0.890196, 0.701961, 0.592157 ,
     0.670588, 0.545098, 0.639216 ,
     0.623529, 0.498039, 0.592157 ,
     0.576471, 0.450980, 0.529412 ,
     0.545098, 0.403922, 0.482353 ,
     0.498039, 0.356863, 0.435294 ,
     0.466667, 0.325490, 0.388235 ,
     0.419608, 0.294118, 0.341176 ,
     0.372549, 0.247059, 0.294118 ,
     0.341176, 0.215686, 0.262745 ,
     0.294118, 0.184314, 0.215686 ,
     0.262745, 0.152941, 0.184314 ,
     0.215686, 0.121569, 0.137255 ,
     0.168627, 0.090196, 0.105882 ,
     0.137255, 0.074510, 0.074510 ,
     0.090196, 0.043137, 0.043137 ,
     0.058824, 0.027451, 0.027451 ,
     0.733333, 0.450980, 0.623529 ,
     0.686275, 0.419608, 0.560784 ,
     0.639216, 0.372549, 0.513725 ,
     0.592157, 0.341176, 0.466667 ,
     0.545098, 0.309804, 0.419608 ,
     0.498039, 0.294118, 0.372549 ,
     0.450980, 0.262745, 0.325490 ,
     0.419608, 0.231373, 0.294118 ,
     0.372549, 0.200000, 0.247059 ,
     0.325490, 0.168627, 0.215686 ,
     0.278431, 0.137255, 0.168627 ,
     0.231373, 0.121569, 0.137255 ,
     0.184314, 0.090196, 0.105882 ,
     0.137255, 0.074510, 0.074510 ,
     0.090196, 0.043137, 0.043137 ,
     0.058824, 0.027451, 0.027451 ,
     0.858824, 0.764706, 0.733333 ,
     0.796078, 0.701961, 0.654902 ,
     0.749020, 0.639216, 0.607843 ,
     0.686275, 0.592157, 0.545098 ,
     0.639216, 0.529412, 0.482353 ,
     0.592157, 0.482353, 0.435294 ,
     0.529412, 0.435294, 0.372549 ,
     0.482353, 0.388235, 0.325490 ,
     0.419608, 0.341176, 0.278431 ,
     0.372549, 0.294118, 0.231373 ,
     0.325490, 0.247059, 0.200000 ,
     0.262745, 0.200000, 0.152941 ,
     0.215686, 0.168627, 0.121569 ,
     0.152941, 0.121569, 0.090196 ,
     0.105882, 0.074510, 0.058824 ,
     0.058824, 0.043137, 0.027451 ,
     0.435294, 0.513725, 0.482353 ,
     0.403922, 0.482353, 0.435294 ,
     0.372549, 0.450980, 0.403922 ,
     0.341176, 0.419608, 0.372549 ,
     0.309804, 0.388235, 0.341176 ,
     0.278431, 0.356863, 0.309804 ,
     0.247059, 0.325490, 0.278431 ,
     0.215686, 0.294118, 0.247059 ,
     0.184314, 0.262745, 0.215686 ,
     0.168627, 0.231373, 0.184314 ,
     0.137255, 0.200000, 0.152941 ,
     0.121569, 0.168627, 0.121569 ,
     0.090196, 0.137255, 0.090196 ,
     0.058824, 0.105882, 0.074510 ,
     0.043137, 0.074510, 0.043137 ,
     0.027451, 0.043137, 0.027451 ,
     1.000000, 0.952941, 0.105882 ,
     0.937255, 0.874510, 0.090196 ,
     0.858824, 0.796078, 0.074510 ,
     0.796078, 0.717647, 0.058824 ,
     0.733333, 0.654902, 0.058824 ,
     0.670588, 0.592157, 0.043137 ,
     0.607843, 0.513725, 0.027451 ,
     0.545098, 0.450980, 0.027451 ,
     0.482353, 0.388235, 0.027451 ,
     0.419608, 0.325490, 0.000000 ,
     0.356863, 0.278431, 0.000000 ,
     0.294118, 0.215686, 0.000000 ,
     0.231373, 0.168627, 0.000000 ,
     0.168627, 0.121569, 0.000000 ,
     0.105882, 0.058824, 0.000000 ,
     0.043137, 0.027451, 0.000000 ,
     0.000000, 0.000000, 1.000000 ,
     0.043137, 0.043137, 0.937255 ,
     0.074510, 0.074510, 0.874510 ,
     0.105882, 0.105882, 0.811765 ,
     0.137255, 0.137255, 0.749020 ,
     0.168627, 0.168627, 0.686275 ,
     0.184314, 0.184314, 0.623529 ,
     0.184314, 0.184314, 0.560784 ,
     0.184314, 0.184314, 0.498039 ,
     0.184314, 0.184314, 0.435294 ,
     0.184314, 0.184314, 0.372549 ,
     0.168627, 0.168627, 0.309804 ,
     0.137255, 0.137255, 0.247059 ,
     0.105882, 0.105882, 0.184314 ,
     0.074510, 0.074510, 0.121569 ,
     0.043137, 0.043137, 0.058824 ,
     0.168627, 0.000000, 0.000000 ,
     0.231373, 0.000000, 0.000000 ,
     0.294118, 0.027451, 0.000000 ,
     0.372549, 0.027451, 0.000000 ,
     0.435294, 0.058824, 0.000000 ,
     0.498039, 0.090196, 0.027451 ,
     0.576471, 0.121569, 0.027451 ,
     0.639216, 0.152941, 0.043137 ,
     0.717647, 0.200000, 0.058824 ,
     0.764706, 0.294118, 0.105882 ,
     0.811765, 0.388235, 0.168627 ,
     0.858824, 0.498039, 0.231373 ,
     0.890196, 0.592157, 0.309804 ,
     0.905882, 0.670588, 0.372549 ,
     0.937255, 0.749020, 0.466667 ,
     0.968627, 0.827451, 0.545098 ,
     0.654902, 0.482353, 0.231373 ,
     0.717647, 0.607843, 0.215686 ,
     0.780392, 0.764706, 0.215686 ,
     0.905882, 0.890196, 0.341176 ,
     0.498039, 0.749020, 1.000000 ,
     0.670588, 0.905882, 1.000000 ,
     0.843137, 1.000000, 1.000000 ,
     0.403922, 0.000000, 0.000000 ,
     0.545098, 0.000000, 0.000000 ,
     0.701961, 0.000000, 0.000000 ,
     0.843137, 0.000000, 0.000000 ,
     1.000000, 0.000000, 0.000000 ,
     1.000000, 0.952941, 0.576471 ,
     1.000000, 0.968627, 0.780392 ,
     1.000000, 1.000000, 1.000000 ,
     0.623529, 0.356863, 0.325490 
};

typedef enum
{
	P_NONE,
	P_WEATHER,
	P_FLAT,
	P_SMOKE,
	P_ROTATE,
	P_WEATHER_TURBULENT,
	P_ANIM,	// Ridah
	P_BAT,
	P_BLEED,
	P_FLAT_SCALEUP,
	P_FLAT_SCALEUP_FADE,
	P_WEATHER_FLURRY,
	P_SMOKE_IMPACT,
	P_BUBBLE,
	P_BUBBLE_TURBULENT,
	P_SPRITE,
	P_BEAM,	// leilei	- angle not calculated
	P_SPARK,	// leilei	- angle and length recalculated from velocity
	P_LFX,		// leilei - LFX smoke and balls and
	P_QUAKE		// leilei - use quake style 'properties'
} particle_type_t;

#define	MAX_SHADER_ANIMS		32
#define	MAX_SHADER_ANIM_FRAMES	64

static char *shaderAnimNames[MAX_SHADER_ANIMS] = {
	"explode1",
	NULL
};
static qhandle_t shaderAnims[MAX_SHADER_ANIMS][MAX_SHADER_ANIM_FRAMES];
static int	shaderAnimCounts[MAX_SHADER_ANIMS] = {
	23
};
static float	shaderAnimSTRatio[MAX_SHADER_ANIMS] = {
	1.0f
};
static int	numShaderAnims;
// done.

#define		PARTICLE_GRAVITY	40
#define		MAX_PARTICLES	2048

cparticle_t	*active_particles, *free_particles;
cparticle_t	particles[MAX_PARTICLES];
int		cl_numparticles = MAX_PARTICLES;

qboolean		initparticles = qfalse;
vec3_t			pvforward, pvright, pvup;
vec3_t			rforward, rright, rup;

float			oldtime;


/*
===============
CL_ClearParticles
===============
*/
void CG_ClearParticles (void)
{
	int		i;

	memset( particles, 0, sizeof(particles) );

	free_particles = &particles[0];
	active_particles = NULL;

	for (i=0 ;i<cl_numparticles ; i++)
	{
		particles[i].next = &particles[i+1];
		particles[i].type = 0;
	}
	particles[cl_numparticles-1].next = NULL;

	oldtime = cg.time;

	// Ridah, init the shaderAnims
	for (i=0; shaderAnimNames[i]; i++) {
		int j;

		for (j=0; j<shaderAnimCounts[i]; j++) {
			shaderAnims[i][j] = trap_R_RegisterShader( va("%s%i", shaderAnimNames[i], j+1) );
		}
	}
	numShaderAnims = i;
	// done.

	initparticles = qtrue;
}


/*
=====================
CG_AddParticleToScene
=====================
*/
void CG_AddParticleToScene (cparticle_t *p, vec3_t org, float alpha)
{

	vec3_t		point;
	polyVert_t	verts[4];
	float		width;
	float		height;
	float		time, time2;
	float		ratio;
	float		invratio;
	vec3_t		color;
	polyVert_t	TRIverts[3];
	vec3_t		rright2, rup2;
	vec3_t		oldorg;
	vec3_t		stretchorg;
  	float frametime;

	if (p->type == P_WEATHER || p->type == P_WEATHER_TURBULENT || p->type == P_WEATHER_FLURRY
		|| p->type == P_BUBBLE || p->type == P_BUBBLE_TURBULENT)
	{// create a front facing polygon
			
	
	
		// Ridah, had to do this or MAX_POLYS is being exceeded in village1.bsp
		if (Distance( cg.snap->ps.origin, org ) > 1024) {
			return;
		}
		// done.
	
		if (p->type == P_BUBBLE || p->type == P_BUBBLE_TURBULENT)
		{
			VectorMA (org, -p->height, pvup, point);	
			VectorMA (point, -p->width, pvright, point);	
			VectorCopy (point, verts[0].xyz);	
			verts[0].st[0] = 0;	
			verts[0].st[1] = 0;	
			verts[0].modulate[0] = 255;	
			verts[0].modulate[1] = 255;	
			verts[0].modulate[2] = 255;	
			verts[0].modulate[3] = 255 * p->alpha;	

			VectorMA (org, -p->height, pvup, point);	
			VectorMA (point, p->width, pvright, point);	
			VectorCopy (point, verts[1].xyz);	
			verts[1].st[0] = 0;	
			verts[1].st[1] = 1;	
			verts[1].modulate[0] = 255;	
			verts[1].modulate[1] = 255;	
			verts[1].modulate[2] = 255;	
			verts[1].modulate[3] = 255 * p->alpha;	

			VectorMA (org, p->height, pvup, point);	
			VectorMA (point, p->width, pvright, point);	
			VectorCopy (point, verts[2].xyz);	
			verts[2].st[0] = 1;	
			verts[2].st[1] = 1;	
			verts[2].modulate[0] = 255;	
			verts[2].modulate[1] = 255;	
			verts[2].modulate[2] = 255;	
			verts[2].modulate[3] = 255 * p->alpha;	

			VectorMA (org, p->height, pvup, point);	
			VectorMA (point, -p->width, pvright, point);	
			VectorCopy (point, verts[3].xyz);	
			verts[3].st[0] = 1;	
			verts[3].st[1] = 0;	
			verts[3].modulate[0] = 255;	
			verts[3].modulate[1] = 255;	
			verts[3].modulate[2] = 255;	
			verts[3].modulate[3] = 255 * p->alpha;	
		}
		else
		{
			VectorMA (org, -p->height, pvup, point);	
			VectorMA (point, -p->width, pvright, point);	
			VectorCopy( point, TRIverts[0].xyz );
			TRIverts[0].st[0] = 1;
			TRIverts[0].st[1] = 0;
			TRIverts[0].modulate[0] = 255;
			TRIverts[0].modulate[1] = 255;
			TRIverts[0].modulate[2] = 255;
			TRIverts[0].modulate[3] = 255 * p->alpha;	

			VectorMA (org, p->height, pvup, point);	
			VectorMA (point, -p->width, pvright, point);	
			VectorCopy (point, TRIverts[1].xyz);	
			TRIverts[1].st[0] = 0;
			TRIverts[1].st[1] = 0;
			TRIverts[1].modulate[0] = 255;
			TRIverts[1].modulate[1] = 255;
			TRIverts[1].modulate[2] = 255;
			TRIverts[1].modulate[3] = 255 * p->alpha;	

			VectorMA (org, p->height, pvup, point);	
			VectorMA (point, p->width, pvright, point);	
			VectorCopy (point, TRIverts[2].xyz);	
			TRIverts[2].st[0] = 0;
			TRIverts[2].st[1] = 1;
			TRIverts[2].modulate[0] = 255;
			TRIverts[2].modulate[1] = 255;
			TRIverts[2].modulate[2] = 255;
			TRIverts[2].modulate[3] = 255 * p->alpha;	
		}
	
	}
	else if (p->type == P_SPRITE)
	{
		vec3_t	rr, ru;
		vec3_t	rotate_ang;

		VectorSet (color, 1.0, 1.0, 0.5);
		time = cg.time - p->time;
		time2 = p->endtime - p->time;
		ratio = time / time2;

		width = p->width + ( ratio * ( p->endwidth - p->width) );
		height = p->height + ( ratio * ( p->endheight - p->height) );

		if (p->roll) {
			vectoangles( cg.refdef.viewaxis[0], rotate_ang );
			rotate_ang[ROLL] += p->roll;
			AngleVectors ( rotate_ang, NULL, rr, ru);
		}

		if (p->roll) {
			VectorMA (org, -height, ru, point);	
			VectorMA (point, -width, rr, point);	
		} else {
			VectorMA (org, -height, pvup, point);	
			VectorMA (point, -width, pvright, point);	
		}
		VectorCopy (point, verts[0].xyz);	
		verts[0].st[0] = 0;	
		verts[0].st[1] = 0;	
		verts[0].modulate[0] = 255;	
		verts[0].modulate[1] = 255;	
		verts[0].modulate[2] = 255;	
		verts[0].modulate[3] = 255;

		if (p->roll) {
			VectorMA (point, 2*height, ru, point);	
		} else {
			VectorMA (point, 2*height, pvup, point);	
		}
		VectorCopy (point, verts[1].xyz);	
		verts[1].st[0] = 0;	
		verts[1].st[1] = 1;	
		verts[1].modulate[0] = 255;	
		verts[1].modulate[1] = 255;	
		verts[1].modulate[2] = 255;	
		verts[1].modulate[3] = 255;	

		if (p->roll) {
			VectorMA (point, 2*width, rr, point);	
		} else {
			VectorMA (point, 2*width, pvright, point);	
		}
		VectorCopy (point, verts[2].xyz);	
		verts[2].st[0] = 1;	
		verts[2].st[1] = 1;	
		verts[2].modulate[0] = 255;	
		verts[2].modulate[1] = 255;	
		verts[2].modulate[2] = 255;	
		verts[2].modulate[3] = 255;	

		if (p->roll) {
			VectorMA (point, -2*height, ru, point);	
		} else {
			VectorMA (point, -2*height, pvup, point);	
		}
		VectorCopy (point, verts[3].xyz);	
		verts[3].st[0] = 1;	
		verts[3].st[1] = 0;	
		verts[3].modulate[0] = 255;	
		verts[3].modulate[1] = 255;	
		verts[3].modulate[2] = 255;	
		verts[3].modulate[3] = 255;	
	}
		else if (p->type == P_SPARK)	// leilei - to return......
	{
	
		
	}
	else if (p->type == P_SMOKE || p->type == P_SMOKE_IMPACT)
	{// create a front rotating facing polygon

		if ( p->type == P_SMOKE_IMPACT && Distance( cg.snap->ps.origin, org ) > 1024) {
			return;
		}

		if (p->color == BLOODRED)
			VectorSet (color, 0.22f, 0.0f, 0.0f);
		else if (p->color == GREY75)
		{
			float	len;
			float	greyit;
			float	val;
			len = Distance (cg.snap->ps.origin, org);
			if (!len)
				len = 1;

			val = 4096/len;
			greyit = 0.25 * val;
			if (greyit > 0.5)
				greyit = 0.5;

			VectorSet (color, greyit, greyit, greyit);
		}
		else
			VectorSet (color, 1.0, 1.0, 1.0);

		time = cg.time - p->time;
		time2 = p->endtime - p->time;
		ratio = time / time2;
		
		if (cg.time > p->startfade)
		{
			invratio = 1 - ( (cg.time - p->startfade) / (p->endtime - p->startfade) );

			if (p->color == EMISIVEFADE)
			{
				float fval;
				fval = (invratio * invratio);
				if (fval < 0)
					fval = 0;
				VectorSet (color, fval , fval , fval );
			}
			invratio *= p->alpha;
		}
		else 
			invratio = 1 * p->alpha;

		if ( cgs.glconfig.hardwareType == GLHW_RAGEPRO )
			invratio = 1;

		if (invratio > 1)
			invratio = 1;
	
		width = p->width + ( ratio * ( p->endwidth - p->width) );
		height = p->height + ( ratio * ( p->endheight - p->height) );

		if (p->type != P_SMOKE_IMPACT)
		{
			vec3_t temp;

			vectoangles (rforward, temp);
			p->accumroll += p->roll;
			temp[ROLL] += p->accumroll * 0.1;
			AngleVectors ( temp, NULL, rright2, rup2);
		}
		else
		{
			VectorCopy (rright, rright2);
			VectorCopy (rup, rup2);
		}
		
		if (p->rotate)
		{
			VectorMA (org, -height, rup2, point);	
			VectorMA (point, -width, rright2, point);	
		}
		else
		{
			VectorMA (org, -p->height, pvup, point);	
			VectorMA (point, -p->width, pvright, point);	
		}
		VectorCopy (point, verts[0].xyz);	
		verts[0].st[0] = 0;	
		verts[0].st[1] = 0;	
		verts[0].modulate[0] = 255 * color[0];	
		verts[0].modulate[1] = 255 * color[1];	
		verts[0].modulate[2] = 255 * color[2];	
		verts[0].modulate[3] = 255 * invratio;	

		if (p->rotate)
		{
			VectorMA (org, -height, rup2, point);	
			VectorMA (point, width, rright2, point);	
		}
		else
		{
			VectorMA (org, -p->height, pvup, point);	
			VectorMA (point, p->width, pvright, point);	
		}
		VectorCopy (point, verts[1].xyz);	
		verts[1].st[0] = 0;	
		verts[1].st[1] = 1;	
		verts[1].modulate[0] = 255 * color[0];	
		verts[1].modulate[1] = 255 * color[1];	
		verts[1].modulate[2] = 255 * color[2];	
		verts[1].modulate[3] = 255 * invratio;	

		if (p->rotate)
		{
			VectorMA (org, height, rup2, point);	
			VectorMA (point, width, rright2, point);	
		}
		else
		{
			VectorMA (org, p->height, pvup, point);	
			VectorMA (point, p->width, pvright, point);	
		}
		VectorCopy (point, verts[2].xyz);	
		verts[2].st[0] = 1;	
		verts[2].st[1] = 1;	
		verts[2].modulate[0] = 255 * color[0];	
		verts[2].modulate[1] = 255 * color[1];	
		verts[2].modulate[2] = 255 * color[2];	
		verts[2].modulate[3] = 255 * invratio;	

		if (p->rotate)
		{
			VectorMA (org, height, rup2, point);	
			VectorMA (point, -width, rright2, point);	
		}
		else
		{
			VectorMA (org, p->height, pvup, point);	
			VectorMA (point, -p->width, pvright, point);	
		}
		VectorCopy (point, verts[3].xyz);	
		verts[3].st[0] = 1;	
		verts[3].st[1] = 0;	
		verts[3].modulate[0] = 255 * color[0];	
		verts[3].modulate[1] = 255 * color[1];	
		verts[3].modulate[2] = 255 * color[2];	
		verts[3].modulate[3] = 255  * invratio;	
		
	}
	else if (p->type == P_LFX)
	{// create a front rotating facing polygon
		// that can change colors
		// and........ something.
		vec3_t	rr, ru;
		vec3_t	rotate_ang;

		if ( p->type == P_SMOKE_IMPACT && Distance( cg.snap->ps.origin, org ) > 1024) {
			return;
		}

		VectorSet (color, p->cols[0][0], p->cols[1][0], p->cols[2][0]);
		time = cg.time - p->time;
		time2 = p->endtime - p->time;
		ratio = time / time2;
		
		if (cg.time > p->startfade)
		{
			float inv1,inv2,inv3,inv4;
			invratio = 1 - ( (cg.time - p->startfade) / (p->endtime - p->startfade) );

			inv1 = invratio * 4.0;
			inv2 = invratio * 4.0 - 1;
			inv3 = invratio * 4.0 - 2; 
			inv4 = invratio * 4.0 - 3;

			if (inv1 > 1.0f) inv1 = 1.0f;
			if (inv2 > 1.0f) inv2 = 1.0f;
			if (inv3 > 1.0f) inv3 = 1.0f;
			if (inv4 > 1.0f) inv4 = 1.0f;

		//	inv1 *= 4;
		

			//if (p->color == EMISIVEFADE)
			{
				float fval, eval;
				int et;
				vec4_t	fcol;
					eval = fval;
				fval = (invratio * invratio* invratio * invratio);
					for(et=0;et<4;et++)
					{
				

						if (invratio < 0.25f)
						fcol[et] = (p->cols[et][3] * inv1) + (p->cols[et][4] * (1 - inv1));
						else if (invratio < 0.50f)
						fcol[et] = (p->cols[et][2] * inv2) + (p->cols[et][3] * (1 - inv2));
						else if (invratio < 0.75f)
						fcol[et] = (p->cols[et][1] * inv3) + (p->cols[et][2] * (1 - inv3));
						else 
						fcol[et] = (p->cols[et][0] * inv4) + (p->cols[et][1] * (1 - inv4));


					}
				
				if (fval < 0)
					fval = 0;

				color[0] = fcol[0];
				color[1] = fcol[1];
				color[2] = fcol[2];
				color[3] = fcol[3];

				for(et=0;et<4;et++){
						if (fcol[et]>1)fcol[et]=1.0f;
						if (fcol[et]<0)fcol[et]=0.0f;
					}
			}
		
		}

		if (invratio > 4)
			invratio = 4;
	
		if (p->color == BLOODRED)
		{	// ORIENTED sprite - used for shockwaves, water waves, waves, etc.
				vec3_t	argles;
				vec3_t		forward, right, up;
				vectoangles( p->dir, argles );
				AngleVectors ( argles, NULL, right, up);

				width = p->width + ( ratio * ( p->endwidth - p->width) );
				height = p->height + ( ratio * ( p->endheight - p->height) );
		
				if (p->roll) {
					vectoangles( p->dir, rotate_ang );
					rotate_ang[ROLL] += p->roll;
					AngleVectors ( rotate_ang, NULL, right, up);
				}
		
		
				VectorMA (org, -height, right, point);	
				VectorMA (point, -width, up, point);	

				VectorCopy (point, verts[0].xyz);	
				verts[0].st[0] = 0;	
				verts[0].st[1] = 0;	
				verts[0].modulate[0] = 255 * color[0];	
				verts[0].modulate[1] = 255 * color[1];	
				verts[0].modulate[2] = 255 * color[2];	
				verts[0].modulate[3] = 255 * invratio;
		

				VectorMA (point, 2*height, up, point);	
				VectorCopy (point, verts[1].xyz);	
				verts[1].st[0] = 0;	
				verts[1].st[1] = 1;	
				verts[1].modulate[0] = 255 * color[0];	
				verts[1].modulate[1] = 255 * color[1];	
				verts[1].modulate[2] = 255 * color[2];	
				verts[1].modulate[3] = 255 * invratio;
		
	
				VectorMA (point, 2*width, right, point);	
				VectorCopy (point, verts[2].xyz);	
				verts[2].st[0] = 1;	
				verts[2].st[1] = 1;	
				verts[2].modulate[0] = 255 * color[0];	
				verts[2].modulate[1] = 255 * color[1];	
				verts[2].modulate[2] = 255 * color[2];	
				verts[2].modulate[3] = 255 * invratio;
		
	
				VectorMA (point, -2*height, up, point);	
				VectorCopy (point, verts[3].xyz);	
				verts[3].st[0] = 1;	
				verts[3].st[1] = 0;	
				verts[3].modulate[0] = 255 * color[0];	
				verts[3].modulate[1] = 255 * color[1];	
				verts[3].modulate[2] = 255 * color[2];	
				verts[3].modulate[3] = 255 * invratio;

		}
		else if (p->color == GREY75)
		{	// STRETCHY SPARK sprite - used for sparks etc
				vec3_t	argles;
				vec3_t		forward, right, up;
				vec3_t		fwd, rite;
				vec3_t		line;
				float		len, begin, end;
				vec3_t		start, finish;

				vec3_t oldorgstretch;
				vectoangles( p->dir, argles );
				AngleVectors ( argles, NULL, right, up);

			// Set up the 'beam'

			
				VectorCopy(p->stretchorg, oldorgstretch);

				VectorSubtract( org, oldorgstretch, fwd );
				len = VectorNormalize( fwd );
				//len = DotProduct(p->vel);// * (5 / (cg.time - cg.oldTime)) ;
				len *= p->height + p->width;

				len *= 7;	// doesn't look good in low framerates :/
			//	len = p->len;
				begin = -len / 2;
				end = len / 2;
		


			// Set up the particle

				width = p->width + ( ratio * ( p->endwidth - p->width) );
				height = p->height + ( ratio * ( p->endheight - p->height) );
		

				VectorMA( org, begin, fwd, start );
				VectorMA( org, end, fwd, finish );

				line[0] = DotProduct( fwd, cg.refdef.viewaxis[1] );
				line[1] = DotProduct( fwd, cg.refdef.viewaxis[2] );

				VectorScale( cg.refdef.viewaxis[1], line[1], right );
				VectorMA( right, -line[0], cg.refdef.viewaxis[2], right );
				VectorNormalize( right );

				VectorMA( finish, width, right, verts[0].xyz );	
				verts[0].st[0] = 0;	
				verts[0].st[1] = 0;	
				verts[0].modulate[0] = 255 * color[0];	
				verts[0].modulate[1] = 255 * color[1];	
				verts[0].modulate[2] = 255 * color[2];	
				verts[0].modulate[3] = 255 * invratio;
		
				VectorMA( finish, -width, right, verts[1].xyz );	
				verts[1].st[0] = 0;	
				verts[1].st[1] = 1;	
				verts[1].modulate[0] = 255 * color[0];	
				verts[1].modulate[1] = 255 * color[1];	
				verts[1].modulate[2] = 255 * color[2];	
				verts[1].modulate[3] = 255 * invratio;
		
	
				VectorMA( start, -width, right, verts[2].xyz );	
				verts[2].st[0] = 1;	
				verts[2].st[1] = 1;	
				verts[2].modulate[0] = 255 * color[0];	
				verts[2].modulate[1] = 255 * color[1];	
				verts[2].modulate[2] = 255 * color[2];	
				verts[2].modulate[3] = 255 * invratio;
		
	
				VectorMA( start, width, right, verts[3].xyz );	
				verts[3].st[0] = 1;	
				verts[3].st[1] = 0;	
				verts[3].modulate[0] = 255 * color[0];	
				verts[3].modulate[1] = 255 * color[1];	
				verts[3].modulate[2] = 255 * color[2];	
				verts[3].modulate[3] = 255 * invratio;
			
				// center glow of softness?
				{
					float wham = begin - end;
						trap_R_AddPolyToScene( p->pshader, 4, verts );

									width = p->width + ( ratio * ( p->endwidth - p->width) );
				height = p->height + ( ratio * ( p->endheight - p->height) );
		
				height *= wham;
				width *= wham;
				VectorMA (org, -height, pvup, point);	
				VectorMA (point, -width, pvright, point);	
	
		
				VectorCopy (point, verts[0].xyz);	
				verts[0].st[0] = 0;	
				verts[0].st[1] = 0;	
				verts[0].modulate[0] = 32 * color[0];	
				verts[0].modulate[1] = 32 * color[1];	
				verts[0].modulate[2] = 32 * color[2];	
				verts[0].modulate[3] = 32 * invratio;
		
				VectorMA (point, 2*height, pvup, point);	
				VectorCopy (point, verts[1].xyz);	
				verts[1].st[0] = 0;	
				verts[1].st[1] = 1;	
				verts[1].modulate[0] = 32 * color[0];	
				verts[1].modulate[1] = 32 * color[1];	
				verts[1].modulate[2] = 32 * color[2];	
				verts[1].modulate[3] = 32 * invratio;
		
				VectorMA (point, 2*width, pvright, point);	
				VectorCopy (point, verts[2].xyz);	
				verts[2].st[0] = 1;	
				verts[2].st[1] = 1;	
				verts[2].modulate[0] = 32 * color[0];	
				verts[2].modulate[1] = 32 * color[1];	
				verts[2].modulate[2] = 32 * color[2];	
				verts[2].modulate[3] = 32 * invratio;
		
			
				VectorMA (point, -2*height, pvup, point);	
				VectorCopy (point, verts[3].xyz);	
				verts[3].st[0] = 1;	
				verts[3].st[1] = 0;	
				verts[3].modulate[0] = 32 * color[0];	
				verts[3].modulate[1] = 32 * color[1];	
				verts[3].modulate[2] = 32 * color[2];	
				verts[3].modulate[3] = 32 * invratio;
					
						
				}

		}
		else
			// VP PARALLEL sprite 
		{
				width = p->width + ( ratio * ( p->endwidth - p->width) );
				height = p->height + ( ratio * ( p->endheight - p->height) );
		
				if (p->roll) {
					vectoangles( cg.refdef.viewaxis[0], rotate_ang );
					rotate_ang[ROLL] += p->roll;
					AngleVectors ( rotate_ang, NULL, rr, ru);
				}
		
				if (p->roll) {
					VectorMA (org, -height, ru, point);	
					VectorMA (point, -width, rr, point);	
				} else {
					VectorMA (org, -height, pvup, point);	
					VectorMA (point, -width, pvright, point);	
				}
		
		
				VectorCopy (point, verts[0].xyz);	
				verts[0].st[0] = 0;	
				verts[0].st[1] = 0;	
				verts[0].modulate[0] = 255 * color[0];	
				verts[0].modulate[1] = 255 * color[1];	
				verts[0].modulate[2] = 255 * color[2];	
				verts[0].modulate[3] = 255 * invratio;
		
				if (p->roll) {
					VectorMA (point, 2*height, ru, point);	
				} else {
					VectorMA (point, 2*height, pvup, point);	
				}
				VectorCopy (point, verts[1].xyz);	
				verts[1].st[0] = 0;	
				verts[1].st[1] = 1;	
				verts[1].modulate[0] = 255 * color[0];	
				verts[1].modulate[1] = 255 * color[1];	
				verts[1].modulate[2] = 255 * color[2];	
				verts[1].modulate[3] = 255 * invratio;
		
				if (p->roll) {
					VectorMA (point, 2*width, rr, point);	
				} else {
					VectorMA (point, 2*width, pvright, point);	
				}
				VectorCopy (point, verts[2].xyz);	
				verts[2].st[0] = 1;	
				verts[2].st[1] = 1;	
				verts[2].modulate[0] = 255 * color[0];	
				verts[2].modulate[1] = 255 * color[1];	
				verts[2].modulate[2] = 255 * color[2];	
				verts[2].modulate[3] = 255 * invratio;
		
				if (p->roll) {
					VectorMA (point, -2*height, ru, point);	
				} else {
					VectorMA (point, -2*height, pvup, point);	
				}
				VectorCopy (point, verts[3].xyz);	
				verts[3].st[0] = 1;	
				verts[3].st[1] = 0;	
				verts[3].modulate[0] = 255 * color[0];	
				verts[3].modulate[1] = 255 * color[1];	
				verts[3].modulate[2] = 255 * color[2];	
				verts[3].modulate[3] = 255 * invratio;
				}
		
	}

	else if (p->type == P_QUAKE)		// leilei - quake style particles
	{
		float scale;
		float resscale;
		vec3_t r_origin, vpn;
		resscale = 480 /  cgs.glconfig.vidHeight + 0.4;
		VectorCopy(cg.snap->ps.origin, r_origin);
		
		time = cg.time - p->time;
		time2 = p->endtime - p->time;
		ratio = time / time2;
		
		scale =
		    (p->org[0] - r_origin[0]) + (p->org[1] -  r_origin[1])   + (p->org[2] - r_origin[2]);
		   // (p->org[0] - r_origin[0]) * vpn[0] + (p->org[1] -  r_origin[1]) * vpn[1]    + (p->org[2] - r_origin[2]) * vpn[2];
		if (scale < 20)
		    scale = 1;
		else
		    scale = 1 + scale * 0.004;

			scale *= resscale;

		if (scale > 2) scale = 2;


		p->width = scale;
		p->height = scale;

		p->endwidth = scale;
		p->endheight = scale;

		// rockets

		if (p->qarticle == 2){
				int i;
    				float dvel = 4 * cg.time;
			//    p->ramp += ((time2 - time));
			    if (p->ramp >= 8)
				p->endtime = cg.time;
			    else
				p->qolor = ramp1[(int)p->ramp];
			//    for (i = 0; i < 3; i++)
			//	p->vel[i] += p->vel[i] * dvel;
			//    p->vel[2] -= grav;
			}

		// leilei - set color from translation table
		VectorSet(color, qpalette[p->qolor][0], qpalette[p->qolor][1], qpalette[p->qolor][2]);

		if (cg.time > p->startfade)
		{
			invratio = 1 - ( (cg.time - p->startfade) / (p->endtime - p->startfade) );

			if (p->color == EMISIVEFADE)
			{
				float fval;
				fval = (invratio * invratio);
				if (fval < 0)
					fval = 0;
				VectorSet (color, fval , fval , fval );
			}
			invratio *= p->alpha;
		}
		else 
			invratio = 1 * p->alpha;

		if ( cgs.glconfig.hardwareType == GLHW_RAGEPRO )
			invratio = 1;

		if (invratio > 1)
			invratio = 1;
	
		width = p->width + ( ratio * ( p->endwidth - p->width) );
		height = p->height + ( ratio * ( p->endheight - p->height) );


		{
			VectorCopy (rright, rright2);
			VectorCopy (rup, rup2);
		}
		
		{
			VectorMA (org, -p->height, pvup, point);	
			VectorMA (point, -p->width, pvright, point);	
		}
		VectorCopy (point, verts[0].xyz);	
		verts[0].st[0] = 0;	
		verts[0].st[1] = 0;	
		verts[0].modulate[0] = 255 * color[0];	
		verts[0].modulate[1] = 255 * color[1];	
		verts[0].modulate[2] = 255 * color[2];	
		verts[0].modulate[3] = 255 * invratio;	

		{
			VectorMA (org, -p->height, pvup, point);	
			VectorMA (point, p->width, pvright, point);	
		}
		VectorCopy (point, verts[1].xyz);	
		verts[1].st[0] = 0;	
		verts[1].st[1] = 1;	
		verts[1].modulate[0] = 255 * color[0];	
		verts[1].modulate[1] = 255 * color[1];	
		verts[1].modulate[2] = 255 * color[2];	
		verts[1].modulate[3] = 255 * invratio;	

		{
			VectorMA (org, p->height, pvup, point);	
			VectorMA (point, p->width, pvright, point);	
		}
		VectorCopy (point, verts[2].xyz);	
		verts[2].st[0] = 1;	
		verts[2].st[1] = 1;	
		verts[2].modulate[0] = 255 * color[0];	
		verts[2].modulate[1] = 255 * color[1];	
		verts[2].modulate[2] = 255 * color[2];	
		verts[2].modulate[3] = 255 * invratio;	
		{
			VectorMA (org, p->height, pvup, point);	
			VectorMA (point, -p->width, pvright, point);	
		}
		VectorCopy (point, verts[3].xyz);	
		verts[3].st[0] = 1;	
		verts[3].st[1] = 0;	
		verts[3].modulate[0] = 255 * color[0];	
		verts[3].modulate[1] = 255 * color[1];	
		verts[3].modulate[2] = 255 * color[2];	
		verts[3].modulate[3] = 255  * invratio;	
		
	}

	else if (p->type == P_BLEED)
	{
		// not used
	}
	else if (p->type == P_FLAT_SCALEUP)
	{
		// not used
	}
	else if (p->type == P_FLAT)
	{
		// not used
	}
	// Ridah
	else if (p->type == P_ANIM) {
		// not used
	}
	// done.
	
	if (!p->pshader) {
// (SA) temp commented out for DM
//		CG_Printf ("CG_AddParticleToScene type %d p->pshader == ZERO\n", p->type);
		return;
	}


	// not used
//	if (p->type == P_WEATHER || p->type == P_WEATHER_TURBULENT || p->type == P_WEATHER_FLURRY)
//		trap_R_AddPolyToScene( p->pshader, 3, TRIverts );
//	else
		trap_R_AddPolyToScene( p->pshader, 4, verts );



}

// Ridah, made this static so it doesn't interfere with other files
static float roll = 0.0;
#define MINe(p,q) ((p <= q) ? p : q)
/*
===============
CG_AddParticles
===============
*/

void CG_LightningHalo( vec3_t source, vec3_t dest );
void CG_AddParticles (void)
{
	cparticle_t		*p, *next;
	float			alpha;
	float			time, time2;
	vec3_t			org;
	vec3_t			oldorg;	// leilei
	vec3_t			diffed;	// leilei
	int				color;
	cparticle_t		*active, *tail;
	float *v3f, *t2f, *c4f;
	int				type;
	vec3_t			rotate_ang;
	vec3_t			colar;
	vec3_t			africt;	

	float			frametime;
	float			time3;
	float			time1;
	float			dvel;
	float			grav;

	if (!initparticles)
		CG_ClearParticles ();

	// leilei - quake time stuf
	frametime = cg.time - cg.oldTime;
	frametime *= 0.001;
	time3 = frametime * 15;
	time2 = frametime * 10; // 15;
	time1 = frametime * 5;
	grav = frametime * -800;
	dvel = 4*frametime;



	VectorCopy( cg.refdef.viewaxis[0], pvforward );
	VectorCopy( cg.refdef.viewaxis[1], pvright );
	VectorCopy( cg.refdef.viewaxis[2], pvup );

	vectoangles( cg.refdef.viewaxis[0], rotate_ang );
	roll += ((cg.time - oldtime) * 0.1) ;
	rotate_ang[ROLL] += (roll*0.9);
	AngleVectors ( rotate_ang, rforward, rright, rup);
	
	oldtime = cg.time;


	active = NULL;
	tail = NULL;

	for (p=active_particles ; p ; p=next)
	{

		next = p->next;

		//time = (cg.time - p->time)*0.001;

		VectorCopy(p->org, oldorg);

		//CG_AddParticleToScene (p, p->org, alpha);

		p->vel[0] += p->accel[0]*frametime;
		p->vel[1] += p->accel[1]*frametime;
		p->vel[2] += p->accel[2]*frametime;

		if (alpha > 1.0)
			alpha = 1;

		color = p->color;
		

		oldorg[0] = p->org[0];
		oldorg[1] = p->org[1];
		oldorg[2] = p->org[2];

		p->org[0] += p->vel[0]*frametime;
		p->org[1] += p->vel[1]*frametime;
		p->org[2] += p->vel[2]*frametime;

		//p->roll += 80.8;


		
		alpha = p->alpha + time*p->alphavel;
		if (alpha <= 0)
		{	// faded out
			p->next = free_particles;
			free_particles = p;
			p->type = 0;
			p->color = 0;
			p->alpha = 0;
			continue;
		}
		

		if (p->type == P_SMOKE || p->type == P_ANIM || p->type == P_LFX || p->type == P_BLEED || p->type == P_SMOKE_IMPACT)
		{
			if (cg.time > p->endtime)
			{
				p->next = free_particles;
				free_particles = p;
				p->type = 0;
				p->color = 0;
				p->alpha = 0;
				p->qolor = 0;
				p->accel[0] = 0;
				p->accel[1] = 0;
				p->accel[2] = 0;
				p->qarticle = 0;
				p->cols[0][0] = 0;	
				p->cols[1][0] = 0;	
				p->cols[2][0] = 0;	
				p->cols[3][0] = 0;	
				p->cols[0][1] = 0;	
				p->cols[1][1] = 0;	
				p->cols[2][1] = 0;	
				p->cols[3][1] = 0;	
				p->cols[0][2] = 0;	
				p->cols[1][2] = 0;	
				p->cols[2][2] = 0;	
				p->cols[3][2] = 0;	
				p->cols[0][3] = 0;	
				p->cols[1][3] = 0;	
				p->cols[2][3] = 0;	
				p->cols[3][3] = 0;	
				p->cols[0][4] = 0;	
				p->cols[1][4] = 0;	
				p->cols[2][4] = 0;	
				p->cols[3][4] = 0;	
				p->qarticle = 0;
				p->airfriction = 0;
				p->bounce = 0;
				p->len = 0;
				continue;
			}

		}

		// leilei
		if (p->type == P_QUAKE)
		{
			if (cg.time > p->endtime)
			{
				p->next = free_particles;
				free_particles = p;
				p->type = 0;
				p->color = 0;
				p->qolor = 0;
				p->qarticle = 0;
				p->alpha = 0;
			
				continue;
			}

		}

		p->next = NULL;
		if (!tail)
			active = tail = p;
		else
		{
			tail->next = p;
			tail = p;
		}


		p->vel[0] = p->vel[0] * (1 + (africt[0]));	
		p->vel[1] = p->vel[1] * (1 + (africt[1]));	
		p->vel[2] = p->vel[2] * (1 + (africt[2]));	

		//if (oldtime > p->timetrail){
		//	p->timetrail = cg.time + 60;
			{
				int r;
				for (r=0;r<3;r++)
					p->stretchorg[r] = oldorg[r];
	
			}
		//}

	// leilei - trail stuffs

		vectoangles( p->vel, p->angle );
		p->stretch = 34;
		p->src[0] = p->org[0];
		p->src[1] = p->org[1];
		p->src[2] = p->org[2];

		p->dest[0] = p->org[0] + p->vel[0] + p->accel[0];
		p->dest[1] = p->org[1] + p->vel[1] + p->accel[1];
		p->dest[2] = p->org[2] + p->vel[2] + p->accel[2];

		type = p->type;




	//
		
				//VectorSubtract( p->org, p->oldorgstretch, fwd );
				//len = VectorNormalize( fwd );
				//p->len = DotProduct(p->vel) ;
				//p->len *= p->height + p->width;

				//p->len *= -2;


	// leilei - bounce physics
	if(p->bounce > 0)
		{
			trace_t	trace;
			float dist;
			vec3_t invtrace;	// used to move the particle from the plane we hit
	
			// Do the trace of truth
			CG_Trace (&trace, oldorg, NULL, NULL, p->org, -1, CONTENTS_SOLID);
			if (!(trace.entityNum < (MAX_ENTITIES - 1))) // may only land on world
			{
				if (trace.fraction < 1){
				
					//CG_Printf (" da da da dada %f %f %f afro circus\n", p->vel[0], p->vel[1], p->vel[2]);
						//CG_Printf (" da da da dada %f %f %f afro circus\n", trace.plane.normal[0], trace.plane.normal[1], trace.plane.normal[2]);
						//dist = DotProduct(p->vel, trace.plane.normal) * p->bounce;
						//VectorMA(p->vel, dist, trace.plane.normal, p->vel);
						VectorCopy(trace.endpos, p->org);	// particle where we've hit from
						//VectorClear(p->vel);
					
						if (p->bounce == 666){ // blood
						 CG_ImpactMark( cgs.media.bloodMarkShader, p->org, trace.plane.normal, random()*360,1,1,1, p->alpha, qtrue, 15, qfalse );
							p->endtime = p->time; // time to die!
						}

						// anything else - bounce off solid
						dist = DotProduct(p->vel, trace.plane.normal) * -p->bounce;
						VectorMA(p->vel, dist, trace.plane.normal, p->vel);
						// lets roll
						//p->vel[2] = 7;
					}
			}	
	}
		CG_AddParticleToScene (p, p->org, alpha);

	}

	active_particles = active;
}

/*
======================
CG_AddParticles
======================
*/
void CG_ParticleSnowFlurry (qhandle_t pshader, centity_t *cent)
{
// leilei - proto-rtcw code not used here
}

void CG_ParticleSnow (qhandle_t pshader, vec3_t origin, vec3_t origin2, int turb, float range, int snum)
{
// leilei - proto-rtcw code not used here
}

void CG_ParticleBubble (qhandle_t pshader, vec3_t origin, vec3_t origin2, int turb, float range, int snum)
{
// leilei - proto-rtcw code not used here
}

void CG_ParticleSmoke (qhandle_t pshader, centity_t *cent)
{
// leilei - proto-rtcw code not used here
}


void CG_ParticleBulletDebris (vec3_t org, vec3_t vel, int duration)
{
// leilei - proto-rtcw code not used here
	
}

/*
======================
CG_ParticleExplosion
======================
*/

void CG_ParticleExplosion (char *animStr, vec3_t origin, vec3_t vel, int duration, int sizeStart, int sizeEnd)
{
// leilei - proto-rtcw code not used here
}


// Rafael Shrapnel
void CG_AddParticleShrapnel (localEntity_t *le)
{
	return;
}
// done.

int CG_NewParticleArea (int num)
{
	// const char *str;
	char *str;
	char *token;
	int type;
	vec3_t origin, origin2;
	int		i;
	float range = 0;
	int turb;
	int	numparticles;
	int	snum;
	
	str = (char *) CG_ConfigString (num);
	if (!str[0])
		return (0);
	
	// returns type 128 64 or 32
	token = COM_Parse (&str);
	type = atoi (token);
	
	if (type == 1)
		range = 128;
	else if (type == 2)
		range = 64;
	else if (type == 3)
		range = 32;
	else if (type == 0)
		range = 256;
	else if (type == 4)
		range = 8;
	else if (type == 5)
		range = 16;
	else if (type == 6)
		range = 32;
	else if (type == 7)
		range = 64;


	for (i=0; i<3; i++)
	{
		token = COM_Parse (&str);
		origin[i] = atof (token);
	}

	for (i=0; i<3; i++)
	{
		token = COM_Parse (&str);
		origin2[i] = atof (token);
	}
		
	token = COM_Parse (&str);
	numparticles = atoi (token);
	
	token = COM_Parse (&str);
	turb = atoi (token);

	token = COM_Parse (&str);
	snum = atoi (token);
	
	for (i=0; i<numparticles; i++)
	{
		if (type >= 4)
			CG_ParticleBubble (cgs.media.waterBubbleShader, origin, origin2, turb, range, snum);
		else
			CG_ParticleSnow (cgs.media.waterBubbleShader, origin, origin2, turb, range, snum);
	}

	return (1);
}

void	CG_SnowLink (centity_t *cent, qboolean particleOn)
{
// leilei - proto-rtcw code not used here
}

void CG_ParticleImpactSmokePuff (qhandle_t pshader, vec3_t origin)
{
// leilei - proto-rtcw code not used here
}

void CG_Particle_Bleed (qhandle_t pshader, vec3_t start, vec3_t dir, int fleshEntityNum, int duration)
{
// leilei - proto-rtcw code not used here
}

void CG_Particle_OilParticle (qhandle_t pshader, centity_t *cent)
{
// leilei - proto-rtcw code not used here
}


void CG_Particle_OilSlick (qhandle_t pshader, centity_t *cent)
{
// leilei - proto-rtcw code not used here
}

void CG_OilSlickRemove (centity_t *cent)
{
// leilei - proto-rtcw code not used here
}

qboolean ValidBloodPool (vec3_t start)
{
// leilei - proto-rtcw code not used here
}

void CG_BloodPool (localEntity_t *le, qhandle_t pshader, trace_t *tr)
{	
// leilei - proto-rtcw code not used here
}

#define NORMALSIZE	16
#define LARGESIZE	32

void CG_ParticleBloodCloud (centity_t *cent, vec3_t origin, vec3_t dir)
{
// leilei - proto-rtcw code not used here
	
}

void CG_ParticleSparks (vec3_t org, vec3_t vel, int duration, float x, float y, float speed)
{
// leilei - proto-rtcw code not used here
}

void CG_ParticleDust (centity_t *cent, vec3_t origin, vec3_t dir)
{
// leilei - proto-rtcw code not used here
	
}

void CG_ParticleMisc (qhandle_t pshader, vec3_t origin, int size, int duration, float alpha)
{
// leilei - proto-rtcw code not used here
}





// LEILEI ENHANCEMENT PARTICLE EFFECTS


/*
===============
CG_LeiTrailPoly

Stupid hack based on the bullet tracer to make a particle appear to streak.
===============
*/
void CG_LeiTrailPoly( vec3_t source, vec3_t dest, vec3_t colar, qhandle_t ashader, vec3_t val, float scaled) 
{
// leilei - no longer used
}



// sparks!
// for small arms

void CG_LeiSparks (vec3_t org, vec3_t vel, int duration, float x, float y, float speed)
{
// leilei - no longer used
}






void CG_LeiBlood2 (vec3_t org, vec3_t vel, int duration, float x, float y, float speed, float dam)
{
	cparticle_t	*p;

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = cg.time;
	
	p->endtime = cg.time + duration;
	p->startfade = cg.time + duration/2;
	
	p->color = EMISIVEFADE;
	p->alpha = 1.0f;
	p->alphavel = 0.8f;

	p->height = 8 * dam;
	p->width = 8 * dam;
	p->endheight = 4;
	p->endwidth = 4;

	p->pshader = cgs.media.lbldShader1;

	p->type = P_SMOKE;
	//p->type = P_SPARK;

	
	VectorCopy(org, p->org);

//	p->org[0] += (crandom() * x);
//	p->org[1] += (crandom() * y);

//	p->vel[0] = vel[0] * 75;
//	p->vel[1] = vel[1] * 75;
//	p->vel[2] = vel[2] * 75;


	p->accel[0] = p->accel[1] = p->accel[2] = 0;

//	p->vel[0] += (crandom() * speed);
//	p->vel[1] += (crandom() * speed);
//	p->vel[2] += speed + (crandom() * speed);	

	p->vel[0] += (crandom() * 824);
	p->vel[1] += (crandom() * 824);
	p->vel[2] += (crandom() * 824);

	p->vel[0] *= vel[0] * speed;
	p->vel[1] *= vel[1] * speed;
	p->vel[2] *= vel[2] * speed;


	//p->vel[2] += (20 + (crandom() `* 180));	


		p->accel[0] = 0;
		p->accel[1] = 0;
		p->accel[2] = -PARTICLE_GRAVITY * 3;


//		p->accel[0] = crandom()*6;
//		p->accel[1] = crandom()*6;
//		p->accel[2] = -PARTICLE_GRAVITY*7.2;

	//p->airfriction = 19;
	
}


// a different sort of puff

void CG_LeiPuff (vec3_t org, vec3_t vel, int duration, float x, float y, float speed, float size)
{
	cparticle_t	*p;

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = cg.time;
	
	p->endtime = cg.time + duration;
	p->startfade = cg.time + duration/2;
	
	p->color = EMISIVEFADE;
	p->alpha = 0.8f;
	p->alphavel = 0.8f;

	p->height = size;
	p->width = size;
	p->endheight = size * 1.8;
	p->endwidth = size * 1.8;

	p->pshader = cgs.media.lspkShader1;

	p->type = P_SMOKE;
	
	VectorCopy(org, p->org);

	p->org[0] += (crandom() * x);
	p->org[1] += (crandom() * y);

	p->vel[0] = vel[0] * speed;
	p->vel[1] = vel[1] * speed;
	p->vel[2] = vel[2] * speed;

	p->accel[0] = p->accel[1] = p->accel[2] = 0;

	p->vel[0] += (crandom() * 44);
	p->vel[1] += (crandom() * 44);
	p->vel[2] += (crandom() * 44);	
	p->roll = (crandom() * 256 - 128);	


	//	p->vel[0] += (crandom() * 24);
	//p->vel[1] += (crandom() * 24);
	//p->vel[2] += (20 + (crandom() * 180)) * speed;	

		p->accel[0] = -2;
		p->accel[1] = -2;
		p->accel[2] = -2;
	
}

// a violent blast puff

void CG_LeiBlast (vec3_t org, vec3_t vel, int duration, float x, float y, float speed, float size)
{
	cparticle_t	*p;

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = cg.time;
	
	p->endtime = cg.time + duration;
	p->startfade = cg.time + duration/2;
	
	p->color = EMISIVEFADE;
	p->alpha = 1.0f;
	p->alphavel = 0.72f;

	p->height = size;
	p->width = size;
	p->endheight = size * 6;
	p->endwidth = size * 6;

	p->pshader = cgs.media.lbumShader1;

	p->type = P_SMOKE;
	
	VectorCopy(org, p->org);

	p->org[0] += (crandom() * x);
	p->org[1] += (crandom() * y);

	p->vel[0] = vel[0] * speed;
	p->vel[1] = vel[1] * speed;
	p->vel[2] = vel[2] * speed;

	p->accel[0] = p->accel[1] = p->accel[2] = 0;

	p->vel[0] += (crandom() * 84);
	p->vel[1] += (crandom() * 84);
	p->vel[2] += (crandom() * 84);	
	p->roll = (crandom() * 256 - 128);	


	//	p->vel[0] += (crandom() * 24);
	//p->vel[1] += (crandom() * 24);
	//p->vel[2] += (20 + (crandom() * 180)) * speed;	

		p->accel[0] = -2;
		p->accel[1] = -2;
		p->accel[2] = -2;
	
}

// for explosions

void CG_LeiSparks2 (vec3_t org, vec3_t vel, int duration, float x, float y, float speed)
{
	cparticle_t	*p;

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = cg.time;
	
	p->endtime = cg.time + duration;
	p->startfade = cg.time + duration/2;
	
	p->color = EMISIVEFADE;
	p->alpha = 0.8f;
	p->alphavel = 0;

	p->height = 9;
	p->width = 9;
	p->endheight = 32;
	p->endwidth = 32;

	p->pshader = cgs.media.lspkShader1;

	p->type = P_SMOKE;
	
	VectorCopy(org, p->org);

	p->org[0] += (crandom() * x);
	p->org[1] += (crandom() * y);

	p->vel[0] = vel[0] * 15;
	p->vel[1] = vel[1] * 15;
	p->vel[2] = vel[2] * 15;

	p->accel[0] = p->accel[1] = p->accel[2] = 0;

	p->vel[0] += (crandom() * 524);
	p->vel[1] += (crandom() * 524);
	p->vel[2] += (120 + (crandom() * 780)) * speed;	

	//	p->accel[0] = crandom()*76;
	//	p->accel[1] = crandom()*76;
	//	p->accel[2] = crandom()*76;
	

//	VectorCopy( origin, p->org );
//	VectorCopy( vel, p->vel );
//	VectorClear( p->accel );


	
}

// not so friendly water splash
void CG_LeiSplash2 (vec3_t org, vec3_t vel, int duration, float x, float y, float speed)
{
	cparticle_t	*p;

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = cg.time;
	
	p->endtime = cg.time + duration;
	p->startfade = cg.time + duration/2;
	
	p->color = EMISIVEFADE;
	p->alpha = 0.9f;
	p->alphavel = 0;

	p->height = 4;
	p->width = 4;
	p->endheight = 2;
	p->endwidth = 2;

	p->pshader = cgs.media.lsplShader;

	p->type = P_SMOKE;
	
	VectorCopy(org, p->org);

	p->org[0] += (crandom() * x);
	p->org[1] += (crandom() * y);

	p->vel[0] = vel[0] * 44;
	p->vel[1] = vel[1] * 44;
	p->vel[2] = vel[2] * 872;

	p->accel[0] = p->accel[1] = p->accel[2] = 0;

	p->vel[0] += (crandom() * 4);
	p->vel[1] += (crandom() * 4);
	p->vel[2] += (20 + (crandom() * 10)) * speed;	

		p->accel[0] = crandom()*3;
		p->accel[1] = crandom()*3;
		p->accel[2] = -PARTICLE_GRAVITY*4.2;
	
}












//
// leilei's LFX particle system effects
//


void CG_LFX_Blood (vec3_t org, vec3_t dir, float pressure) 
{
	int i, j;
	int	o, m, f, g;
	int count = pressure * 4;
	vec3_t  shoo, ting, tha, tway;
	cparticle_t	*p;

    for (i = 0; i < count; i++) {
	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = cg.time;
	
	p->endtime = cg.time + (1200 * (crandom() * 22));
	//p->startfade = p->endtime;
	//p->color = GREY75;
	p->color = EMISIVEFADE;
	p->alpha = 1.0f;
	p->alphavel = 2.0f;
	p->height = 1.5;
	p->width = 1.5;
	p->endheight = 1.5;
	p->endwidth = 1.5;

	p->cols[0][0] = p->cols[0][1] = p->cols[0][2] = p->cols[0][3] = p->cols[0][4] = 1.0;
	p->cols[1][0] = p->cols[1][1] = p->cols[1][2] = p->cols[1][3] = p->cols[1][4] = 0.0;
	p->cols[2][0] = p->cols[2][1] = p->cols[2][2] = p->cols[2][3] = p->cols[2][4] = 0.0;

	// Manage blending Functions
	p->pshader = cgs.media.alfball;

	p->type = P_LFX;

	
	VectorCopy(org, p->org);
	p->bounce = 1.1f;
 for (j = 0; j < 3; j++) {
	//p->org[j] = org[j] + ((rand() & (count/8)) - (count/16));
	p->org[j] = org[j] + ((crandom() * (count/16)) - (count/32));
	p->vel[j] = dir[j] * (i * 2.6);

	p->vel[j] *= 0.01;
	}

	p->accel[0] = p->accel[1] = p->accel[2] = 0;
	p->accel[2] = -(PARTICLE_GRAVITY*8);

	p->airfriction = 0;
	//	p->org[j] = org[j] + (rand()&((count))-(count));
	//	p->vel[j] =  dir[j]*(rand()*(count*0.5))-(count) * 0.7; 
	//	p->vel[2] += (i / 5);

		//	}

	}


}

// NOT FROM QUAKE FOLLOWS THESE NEW "STANDARD" PARTICLE FUNCTIONS FOR VARIOUS WEAPONS.
// for consistent, particley appearance similar to 1999/2000 games
// note that it's an option, your old simple model/sprite effects are preserved already.

void CG_LFX_Smoke (vec3_t org, vec3_t dir, float spread, float speed, vec4_t color1, vec4_t color2, vec4_t color3, vec4_t color4, vec4_t color5, int count, int duration, float scaleup, int blendfunc)
{
	int i, j;
	int cont = 50;
	cparticle_t	*p;

	cont = (44 / (count / 2 + 1)) + 1;

    for (i = 0; i < cont; i++) {
	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = cg.time;
	
	p->endtime = cg.time + duration;
	p->startfade = cg.time;
	
	p->color = EMISIVEFADE;
	p->alpha = 0.1f;
	p->alphavel = 0.0f;
	p->height = p->width = 1.0 * (count / 3);

	p->endheight = p->height + scaleup;
	p->endwidth = p->width + scaleup;


	// Manage random roll and 
	p->rotate = qtrue;
	p->roll = crandom()*179;
	
	p->cols[0][0] = color1[0]; 
	p->cols[1][0] = color1[1];
	p->cols[2][0] = color1[2];
	p->cols[3][0] = color1[3];

	p->cols[0][1] = color2[0]; 
	p->cols[1][1] = color2[1];
	p->cols[2][1] = color2[2];
	p->cols[3][1] = color2[3];

	p->cols[0][2] = color3[0]; 
	p->cols[1][2] = color3[1];
	p->cols[2][2] = color3[2];
	p->cols[3][2] = color3[3];

	p->cols[0][3] = color4[0]; 
	p->cols[1][3] = color4[1];
	p->cols[2][3] = color4[2];
	p->cols[3][3] = color4[3];

	p->cols[0][4] = color5[0]; 
	p->cols[1][4] = color5[1];
	p->cols[2][4] = color5[2];
	p->cols[3][4] = color5[3];

	// Manage blending Functions
	if (blendfunc == 1)
	p->pshader = cgs.media.addsmoke;
	else if (blendfunc == 2)
	p->pshader = cgs.media.modsmoke;
	else if (blendfunc == 3)
	p->pshader = cgs.media.subsmoke;
	else
	p->pshader = cgs.media.alfsmoke;

	p->type = P_LFX;
	
	VectorCopy(org, p->org);

	// Manage spread of origin and velocity
	 for (j = 0; j < 3; j++) {
		//p->org[j] = org[j] + ((crandom() * (spread / 8)) - (spread/16));
		p->org[j] = org[j];

		p->vel[j] = (crandom() * dir[j]) * speed;
		p->vel[j] += ((crandom() * (spread)) - (spread/2));
		}

	p->airfriction = 0.2;
	}
}




void CG_LFX_Smoke2 (vec3_t org, vec3_t dir, float spread, float speed, vec4_t color1, vec4_t color2, vec4_t color3, vec4_t color4, vec4_t color5, int count, int duration, float scale, float scaleup, int blendfunc)
{
	int i, j;
	int cont = 50;
	cparticle_t	*p;

	cont = count;

    for (i = 0; i < cont; i++) {
	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = cg.time;
	
	p->endtime = cg.time + duration;
	p->startfade = cg.time;
	
	p->color = EMISIVEFADE;
	p->alpha = 0.1f;
	p->alphavel = 0.0f;
	p->height = p->width = (1.0 * scale);

	p->endheight = p->height + scaleup;
	p->endwidth = p->width + scaleup;


	// Manage random roll and 
	p->rotate = qtrue;
	p->roll = crandom()*179;
	
	p->cols[0][0] = color1[0]; 
	p->cols[1][0] = color1[1];
	p->cols[2][0] = color1[2];
	p->cols[3][0] = color1[3];

	p->cols[0][1] = color2[0]; 
	p->cols[1][1] = color2[1];
	p->cols[2][1] = color2[2];
	p->cols[3][1] = color2[3];

	p->cols[0][2] = color3[0]; 
	p->cols[1][2] = color3[1];
	p->cols[2][2] = color3[2];
	p->cols[3][2] = color3[3];

	p->cols[0][3] = color4[0]; 
	p->cols[1][3] = color4[1];
	p->cols[2][3] = color4[2];
	p->cols[3][3] = color4[3];

	p->cols[0][4] = color5[0]; 
	p->cols[1][4] = color5[1];
	p->cols[2][4] = color5[2];
	p->cols[3][4] = color5[3];


	// Manage blending Functions
	if (blendfunc == 1)
	p->pshader = cgs.media.addsmoke;
	else if (blendfunc == 2)
	p->pshader = cgs.media.modsmoke;
	else if (blendfunc == 3)
	p->pshader = cgs.media.subsmoke;
	else
	p->pshader = cgs.media.alfsmoke;

	p->type = P_LFX;
	
	VectorCopy(org, p->org);

	// Manage spread of origin and velocity
	 for (j = 0; j < 3; j++) {
		//p->org[j] = org[j] + ((crandom() * (spread / 8)) - (spread/16));
		p->org[j] = org[j];

		p->vel[j] = (crandom() * dir[j]) * speed;
		p->vel[j] += ((crandom() * (spread)) - (spread/2));
		}

	// Manage the Air Friction hack.

	p->airfriction = 0.2;
	}
}


void CG_LFX_Shock (vec3_t org, vec3_t dir, float spread, float speed, vec4_t color1, vec4_t color2, vec4_t color3, vec4_t color4, vec4_t color5, int count, int duration, float scaleup, int blendfunc)
{
	int i, j;
	int cont = 50;
	cparticle_t	*p;

	cont = (44 / (count / 2 + 1)) + 1;

    for (i = 0; i < count; i++) {
	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = cg.time;
	
	p->endtime = cg.time + duration;
	p->startfade = cg.time;
	
	p->color = BLOODRED;	// note - not actually bloodred.
	p->alpha = 0.1f;
	p->alphavel = 0.0f;
 	//p->qolor = (color & ~7) + (rand() & 7);
	p->height = p->width = 1.0 * (count / 3);

	p->endheight = p->height + scaleup;
	p->endwidth = p->width + scaleup;


	// Manage random roll and 
	p->rotate = qtrue;
	//p->rotate = qtrue;
	p->roll = crandom()*179;
	//p->roll = rand()%179;

	p->dir[0] = dir[0]; 
	p->dir[1] = dir[1]; 
	p->dir[2] = dir[2]; 

	p->cols[0][0] = color1[0]; 
	p->cols[1][0] = color1[1];
	p->cols[2][0] = color1[2];
	p->cols[3][0] = color1[3];

	p->cols[0][1] = color2[0]; 
	p->cols[1][1] = color2[1];
	p->cols[2][1] = color2[2];
	p->cols[3][1] = color2[3];

	p->cols[0][2] = color3[0]; 
	p->cols[1][2] = color3[1];
	p->cols[2][2] = color3[2];
	p->cols[3][2] = color3[3];

	p->cols[0][3] = color4[0]; 
	p->cols[1][3] = color4[1];
	p->cols[2][3] = color4[2];
	p->cols[3][3] = color4[3];

	p->cols[0][4] = color5[0]; 
	p->cols[1][4] = color5[1];
	p->cols[2][4] = color5[2];
	p->cols[3][4] = color5[3];


	// Manage blending Functions
	if (blendfunc == 1)
	p->pshader = cgs.media.addshock;
	else if (blendfunc == 2)
	p->pshader = cgs.media.modshock;
	else if (blendfunc == 3)
	p->pshader = cgs.media.subshock;
	else
	p->pshader = cgs.media.alfshock;

	p->type = P_LFX;
	
	VectorCopy(org, p->org);

	// Manage spread of origin and velocity
	 for (j = 0; j < 3; j++) {
		//p->org[j] = org[j] + ((crandom() * (spread / 8)) - (spread/16));
		p->org[j] = org[j];

		p->vel[j] = (crandom() * dir[j]) * speed;
		p->vel[j] += ((crandom() * (spread)) - (spread/2));
		}

	// Manage the Air Friction hack.


	// for (j = 0; j < 3; j++) {
	//	p->accel[j] = (p->vel[j] * -1);
	//	}


	
	
	//p->accel[0] = p->accel[1] = p->accel[2] = 0;
	//p->accel[2] = -(PARTICLE_GRAVITY / 2);

	p->airfriction = 0.2;
	}
}


void CG_LFX_Spark (vec3_t org, vec3_t dir, float spread, float speed, vec4_t color1, vec4_t color2, vec4_t color3, vec4_t color4, vec4_t color5, int count, int duration, float scaleup, int blendfunc)
{
	int i, j;
	int cont = 50;
	cparticle_t	*p;

	cont = count;

    for (i = 0; i < cont; i++) {
	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = cg.time;
	
	p->endtime = cg.time + duration;
	p->startfade = cg.time;
	
	p->color = GREY75;
	//p->color = EMISIVEFADE;
	p->alpha = 1.0f;
	p->alphavel = 0.0f;
 	//p->qolor = (color & ~7) + (rand() & 7);
	p->height = p->width = scaleup;

	p->endheight = p->height;
	p->endwidth = p->width;


	p->rotate = qfalse; // sparks don't rotate
	p->roll = 0;	// sparks don't roll
	

	p->accel[0] =	p->accel[1] =	p->accel[2] = 0;

	p->cols[0][0] = color1[0]; 
	p->cols[1][0] = color1[1];
	p->cols[2][0] = color1[2];
	p->cols[3][0] = color1[3];

	p->cols[0][1] = color2[0]; 
	p->cols[1][1] = color2[1];
	p->cols[2][1] = color2[2];
	p->cols[3][1] = color2[3];

	p->cols[0][2] = color3[0]; 
	p->cols[1][2] = color3[1];
	p->cols[2][2] = color3[2];
	p->cols[3][2] = color3[3];

	p->cols[0][3] = color4[0]; 
	p->cols[1][3] = color4[1];
	p->cols[2][3] = color4[2];
	p->cols[3][3] = color4[3];

	p->cols[0][4] = color5[0]; 
	p->cols[1][4] = color5[1];
	p->cols[2][4] = color5[2];
	p->cols[3][4] = color5[3];

	// Manage blending Functions
	if (blendfunc == 1)
	p->pshader = cgs.media.addball;
	else if (blendfunc == 2)
	p->pshader = cgs.media.modball;
	else if (blendfunc == 3)
	p->pshader = cgs.media.subball;
	else if (blendfunc == 666)
	p->pshader = cgs.media.alfball;
	else
	p->pshader = cgs.media.alfball;

	p->type = P_LFX;
	
	VectorCopy(org, p->org);


	// Manage spread of origin and velocity
	 for (j = 0; j < 3; j++) {
		float sped = speed * (1 + crandom()) + (speed * 0.3f);
		//p->org[j] = org[j] + ((crandom() * (spread / 8)) - (spread/16));
		p->org[j] = org[j];

		p->vel[j] = (dir[j]) * sped;
		p->vel[j] += ((crandom() * (spread)) - (spread/2));

	//	p->vel[j] *= crandom();


		}
		// a little kick up
		p->vel[2] += (speed * 0.4f);


	
	p->accel[0] = p->accel[1] = p->accel[2] = 0;
	p->accel[2] = -(800 * 0.5f);	// TODO: insert proper gravity.

	// prepare the initial stretch frame

	VectorCopy(p->org, p->stretchorg);
	//VectorAdd(p->org,p->vel,p->org);

	p->airfriction = 0.5f;
	p->bounce = 1.2f;
	if (blendfunc == 666)
	p->bounce = 666;	// instantly make a red decal and kill itself. for blood
	}
}










// macros kind of




















// QUAKE EFFECTS


// attempt at generic particles function similar to quake...
void CG_RunParticleEffect (vec3_t org, vec3_t dir, int color, int count)
{
	int i, j;
	cparticle_t	*p;

    for (i = 0; i < count; i++) {
	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = cg.time;
	
	p->endtime = cg.time + (100 * (crandom() * 5));
	p->startfade = p->endtime;
	
	p->color = EMISIVEFADE;
	p->alpha = 1.0f;
	p->alphavel = 1.0f;
 	p->qolor = (color & ~7) + (rand() & 7);
	p->height = 0.5;
	p->width = 0.5;
	p->endheight = 0.5;
	p->endwidth = 0.5;

	p->pshader = cgs.media.whiteShader;

	p->type = P_QUAKE;
	
	VectorCopy(org, p->org);

 for (j = 0; j < 3; j++) {
	//p->org[j] = org[j] + ((rand() & 15) - 8);
	p->org[j] = org[j] + ((crandom() * 8) - 4);
	p->vel[j] = dir[j] * 15;
	}

	p->accel[0] = p->accel[1] = p->accel[2] = 0;
	p->accel[2] = -(PARTICLE_GRAVITY / 2);

	p->airfriction = 0;
	}
}


// also from Quake!

void CG_QarticleExplosion(vec3_t org)
{
    int i, j;
    cparticle_t *p;

    for (i = 0; i < 512; i++) {
	if (!free_particles)
	    return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;

	p->color = EMISIVEFADE;
	p->alpha = 1.0f;
	p->alphavel = 1.0f;
	p->time = cg.time;


	// crap for q3's particle system...
	p->endtime = cg.time + 500;
	p->startfade = p->endtime;
	p->height = 0.5;
	p->width = 0.5;
	p->endheight = 0.5;
	p->endwidth = 0.5;
	p->pshader = cgs.media.whiteShader;


	p->qolor = ramp1[0];
	p->qarticle = 2;
//	p->qolor = 44;

	VectorCopy(org, p->org);

	p->ramp = rand() & 3;
	if (i & 1) {
	    p->type = P_QUAKE;
	    for (j = 0; j < 3; j++) {
		p->org[j] = org[j] + ((rand() % 32) - 16);
		p->vel[j] = (rand() % 512) - 256;
		p->accel[j] = p->vel[j] * 4;
	    }
	} else {
	    p->type = P_QUAKE;
	    for (j = 0; j < 3; j++) {
		p->org[j] = org[j] + ((rand() % 32) - 16);
		p->vel[j] = (rand() % 512) - 256;
		p->accel[j] = p->vel[j] * 4;
	    }
	}
    }
}





