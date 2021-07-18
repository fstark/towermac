#ifndef TOWER_INCLUDED__
#define TOWER_INCLUDED__

#include "simulation.hpp"
#include "sprite.hpp"
#include "sound_manager.hpp"

class tower : public simulated
{
	sprite sprite_{ "assets/towers/tower01.bmp" };
	size_t charge_;        //  When 0, tower is charged

	point location_;
	size_t cooldown_;      //  Number of ticks between ready to fire

protected:
	void virtual do_effect() = 0;

public:
	tower( simulation &simulation, point location, size_t cooldown ) :
		simulated{ simulation },
		location_{location},
		charge_{ cooldown },
		cooldown_{ cooldown }
		{}
	virtual ~tower(){}

	size_t cooldown() const { return cooldown_; }
	void set_cooldown( size_t cooldown ) { cooldown_ = cooldown; }

	point location() const { return location_; }

	void render()
	{
		sprite_.render( location_ );
	}

	void step()
	{
		if (charge_)
		{
			charge_--;
		}

		if (charge_==0)
		{
			charge_ = cooldown_;
			do_effect();
		}
	}
};

class basic_tower : public tower
{
	size_t bullet_speed_ = 5;   //  Speed of the buller

	virtual void do_effect()
	{
		simulation_.create_bullet( location(), bullet_speed_ );
	}

public:
	basic_tower( simulation &simulation, point location ) : tower( simulation, location, 30 ) {}
};

class bi_tower : public tower
{
	size_t bullet_speed_ = 5;   //  Speed of the buller

	virtual void do_effect()
	{
		simulation_.create_bi_bullet( location(), bullet_speed_, 8 );
	}

public:
	bi_tower( simulation &simulation, point location ) : tower( simulation, location, 15 ) {}
};

class tri_tower : public tower
{
	size_t bullet_speed_ = 5;   //  Speed of the buller

	virtual void do_effect()
	{
		simulation_.create_tri_bullet( location(), bullet_speed_, 4 );
	}

public:
	tri_tower( simulation &simulation, point location ) : tower( simulation, location, 15 ) {}
};

#endif

