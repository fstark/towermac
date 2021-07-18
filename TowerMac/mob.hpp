#ifndef MOB_INCLUDED__
#define MOB_INCLUDED__

#include "simulation.hpp"

#include "game_def.hpp" //  #### For mob_def, but we should have a POD with characteristics used by game_def, scheduler, and mob

class mob : public node<mob>, public simulated
{
	const path &path_;

	float position_ = 0;

	image image_;
	size_t hp_;
	float speed_;
	size_t damage_;

public:
	mob( simulation &simulation, const path &path, const mob_def &mob_def ) :
		simulated{simulation},
		path_{ path },
		image_{ mob_def.image_name.c_str() },
		hp_{ mob_def.hp },
		speed_{ mob_def.speed },
		damage_{ mob_def.damage }
	{
	}

	void render()
	{
		image_.render( path_.at( position_ ), path_.rotation_at( position_ ) );
	}

	void step()
	{
		auto new_position = position_ + speed_;

		if (!path_.contains(new_position))
		{
			simulation_.damage_base( damage_ );
			simulation_.destroy_mob( this );
		}
		else
			position_ = new_position;
	}

	point location() const { return path_.at( position_ ); }

	void damage( size_t damage )
	{
		if (hp_>damage)
		{
			hp_ -= damage;
			return;
		}

		simulation_.destroy_mob( this );
	}
};

#endif
