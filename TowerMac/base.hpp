#ifndef BASE_INCLUDED__
#define BASE_INCLUDED__

#include "core.hpp"
#include "sprite.hpp"

class base
{
    sprite sprite_{ "assets/general/base.bmp", false };

    point location_;

    size_t hp_ = 50;

public:
    base( const point &location ) : location_{ location } {}

    void render() const
    {
        sprite_.render( location_ );
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
