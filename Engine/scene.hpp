#pragma once

#include "entity.hpp"
#include "controller.hpp"
#include "light.hpp"

#include <map>

extern "C"
{
#include "../mylib/slabvec.h"
}

class scene_t
{
	public:
		svec_t instv;
		svec_t entityv;
		std::map<entity_id_t, entity_t *> entityh; /* Refs to entityv */
		svec_t lightv;

	public:
		scene_t();
		~scene_t();

		void add_entity(entity_t *e);
		void add_light(light_t *light);
		void load_entities();
		void spawn(entity_id_t id, vec3f_t offset, vec4f_t quat,
				controller_t *controller = NULL);
};



