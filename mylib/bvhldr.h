/*
 * Biovision Hierarchical Data Loader
 * Skeleton animation loader
 *
 * Joint type: first character of name
 *
 * Motion loading: linear traversal
 *
 * Motion matrix computing: tree traversal
 * 
 * Model must fit skeleton before loading.
 * rewind() sets matrices to first frame, NOT rest pose.
 * For rest pose use rest(). Don't forget to call next()
 * after rewind() and rest().
 *
 * No check (will crash) for calling rewind() or rest()
 * before any next(); No check for multiple calling rest().
 * Calling rewind() after rest() is pointless.
 *
 */

#pragma once

#include "slabvec.h"
#include "matrix.h"

#define BVH_JOINT_NAME_MAX 16

enum BVH_State {
	BS_UNKNOWN,
	BS_HIERARCHY,
	BS_MOTION
};

enum BVH_Line {
	BL_UNKNOWN,
	BL_HIERARCHY,
	BL_MOTION,
	BL_ROOT,
	BL_OFFSET,
	BL_CHANNELS,
	BL_JOINT,
	BL_END_SITE,
	BL_BEGIN,
	BL_END
};

typedef struct bvh_joint_struct
{
	struct bvh_joint_struct *next;
	struct bvh_joint_struct *child;
	char name[BVH_JOINT_NAME_MAX];
	int channels;
	signed char channel_mapv[6]; /* Indeces of channels's order, XtYtZt XrYrZr */
	vec3f_t offset;
} bvh_joint_t;

typedef struct bvh_motion_struct
{
	int frames;
	float frame_time;
	int channels;
	svec_t framev;		/* Pre-calc matrices for each frame */
	svec_t inv_framev;	/* Pre-calc rewind matrices for each frame */
	svec_t rest_framev;	/* Pre-calc rest matrices for each frame */
} bvh_motion_t;

typedef struct bvh_struct
{
	bvh_joint_t *root;	/* Bones (Joints) Tree */
	svec_t jointv;
	bvh_motion_t motion;
} bvh_t;

void bvh_init(bvh_t *h);
void bvh_free(bvh_t *h);
void bvh_load(const char *filename, bvh_t *h);
void bvh_dump(bvh_t *h);

typedef struct bvh_iter_struct
{
	bvh_joint_t *joint;
	bvh_joint_t *parent;
	svec_t stack;
	int course;
} bvh_iter_t;

void bvh_iter_init(bvh_iter_t *i, const bvh_t *h);
void bvh_iter_free(bvh_iter_t *i);
bvh_joint_t *bvh_iter_next(bvh_iter_t *i);
bvh_joint_t *bvh_iter_step(bvh_iter_t *i);

enum BVH_Player_State {
	BPS_NONE,
	BPS_REWIND = 1,
	BPS_REST = 2
};

typedef struct bvh_player_struct
{
	bvh_t *bvh;
	svec_iter_t framev_iter;
	svec_iter_t inv_framev_iter;
	svec_iter_t rest_framev_iter;
	mat4f_t *matrixv;
	mat4f_t *inv_matrixv;
	mat4f_t *rest_matrixv;
	enum BVH_Player_State state;
} bvh_player_t;

void bvh_player_init(bvh_player_t *bp, bvh_t *h);
void bvh_player_next(bvh_player_t *bp);
void bvh_player_rewind(bvh_player_t *bp);
void bvh_player_rest(bvh_player_t *bp);

