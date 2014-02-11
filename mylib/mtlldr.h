/*
 * Material loader
 * 
 * .mtl file should contain UNIX linefeeds \n
 *
 * Material name length is limited
 *
 */

#pragma once

#include "slabvec.h"

#define MTL_MAX_NAME 32

typedef struct vec3f_col_struct {
	float r, g, b;
} vec3f_col_t;

enum Mtl_Line {
	ML_UNKNOWN = 0,
	ML_COMMENT,
	ML_NEW,
	ML_AMBIENT,
	ML_DIFFUSE,
	ML_SPECULAR,
	ML_SHININESS,
	ML_EMISSION,
	ML_ALPHA,
	ML_MAP_AMBIENT,
	ML_MAP_DIFFUSE,
	ML_MAP_SPECULAR
};

typedef struct mtl_struct {
	char name[MTL_MAX_NAME];
	vec3f_col_t amb;
	vec3f_col_t dif;
	vec3f_col_t spec;
	vec3f_col_t emis;
	float shininess;
	float alpha;
	int map_amb;
	int map_dif;
	int map_spec;
} mtl_t;

/* For 'usemtl' directive handling */
typedef struct mtl_use_struct {
	mtl_t *material;
	int face_id;
} mtl_use_t;

void mtl_load(const char *model_filename, const char *filename,
		svec_t *mtlv);
void mtl_dump(svec_t *mtlv);
mtl_t *mtl_get_by_name(svec_t *mtlv, const char *name);
void mtl_glob_tex_path(const char *p);

/* Material presets */

extern const mtl_t material_white;

/* Global texture image path */

extern const char *_mtl_glob_tex_path;

