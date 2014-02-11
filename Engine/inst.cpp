#include "inst.hpp"

extern "C"
{
#include "../mylib/matrix.h"
}

inst_t::inst_t()
{
	svec_init(&vbo_modelv, 1, sizeof(vbo_model_t));
	svec_init(&model_dv, 3, sizeof(model_d_t));
}

inst_t::~inst_t()
{
	svec_iter_t iter;
	vbo_model_t *vbo_model;
	model_d_t *model_d;

	svec_iter_init(&iter, &vbo_modelv);

	while ( (vbo_model = (vbo_model_t *) svec_iter_next(&iter)) )
		vbo_free(vbo_model); 
	
	svec_iter_init(&iter, &model_dv);

	while ( (model_d = (model_d_t *) svec_iter_next(&iter)) )
		model_d_free(model_d);

	svec_free(&vbo_modelv);
	svec_free(&model_dv);

	delete controller;
}

void inst_t::calc_matrices()
{
	mat4f_t trans, inv_rot;

	quat_to_m4f(&quat, &matrix);

	m4f_transp(&matrix, &inv_rot);

	matrix.tx = offset.x;
	matrix.ty = offset.y;
	matrix.tz = offset.z;

	m4f_set_id(&trans);
	trans.tx = - offset.x;
	trans.ty = - offset.y;
	trans.tz = - offset.z;

	m4f_op_mul(&inv_rot, &trans, &inv_matrix);
}

const mat4f_t *inst_t::get_matrix() const
{
	return &matrix;
}

const mat4f_t *inst_t::get_inv_matrix() const
{
	return &inv_matrix;
}

