#include "controller.hpp"

controller_t::controller_t()
{
	model_id = 0;
	anim_id = 0;
	anim_changed = false;
}

void controller_t::set_anim_id(int id)
{
	anim_id = id;
	anim_changed = true;
}

int controller_t::get_model_id() const
{
	return model_id;
}

int controller_t::get_anim_id() const
{
	return anim_id;
}

bool controller_t::is_anim_changed() 
{
	bool t = anim_changed;
	anim_changed = false;
	return t;
}


