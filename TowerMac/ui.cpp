//
//  ui.cpp
//  TowerMac
//
//  Created by Frederic Stark on 18/07/2021.
//

#include "ui.hpp"

static void set_color( graphics::color c )
{
	if (c==graphics::kWhite)
		SDL_SetRenderDrawColor( gRenderer, 255, 255, 255, 255 );
	else
		SDL_SetRenderDrawColor( gRenderer, 0, 0, 0, 255 );
}

void graphics::frame_rect( const rect & r )
{
	set_color( state_.stroke );

	SDL_Rect rect;
	rect.x = (int)r.o.x+state_.origin.x;
	rect.y = (int)r.o.y+state_.origin.y;
	rect.w = (int)r.s.w+1;
	rect.h = (int)r.s.h+1;
	SDL_RenderDrawRect( gRenderer, &rect );
}

void graphics::fill_rect( const rect & r )
{
	set_color( state_.fill );

	SDL_Rect rect;
	rect.x = (int)r.o.x+state_.origin.x;
	rect.y = (int)r.o.y+state_.origin.y;
	rect.w = (int)r.s.w;
	rect.h = (int)r.s.h;
	SDL_RenderFillRect( gRenderer, &rect );
}

void graphics::move_to( point p )
{
	state_.location = state_.origin+p;
}

void graphics::draw_text( const line &line )
{
    for(auto &word : line)
        for(auto &letter : word.letters)
        {
            state_.font->draw_letter(state_.location, letter.c);
            state_.location.x+=letter.spacing;
        }
        
}

void window::draw()
{
	set_color( graphics::kWhite );
	SDL_RenderClear( gRenderer );

	root_.draw( graphics_ );

	SDL_RenderPresent( gRenderer );
}
