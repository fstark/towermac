//
//  game.hpp
//  TowerMac
//
//  Created by Frederic Stark on 18/07/2021.
//

#ifndef GAME_INCLUDED__
#define GAME_INCLUDED__

#include <memory>
#include <array>

#include "simulation.hpp"
#include "game_def.hpp"
#include "tower.hpp"
#include "bullet.hpp"
#include <iostream>

///	Anything that changes a simulation (towerplacement, powerup, etc)
class item
{
protected:
	enum class eItemClass
	{
		kTowerItem,
		kCooldownItem,
		kItemClassCount
	};
	virtual eItemClass item_class() const = 0;
	virtual void do_save( std::ostream &s ) const = 0;

private:
	size_t priority_;
	static std::array<std::string,(size_t)eItemClass::kItemClassCount> item_class_names;

public:
	item( size_t priority=0 ) : priority_{ priority } {}
	virtual ~item() {}
	
	///	Applies to a simulation to set it up
	virtual void apply( simulation &simulation ) const = 0;

	static bool compare_ptr( const item* a, const item* b) { return a->priority_<b->priority_; }
	
	static std::unique_ptr<item> load( std::istream &s );

	void save( std::ostream &s ) const { s << item_class_names[(size_t)item_class()] << " "; do_save( s ); }
};

class tower_item : public item
{
	const spot *spot_;	//	#### Maybe have a 'spotted' item superclass
	virtual eItemClass item_class() const { return eItemClass::kTowerItem; };

	virtual void do_save( std::ostream &s ) const
	{
		s << spot_->key << " ";
	}

public:
	tower_item( const spot *spot ) : spot_{spot} {}
	tower_item( std::istream &s )
	{
		std::string spot;
		s >> spot;
		spot_ = game_def::spec.spot_by_name( spot );
	}

	virtual void apply( simulation &simulation ) const
	{
		simulation.create_tower( spot_->location );
	}
};

class cooldown_item : public item
{
	virtual eItemClass item_class() const { return eItemClass::kCooldownItem; };

	virtual void do_save( std::ostream &s ) const {}
public:
	cooldown_item() : item{ 1 } {}
	cooldown_item( std::istream &s ) : item(1) {}

	virtual void apply( simulation &simulation ) const
	{
		for (auto &t:simulation.all_towers())
		{
			t->set_cooldown( t->cooldown()/2 );
		}
	}
};

/// Contains the state of the whole game (tower placements, opened spots, health, wave number, buffs, etc)
///	Fundamentlly, this is a save file
class game
{
	///	The currently opened spots for tower placement
	std::vector<const spot *> open_spots_;

	///	The items are what recreates the game state
	std::vector<std::unique_ptr<item>> items_;
public:
	static std::unique_ptr<game> load( const std::string &filename );
	void save( const std::string &filename );

	void add_spot( const spot *spot ) { open_spots_.push_back( spot ); }
	void close_spot( const spot *spot )
	{
		//	Who needs INTERCAL when you have C++ ?
		open_spots_.erase(std::remove(std::begin(open_spots_), std::end(open_spots_), spot));
	}

	void add_item( std::unique_ptr<item> item ) { items_.emplace_back( std::move( item ) ); }
	
	const std::vector<const spot *> &open_spots() const { return open_spots_; }
	const std::vector<const item *> items() const
	{
		std::vector<const item *> res;
		for (auto &i:items_)
			res.push_back( i.get() );
		std::sort( std::begin(res), std::end(res), item::compare_ptr );
		return res;
	}
	
	void apply( simulation &simulation )
	{
		auto its = items();
		for (auto &i:its)
			i->apply( simulation );
	}
};

#endif

