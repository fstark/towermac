//
//  game.cpp
//  TowerMac
//
//  Created by Frederic Stark on 18/07/2021.
//

#include "game.hpp"

#include <fstream>

std::array<std::string,(size_t)item::eItemClass::kItemClassCount> item::item_class_names = { "tower", "cooldown" };

std::unique_ptr<item> item::load( std::istream &s )
{
	std::string tag;
	s >> tag;
	for (int i=0;i!=(int)eItemClass::kItemClassCount;i++)
		if (item_class_names[i]==tag)
		{
			switch ((eItemClass)i)
			{
				case eItemClass::kTowerItem:
					return std::make_unique<tower_item>( s );
				case eItemClass::kCooldownItem:
					return std::make_unique<cooldown_item>( s );
				case eItemClass::kItemClassCount:
					throw "Unknow item in savefile";
			}
		}
	throw "Unknow item in savefile";
}

std::unique_ptr<game> game::load( const std::string &filename )
{
	auto g = std::make_unique<game>();
	
	std::ifstream f( filename, std::ios::in );

	int count;
	f >> count;
	for (int i=0;i!=count;i++)
	{
		std::string key;
		f >>key;
		g->add_spot( game_def::spec.spot_by_name( key ) );
	}

	f >> count;
	for (int i=0;i!=count;i++)
	{
		g->add_item( item::load(f) );
	}

	return g;
}

void game::save( const std::string &filename )
{
	std::ofstream f( filename, std::ios::out );
	
	f << open_spots_.size() << " ";
	for (auto &s:open_spots_)
		f << s->key << " ";
	f << "\n";
	f << "\n";

	f << items_.size() << "\n";
	for (auto &i:items_)
	{
		f << "  ";
		i->save( f );
		f << "\n";
	}
	f << "\n";
}

