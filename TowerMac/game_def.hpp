#ifndef GAME_DEF_INCLUDED__
#define GAME_DEF_INCLUDED__

#include <map>
#include <string>
#include <vector>

#include "core.hpp"

#include "path.hpp"

/// A mob
struct mob_def
{
	std::string sprite_name;
	size_t hp;
	float speed;
	size_t damage;
};

/// A group of similar mob, with their spawn time and rate
struct mob_group_def
{
	size_t count;
	std::string mob_key;
	size_t spawn_delay;
	size_t spawn_rate;
	mob_def *mob_def_;
};

/// A wave on a specific lane
struct wavelet_def
{
	std::string lane_key;
	std::vector<mob_group_def> mob_groups;
	path *path_;
};

/// A wave with the several lanes
struct wave_def
{
	std::vector<wavelet_def> wavelets;
};

/// A place where one can put a tower
struct spot
{
	std::string key;
	point location;
	std::string description;
	
	bool contains( const point &p ) const
	{
		return distance( p, location )< 16;
	}
};

struct spot_group
{
	std::string description;
	std::vector<const spot*> spots;
};

class game_def
{
	std::map<std::string,path> lane_defs_;              //  All known lanes
	std::map<std::string,spot*> spot_defs_;          //  Tower spots
	std::map<const std::string, mob_def> mob_defs_;     //  All the mobs
	std::vector<wave_def> wave_defs_;

	std::map<std::string,spot_group> groups_;
	
	game_def();

public:
	static const game_def spec;

	const std::vector<wave_def> &wave_defs() const { return wave_defs_; };

	const std::vector<const spot *> spot_defs() const
	{
		std::vector<const spot*> res;
		for (auto [k,v]:spot_defs_)
			res.push_back( v );

		return res;
	};

	const spot *spot_by_name( const std::string &key ) const
	{
		return spot_defs_.at( key );
	}
	
	const wave_def &get_wave( int wave ) const
	{
		assert( wave>=0 && wave<wave_defs_.size() );
		return wave_defs_[wave];
	}

	const std::vector<path*> get_lanes( int wave ) const
	{
		std::vector<path*> res;
		for (auto &wl:get_wave(wave).wavelets)
			res.push_back( wl.path_ );
		return res;
	}
};

#endif
