#pragma once

#include <GL/gl.h>

#include "slabvec.h"
#include "objldr.h"

enum vbo_part_flag {
	VPF_MATERIAL = 1,
	VPF_GROUP = 2
};

typedef struct vbo_part_struct {
	GLuint data_buffer_id;
	int offset;
	int elems;
	enum vbo_part_flag flags;
} vbo_part_t;

typedef struct vbo_model_struct {
	svec_t part;

	/* TODO: Remake links to model_t */
	svec_t mtl;
	svec_t mtl_use;
	svec_t group;
} vbo_model_t;

int vbo_model(const model_t *mdl, const model_d_t *d_mdl,
		vbo_model_t *vbo_mdl, GLenum usage);
void vbo_free(vbo_model_t *vbo_mdl);


