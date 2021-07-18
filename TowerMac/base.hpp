#ifndef BASE_INCLUDED__
#define BASE_INCLUDED__

#include "core.hpp"
#include "image.hpp"

///	The base we protect (the 68x)
class base
{
	image image_{ "assets/general/base.bmp", false };

	point location_;

	size_t hp_ = 50;
	
public:
	base( const point &location ) : location_{ location } {}

	void render() const
	{
		image_.render( location_ );
	}

	bool damage( size_t damage )
	{
		if (damage>=hp_)
		{
			hp_ = 0;
			return true;
		}

		hp_ -= damage;

		return false;
	}

	size_t get_hp() const { return hp_; }
};

#endif
