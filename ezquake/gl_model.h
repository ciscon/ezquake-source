/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef __MODEL__
#define __MODEL__

#include "modelgen.h"
#include "spritegn.h"
#include "bspfile.h"

/*
d*_t structures are on-disk representations
m*_t structures are in-memory
*/

// entity effects

#define	EF_BRIGHTFIELD			1
#define	EF_MUZZLEFLASH 			2
#define	EF_BRIGHTLIGHT 			4
#define	EF_DIMLIGHT 			8
#define	EF_FLAG1	 			16
#define	EF_FLAG2	 			32
#define EF_BLUE					64
#define EF_RED					128


// in memory representation
// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct {
	vec3_t		position;
} mvertex_t;

#define	SIDE_FRONT	0
#define	SIDE_BACK	1
#define	SIDE_ON		2


// plane_t structure
// !!! if this is changed, it must be changed in asm_i386.h too !!!
typedef struct mplane_s {
	vec3_t	normal;
	float	dist;
	byte	type;			// for texture axis selection and fast side tests
	byte	signbits;		// signx + signy<<1 + signz<<1
	byte	pad[2];
} mplane_t;

typedef struct texture_s {
	char				name[16];
	unsigned			width, height;
	int					gl_texturenum;
	int					fb_texturenum;			//index of fullbright mask or 0
	struct msurface_s	*texturechain[2];		
	struct msurface_s	**texturechain_tail[2];	
												//		Points to the node right after the last non-NULL node in the texturechain.
	int					anim_total;				//total tenths in sequence ( 0 = no)
	int					anim_min, anim_max;		//time for this frame min <=time< max
	struct texture_s	*anim_next;				//in the animation sequence
	struct texture_s	*alternate_anims;		//bmodels in frame 1 use these
	unsigned			offsets[MIPLEVELS];		//four mip maps stored
	unsigned			colour;					
	int					isLumaTexture;
} texture_t;


#define	SURF_PLANEBACK		2
#define	SURF_DRAWSKY		4
#define SURF_DRAWSPRITE		8
#define SURF_DRAWTURB		0x10
#define SURF_DRAWTILED		0x20
#define SURF_DRAWBACKGROUND	0x40
#define SURF_UNDERWATER		0x80
#define SURF_DRAWALPHA		0x100

// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct {
	unsigned short	v[2];
	unsigned int	cachededgeoffset;
} medge_t;

typedef struct {
	float		vecs[2][4];
	texture_t	*texture;
	int			flags;
} mtexinfo_t;

#define	VERTEXSIZE	9	//xyz s1t1 s2t2 s3t3 where xyz = vert coords; s1t1 = normal tex coords; 
						//s2t2 = lightmap tex coords; s3t2 = detail tex coords

typedef struct glpoly_s {
	struct	glpoly_s	*next;
	struct	glpoly_s	*chain;				//next lightmap poly in chain
	struct	glpoly_s	*fb_chain;			//next fb poly in chain
	struct	glpoly_s	*luma_chain;			//next luma poly in chain
	struct	glpoly_s	*caustics_chain;	//next caustic poly in chain
	struct	glpoly_s	*detail_chain;		//next detail poly in chain
	int		numverts;
	float	verts[4][VERTEXSIZE];	// variable sized (xyz s1t1 s2t2)
} glpoly_t;

typedef struct msurface_s {
	int			visframe;		// should be drawn when node is crossed

	mplane_t	*plane;
	int			flags;

	int			firstedge;	// look up in model->surfedges[], negative numbers
	int			numedges;	// are backwards edges
	
	short		texturemins[2];
	short		extents[2];

	int			light_s, light_t;	// gl lightmap coordinates

	glpoly_t	*polys;				// multiple if warped
	struct	msurface_s	*texturechain;

	mtexinfo_t	*texinfo;
	
// lighting info
	int			dlightframe;
	int			dlightbits;

	int			lightmaptexturenum;
	byte		styles[MAXLIGHTMAPS];
	int			cached_light[MAXLIGHTMAPS];	// values currently used in lightmap
	qbool	cached_dlight;				// true if dynamic light in cache
	byte		*samples;		// [numstyles*surfsize]
} msurface_t;

typedef struct mnode_s {
// common with leaf
	int			contents;		// 0, to differentiate from leafs
	int			visframe;		// node needs to be traversed if current
	
	float		minmaxs[6];		// for bounding box culling

	struct mnode_s	*parent;

// node specific
	mplane_t	*plane;
	struct mnode_s	*children[2];	

	unsigned short		firstsurface;
	unsigned short		numsurfaces;
} mnode_t;

typedef struct mleaf_s {
// common with node
	int			contents;		// wil be a negative contents number
	int			visframe;		// node needs to be traversed if current

	float		minmaxs[6];		// for bounding box culling

	struct mnode_s	*parent;

// leaf specific
	byte		*compressed_vis;
	struct efrag_s	*efrags;

	msurface_t	**firstmarksurface;
	int			nummarksurfaces;
	byte		ambient_sound_level[NUM_AMBIENTS];
} mleaf_t;

// !!! if this is changed, it must be changed in asm_i386.h too !!!
typedef struct {
	dclipnode_t	*clipnodes;
	mplane_t	*planes;
	int			firstclipnode;
	int			lastclipnode;
	vec3_t		clip_mins;
	vec3_t		clip_maxs;
} hull_t;


// FIXME: shorten these?
typedef struct mspriteframe_s {
	int		width;
	int		height;
	float	up, down, left, right;
	int		gl_texturenum;
} mspriteframe_t;

typedef struct {
	int				numframes;
	float			*intervals;
	mspriteframe_t	*frames[1];
} mspritegroup_t;

typedef struct {
	spriteframetype_t	type;
	mspriteframe_t		*frameptr;
} mspriteframedesc_t;

typedef struct {
	int					type;
	int					maxwidth;
	int					maxheight;
	int					numframes;
	float				beamlength;		// remove?
	void				*cachespot;		// remove?
	mspriteframedesc_t	frames[1];
} msprite_t;


typedef struct {
	int					firstpose;
	int					numposes;
	float				interval;
	vec3_t				bboxmin;
	vec3_t				bboxmax;
	float				radius;
	int					frame;
	char				name[16];
} maliasframedesc_t;

typedef struct {
	trivertx_t			bboxmin;
	trivertx_t			bboxmax;
	int					frame;
} maliasgroupframedesc_t;

typedef struct {
	int						numframes;
	int						intervals;
	maliasgroupframedesc_t	frames[1];
} maliasgroup_t;

// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct mtriangle_s {
	int					facesfront;
	int					vertindex[3];
} mtriangle_t;


#define	MAX_SKINS	32
typedef struct {
	int			ident;
	int			version;
	vec3_t		scale;
	vec3_t		scale_origin;
	float		boundingradius;
	vec3_t		eyeposition;
	int			numskins;
	int			skinwidth;
	int			skinheight;
	int			numverts;
	int			numtris;
	int			numframes;
	synctype_t	synctype;
	int			flags;
	float		size;

	int					numposes;
	int					poseverts;
	int					posedata;	// numposes*poseverts trivert_t
	int					commands;	// gl command list with embedded s/t
	int					gl_texturenum[MAX_SKINS][4];
	int					fb_texturenum[MAX_SKINS][4];
	maliasframedesc_t	frames[1];	// variable sized
} aliashdr_t;

#define	MAXALIASVERTS	2048
#define	MAXALIASFRAMES	256
#define	MAXALIASTRIS	2048
extern	aliashdr_t	*pheader;
extern	stvert_t	stverts[MAXALIASVERTS];
extern	mtriangle_t	triangles[MAXALIASTRIS];
extern	trivertx_t	*poseverts[MAXALIASFRAMES];


typedef enum {mod_brush, mod_sprite, mod_alias, mod_alias3} modtype_t;

// some models are special
//VULT MODELS
//typedef enum {MOD_NORMAL, MOD_PLAYER, MOD_EYES, MOD_FLAME, MOD_THUNDERBOLT, MOD_BACKPACK} modhint_t;
typedef enum {MOD_NORMAL, MOD_PLAYER, MOD_EYES, MOD_FLAME, MOD_THUNDERBOLT, MOD_CLUSTER,
	MOD_FLAG, MOD_SPIKE, MOD_TF_TRAIL, MOD_GRENADE, MOD_TESLA, MOD_SENTRYGUN, MOD_DETPACK,
	MOD_LASER, MOD_DEMON, MOD_SOLDIER, MOD_OGRE, MOD_ENFORCER, MOD_VOORSPIKE, MOD_LAVABALL,
	MOD_RAIL, MOD_RAIL2, MOD_BUILDINGGIBS, MOD_BACKPACK, MOD_SHAMBLER, MOD_TELEPORTDESTINATION} modhint_t;

#define	EF_ROCKET	1			// leave a trail
#define	EF_GRENADE	2			// leave a trail
#define	EF_GIB		4			// leave a trail
#define	EF_ROTATE	8			// rotate (bonus items)
#define	EF_TRACER	16			// green split trail
#define	EF_ZOMGIB	32			// small blood trail
#define	EF_TRACER2	64			// orange split trail + rotate
#define	EF_TRACER3	128			// purple trail

typedef struct model_s {
	char		name[MAX_QPATH];
	qbool	needload;		// bmodels and sprites don't cache normally

	modhint_t	modhint;

	modtype_t	type;
	int			numframes;
	synctype_t	synctype;
	
	int			flags;

// volume occupied by the model graphics
	vec3_t		mins, maxs;
	float		radius;

// brush model
	int			firstmodelsurface, nummodelsurfaces;

	int			numsubmodels;
	dmodel_t	*submodels;

	int			numplanes;
	mplane_t	*planes;

	int			numleafs;		// number of visible leafs, not counting 0
	mleaf_t		*leafs;

	int			numvertexes;
	mvertex_t	*vertexes;

	int			numedges;
	medge_t		*edges;

	int			numnodes;
	mnode_t		*nodes;

	int			numtexinfo;
	mtexinfo_t	*texinfo;

	int			numsurfaces;
	msurface_t	*surfaces;

	int			numsurfedges;
	int			*surfedges;

	int			numclipnodes;
	dclipnode_t	*clipnodes;

	int			nummarksurfaces;
	msurface_t	**marksurfaces;

	hull_t		hulls[MAX_MAP_HULLS];

	int			numtextures;
	texture_t	**textures;

	byte		*visdata;
	byte		*lightdata;
	char		*entities;

	unsigned	checksum;
	unsigned	checksum2;

	int			bspversion;
	qbool	isworldmodel;

	// additional model data
	cache_user_t	cache;		// only access through Mod_Extradata

} model_t;


void	Mod_Init (void);
void	Mod_ClearAll (void);
model_t *Mod_ForName (char *name, qbool crash);
void	*Mod_Extradata (model_t *mod);	// handles caching
void	Mod_TouchModel (char *name);

mleaf_t *Mod_PointInLeaf (float *p, model_t *model);
byte	*Mod_LeafPVS (mleaf_t *leaf, model_t *model);

qbool Img_HasFullbrights (byte *pixels, int size);

#include "gl_md3.h" 

#endif	// __MODEL__
