#include "scorp.hpp"
#include "engine.hpp"
#include "scene.hpp"

#if (defined WIN32) || (defined _WIN32)
extern "C"
{
#include "../mylib/win32.h"
}
#endif

#include <cmath>
#include <cstdio>

#define SCORPS 7.0f
#define RADIUS 100.0f

int main(int argc, char *argv[])
{
	engine_t engine;
	scene_t scene;

	scene.add_entity(new entity_skybox_t());
	scene.add_entity(new entity_terrain_t());
	scene.add_entity(new entity_scorp_t());

	scene.add_light(new light_t()); // TODO: Incomplete, dummy light

	engine.set_scene(&scene);
	engine.init(argc, argv);	
	scene.load_entities();

	scene.spawn(SS_SKYBOX,
			(vec3f_t) { 0.0f, 0.0f, 0.0f },
			(vec4f_t) { 0.0f, 0.0f, 0.0f, 1.0f });
	scene.spawn(SS_TERRAIN,
			(vec3f_t) { 0.0f, 0.0f, 0.0f },
			(vec4f_t) { 0.0f, 0.0f, 0.0f, 1.0f });

	for (int i = 0; i < SCORPS; ++i)
	{
		float t = 2 * M_PI * i / SCORPS;
		vec4f_t rot;

		quat_from_axang(&rot, 0.0f, 0.0f, 1.0f, -t);
		scene.spawn(SS_SCORP,
				(vec3f_t) {
					(float) sin(t) * RADIUS,
					(float) cos(t) * RADIUS,
					0.0f
				},
				rot,
				new contr_scorp_t());
	}
	
	engine.run();

	std::printf("Shutting down.\n");
	return 0;
}

void contr_scorp_t::think()
{
	++ticks;

	if (ticks == 25)
		set_anim_id(1); // Switch to"Run" animation
}



