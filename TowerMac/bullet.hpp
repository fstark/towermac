#ifndef BULLET_INCLUDED__
#define BULLET_INCLUDED__

/**
 * Potential issues:
 *  Too many indirections with step modifiers (array of pointers)
 *  + Heap costs to delete bullet and non-reusability of bullets
 */

#include "core.hpp"
#include "image.hpp"
#include "simulation.hpp"
#include "mob.hpp"

#include <math.h>

inline size_t tm_random( size_t from, size_t to )
{
	return rand() / (RAND_MAX / (to-from+1) + 1) + from;
}

class step_modifier        //  #### maybe should inherit from simulated instead of fetching the simulation in apply?
{
protected:
	size_t arg0_;

public:

	step_modifier() {}
	step_modifier( const step_modifier& o ) : arg0_(o.arg0_) {}
	step_modifier( size_t arg0 ) : arg0_{arg0} {}
	
	virtual ~step_modifier(){};

	virtual step_modifier *clone() const = 0;
	virtual void apply( bullet &bullet ) {}
};

class bullet : public node<bullet>, public simulated
{
	image image_{ "assets/bullets/bullet00-0.bmp" };

	// float speed_ = 2;
	// mob &target_;
	size_t damage_ = 50;

	std::vector<step_modifier *> step_modifiers_;

public:
	vector2f position_;
	vector2f direction_;

	bullet( simulation &simulation, const point &position, const vector2f &direction ) :
		simulated{ simulation },
		position_{ position },
		direction_{ direction }
		{
		}

	~bullet()
	{
		for (auto &m:step_modifiers_)
			delete m;
	}

	virtual bullet *clone()
	{
		auto b = new bullet( simulation_, position_, direction_ );  //  #### todo: copy constructor
		for (auto m:step_modifiers_)
			b->step_modifiers_.push_back( m->clone() );
		return b;
	}

	void add_modifier( step_modifier *modifier ) { step_modifiers_.push_back( modifier ); }

	void render()
	{
		image_.render( position_ );
	}

	void step()
	{
		for (auto &m:step_modifiers_)
			m->apply( *this );

		position_ = position_ + direction_;
		if (!in_map(position_))
		{
			simulation_.destroy_bullet( this );
			return;
		}

		auto mob = simulation_.find_mob( position_, 10 );
		if (mob)
		{
			mob->damage( damage_ );
			simulation_.destroy_bullet( this );
		}
	}
};

class drunken_modifier : public step_modifier
{
public:
	drunken_modifier( const drunken_modifier &o ) : step_modifier( o ) {}
	drunken_modifier() : step_modifier( tm_random( 0, 63 ) ) {}

	virtual step_modifier *clone() const { return new drunken_modifier( *this ); }

	virtual void apply( bullet &bullet )
	{
		const size_t kDrunkLoop = 64;
		static vector2f drunk[kDrunkLoop];
		static bool inited = false;
		if (!inited)
		{
			for (int i=0;i!=kDrunkLoop;i++)
				drunk[i] = vector2f{ cos(i*kDrunkLoop/360.0 )*3, sin(i*kDrunkLoop/360.0 )*3 };
		}

		bullet.position_ = bullet.position_ + drunk[arg0_] /* *step_ */;
		arg0_++;
		if (arg0_==kDrunkLoop)
			arg0_ = 0;
	}
};

class accelerating_modifier : public step_modifier
{
public:
	accelerating_modifier( const accelerating_modifier &o ) : step_modifier( o ) {}
	virtual accelerating_modifier *clone() const { return new accelerating_modifier( *this ); }
	virtual void apply( bullet &bullet )
	{
		bullet.direction_ = bullet.direction_ * (1+1/32.0);
	}
};

inline void turn2( const vector2f &v, vector2f &left, vector2f &right )
{
	auto vx = v.x;
	auto vy = v.y;
	left = vector2f{ vx-vy/8, vy+vx/8 };
	right = vector2f{ vx+vy/8, vy-vx/8 };
}

class splitting_modifier : public step_modifier
{
public:
	splitting_modifier( const splitting_modifier &o ) : step_modifier( o ) {}
	virtual splitting_modifier *clone() const { return new splitting_modifier( *this ); }
	virtual void apply( bullet &bullet )
	{
		if (arg0_>0)
		{
			arg0_--;
			if (arg0_==0)
			{
				arg0_ = 20;

				vector2f new_speed;
				turn2( bullet.direction_, bullet.direction_, new_speed );

				auto b = bullet.clone();

				// auto b = new standard_bullet( bullet.position_+bullet.direction_, new_speed );
				// b->add_modifier( new drunken_modifier() );
				// b->add_modifier( new splitting_modifier() );

				b->direction_ = new_speed;
				b->position_ = b->position_+b->direction_;

				bullet.get_simulation().register_bullet( b );
			}
		}
	}

public:
	splitting_modifier() : step_modifier( 20 ) {}
};

#endif
