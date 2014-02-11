#pragma once

#include "entity.hpp"
#include "controller.hpp"

extern "C"
{
#include "../mylib/objvbo.h"
#include "../mylib/bvhldr.h"
}

class inst_t
{
	private:
		mat4f_t matrix;
		mat4f_t inv_matrix;

	public:
		entity_t *entity;
		svec_t vbo_modelv;
		svec_t model_dv;
		bvh_player_t anim_player;
		vec3f_t offset;
		vec4f_t quat;
		controller_t *controller;

		inst_t();
		~inst_t();

		void calc_matrices();
		const mat4f_t *get_matrix() const;
		const mat4f_t *get_inv_matrix() const;
};

