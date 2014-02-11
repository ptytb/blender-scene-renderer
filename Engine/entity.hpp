#pragma once

extern "C"
{
#include "../mylib/matrix.h"
#include "../mylib/slabvec.h"
}

typedef int entity_id_t;
typedef int entity_flags_t;

#define EF_STATIC 1
#define EF_DRAW 2
#define EF_STANDALONE 4

class entity_t
{
	public:
		svec_t modelv;
		svec_t animv;

		entity_t();
		virtual ~entity_t();

		virtual entity_id_t get_id() const = 0;
		virtual entity_flags_t get_flags() const = 0;
		virtual const char **get_model_pathv() const = 0;
		virtual const char **get_anim_pathv() const = 0;
};

