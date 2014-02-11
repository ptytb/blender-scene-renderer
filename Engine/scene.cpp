extern "C"
{
#include "render.h"
#include "../mylib/objldr.h"
#include "../mylib/bvhldr.h"
}

#include "scene.hpp"
#include "engine.hpp"
#include "inst.hpp"

#include <cstdio>

scene_t::scene_t()
{
	svec_init(&entityv, 10, sizeof(entity_t *));
	svec_init(&instv, 10, sizeof(inst_t *));
	svec_init(&lightv, 5, sizeof(light_t *));
}

scene_t::~scene_t()
{
	svec_iter_t iter;
	entity_t **entity;
	inst_t **inst;
	light_t **light;

	svec_iter_init(&iter, &entityv);

	while ( (entity = (entity_t **) svec_iter_next(&iter)) )
	{
		delete *entity;
	}

	svec_iter_init(&iter, &instv);

	while ( (inst = (inst_t **) svec_iter_next(&iter)) )
	{
		delete *inst;
	}

	svec_iter_init(&iter, &lightv);

	while ( (light = (light_t **) svec_iter_next(&iter)) )
	{
		delete *light;
	}

	svec_free(&entityv);
	svec_free(&instv);
	svec_free(&lightv);
}

void scene_t::load_entities()
{
	svec_iter_t e_iter;
	entity_t **entity;

	svec_iter_init(&e_iter, &this->entityv);

	while ( (entity = (entity_t **) svec_iter_next(&e_iter)) )
	{	
		this->entityh[(*entity)->get_id()] = *entity;

		for (const char **model_path = (*entity)->get_model_pathv();
				*model_path; ++model_path)
		{
			model_t *model = (model_t *) svec_put(&(*entity)->modelv);
			model_init(model);
			model_load(*model_path, model);

			std::printf("entity: %d vert, %d vtex, %d vnor, %d face\n",
					model->vert.elems, model->vtex.elems,
					model->vnor.elems, model->face.elems);
			std::fflush(stdout);
		}

		for (const char **anim_path = (*entity)->get_anim_pathv();
				*anim_path; ++anim_path)
		{
			bvh_t *anim = (bvh_t *) svec_put(&(*entity)->animv);
			bvh_init(anim);
			bvh_load(*anim_path, anim);

			std::printf("anim: %d frames.\n", anim->motion.frames);
			std::fflush(stdout);
		}
	}
}

void scene_t::add_entity(entity_t *e)
{
	( * (entity_t **) svec_put(&entityv) ) = e;
}

void scene_t::add_light(light_t *light)
{
	( * (light_t **) svec_put(&lightv) ) = light;
}

void scene_t::spawn(entity_id_t id, vec3f_t offset, vec4f_t quat,
		controller_t *controller)
{
	entity_t *entity;
	inst_t *inst;
	svec_iter_t iter;
	model_t *model;

	entity = this->entityh[id];
	* (inst_t **) svec_put(&this->instv) = inst = new inst_t();

	inst->entity = entity;
	inst->offset = offset;
	inst->quat = quat;
	inst->calc_matrices();
	inst->controller = controller;

	svec_iter_init(&iter, &entity->modelv);
	while ( (model = (model_t *) svec_iter_next(&iter)) )
	{
		model_d_t *model_d;

		/* Create flat buffers for VBO or animations */
		model_d = (model_d_t *) svec_put(&inst->model_dv);
		model_deinterleave(model, model_d);

		/* Move to it's initial position and rotation */
		model_d_transform_full(model_d, inst->get_matrix());

		/* Move models to VBO */
		int chunk = vbo_model(model, model_d,
				(vbo_model_t *) svec_put(&inst->vbo_modelv),
				(entity->get_flags() & EF_STATIC)
				? GL_STATIC_DRAW
				: GL_STREAM_DRAW);
	
		if (entity->get_flags() & EF_STATIC)
		{
			model_d_free(model_d);
			svec_pop(&inst->model_dv);
		}

		std::printf("VBO chunk %d bytes.\n", chunk);
	}

	if ( ! (entity->get_flags() & EF_STATIC) )
		bvh_player_init(&inst->anim_player,
				(bvh_t *) svec_get(&inst->entity->animv,
					inst->controller->get_anim_id()));
}


