#pragma once

#include "scene.hpp"

class engine_t
{
	private:
		scene_t *scene;
		
		void init_schedulers(void);
		void render(void);
		void tick(void);

	public:
		void set_scene(scene_t *s) { this->scene = s; }
		void init(int argc, char *argv[]);
		void run(void);

};

