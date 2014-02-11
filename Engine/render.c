#include "render.h"

#include <stdlib.h>

void draw_light_position(float pos[4])
{
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	/* Позиция света */
	glPointSize(10.0f);
	glBegin(GL_POINTS);
	glColor3f(1.0f, 1.0f, 0.0f);
	glVertex3fv(pos);
	glEnd();

	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
}

void draw_axis(void)
{
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	glLineWidth(2.0f);
	/*OX RED*/
	glBegin(GL_LINES);
	glColor3f( 1.0f,0.0f,0.0f);
	glVertex3f(-30.0f,0.0f,0.0f);
	glVertex3f(30.0f,0.0f,0.0f);
	glEnd();

	/* OY GREEN*/
	glBegin(GL_LINES);
	glColor3f( 0.0f,1.0f,0.0f);
	glVertex3f(0.0f,-30.0f,0.0f);
	glVertex3f(0.0f,30.0f,0.0f);
	glEnd();

	/* OZ BLUE*/
	glBegin(GL_LINES);
	glColor3f(0.0f,0.0f,1.0f);
	glVertex3f(0.0f,0.0f,-30.0f);
	glVertex3f(0.0f,0.0f,30.0f);
	glEnd();

	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
}

/*
 * Models
 */

void use_material(const mtl_t *mtl)
{
	float v[4];

	v[3] = mtl->alpha;

	v[0] = mtl->amb.r;
	v[1] = mtl->amb.g;
	v[2] = mtl->amb.b;
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, v);

	v[0] = mtl->dif.r;
	v[1] = mtl->dif.g;
	v[2] = mtl->dif.b;
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, v);

	v[0] = mtl->spec.r;
	v[1] = mtl->spec.g;
	v[2] = mtl->spec.b;
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, v);

	v[0] = mtl->emis.r;
	v[1] = mtl->emis.g;
	v[2] = mtl->emis.b;
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, v);

	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mtl->shininess);

	if (mtl->map_dif >= 0) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, mtl->map_dif);
	} else {
		glDisable(GL_TEXTURE_2D);
	}
}

/* Draw with triangles */
void draw_model(model_t *mdl)
{
	int face_i, mtl_use_at; //, group_at;
	svec_iter_t face_iter;
	svec_iter_t mtl_use_iter;
//	svec_iter_t group_iter;
	face_t *f;
	mtl_use_t *m;

	svec_iter_init(&face_iter, &mdl->face);
	svec_iter_init(&mtl_use_iter, &mdl->mtl_use);
//	svec_iter_init(&group_iter, &mdl->group);

	if ( (m = svec_iter_next(&mtl_use_iter)) ) {
		mtl_use_at = m->face_id;
	} else {
		mtl_use_at = -1;
	}

	use_material(&material_white);

	for (face_i = 0; ( f = svec_iter_next(&face_iter) ); ++face_i) {

		/* Use materials */
		if (face_i == mtl_use_at) {
			use_material(m->material);
			if ( (m = svec_iter_next(&mtl_use_iter)) )
				mtl_use_at = m->face_id;
		}

		/* Draw triangle */
		glBegin(GL_TRIANGLES);
		if (f->index[2])
			glNormal3fv(svec_get(&mdl->vnor, f->index[2] - 1));
		if (f->index[1])
			glTexCoord2fv(svec_get(&mdl->vtex, f->index[1] - 1));
		glVertex3fv(svec_get(&mdl->vert, f->index[0] - 1));

		if (f->index[5])
			glNormal3fv(svec_get(&mdl->vnor, f->index[5] - 1));
		if (f->index[4])
			glTexCoord2fv(svec_get(&mdl->vtex, f->index[4] - 1));
		glVertex3fv(svec_get(&mdl->vert, f->index[3] - 1));

		if (f->index[8])
			glNormal3fv(svec_get(&mdl->vnor, f->index[8] - 1));
		if (f->index[7])
			glTexCoord2fv(svec_get(&mdl->vtex, f->index[7] - 1));
		glVertex3fv(svec_get(&mdl->vert, f->index[6] - 1));
		glEnd();
	}
}

void vbo_draw_model(vbo_model_t *vbo_mdl, model_d_t *d_mdl)
{
	svec_iter_t part;
	svec_iter_t mtl_use;
	vbo_part_t *p;
	int offset;

	svec_iter_init(&part, &vbo_mdl->part);
	svec_iter_init(&mtl_use, &vbo_mdl->mtl_use);

	offset = 0;

	use_material(&material_white);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	while ( (p = svec_iter_next(&part) ) )
	{
		if (p->flags & VPF_MATERIAL) {
			use_material( ( (mtl_use_t *) svec_iter_next(&mtl_use) )
					->material );
		}
		
		glBindBuffer(GL_ARRAY_BUFFER, p->data_buffer_id);

		if ( d_mdl ) {
			glBufferSubData(GL_ARRAY_BUFFER, 0,
					p->elems * sizeof(vert_t),
					d_mdl->vert + offset);

			glBufferSubData(GL_ARRAY_BUFFER, p->offset,
					p->elems * sizeof(vnor_t),
					d_mdl->vnor + offset);
			offset += p->elems;
		}

		glVertexPointer(3, GL_FLOAT, 0, (GLvoid const *) 0);
		glNormalPointer(GL_FLOAT, 0, (GLvoid const *) p->offset);
		glTexCoordPointer(2, GL_FLOAT, 0, (GLvoid const *) (2 * p->offset) );

		glDrawArrays(GL_TRIANGLES, 0, p->elems);
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void model_d_transform(model_d_t *d_mdl, int begin, int end,
		const mat4f_t *m)
{
	vert_t *vert, *vend;
	vnor_t *vnor;
	vec4f_t v, n, V, N;

	vert = d_mdl->vert + begin;
	vend = d_mdl->vert + end;
	vnor = d_mdl->vnor + begin;

	while (vert < vend)
	{
		v.x = vert->x;
		v.y = vert->y;
		v.z = vert->z;
		v.w = 1.0f;

		n.x = vnor->x;
		n.y = vnor->y;
		n.z = vnor->z;
		n.w = 1.0f;

		v4f_addv(&v, &n, &N);
		m4f_op_mulv(m, &N, &n);

		m4f_op_mulv(m, &v, &V);
		v4f_subv(&n, &V, &N);

		v = V;
		n = N;

		vert->x = v.x;
		vert->y = v.y;
		vert->z = v.z;

		vnor->x = n.x;
		vnor->y = n.y;
		vnor->z = n.z;

		++vert;
		++vnor;
	}
}

void model_d_transform_full(model_d_t *d_mdl, const mat4f_t *m)
{
	model_d_transform(d_mdl, 0, d_mdl->elems, m);
}

mat4f_t *bvh_matrix_by_name(const bvh_player_t *bp, const char *name)
{
	svec_iter_t joint_iter;
	bvh_joint_t *joint;
	int matrix_offset;

	svec_iter_init(&joint_iter, &bp->bvh->jointv);
	matrix_offset = 0;

	while ( (joint = svec_iter_next(&joint_iter)) ) {
		if (strcmp(joint->name, name) == 0) {
			return bp->matrixv + matrix_offset;
		}
		++matrix_offset;
	}

	return NULL;
}

void vbo_animate(const vbo_model_t *vbo_mdl, model_d_t *d_mdl, const bvh_player_t *bp,
		const mat4f_t *global, const mat4f_t *inv_global)
{
	svec_iter_t i;
	group_t *g;
	int group_begin;
	int group_end;
	int present;
	mat4f_t *matrix;
	mat4f_t final_matrix, tmp_matrix;

	svec_iter_init(&i, &vbo_mdl->group);
	group_begin = 0;
	group_end = 0;
	present = 0;
	matrix = NULL;

	while ( (g = svec_iter_next(&i)) ) {
		group_end = 3 * g->face_id;

		if (group_end - group_begin > 0) {
			if (matrix) {
				model_d_transform(d_mdl,
						group_begin,
						group_end, 
						&final_matrix);
			}
		}

		matrix = bvh_matrix_by_name(bp, g->name);

		m4f_op_mul(global, matrix, &tmp_matrix);
		m4f_op_mul(&tmp_matrix, inv_global, &final_matrix);

		group_begin = group_end;
		present = 1;
	}

	if (present && matrix) {
		group_end = d_mdl->elems;
		model_d_transform(d_mdl,
				group_begin,
				group_end,
				&final_matrix);
	}
}

