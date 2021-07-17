#include "simulation.hpp"

#include "tower.hpp"
#include "mob.hpp"
#include "bullet.hpp"

simulation::~simulation()
{
	for (auto &t:towers_)
		delete t;
	while (!mobs_.is_empty())
		mobs_.begin()->remove();
	while (!bullets_.is_empty())
		bullets_.begin()->remove();
}

void simulation::step()
{
	for (auto t:towers_)
		t->step();
	for (auto m=mobs_.begin();m!=mobs_.end();m=m->next_)
		m->step();
	for (auto b=bullets_.begin();b!=bullets_.end();b=b->next_)
		b->step();

	for (auto m:dead_mobs_)
		delete m;
	dead_mobs_.clear();
	for (auto b:dead_bullets_)
		delete b;
	dead_bullets_.clear();

	timestamp_++;
}

tower *simulation::create_tower( const point &location )
{
	auto t = new basic_tower( *this, location );
	towers_.push_back( t );
	return t;
}

void simulation::register_mob( mob *m )
{
	mobs_.add( m );
}

bool simulation::has_towers()
{
	return !towers_.empty();
}

void simulation::create_bullet( const point &location, double speed )
{
	sound_manager::sm.play_foreground( snd_bullet_, 9 );

	auto b = new bullet( *this, location, normalize( (vector2f)target_-(vector2f)location )*speed );
//    b->add_modifier( new drunken_modifier() );
	b->add_modifier( new splitting_modifier() );
	register_bullet( b );
}

void simulation::create_bi_bullet( const point &location, double speed, size_t spread )
{
	auto dir = normalize( (vector2f)target_-(vector2f)location )*speed;
	vector2f dir1{ dir.x-dir.y/spread, dir.y+dir.x/spread };
	vector2f dir2{ dir.x+dir.y/spread, dir.y-dir.x/spread };

	// dir1 = dir1 * 0.992;
	// dir2 = dir2 * 0.992;

	bullets_.add( new bullet( *this, location, dir1 ) );
	bullets_.add( new bullet( *this, location, dir2 ) );
}

void simulation::create_tri_bullet( const point &location, double speed, size_t spread )
{
	auto dir = normalize( (vector2f)target_-(vector2f)location )*speed;
	vector2f dir1{ dir.x-dir.y/spread, dir.y+dir.x/spread };
	vector2f dir2{ dir.x+dir.y/spread, dir.y-dir.x/spread };

	// dir1 = dir1 * 0.992;
	// dir2 = dir2 * 0.992;

	bullets_.add( new bullet( *this, location, dir ) );
	bullets_.add( new bullet( *this, location, dir1 ) );
	bullets_.add( new bullet( *this, location, dir2 ) );
}

void simulation::damage_base( size_t damage )
{
	if (base_.damage( damage ))
	{
		std::clog << "GAME OVER\n";
		sound_manager::sm.play_foreground( snd_game_over_, 999 );
	}
	std::clog << "Remaining health :" << base_.get_hp() << " (-" << damage << ")\n";
}

void simulation::destroy_mob( mob *m )
{
	std::clog << "destroy mob " << m << std::endl;
	m->remove();
	dead_mobs_.push_back(m);
}

void simulation::destroy_bullet( bullet *b )
{
	b->remove();
	dead_bullets_.push_back(b);
}

mob *simulation::find_mob( const point &location, size_t radius )
{
	for (auto m=mobs_.begin();m!=mobs_.end();m=m->next_)
		if (distance(m->location(),location)<=radius)
			return m;
	return nullptr;
}

