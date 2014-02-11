/*
 * Object loader
 *
 * .obj file must contain UNIX linefeeds \n
 *
 * Non-existing material use attempts are ignored
 *
 * `mtllib` directive can take only 1 filename
 *
 * Groups must be defined once. Only one group can be
 * specified in `g ` directive.
 *
 */

#pragma once

#include "slabvec.h"

/* 4 x 4 int. For fast comparsion */
#define MODEL_GROUP_MAX_NAME 16

/* Don't use very slow sscanf() */
#define MODEL_FAST_PARSE 1

#define MODEL_INIT_VERTS 4000
#define MODEL_INIT_VTEXS 2500
#define MODEL_INIT_VNORS 4500
#define MODEL_INIT_FACES 5000
#define MODEL_INIT_MTLS  10
#define MODEL_INIT_MTLUS 10
#define MODEL_INIT_GROUPS 20

enum Obj_Line {
	OL_UNKNOWN = 0,
	OL_COMMENT,
	OL_OBJECT,
	OL_VERTEX,
	OL_TEXTURE,
	OL_NORMAL,
	OL_FACE,
	OL_MATERIAL_LIB,
	OL_MATERIAL_USE,
	OL_GROUP
};

typedef struct vert_struct {
	float x, y, z;
} vert_t;

typedef struct vtex_struct {
	float x, y;
} vtex_t;

typedef struct vnor_struct {
	float x, y, z;
} vnor_t;

typedef struct face_struct {
	int index[9];
} face_t;

typedef struct group_struct {
	char name[MODEL_GROUP_MAX_NAME];
	int face_id;
} group_t;

typedef struct model_struct {
	svec_t vert; /* vertices */
	svec_t vtex; /* texture UV coords */
	svec_t vnor; /* normals */
	svec_t face; /* faces */
	svec_t mtl;  /* materials, NOTE: referenced by `mtl_use` */
	svec_t mtl_use; /* material use marker, NOTE: references to `mtl` */
	svec_t group;  /* groups */
} model_t;

void model_init(model_t *mdl);
void model_load(const char *filename, model_t *mdl);
void model_free(model_t *mdl);
void model_vbo_free(model_t *mdl); /* keeps mtl, mtl_use, group */
void model_dump(model_t *mdl);

/* Model with deinterleaved flat arrays.
 * Useful for mesh transformations */
typedef struct model_d_struct {
	vert_t *vert;
	vtex_t *vtex;
	vnor_t *vnor;
	int elems;
} model_d_t;

void model_deinterleave(const model_t *mdl, model_d_t *d_mdl);
void model_d_free(model_d_t *d_mdl);


