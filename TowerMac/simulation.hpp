#ifndef SIMULATION_INCLUDED__
#define SIMULATION_INCLUDED__

#include <vector>

#include "core.hpp"
#include "path.hpp"
#include "base.hpp"
#include "sound_manager.hpp"

class mob;
class bullet;
class tower;

class simulation
{
	size_t timestamp_=0;

	base base_;

	dlist<mob> mobs_;
	dlist<bullet> bullets_;
	std::vector<tower *> towers_; //{ 192, 160 }

	point target_{ 0,0 };

	std::vector<mob*> dead_mobs_;
	std::vector<bullet*> dead_bullets_;

	size_t snd_bullet_;
	size_t snd_game_over_;

public:
	simulation() :
		base_{ point{ kBaseX, kBaseY } },
	snd_bullet_{ sound_manager::sm.register_sound( "assets/bullets/bullet00.wav" ) },
	snd_game_over_{ sound_manager::sm.register_sound( "assets/general/game-over.wav" ) }
	{}
	~simulation();

	simulation( const simulation & ) = delete;

	size_t timestamp() const { return timestamp_; }

	void set_target( const point &p ) { target_ = p; }

	/// Register a new bullet
	void register_bullet( bullet *bullet ) { bullets_.add(bullet); }

	void step();

	tower *create_tower( const point &location );

	void register_mob( mob *mob );

	void create_bullet( const point &location, double speed );
	void create_bi_bullet( const point &location, double speed, size_t spread );
	void create_tri_bullet( const point &location, double speed, size_t spread );

	void damage_base( size_t damage );
	void destroy_mob( mob *m );
	void destroy_bullet( bullet *b );

	bool has_towers();

	const base &get_base() const { return base_; }
	std::vector<tower *> get_towers() const { return towers_; }
	const dlist<mob> *get_mobs() const { return &mobs_; }
	const dlist<bullet> *get_bullets() const { return &bullets_; }

	mob *find_mob( const point &location, size_t radius );
};

class simulated
{
protected:
	simulation &simulation_;
public:
	simulated( simulation &simulation ) : simulation_{ simulation } {}

	simulation &get_simulation() { return simulation_; }
};

#endif
