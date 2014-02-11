#include <GL/glew.h>
#include <assert.h>
#include <string.h>

#include "objldr.h"
#include "objvbo.h"
#include "slabvec.h"
#include "mtlldr.h"

static GLuint vbo_create(GLenum target, int size, const void *data, GLenum usage)
{
	GLuint id = 0;
	int vbo_size;

	glGenBuffers(1, &id);
	glBindBuffer(target, id);
	glBufferData(target, size, data, usage);
	
	glGetBufferParameteriv(target, GL_BUFFER_SIZE, &vbo_size);
	if (size != vbo_size) {
		glDeleteBuffers(1, &id);
		assert(0);
	}

	return id;
}

static void vbo_push(vbo_part_t *part, int size, vert_t *vert,
		vtex_t *vtex, vnor_t *vnor)
{
	char *map_vert;

	glBindBuffer(GL_ARRAY_BUFFER, part->data_buffer_id);
	map_vert = (char *) glMapBuffer(GL_ARRAY_BUFFER,
			GL_WRITE_ONLY);
	
	memcpy(map_vert, vert, size * sizeof(vert_t));
	memcpy(map_vert + 2 * part->offset, vtex, size * sizeof(vtex_t));
	memcpy(map_vert + part->offset, vnor, size * sizeof(vnor_t));

	glUnmapBuffer(GL_ARRAY_BUFFER);
}

int vbo_model(const model_t *mdl, const model_d_t *d_mdl,
		vbo_model_t *vbo_mdl, GLenum usage)
{
	svec_iter_t mtl_use;
	svec_iter_t group;
	mtl_use_t *p_mtl_use;
	group_t *p_group;

	vbo_part_t *p;
	int part_begin, part_end, flags, flags_current,
	    chunk, chunk_sum;

	chunk_sum = 0;

	vbo_mdl->mtl = mdl->mtl;
	vbo_mdl->mtl_use = mdl->mtl_use;
	vbo_mdl->group = mdl->group;
	svec_init(&vbo_mdl->part, mdl->mtl_use.elems * mdl->group.elems + 1,
			sizeof(vbo_part_t));
	
	svec_iter_init(&mtl_use, &mdl->mtl_use);
	svec_iter_init(&group, &mdl->group);

	part_begin = 0;
	part_end = 0;
	flags = 0;
	flags_current = 0;
	p_mtl_use = svec_iter_next(&mtl_use);
	p_group = svec_iter_next(&group);

	for (; part_begin != mdl->face.elems;) {

		part_begin = part_end;

		if (p_mtl_use && p_group) {
			if (p_mtl_use->face_id < p_group->face_id) {
				part_end = p_mtl_use->face_id;
				flags |= VPF_MATERIAL;
				p_mtl_use = svec_iter_next(&mtl_use);
			} else if (p_group->face_id < p_mtl_use->face_id) {
				part_end = p_group->face_id;
				flags |= VPF_GROUP;
				p_group = svec_iter_next(&group);
			} else {
				part_end = p_mtl_use->face_id;
				flags = VPF_MATERIAL | VPF_GROUP;
				p_mtl_use = svec_iter_next(&mtl_use);
				p_group = svec_iter_next(&group);
			}
		} else if (p_mtl_use) {
			part_end = p_mtl_use->face_id;
			flags |= VPF_MATERIAL;
			p_mtl_use = svec_iter_next(&mtl_use);
		} else if (p_group) {
			part_end = p_group->face_id;
			flags |= VPF_GROUP;
			p_group = svec_iter_next(&group);
		} else {
			part_end = mdl->face.elems;
		}
		
		if (part_end - part_begin > 0) {
			p = svec_put(&vbo_mdl->part);
			p->flags = flags_current;

			p->elems = 3 * (part_end - part_begin);
			p->offset = p->elems * sizeof(vert_t);
			chunk = 2 * p->offset + p->elems * sizeof(vtex_t);

			p->data_buffer_id = vbo_create(GL_ARRAY_BUFFER,
					chunk, NULL, usage);
			vbo_push(p, p->elems,
					d_mdl->vert + part_begin * 3,
					d_mdl->vtex + part_begin * 3,
					d_mdl->vnor + part_begin * 3);
			chunk_sum += chunk;
		}

		flags_current = flags;
		flags = 0;
	}

	svec_shrink(&vbo_mdl->part);
	return chunk_sum;
}

void vbo_free(vbo_model_t *vbo_mdl)
{
	svec_iter_t i;
	vbo_part_t *p;

	svec_iter_init(&i, &vbo_mdl->part);

	while ( (p = svec_iter_next(&i)) ) {
		glDeleteBuffers(1, &p->data_buffer_id);
	}

	svec_free(&vbo_mdl->part);
}

