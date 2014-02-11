#pragma once

#include "entity.hpp"
#include "controller.hpp"

#include <cstdlib>

enum Scorp_Scene
{
	SS_SKYBOX = 1,
	SS_TERRAIN,
	SS_SCORP
};

class entity_scorp_t : public entity_t
{
	public:
		virtual entity_id_t get_id() const
		{
			return SS_SCORP;
		}

		virtual entity_flags_t get_flags() const
		{
			return EF_DRAW;
		}

		virtual const char **get_model_pathv() const
		{
			static const char *model_pathv[] =
			{
				"obj/Scorp.obj",
				NULL
			};

			return model_pathv;
		}

		virtual const char **get_anim_pathv() const 
		{
			static const char *anim_pathv[] =
			{
				"bvh/Scorp_awake.bvh",
				"bvh/Scorp_walk.bvh",
				NULL
			};

			return anim_pathv;
		}

};

class entity_skybox_t : public entity_t
{
	public:
		virtual entity_id_t get_id() const
		{
			return SS_SKYBOX;
		}

		virtual entity_flags_t get_flags() const
		{
			return EF_DRAW | EF_STATIC;
		}

		virtual const char **get_model_pathv() const
		{
			static const char *model_pathv[] =
			{
				"obj/skybox.obj",
				NULL
			};

			return model_pathv;
		}

		virtual const char **get_anim_pathv() const 
		{
			static const char *anim_pathv[] =
			{
				NULL
			};

			return anim_pathv;
		}

};

class entity_terrain_t : public entity_t
{
	public:
		virtual entity_id_t get_id() const
		{
			return SS_TERRAIN;
		}

		virtual entity_flags_t get_flags() const
		{
			return EF_DRAW | EF_STATIC;
		}

		virtual const char **get_model_pathv() const
		{
			static const char *model_pathv[] =
			{
				"obj/terrain.obj",
				NULL
			};

			return model_pathv;
		}

		virtual const char **get_anim_pathv() const 
		{
			static const char *anim_pathv[] =
			{
				NULL
			};

			return anim_pathv;
		}

};

class contr_scorp_t : public controller_t
{
	private:
		unsigned long ticks;

	public:
		contr_scorp_t() { ticks = 0; }

		void think();
};

