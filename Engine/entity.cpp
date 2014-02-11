#include "entity.hpp"

extern "C"
{
#include "../mylib/objldr.h"
#include "../mylib/bvhldr.h"
}

entity_t::entity_t()
{
	svec_init(&modelv, 10, sizeof(model_t));
	svec_init(&animv, 10, sizeof(bvh_t));
}

entity_t::~entity_t()
{
	svec_iter_t iter;
	model_t *model;
	bvh_t *anim;

	svec_iter_init(&iter, &modelv);

	while ( (model = (model_t *) svec_iter_next(&iter)) )
		model_free(model);

	svec_iter_init(&iter, &animv);

	while ( (anim = (bvh_t *) svec_iter_next(&iter)) )
		bvh_free(anim);

	svec_free(&modelv);
	svec_free(&animv);
}

