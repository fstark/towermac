//
//  main.cpp
//  TowerMac
//
//  Created by Frederic Stark on 23/06/2021.
//

#include <iostream>
#include <cassert>
#include <vector>
#include <algorithm>

#include <SDL2/SDL.h>

SDL_Renderer *gRenderer = nullptr;

#include "simulation.hpp"
#include "tower.hpp"
#include "mob.hpp"
#include "bullet.hpp"
#include "game_def.hpp"
#include "sound_manager.hpp"
#include "game.hpp"
#include "font.hpp"

class mob_scheduler
{
	/// An mob creation at a certain timestamp on a certain lane
	class spawn_event
	{
		protected:
		size_t timestamp_;
		mob *mob_;
	public:
		spawn_event( size_t timestamp, mob *mob ) :
			timestamp_{ timestamp },
			mob_{ mob }
		{}
		bool operator<( const spawn_event &o ) const { return timestamp_<o.timestamp_; }
		bool operator==( const spawn_event &o ) const { return timestamp_==o.timestamp_; }
		friend mob_scheduler;
	};

	std::vector<spawn_event> events_; //  ordered
	std::vector<spawn_event>::const_iterator current_;


public:
	void add_event( size_t ts, mob *mob )
	{
		events_.emplace_back( ts, mob );
	}

	void prepare()
	{
		std::sort(std::begin(events_),std::end(events_));
		current_ = std::begin(events_);
	}

	bool empty() const { return current_==std::end(events_); }
	
	bool step( simulation &simulation )
	{
		if (empty())
			return false;
		auto ts = simulation.timestamp();
		while (ts==current_->timestamp_)
		{
			simulation.register_mob( current_->mob_ );
			current_++;
		}
		return true;
	}
};

mob_scheduler schedule_wave( const wave_def &wave, simulation &simulation )
{
	mob_scheduler sched;
	size_t ts;

	for (auto &wl:wave.wavelets)
	{
		ts = 0;
		for (auto &mg:wl.mob_groups)
		{
			ts += mg.spawn_delay;
			for (int i=0;i!=mg.count;i++)
			{
				ts += mg.spawn_rate;
				mob *m = new mob( simulation, *wl.path_, *mg.mob_def_ );
				sched.add_event( ts, m );
			}
			ts -= mg.spawn_rate;
		}
	}

	sched.prepare();

	return sched;
}

SDL_Window* window_ = NULL;

class game_loop
{
	point target_{ 128, 128 };	///	Current mouse target

	image map_background_{ "assets/general/map.bmp", false };
	image map_background_gray_{ "assets/general/map-gray.bmp", false };
	
	enum eGameState
	{
		kTowerPlacement = 0,
		kGameRunning,
		kGameStep,
		kGamePaused,
		kGameExiting
	};

	image empty_towers_[4] = { "assets/towers/empty0.bmp", "assets/towers/empty1.bmp", "assets/towers/empty2.bmp", "assets/towers/empty3.bmp" };

	eGameState state_ = kTowerPlacement;

	size_t ticks_ = 0;

	std::unique_ptr<game> game_;
	std::unique_ptr<simulation> simulation_;

	mob_scheduler scheduler_;

	void do_user_input()
	{
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
			{
				state_ = kGameExiting;
				std::clog << "SDL_QUIT\n";
			}
			else if (e.key.keysym.sym==SDLK_ESCAPE || e.key.keysym.sym==SDLK_LCTRL)
			{
				state_ = kGameExiting;
				std::clog << "ESC/CTRL\n";
			}

			switch (state_)
			{
				case kGamePaused:
					if (e.type == SDL_KEYDOWN && e.key.keysym.sym==SDLK_SPACE)
						state_ = kGameRunning;
					else if (e.type == SDL_KEYDOWN && e.key.keysym.sym==SDLK_RIGHT)
						state_ = kGameStep;
					break;
				case kTowerPlacement:
					if (e.type == SDL_MOUSEBUTTONDOWN)
					{
						point p{ e.button.x/ZoomFactor, e.button.y/ZoomFactor };
						bool found = false;
						for (auto s:game_->open_spots())
							if (s->contains(p))
							{
								game_->add_item( std::make_unique<tower_item>( s ) );
								game_->close_spot( s );
								found = true;
								break;
							}
						if (!found)
							break;

						simulation_ = std::make_unique<simulation>();

						game_->apply( *simulation_ );

						scheduler_ = schedule_wave( game_def::spec.get_wave(0), *simulation_ );
						state_ = kGameRunning;
					}
					break;
				case kGameRunning:
					if (e.type == SDL_KEYDOWN && e.key.keysym.sym==SDLK_SPACE)
						state_ = kGamePaused;
					if (e.type == SDL_MOUSEMOTION)
					{
						target_ = point{ e.button.x/ZoomFactor, e.button.y/ZoomFactor };
					}
					break;
				default:
					break;
			}

		}
	}

	void do_physics()
	{
		if (state_==kGameRunning || state_==kGameStep)
		{
			simulation_->set_target( target_ );
			scheduler_.step( *simulation_ );
			simulation_->step();
		}

		if (state_==kGameStep)
			state_ = kGamePaused;
	}

	void draw_spot( const spot &s )
	{
		// SDL_Rect r;
		// r.x = s.location.x-8+kMapX;
		// r.y = s.location.y-8+kMapY;
		// r.w = 16;
		// r.h = 16;
		// SDL_SetRenderDrawColor( gRenderer, 0, 255, 0, 255 );
		// SDL_RenderDrawRect( gRenderer, &r );

		empty_towers_[(ticks_/2)%4].render( { s.location.x+kMapX, s.location.y+kMapY } );
	}

	void draw_path( const path &path )
	{
		SDL_SetRenderDrawColor( gRenderer, 255, 0, 0, 255 );
		for (auto &p:path.get_points())
			SDL_RenderDrawPoint( gRenderer, (int)p.x, (int)p.y );
	}

	void do_render()
	{
//        SDL_FillRect(gScreen, NULL, SDL_MapRGB(gScreen->format, 0xFF, 0xFF, 0xFF));
		SDL_SetRenderDrawColor( gRenderer, 255, 255, 255, 255 );
		SDL_RenderClear( gRenderer );

		map_background_gray_.render( point{ kMapX, kMapY } );
//		font::normal->render_text( { 20, 20 } , "Hello, World" );
//		font::normal->render_text( { 20, 20 } , "!\"#$%&'()*+,-./0123456789:;<=>?@abcdefghijklmnopqrstuvwxyz{|}~" );
//		font::normal->render_text( { 1, 1 } , "   Hello, World: we can now write stuff on screen, including numb3r5!" );
		font::bold->render_text( { 1, 1 } , "   And we even support BOLD!" );

		if (state_==kTowerPlacement)
		{
			for (auto s:game_->open_spots())
				draw_spot( *s );
		
			for (auto p:game_def::spec.get_lanes(0))
				draw_path( *p );
		}

		if (simulation_)
		{
			simulation_->get_base().render();

			for (auto t:simulation_->get_towers())       //  #### not simulation, game
				t->render();

			if (state_==kGameRunning || state_==kGamePaused || state_==kGameStep)
			{
				for (auto m=simulation_->get_mobs()->begin();m!=simulation_->get_mobs()->end();m=m->next_)
					m->render();

				for (auto b=simulation_->get_bullets()->begin();b!=simulation_->get_bullets()->end();b=b->next_)
					b->render();
			}
		}

//        SDL_UpdateWindowSurface(window);

		SDL_RenderPresent( gRenderer );

		ticks_++;
	}

public:
	game_loop()
	{
		game_ = std::make_unique<game>();
		for (auto &s:game_def::spec.spot_defs())
			game_->add_spot( s );
		game_->add_item( std::make_unique<cooldown_item>() );
		game_->save( "/tmp/1.tm" );
	}

	~game_loop()
	{
	}

	bool step()
	{
		uint32_t start_time = SDL_GetTicks();
  
		do_user_input();
		do_physics();
		do_render();

		uint32_t duration = SDL_GetTicks()-start_time;

		if (duration<33)
		{
			SDL_Delay( 33-duration );
		}
		else
		{
			std::clog << "#### Unsustainable framerate " << duration << "ms >= 33ms\n";
			// std::clog << "Bullet count = " << simulation_.get_bullets().size() << "\n";
			// std::clog << "Tower count = " << simulation_.get_towers().size() << "\n";
			// std::clog << "Mob count = " << simulation_.get_mobs().size() << "\n";
		}

		if (simulation_)
		{
			if (simulation_->game_over())
			{
				simulation_ = nullptr;
				state_ = kTowerPlacement;
				game_ = game::load( "/tmp/1.tm" );
			}
			else
			{
				if (scheduler_.empty() && !simulation_->has_mobs())
				{
					simulation_ = nullptr;
					state_ = kTowerPlacement;
					game_->save( "/tmp/1.tm" );
				}
			}
		}
		return state_==kGameExiting;
	}
};

#include "game_def.hpp"

int main(int argc, char* args[])
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		fprintf(stderr, "could not initialize sdl2: %s\n", SDL_GetError());
		throw "SDL_Init failed";
	}
	window_ = SDL_CreateWindow(
				"Tower Mac Dev",
				SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
				SCREEN_WIDTH*ZoomFactor, SCREEN_HEIGHT*ZoomFactor,
				SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI
				);
	if (window_ == NULL)
	{
		fprintf(stderr, "could not create window: %s\n", SDL_GetError());
		throw "SDL_CreateWindow failed";
	}
	
	int ww = 0;
	int wh = 0;
	SDL_GetWindowSize(window_, &ww, &wh);
	int dw = 0;
	int dh = 0;
	SDL_GL_GetDrawableSize(window_, &dw, &dh);
	int retina_factor = ww == dw && wh == dh ? 1 : 2; // we need to double the renderer scale on retina displays, and this is sadly the simplest way to test for them.
	
	gRenderer =  SDL_CreateRenderer( window_, -1, SDL_RENDERER_ACCELERATED);
	SDL_RenderSetScale( gRenderer, ZoomFactor * retina_factor, ZoomFactor * retina_factor);
	font::init();

	game_def::spec.wave_defs();
	
	game_loop gl;
	
	auto snd = sound_manager::sm.register_sound( "assets/general/sample.wav" );
	sound_manager::sm.play_background( snd );

	try
	{
		while (!gl.step())
			;
	}
	catch(const char *e)
	{
		std::cerr << e << '\n';
	}
	
	SDL_DestroyWindow( window_ );
	SDL_Quit();

	return 0;
}

