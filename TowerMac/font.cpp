//
//  font.cpp
//  TowerMac
//
//  Created by Frederic Stark on 18/07/2021.
//

#include "font.hpp"

#include <iostream>

#include <SDL2/SDL.h>

#ifdef __linux__
#include <SDL2/SDL_image.h>
#else // macosx
#include <SDL2_image/SDL_image.h>
#endif

std::unique_ptr<font> font::normal;
std::unique_ptr<font> font::bold;

void font::init()
{
	normal = std::unique_ptr<font>( new font{ "assets/general/font-normal.png" } );
	bold = std::unique_ptr<font>( new font{ "assets/general/font-bold.png" } );

//	normal = std::make_unique<font>( "assets/general/font-normal.png" );
//	bold = std::make_unique<font>( "assets/general/font-bold.png" );
}

static uint32_t get_pixel( SDL_Surface *s, size_t x, size_t y )
{
	assert( s->format->BitsPerPixel==32 );
	const uint32_t *p = (const uint32_t *)((const char *)s->pixels + y * s->pitch);
	auto v = p[ x ];
//	std::clog << "[" << (v & (~s->format->Amask)) << "]";
	return v & (~s->format->Amask);
}

static void set_pixel( SDL_Surface *s, size_t x, size_t y, uint32_t color )
{
	assert( s->format->BitsPerPixel==32 );
	uint32_t *p = (uint32_t *)((char *)s->pixels + y * s->pitch);
	p[x] = color | s->format->Amask;
}
font::font( const std::string &filename )
{
	for (int i=0;i!=256;i++)
		sprites_[i] = nullptr;
	
	SDL_Surface *s = IMG_Load( filename.c_str() );
	if (!s)
	{
		std::cerr << "Cannot load font [" << filename << "]\n";
		throw "Cannot load font";
	}
	SDL_LockSurface( s );

	const std::string letters = " !\"#$%&'()*+,-./0123456789:;<=>?@abcdefghijklmnopqrstuvwxyz{|}~";
	size_t x = 0;
	const char *p = (const char *)s->pixels+s->pitch;	//	Point to first line
	for (auto c:letters)
	{
		//	Look for letter end
		auto bx = x;
		while (get_pixel(s,x++,0)!=0)
			;

		std::clog << c << ":" << x-bx-1 << " ";

		SDL_Surface *sub_surf = SDL_CreateRGBSurface(
														0,
														(int)(x-bx-1),
														s->h-1,
														s->format->BitsPerPixel,
														s->format->Rmask,
														s->format->Gmask,
														s->format->Bmask,
														s->format->Amask
														);
		for (int y0=0;y0!=sub_surf->h;y0++)
			for (int x0=0;x0!=sub_surf->w;x0++)
				set_pixel( sub_surf, x0, y0, get_pixel( s, bx+x0, 1+y0 ) );

		sprites_[c] = std::make_shared<sprite>( sub_surf, false );
		SDL_FreeSurface( sub_surf );
		
		p += x-bx;
	}
	assert ( x==s->w );
	std::clog << "\n";

		//	Uppercase letters
	for (char c='a';c<='z';c++)
		sprites_[c-'a'+'A'] = sprites_[c];

	SDL_UnlockSurface( s );

	SDL_FreeSurface( s );
}

void font::render_text( const point &origin, const char *string )
{
	point p{ origin };
	int c;
	while((c=*string++))
	{
		if (sprites_[c])
		{
			sprites_[c]->render( p );
			p.x += sprites_[c]->width()+1;
		}
	}
}
