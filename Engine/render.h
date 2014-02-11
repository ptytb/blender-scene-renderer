#pragma once

#include <GL/glew.h>

#include "../mylib/mtlldr.h"
#include "../mylib/objldr.h"
#include "../mylib/objvbo.h"
#include "../mylib/matrix.h"
#include "../mylib/bvhldr.h"

enum Model_Flags {
	MF_DRAW = 1,
	MF_STATIC = 2
};

void draw_light_position(float pos[4]);
void draw_axis(void);
void use_material(const mtl_t *mtl);
void draw_model(model_t *mdl);
void model_d_transform(model_d_t *d_mdl, int begin, int end, const mat4f_t *m);
void model_d_transform_full(model_d_t *d_mdl, const mat4f_t *m);
mat4f_t *bvh_matrix_by_name(const bvh_player_t *bp, const char *name);

void vbo_draw_model(vbo_model_t *vbo_mdl, model_d_t *d_mdl);
void vbo_animate(const vbo_model_t *vbo_mdl, model_d_t *d_mdl, const bvh_player_t *bp,
		const mat4f_t *global, const mat4f_t *inv_global);

