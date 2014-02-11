#pragma once

class controller_t
{
	protected:
		int model_id;
		int anim_id;
		bool anim_changed;

		void set_anim_id(int id);

	public:
		controller_t();

		int get_model_id() const;
		int get_anim_id() const;
		bool is_anim_changed();
		virtual void think() = 0;
};


